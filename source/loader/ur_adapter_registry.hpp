/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */
#ifndef UR_ADAPTER_REGISTRY_HPP
#define UR_ADAPTER_REGISTRY_HPP 1

#include <array>
#include <filesystem>

#include "logger/ur_logger.hpp"
#include "ur_loader_location.hpp"
#include "ur_util.hpp"

namespace fs = std::filesystem;

namespace ur_loader {

class AdapterRegistry {
  public:
    AdapterRegistry() {
        std::optional<std::vector<std::string>> forceLoadedAdaptersOpt;
        try {
            forceLoadedAdaptersOpt = getenv_to_vec("UR_ADAPTERS_FORCE_LOAD");
        } catch (const std::invalid_argument &e) {
            logger::error(e.what());
        }

        if (forceLoadedAdaptersOpt.has_value()) {
            auto forceLoadedAdapters = forceLoadedAdaptersOpt.value();
            discovered_adapters.insert(discovered_adapters.end(),
                                       forceLoadedAdapters.begin(),
                                       forceLoadedAdapters.end());
        } else {
            discoverKnownAdapters();
        }
    }

    struct Iterator {
        using value_type = const std::string;
        using pointer = value_type *;

        Iterator(pointer ptr) noexcept : current_adapter(ptr) {}

        Iterator &operator++() noexcept {
            current_adapter++;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator!=(const Iterator &other) const noexcept {
            return this->current_adapter != other.current_adapter;
        }

        const value_type operator*() const { return *this->current_adapter; }

      private:
        pointer current_adapter;
    };

    const std::string &operator[](size_t i) const {
        return discovered_adapters[i];
    }

    bool empty() const noexcept { return discovered_adapters.size() == 0; }

    size_t size() const noexcept { return discovered_adapters.size(); }

    const Iterator begin() const noexcept {
        return Iterator(&(*discovered_adapters.cbegin()));
    }

    const Iterator end() const noexcept {
        return Iterator(&(*discovered_adapters.cbegin()) +
                        discovered_adapters.size());
    }

  private:
    std::vector<std::string> discovered_adapters;

    static constexpr std::array<const char *, 1> knownPlatformNames{
        MAKE_LIBRARY_NAME("ur_adapter_level_zero", "0")};

    std::optional<std::vector<fs::path>> getEnvAdapterSearchPaths() {
        std::optional<std::vector<std::string>> pathStringsOpt;
        try {
            pathStringsOpt = getenv_to_vec("UR_ADAPTERS_SEARCH_PATH");
        } catch (const std::invalid_argument &e) {
            logger::error(e.what());
            return std::nullopt;
        }

        if (pathStringsOpt.has_value()) {
            std::vector<fs::path> paths;
            auto pathStrings = pathStringsOpt.value();

            std::transform(pathStrings.cbegin(), pathStrings.cend(),
                           std::back_inserter(paths),
                           [](const std::string &s) { return fs::path(s); });
            paths.erase(std::remove_if(paths.begin(), paths.end(),
                                       [](const fs::path &path) {
                                           return !fs::exists(path);
                                       }),
                        paths.end());

            if (!paths.empty()) {
                return paths;
            }
        }

        return std::nullopt;
    }

    void discoverKnownAdapters() {
        std::vector<fs::path> searchPaths;

        auto searchPathsEnvOpt = getEnvAdapterSearchPaths();
        if (searchPathsEnvOpt.has_value()) {
            auto searchPathsEnv = searchPathsEnvOpt.value();
            std::copy(searchPathsEnv.begin(), searchPathsEnv.end(),
                      std::back_inserter(searchPaths));
        }

        auto searchPathsOSOpt = getOSSearchPaths();
        if (searchPathsOSOpt.has_value()) {
            auto searchPathsOS = searchPathsOSOpt.value();
            std::copy(searchPathsOS.begin(), searchPathsOS.end(),
                      std::back_inserter(searchPaths));
        }

        for (const auto &platformName : knownPlatformNames) {
            auto platformExists = [&platformName](const auto &path) {
                return fs::exists(path / platformName);
            };

            auto it = std::find_if(searchPaths.begin(), searchPaths.end(),
                                   platformExists);
            if (it != searchPaths.end()) {
                auto fullPath = *it / platformName;
                discovered_adapters.emplace_back(fullPath.string());
            }
        }
    }
};

} // namespace ur_loader

#endif // UR_ADAPTER_REGISTRY_HPP
