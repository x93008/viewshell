from conan import ConanFile
from conan.tools.cmake import cmake_layout


class ViewshellConan(ConanFile):
    name = "viewshell-deps"
    version = "0.1"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("nlohmann_json/3.11.3")
        self.requires("tl-expected/1.1.0")
        self.requires("gtest/1.14.0")
        if self.settings.os == "Windows":
            self.requires("webview2-sdk/1.0")

    def layout(self):
        cmake_layout(self)
