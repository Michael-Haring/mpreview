#pragma once

#include "file_watcher.hpp"
#include "markdown_renderer.hpp"

#include <filesystem>
#include <memory>
#include <string>

typedef struct _GtkApplication GtkApplication;
typedef struct _WebKitWebView WebKitWebView;

enum class WindowPlacement {
    centered,
    left,
    right,
    maximized,
};

class Application {
public:
    Application(std::filesystem::path markdown_path, WindowPlacement placement);
    int run(char* executable);

private:
    static void onActivate(GtkApplication* gtk_application, void* user_data);
    static void onWindowMapped(void* window, void* user_data);
    void createWindow(GtkApplication* gtk_application);
    void reloadDocument();
    void tileWindow(void* window);

    std::filesystem::path markdown_path_;
    WindowPlacement placement_;
    MarkdownRenderer renderer_;
    std::unique_ptr<FileWatcher> watcher_;
    WebKitWebView* web_view_ = nullptr;
    int exit_status_ = 0;
};
