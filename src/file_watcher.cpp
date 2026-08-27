#include "file_watcher.hpp"

#include <glib-unix.h>
#include <glib.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

constexpr std::uint32_t watched_events =
    IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE | IN_ATTRIB | IN_Q_OVERFLOW;
constexpr guint reload_delay_ms = 60;

}  // namespace

FileWatcher::FileWatcher(std::filesystem::path file_path, std::function<void()> on_change)
    : filename_(file_path.filename()), on_change_(std::move(on_change)) {
    inotify_fd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (inotify_fd_ < 0) {
        throw std::runtime_error("failed to initialize file watcher: " +
                                 std::string(std::strerror(errno)));
    }

    const std::string directory = file_path.parent_path().string();
    watch_descriptor_ = inotify_add_watch(inotify_fd_, directory.c_str(), watched_events);
    if (watch_descriptor_ < 0) {
        const std::string message = std::strerror(errno);
        close(inotify_fd_);
        inotify_fd_ = -1;
        throw std::runtime_error("failed to watch file: " + message);
    }

    source_id_ = g_unix_fd_add(inotify_fd_, static_cast<GIOCondition>(G_IO_IN | G_IO_ERR),
                               onReadable, this);
}

FileWatcher::~FileWatcher() {
    if (reload_timeout_id_ != 0) {
        g_source_remove(reload_timeout_id_);
    }
    if (source_id_ != 0) {
        g_source_remove(source_id_);
    }
    if (watch_descriptor_ >= 0) {
        inotify_rm_watch(inotify_fd_, watch_descriptor_);
    }
    if (inotify_fd_ >= 0) {
        close(inotify_fd_);
    }
}

gboolean FileWatcher::onReadable(gint, GIOCondition condition, gpointer user_data) {
    auto* watcher = static_cast<FileWatcher*>(user_data);
    if ((condition & G_IO_IN) != 0) {
        watcher->processEvents();
    }
    if ((condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) != 0) {
        watcher->source_id_ = 0;
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

gboolean FileWatcher::onReloadTimeout(gpointer user_data) {
    auto* watcher = static_cast<FileWatcher*>(user_data);
    watcher->reload_timeout_id_ = 0;
    watcher->on_change_();
    return G_SOURCE_REMOVE;
}

void FileWatcher::processEvents() {
    alignas(inotify_event) std::array<char, 16 * 1024> buffer{};

    while (true) {
        const ssize_t length = read(inotify_fd_, buffer.data(), buffer.size());
        if (length < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }
            return;
        }
        if (length == 0) {
            return;
        }

        std::size_t offset = 0;
        while (offset < static_cast<std::size_t>(length)) {
            const auto* event = reinterpret_cast<const inotify_event*>(buffer.data() + offset);
            const bool queue_overflow = (event->mask & IN_Q_OVERFLOW) != 0;
            const bool matching_file =
                event->len > 0 && filename_ == std::filesystem::path(event->name);
            if (queue_overflow || matching_file) {
                scheduleReload();
            }
            offset += sizeof(inotify_event) + event->len;
        }
    }
}

void FileWatcher::scheduleReload() {
    if (reload_timeout_id_ != 0) {
        g_source_remove(reload_timeout_id_);
    }
    reload_timeout_id_ = g_timeout_add(reload_delay_ms, onReloadTimeout, this);
}
