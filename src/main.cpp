#include "application.hpp"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>

namespace {

constexpr std::string_view version = "0.1.0";

void printUsage(std::ostream& output) {
    output << "Usage: mpreview [--left|--right|--maximized] <file.md>\n";
}

void printHelp() {
    printUsage(std::cout);
    std::cout << R"HELP(

Preview a Markdown file in a native window and reload it when the file is saved.

Options:
  --left       Tile the window on the left half of the screen (X11)
  --right      Tile the window on the right half of the screen (X11)
  --maximized  Start with the window maximized
  --help       Show this help and exit
  --version    Show the version and exit

The default window is centered at 1100x820. Exact left/right placement is controlled
by the desktop compositor on Wayland and may be ignored there.

Examples:
  mpreview README.md
  mpreview --right docs/design.md
  mpreview --left notes.md
)HELP";
}

}  // namespace

int main(int argc, char* argv[]) {
    WindowPlacement placement = WindowPlacement::centered;
    std::optional<std::filesystem::path> input_path;
    bool placement_selected = false;

    for (int index = 1; index < argc; ++index) {
        const std::string_view argument(argv[index]);
        if (argument == "--help") {
            printHelp();
            return 0;
        }
        if (argument == "--version") {
            std::cout << "mpreview " << version << '\n';
            return 0;
        }

        WindowPlacement requested_placement = WindowPlacement::centered;
        bool is_placement = true;
        if (argument == "--left") {
            requested_placement = WindowPlacement::left;
        } else if (argument == "--right") {
            requested_placement = WindowPlacement::right;
        } else if (argument == "--maximized") {
            requested_placement = WindowPlacement::maximized;
        } else {
            is_placement = false;
        }

        if (is_placement) {
            if (placement_selected) {
                std::cerr << "mpreview: window placement options are mutually exclusive\n";
                return 2;
            }
            placement = requested_placement;
            placement_selected = true;
        } else if (!argument.empty() && argument.front() == '-') {
            std::cerr << "mpreview: unknown option: " << argument << '\n';
            return 2;
        } else if (input_path.has_value()) {
            std::cerr << "mpreview: expected one Markdown file\n";
            return 2;
        } else {
            input_path = std::filesystem::path(argument);
        }
    }

    if (!input_path.has_value()) {
        printUsage(std::cerr);
        return 2;
    }

    std::error_code error;
    const auto status = std::filesystem::status(*input_path, error);
    if (error || !std::filesystem::exists(status)) {
        std::cerr << "mpreview: file does not exist: " << input_path->string() << '\n';
        return 1;
    }
    if (!std::filesystem::is_regular_file(status)) {
        std::cerr << "mpreview: not a regular file: " << input_path->string() << '\n';
        return 1;
    }

    try {
        const auto canonical_path = std::filesystem::canonical(*input_path);
        MarkdownRenderer::readFile(canonical_path);
        Application application(canonical_path, placement);
        return application.run(argv[0]);
    } catch (const std::exception& exception) {
        std::cerr << "mpreview: " << exception.what() << '\n';
        return 1;
    }
}
