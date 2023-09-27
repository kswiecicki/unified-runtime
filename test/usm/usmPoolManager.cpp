// Copyright (C) 2023 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "ur_pool_manager.hpp"

#include "../unified_malloc_framework/common/pool.h"
#include "../unified_malloc_framework/common/provider.h"

#include <uur/fixtures.h>

struct urUsmPoolDescriptorTest
    : public uur::urMultiDeviceContextTest,
      ::testing::WithParamInterface<ur_usm_pool_handle_t> {};

TEST_P(urUsmPoolDescriptorTest, poolIsPerContextTypeAndDevice) {
    auto &devices = uur::DevicesEnvironment::instance->devices;
    auto poolHandle = this->GetParam();

    auto [ret, pool_descriptors] =
        usm::pool_descriptor::create(poolHandle, this->context);
    ASSERT_EQ(ret, UR_RESULT_SUCCESS);

    size_t hostPools = 0;
    size_t devicePools = 0;
    size_t sharedPools = 0;

    for (auto &desc : pool_descriptors) {
        ASSERT_EQ(desc.poolHandle, poolHandle);
        ASSERT_EQ(desc.hContext, this->context);

        if (desc.type == UR_USM_TYPE_DEVICE) {
            devicePools++;
        } else if (desc.type == UR_USM_TYPE_SHARED) {
            sharedPools++;
        } else if (desc.type == UR_USM_TYPE_HOST) {
            hostPools += 2;
        } else {
            FAIL();
        }
    }

    // Each device has pools for Host, Device, Shared, SharedReadOnly only
    ASSERT_EQ(pool_descriptors.size(), 4 * devices.size());
    ASSERT_EQ(hostPools, 1);
    ASSERT_EQ(devicePools, devices.size());
    ASSERT_EQ(sharedPools, devices.size() * 2);
}

INSTANTIATE_TEST_SUITE_P(urUsmPoolDescriptorTest, urUsmPoolDescriptorTest,
                         ::testing::Values(nullptr));

// TODO: add test with sub-devices

struct urUsmPoolManagerTest : public uur::urContextTest {
    void SetUp() override {
        UUR_RETURN_ON_FATAL_FAILURE(urContextTest::SetUp());
        auto [ret, descs] = usm::pool_descriptor::create(nullptr, context);
        ASSERT_EQ(ret, UR_RESULT_SUCCESS);
        poolDescriptors = descs;
    }

    std::vector<usm::pool_descriptor> poolDescriptors;
};

TEST_P(urUsmPoolManagerTest, poolManagerPopulate) {
    auto [ret, manager] = usm::pool_manager<usm::pool_descriptor>::create();
    ASSERT_EQ(ret, UR_RESULT_SUCCESS);

    for (auto &desc : poolDescriptors) {
        // Populate the pool manager
        auto pool = nullPoolCreate();
        ASSERT_NE(pool, nullptr);
        auto poolUnique = umf::pool_unique_handle_t(pool, umfPoolDestroy);
        ASSERT_NE(poolUnique, nullptr);
        ret = manager.addPool(desc, poolUnique);
        ASSERT_EQ(ret, UR_RESULT_SUCCESS);
    }

    for (auto &desc : poolDescriptors) {
        // Confirm that there is a pool for each descriptor
        auto hPoolOpt = manager.getPool(desc);
        ASSERT_TRUE(hPoolOpt.has_value());
        ASSERT_NE(hPoolOpt.value(), nullptr);
    }
}

TEST_P(urUsmPoolManagerTest, poolManagerInsertExisting) {
    auto [ret, manager] = usm::pool_manager<usm::pool_descriptor>::create();
    ASSERT_EQ(ret, UR_RESULT_SUCCESS);

    auto desc = poolDescriptors[0];

    auto pool = nullPoolCreate();
    ASSERT_NE(pool, nullptr);
    auto poolUnique = umf::pool_unique_handle_t(pool, umfPoolDestroy);
    ASSERT_NE(poolUnique, nullptr);

    ret = manager.addPool(desc, poolUnique);
    ASSERT_EQ(ret, UR_RESULT_SUCCESS);

    // Inserting an existing key should return an error
    ret = manager.addPool(desc, poolUnique);
    ASSERT_EQ(ret, UR_RESULT_ERROR_INVALID_ARGUMENT);
}

TEST_P(urUsmPoolManagerTest, poolManagerGetNonexistant) {
    auto [ret, manager] = usm::pool_manager<usm::pool_descriptor>::create();
    ASSERT_EQ(ret, UR_RESULT_SUCCESS);

    for (auto &desc : poolDescriptors) {
        auto hPool = manager.getPool(desc);
        ASSERT_FALSE(hPool.has_value());
    }
}

UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urUsmPoolManagerTest);
