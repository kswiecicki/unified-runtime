// Copyright (C) 2024 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <uur/fixtures.h>

// using urL0EnqueueAllocTest = uur::urQueueTest;
struct urL0EnqueueAllocTest : uur::urQueueTest {
    void ValidateEnqueueFree(void *ptr) {
        ur_event_handle_t freeEvent = nullptr;
        ASSERT_NE(ptr, nullptr);
        ASSERT_SUCCESS(
            urEnqueueUSMFreeExp(queue, nullptr, ptr, 0, nullptr, &freeEvent));
        ASSERT_NE(freeEvent, nullptr);
        ASSERT_SUCCESS(urQueueFinish(queue));
    }

    static constexpr uint32_t DATA = 0xC0FFEE;
};

UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urL0EnqueueAllocTest);

TEST_P(urL0EnqueueAllocTest, SuccessHost) {
    ur_device_usm_access_capability_flags_t hostUSMSupport = 0;
    ASSERT_SUCCESS(uur::GetDeviceUSMHostSupport(device, hostUSMSupport));
    if (!hostUSMSupport) {
        GTEST_SKIP() << "Host USM is not supported.";
    }

    void *ptr = nullptr;
    ur_event_handle_t allocEvent = nullptr;
    ASSERT_SUCCESS(urEnqueueUSMHostAllocExp(queue, nullptr, sizeof(uint32_t),
                                            nullptr, 0, nullptr, &ptr,
                                            &allocEvent));
    ASSERT_SUCCESS(urQueueFinish(queue));
    ASSERT_NE(ptr, nullptr);
    ASSERT_NE(allocEvent, nullptr);
    *(uint32_t *)ptr = DATA;
    ValidateEnqueueFree(ptr);
}
