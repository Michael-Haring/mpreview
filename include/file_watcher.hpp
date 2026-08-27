#pragma once

#include <glib.h>

#include <filesystem>
#include <functional>

class FileWatcher {
public:
    FileWatcher(std::filesystem::path file_path, std::function<void()> on_change);
    ~FileWatcher();

    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;

private:
    static gboolean onReadable(gint fd, GIOCondition condition, gpointer user_data);
    static gboolean onReloadTimeout(gpointer user_data);
    void processEvents();
    void scheduleReload();

    std::filesystem::path filename_;
    std::function<void()> on_change_;
    int inotify_fd_ = -1;
    int watch_descriptor_ = -1;
    guint source_id_ = 0;
    guint reload_timeout_id_ = 0;
};
