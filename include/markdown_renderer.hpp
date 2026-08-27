#pragma once

#include <filesystem>
#include <string>

class MarkdownRenderer {
public:
    std::string render(const std::string& markdown) const;
    static std::string readFile(const std::filesystem::path& path);
};
