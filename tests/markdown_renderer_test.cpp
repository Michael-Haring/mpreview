#include "markdown_renderer.hpp"

#include <iostream>
#include <string>

namespace {

bool contains(const std::string& text, const std::string& expected) {
    if (text.find(expected) != std::string::npos) {
        return true;
    }
    std::cerr << "missing rendered fragment: " << expected << '\n';
    return false;
}

}  // namespace

int main() {
    const std::string markdown =
        "# Unicode: héllo 世界\n\n"
        "~~removed~~\n\n"
        "- [x] complete\n\n"
        "| A | B |\n|---|---|\n| 1 | 2 |\n\n"
        "<script>unsafe()</script>\n";

    const std::string html = MarkdownRenderer().render(markdown);
    bool passed = true;
    passed &= contains(html, "<h1>Unicode: héllo 世界</h1>");
    passed &= contains(html, "<del>removed</del>");
    passed &= contains(html, "type=\"checkbox\"");
    passed &= contains(html, "<table>");
    passed &= contains(html, "<!-- raw HTML omitted -->");
    passed &= contains(html, "prefers-color-scheme: dark");
    return passed ? 0 : 1;
}
