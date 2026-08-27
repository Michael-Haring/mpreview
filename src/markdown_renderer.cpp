#include "markdown_renderer.hpp"

#include <cmark-gfm-core-extensions.h>
#include <cmark-gfm-extension_api.h>
#include <cmark-gfm.h>

#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>

namespace {

constexpr const char* stylesheet = R"CSS(
:root { color-scheme: light dark; }
* { box-sizing: border-box; }
body {
  margin: 0;
  color: #1f2328;
  background: #ffffff;
  font: 16px/1.6 -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
}
.markdown-body { max-width: 980px; margin: 0 auto; padding: 42px 48px 72px; }
h1, h2, h3, h4, h5, h6 { margin: 1.5em 0 0.5em; line-height: 1.25; }
h1, h2 { padding-bottom: 0.3em; border-bottom: 1px solid #d8dee4; }
h1 { font-size: 2em; } h2 { font-size: 1.5em; } h3 { font-size: 1.25em; }
p, blockquote, ul, ol, table, pre { margin: 0 0 1em; }
a { color: #0969da; text-decoration: none; } a:hover { text-decoration: underline; }
blockquote { margin-left: 0; padding: 0 1em; color: #59636e; border-left: 0.25em solid #d0d7de; }
code, pre { font-family: ui-monospace, SFMono-Regular, Consolas, monospace; }
code { padding: 0.2em 0.4em; font-size: 85%; background: #eff1f3; border-radius: 6px; }
pre { overflow: auto; padding: 16px; background: #f6f8fa; border-radius: 6px; }
pre code { padding: 0; font-size: 85%; background: transparent; border-radius: 0; }
table { display: block; max-width: 100%; overflow: auto; border-collapse: collapse; }
th, td { padding: 6px 13px; border: 1px solid #d0d7de; }
tr:nth-child(2n) { background: #f6f8fa; }
img { max-width: 100%; height: auto; }
hr { height: 0.25em; margin: 24px 0; background: #d8dee4; border: 0; }
input[type="checkbox"] { margin: 0 0.5em 0.25em -1.4em; vertical-align: middle; }
@media (max-width: 700px) { .markdown-body { padding: 24px 20px 48px; } }
@media (prefers-color-scheme: dark) {
  body { color: #e6edf3; background: #0d1117; }
  h1, h2 { border-color: #30363d; }
  a { color: #58a6ff; }
  blockquote { color: #9198a1; border-color: #3d444d; }
  code { background: #343941; }
  pre, tr:nth-child(2n) { background: #151b23; }
  th, td { border-color: #3d444d; }
  hr { background: #30363d; }
}
)CSS";

struct ParserDeleter {
    void operator()(cmark_parser* parser) const { cmark_parser_free(parser); }
};

struct NodeDeleter {
    void operator()(cmark_node* node) const { cmark_node_free(node); }
};

struct BufferDeleter {
    void operator()(char* buffer) const { free(buffer); }
};

}  // namespace

std::string MarkdownRenderer::render(const std::string& markdown) const {
    cmark_gfm_core_extensions_ensure_registered();
    std::unique_ptr<cmark_parser, ParserDeleter> parser(cmark_parser_new(CMARK_OPT_DEFAULT));
    if (!parser) {
        throw std::runtime_error("failed to create Markdown parser");
    }

    constexpr const char* extension_names[] = {
        "table", "strikethrough", "tasklist", "autolink"};
    for (const char* name : extension_names) {
        cmark_syntax_extension* extension = cmark_find_syntax_extension(name);
        if (extension == nullptr ||
            !cmark_parser_attach_syntax_extension(parser.get(), extension)) {
            throw std::runtime_error(std::string("failed to enable Markdown extension: ") + name);
        }
    }

    cmark_parser_feed(parser.get(), markdown.data(), markdown.size());
    std::unique_ptr<cmark_node, NodeDeleter> document(cmark_parser_finish(parser.get()));
    if (!document) {
        throw std::runtime_error("failed to parse Markdown");
    }

    std::unique_ptr<char, BufferDeleter> body(
        cmark_render_html(document.get(), CMARK_OPT_DEFAULT,
                          cmark_parser_get_syntax_extensions(parser.get())));
    if (!body) {
        throw std::runtime_error("failed to render Markdown");
    }

    return "<!doctype html><html><head><meta charset=\"utf-8\">"
           "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
           "<style>" + std::string(stylesheet) + "</style></head>"
           "<body><main class=\"markdown-body\">" + body.get() + "</main></body></html>";
}

std::string MarkdownRenderer::readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("unable to open '" + path.string() + "'");
    }

    std::string contents((std::istreambuf_iterator<char>(input)),
                         std::istreambuf_iterator<char>());
    if (input.bad()) {
        throw std::runtime_error("unable to read '" + path.string() + "'");
    }
    return contents;
}
