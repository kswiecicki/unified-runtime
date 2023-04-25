// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#ifndef UR_ADAPTER_REG_TEST_HELPERS_H
#define UR_ADAPTER_REG_TEST_HELPERS_H

#include "ur_adapter_registry.hpp"
#include "ur_util.hpp"
#include <functional>
#include <gtest/gtest.h>

struct adapterRegSearchTest : ::testing::Test {
    ur_loader::AdapterRegistry registry;
    const std::string libName = MAKE_LIBRARY_NAME("ur_adapter_level_zero", "0");

    std::function<int(const std::string &)> isFullLibPath =
        [this](const std::string &path) {
            if (libName.length() > path.length() || libName == path) {
                return false;
            }
            return path.compare(path.length() - libName.length(),
                                libName.length(), libName) == 0;
        };
};

#endif // UR_ADAPTER_REG_TEST_HELPERS_H
