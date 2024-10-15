// Copyright (C) 2024 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <umf/memory_pool.h>
#include <umf/memory_provider.h>
#include <uur/fixtures.h>

#ifndef ASSERT_UMF_SUCCESS
#define ASSERT_UMF_SUCCESS(ACTUAL) ASSERT_EQ(ACTUAL, UMF_RESULT_SUCCESS)
#endif

using urL0AsyncAllocTest = uur::urKernelTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urL0AsyncAllocTest);

TEST_P(urL0AsyncAllocTest, SuccessHostL0Ipc) {
    ur_device_usm_access_capability_flags_t hostUSMSupport = 0;
    ASSERT_SUCCESS(uur::GetDeviceUSMHostSupport(device, hostUSMSupport));
    if (!hostUSMSupport) {
        GTEST_SKIP() << "Host USM is not supported.";
    }

    size_t dataSize = 1024;
    ur_usm_pool_desc_t pool_desc;
    ur_usm_pool_limits_desc_t pool_limits;
    pool_desc.stype = UR_STRUCTURE_TYPE_USM_POOL_DESC;
    pool_desc.pNext = &pool_limits;
    pool_desc.flags = 0;
    pool_limits.stype = UR_STRUCTURE_TYPE_USM_POOL_LIMITS_DESC;
    pool_limits.pNext = nullptr;
    pool_limits.maxPoolableSize = dataSize;
    pool_limits.minDriverAllocSize = dataSize;

    ur_queue_handle_t queue;
    urQueueCreate(context, device, nullptr, &queue);

    ur_usm_pool_handle_t pool;
    urUSMPoolCreate(context, nullptr, &pool);

    void *ptr;
    ASSERT_SUCCESS(urEnqueueUSMDeviceAllocExp(queue, pool, dataSize, nullptr, 0, nullptr, &ptr, nullptr));
    ASSERT_SUCCESS(urKernelSetArgPointer(kernel, 0, nullptr, ptr));
    
    const size_t workDim = 1;
    const size_t globalWorkOffset[] = {0};
    const size_t globalWorkSize[] = {1};
    const size_t localWorkSize[] = {1};
    ASSERT_SUCCESS(urEnqueueKernelLaunch(queue, kernel, workDim,
                                         globalWorkOffset, globalWorkSize,
                                         localWorkSize, 0, nullptr, nullptr));

    ASSERT_SUCCESS(urEnqueueUSMFreeExp(queue, pool, ptr, 0, nullptr, nullptr));

    void *ptr2;
    urEnqueueUSMDeviceAllocExp(queue, pool, dataSize, nullptr, 0, nullptr, &ptr2, nullptr);
    assert(ptr == ptr2); // since this is an in-order queue, and we previously freed ptr, we can reuse that same object

    ASSERT_SUCCESS(urKernelSetArgPointer(kernel, 0, nullptr, ptr2));
    ASSERT_SUCCESS(urEnqueueKernelLaunch(queue, kernel, workDim,
                                         globalWorkOffset, globalWorkSize,
                                         localWorkSize, 0, nullptr, nullptr));
}
