/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UR_LOADER_LOCATION_HPP
#define UR_LOADER_LOCATION_HPP 1

#include <filesystem>

namespace fs = std::filesystem;

namespace ur_loader {

std::optional<std::vector<fs::path>> getOSSearchPaths();

} // namespace ur_loader

#endif /* UR_LOADER_LOCATION_HPP */
