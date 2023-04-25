/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/* This include needs to be before Libloaderapi.h */
#include <windows.h>

#include <Libloaderapi.h>
#include <filesystem>

#include "ur_loader.hpp"

#define MAX_PATH_LEN_WIN 1024

namespace fs = std::filesystem;

namespace ur_loader {

static std::optional<fs::path> getLoaderLibDirectory() {
    char pathStr[MAX_PATH_LEN_WIN];
    if (GetModuleFileNameA(nullptr, pathStr, MAX_PATH_LEN_WIN)) {
        auto libPath = fs::path(pathStr);
        if (fs::exists(libPath)) {
            return fs::absolute(libPath).parent_path();
        }
    }

    return std::nullopt;
}

static std::optional<std::vector<fs::path>> getSysDirectories() {
    std::vector<fs::path> paths;

    char pathStr[MAX_PATH_LEN_WIN];
    if (GetSystemDirectoryA(pathStr, MAX_PATH_LEN_WIN) != 0) {
        auto systemPath = fs::path(pathStr);
        if (fs::exists(systemPath)) {
            paths.push_back(fs::absolute(systemPath));
        }
    }

    if (GetWindowsDirectoryA(pathStr, MAX_PATH_LEN_WIN) == 0) {
        auto winPath = fs::path(pathStr);
        if (fs::exists(winPath)) {
            paths.push_back(fs::absolute(winPath));
        }
    }

    if (!paths.empty()) {
        return paths;
    }

    return std::nullopt;
}

static std::optional<std::vector<fs::path>> getEnvOSSearchPaths() {
    std::optional<std::string> pathStringsOpt;
    try {
        pathStringsOpt = ur_getenv("PATH");
    } catch (const std::invalid_argument &e) {
        logger::error(e.what());
        return std::nullopt;
    }

    if (pathStringsOpt.has_value()) {
        std::vector<fs::path> paths;
        std::stringstream ss(pathStringsOpt.value());
        std::string pathStr;
        while (std::getline(ss, pathStr, ':')) {
            auto searchPath = fs::path(pathStr);
            if (fs::exists(searchPath)) {
                paths.push_back(fs::absolute(searchPath));
            }
        }

        if (!paths.empty()) {
            return paths;
        }
    }

    return std::nullopt;
}

std::optional<std::vector<fs::path>> getOSSearchPaths() {
    std::vector<fs::path> paths;

    auto sysPathsOpt = getSysDirectories();
    if (sysPathsOpt.has_value()) {
        auto sysPaths = sysPathsOpt.value();
        std::copy(sysPaths.begin(), sysPaths.end(), std::back_inserter(paths));
    }

    auto loaderLibPathOpt = getLoaderLibDirectory();
    if (loaderLibPathOpt.has_value()) {
        auto loaderLibPath = loaderLibPathOpt.value();
        paths.push_back(loaderLibPath);
    }

    auto envPathsOpt = getEnvOSSearchPaths();
    if (envPathsOpt.has_value()) {
        auto envPaths = envPathsOpt.value();
        std::copy(envPaths.begin(), envPaths.end(), std::back_inserter(paths));
    }

    if (!paths.empty()) {
        return paths;
    }

    return std::nullopt;
}

} // namespace ur_loader
