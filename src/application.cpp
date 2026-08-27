#include "application.hpp"

#include <gtk/gtk.h>
#include <gdk/x11/gdkx.h>
#include <webkit/webkit.h>
#include <X11/Xlib.h>

#include <iostream>
#include <stdexcept>
#include <utility>

Application::Application(std::filesystem::path markdown_path, WindowPlacement placement)
    : markdown_path_(std::move(markdown_path)), placement_(placement) {}

int Application::run(char* executable) {
    GtkApplication* gtk_application = gtk_application_new(
        "io.github.mpreview", static_cast<GApplicationFlags>(0));
    if (gtk_application == nullptr) {
        throw std::runtime_error("failed to initialize GTK");
    }

    g_signal_connect(gtk_application, "activate", G_CALLBACK(onActivate), this);
    char* gtk_arguments[] = {executable, nullptr};
    const int status = g_application_run(G_APPLICATION(gtk_application), 1, gtk_arguments);
    g_object_unref(gtk_application);
    return exit_status_ != 0 ? exit_status_ : status;
}

void Application::onActivate(GtkApplication* gtk_application, void* user_data) {
    auto* application = static_cast<Application*>(user_data);
    try {
        application->createWindow(gtk_application);
    } catch (const std::exception& exception) {
        std::cerr << "mpreview: " << exception.what() << '\n';
        application->exit_status_ = 1;
        g_application_quit(G_APPLICATION(gtk_application));
    }
}

void Application::onWindowMapped(void* window, void* user_data) {
    static_cast<Application*>(user_data)->tileWindow(window);
}

void Application::createWindow(GtkApplication* gtk_application) {
    if (web_view_ != nullptr) {
        GtkWindow* window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(web_view_)));
        gtk_window_present(window);
        return;
    }

    GtkWidget* window = gtk_application_window_new(gtk_application);
    const std::string title = markdown_path_.filename().string() + " — mpreview";
    gtk_window_set_title(GTK_WINDOW(window), title.c_str());
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 820);
    if (placement_ == WindowPlacement::maximized) {
        gtk_window_maximize(GTK_WINDOW(window));
    } else if (placement_ == WindowPlacement::left || placement_ == WindowPlacement::right) {
        g_signal_connect(window, "map", G_CALLBACK(onWindowMapped), this);
    }

    GtkWidget* view = webkit_web_view_new();
    if (view == nullptr) {
        throw std::runtime_error("failed to initialize WebKitGTK");
    }
    web_view_ = WEBKIT_WEB_VIEW(view);
    WebKitSettings* settings = webkit_web_view_get_settings(web_view_);
    webkit_settings_set_enable_smooth_scrolling(settings, FALSE);
    gtk_window_set_child(GTK_WINDOW(window), view);

    reloadDocument();
    watcher_ = std::make_unique<FileWatcher>(markdown_path_, [this] { reloadDocument(); });
    gtk_window_present(GTK_WINDOW(window));
}

void Application::tileWindow(void* window_pointer) {
    auto* window = GTK_WINDOW(window_pointer);
    GdkSurface* surface = gtk_native_get_surface(GTK_NATIVE(window));
    if (surface == nullptr || !GDK_IS_X11_SURFACE(surface)) {
        return;
    }

    GdkDisplay* display = gdk_surface_get_display(surface);
    GdkMonitor* monitor = gdk_display_get_monitor_at_surface(display, surface);
    if (monitor == nullptr) {
        return;
    }

    GdkRectangle geometry{};
    gdk_monitor_get_geometry(monitor, &geometry);
    const int width = geometry.width / 2;
    const int x = placement_ == WindowPlacement::right ? geometry.x + width : geometry.x;

    Display* x_display = gdk_x11_display_get_xdisplay(display);
    const Window x_window = gdk_x11_surface_get_xid(surface);
    XMoveResizeWindow(x_display, x_window, x, geometry.y, width, geometry.height);
    XFlush(x_display);
}

void Application::reloadDocument() {
    try {
        const std::string markdown = MarkdownRenderer::readFile(markdown_path_);
        const std::string html = renderer_.render(markdown);
        GError* uri_error = nullptr;
        char* directory_uri = g_filename_to_uri(
            markdown_path_.parent_path().c_str(), nullptr, &uri_error);
        if (directory_uri == nullptr) {
            const std::string message = uri_error != nullptr ? uri_error->message : "unknown error";
            g_clear_error(&uri_error);
            throw std::runtime_error("failed to create document URI: " + message);
        }
        const std::string base_uri = std::string(directory_uri) + "/";
        g_free(directory_uri);
        webkit_web_view_load_html(web_view_, html.c_str(), base_uri.c_str());
    } catch (const std::exception& exception) {
        std::cerr << "mpreview: reload failed: " << exception.what() << '\n';
    }
}
