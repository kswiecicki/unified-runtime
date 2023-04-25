// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include "fixtures.hpp"

TEST_F(adapterRegSearchTest, testAdaptersEnvSearchPathSuccess) {
    // Check if any of the paths gathered by AdapterRegistry
    // has full path to the hardcoded lib name
    auto it = std::find_if(registry.begin(), registry.end(), isFullLibPath);
    ASSERT_NE(it, registry.end());
}
