/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <dlfcn.h>
#include <filesystem>
#include <optional>

#include "ur_loader.hpp"

namespace fs = std::filesystem;

namespace ur_loader {

static std::optional<fs::path> getLoaderLibDirectory() {
    Dl_info info;
    if (dladdr((void *)getLoaderLibDirectory, &info)) {
        auto libPath = fs::path(info.dli_fname);
        if (fs::exists(libPath)) {
            return fs::absolute(libPath).parent_path();
        }
    }

    return std::nullopt;
}

static std::optional<std::vector<fs::path>> getEnvOSSearchPaths() {
    std::optional<std::string> pathStringsOpt;
    try {
        pathStringsOpt = ur_getenv("LD_LIBRARY_PATH");
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

    auto envPathsOpt = getEnvOSSearchPaths();
    if (envPathsOpt.has_value()) {
        auto envPaths = envPathsOpt.value();
        std::copy(envPaths.begin(), envPaths.end(), std::back_inserter(paths));
    }

    auto loaderLibPathOpt = getLoaderLibDirectory();
    if (loaderLibPathOpt.has_value()) {
        auto loaderLibPath = loaderLibPathOpt.value();
        paths.push_back(loaderLibPath);
    }

    if (!paths.empty()) {
        return paths;
    }

    return std::nullopt;
}

} // namespace ur_loader
