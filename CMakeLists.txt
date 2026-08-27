cmake_minimum_required(VERSION 3.16)

project(mpreview VERSION 0.1.0 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
pkg_check_modules(WEBKITGTK REQUIRED IMPORTED_TARGET webkitgtk-6.0)
pkg_check_modules(CMARK_GFM REQUIRED IMPORTED_TARGET libcmark-gfm)

add_executable(mpreview
    src/application.cpp
    src/file_watcher.cpp
    src/main.cpp
    src/markdown_renderer.cpp
)

target_compile_features(mpreview PRIVATE cxx_std_17)
target_compile_options(mpreview PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(mpreview PRIVATE include)
target_link_libraries(mpreview PRIVATE
    PkgConfig::GTK4
    PkgConfig::X11
    PkgConfig::WEBKITGTK
    PkgConfig::CMARK_GFM
)

include(GNUInstallDirs)
install(TARGETS mpreview RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

include(CTest)
if(BUILD_TESTING)
    add_executable(markdown_renderer_test
        tests/markdown_renderer_test.cpp
        src/markdown_renderer.cpp
    )
    target_compile_features(markdown_renderer_test PRIVATE cxx_std_17)
    target_compile_options(markdown_renderer_test PRIVATE -Wall -Wextra -Wpedantic)
    target_include_directories(markdown_renderer_test PRIVATE include)
    target_link_libraries(markdown_renderer_test PRIVATE PkgConfig::CMARK_GFM)

    add_test(NAME markdown_renderer COMMAND markdown_renderer_test)
    add_test(NAME cli_help COMMAND mpreview --help)
    add_test(NAME cli_version COMMAND mpreview --version)
    add_test(NAME cli_missing_file COMMAND mpreview missing-test-file.md)
    set_tests_properties(cli_missing_file PROPERTIES WILL_FAIL TRUE)
endif()
