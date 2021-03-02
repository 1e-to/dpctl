//===--- test_sycl_device_interface.cpp - Test cases for device interface  ===//
//
//                      Data Parallel Control (dpCtl)
//
// Copyright 2020-2021 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file has unit test cases for functions defined in
/// dpctl_sycl_device_interface.h.
///
//===----------------------------------------------------------------------===//

#include "dpctl_sycl_device_interface.h"
#include "dpctl_sycl_device_selector_interface.h"
#include "dpctl_sycl_enum_types.h"
#include "dpctl_sycl_platform_interface.h"
#include "dpctl_sycl_queue_interface.h"
#include "dpctl_sycl_queue_manager.h"
#include "dpctl_utils.h"
#include <CL/sycl.hpp>
#include <gtest/gtest.h>

using namespace cl::sycl;

struct TestDPCTLSyclDeviceInterface
    : public ::testing::TestWithParam<const char *>
{
    DPCTLSyclDeviceSelectorRef DSRef = nullptr;

    TestDPCTLSyclDeviceInterface()
    {
        EXPECT_NO_FATAL_FAILURE(DSRef = DPCTLFilterSelector_Create(GetParam()));
    }

    void SetUp()
    {
        if (!DSRef) {
            auto message = "Skipping as no device of type " +
                           std::string(GetParam()) + ".";
            GTEST_SKIP_(message.c_str());
        }
    }

    ~TestDPCTLSyclDeviceInterface()
    {
        EXPECT_NO_FATAL_FAILURE(DPCTLDeviceSelector_Delete(DSRef));
    }
};

struct TestDPCTLSyclDeviceInterfaceAspects
    : public ::testing::TestWithParam<DPCTLSyclAspectType>
{
    DPCTLSyclDeviceRef OpenCL_cpu = nullptr;
    DPCTLSyclDeviceRef OpenCL_gpu = nullptr;
    DPCTLSyclDeviceRef OpenCL_Level0_gpu = nullptr;

    TestDPCTLSyclDeviceInterfaceAspects()
    {
        if (DPCTLQueueMgr_GetNumQueues(DPCTL_OPENCL, DPCTL_CPU)) {
            auto Q = DPCTLQueueMgr_GetQueue(DPCTL_OPENCL, DPCTL_CPU, 0);
            OpenCL_cpu = DPCTLQueue_GetDevice(Q);
            DPCTLQueue_Delete(Q);
        }

        if (DPCTLQueueMgr_GetNumQueues(DPCTL_OPENCL, DPCTL_GPU)) {
            auto Q = DPCTLQueueMgr_GetQueue(DPCTL_OPENCL, DPCTL_GPU, 0);
            OpenCL_gpu = DPCTLQueue_GetDevice(Q);
            DPCTLQueue_Delete(Q);
        }

        if (DPCTLQueueMgr_GetNumQueues(DPCTL_LEVEL_ZERO, DPCTL_GPU)) {
            auto Q = DPCTLQueueMgr_GetQueue(DPCTL_LEVEL_ZERO, DPCTL_GPU, 0);
            OpenCL_Level0_gpu = DPCTLQueue_GetDevice(Q);
            DPCTLQueue_Delete(Q);
        }
    }

    ~TestDPCTLSyclDeviceInterfaceAspects()
    {
        DPCTLDevice_Delete(OpenCL_cpu);
        DPCTLDevice_Delete(OpenCL_gpu);
        DPCTLDevice_Delete(OpenCL_Level0_gpu);
    }
};

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetBackend)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    DPCTLSyclBackendType BTy = DPCTLSyclBackendType::DPCTL_UNKNOWN_BACKEND;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(BTy = DPCTLDevice_GetBackend(DRef));
    EXPECT_TRUE([BTy] {
        switch (BTy) {
        case DPCTLSyclBackendType::DPCTL_CUDA:
            return true;
        case DPCTLSyclBackendType::DPCTL_HOST:
            return true;
        case DPCTLSyclBackendType::DPCTL_LEVEL_ZERO:
            return true;
        case DPCTLSyclBackendType::DPCTL_OPENCL:
            return true;
        default:
            return false;
        }
    }());
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetDeviceType)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    DPCTLSyclDeviceType DTy = DPCTLSyclDeviceType::DPCTL_UNKNOWN_DEVICE;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(DTy = DPCTLDevice_GetDeviceType(DRef));
    EXPECT_TRUE(DTy != DPCTLSyclDeviceType::DPCTL_UNKNOWN_DEVICE);
    EXPECT_TRUE(DTy != DPCTLSyclDeviceType::DPCTL_ALL);
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetDriverInfo)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    const char *DriverInfo = nullptr;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(DriverInfo = DPCTLDevice_GetDriverInfo(DRef));
    EXPECT_TRUE(DriverInfo != nullptr);
    EXPECT_NO_FATAL_FAILURE(DPCTLCString_Delete(DriverInfo));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetName)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    const char *Name = nullptr;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(Name = DPCTLDevice_GetName(DRef));
    EXPECT_TRUE(Name != nullptr);
    EXPECT_NO_FATAL_FAILURE(DPCTLCString_Delete(Name));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetVendorName)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    const char *VendorName = nullptr;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(VendorName = DPCTLDevice_GetVendorName(DRef));
    EXPECT_TRUE(VendorName != nullptr);
    EXPECT_NO_FATAL_FAILURE(DPCTLCString_Delete(VendorName));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetMaxComputeUnits)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    size_t n = 0;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(n = DPCTLDevice_GetMaxComputeUnits(DRef));
    EXPECT_TRUE(n > 0);
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetMaxWorkItemDims)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    size_t n = 0;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(n = DPCTLDevice_GetMaxWorkItemDims(DRef));
    EXPECT_TRUE(n > 0);
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetMaxWorkItemSizes)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    size_t *sizes = nullptr;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(sizes = DPCTLDevice_GetMaxWorkItemSizes(DRef));
    EXPECT_TRUE(sizes != nullptr);
    EXPECT_NO_FATAL_FAILURE(DPCTLSize_t_Array_Delete(sizes));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetMaxWorkGroupSize)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    size_t n = 0;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(n = DPCTLDevice_GetMaxWorkGroupSize(DRef));
    if (DPCTLDevice_IsAccelerator(DRef))
        EXPECT_TRUE(n >= 0);
    else
        EXPECT_TRUE(n > 0);
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetMaxNumSubGroups)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    size_t n = 0;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(n = DPCTLDevice_GetMaxNumSubGroups(DRef));
    if (DPCTLDevice_IsAccelerator(DRef))
        EXPECT_TRUE(n >= 0);
    else
        EXPECT_TRUE(n > 0);
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_GetPlatform)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    DPCTLSyclPlatformRef PRef = nullptr;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(PRef = DPCTLDevice_GetPlatform(DRef));
    ASSERT_TRUE(PRef);
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
    EXPECT_NO_FATAL_FAILURE(DPCTLPlatform_Delete(PRef));
}

// TODO: Update when DPC++ properly supports aspects
TEST_P(TestDPCTLSyclDeviceInterface, Chk_HasInt64BaseAtomics)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    bool atomics = 0;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(atomics = DPCTLDevice_HasInt64BaseAtomics(DRef));
    auto D = reinterpret_cast<device *>(DRef);
    auto has_atomics = D->has(aspect::int64_base_atomics);
    EXPECT_TRUE(has_atomics == atomics);
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

// TODO: Update when DPC++ properly supports aspects
TEST_P(TestDPCTLSyclDeviceInterface, Chk_HasInt64ExtendedAtomics)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    bool atomics = 0;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(atomics =
                                DPCTLDevice_HasInt64ExtendedAtomics(DRef));
    auto D = reinterpret_cast<device *>(DRef);
    auto has_atomics = D->has(aspect::int64_extended_atomics);
    EXPECT_TRUE(has_atomics == atomics);
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_IsAccelerator)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_IsAccelerator(DRef));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_IsCPU)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_IsCPU(DRef));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_IsGPU)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_IsGPU(DRef));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST_P(TestDPCTLSyclDeviceInterface, Chk_IsHost)
{
    DPCTLSyclDeviceRef DRef = nullptr;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    if (!DRef)
        GTEST_SKIP_("Device not found");
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_IsHost(DRef));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

INSTANTIATE_TEST_SUITE_P(DPCTLDevice_Fns,
                         TestDPCTLSyclDeviceInterface,
                         ::testing::Values("opencl",
                                           "opencl:gpu",
                                           "opencl:cpu",
                                           "opencl:gpu:0",
                                           "gpu",
                                           "cpu",
                                           "level_zero",
                                           "level_zero:gpu",
                                           "opencl:cpu:0",
                                           "level_zero:gpu:0",
                                           "gpu:0",
                                           "gpu:1",
                                           "1"));

TEST_P(TestDPCTLSyclDeviceInterfaceAspects, CheckOCLCPU_HasAspect)
{
    if (!OpenCL_cpu)
        GTEST_SKIP_("Skipping as no OpenCL CPU device found.");

    DPCTLSyclAspectType GetAspect = GetParam();

    if (GetAspect == emulated ||
        // GetAspect == image ||
        GetAspect == usm_device_allocations ||
        GetAspect == usm_host_allocations ||
        GetAspect == usm_atomic_host_allocations ||
        GetAspect == usm_shared_allocations ||
        GetAspect == usm_atomic_shared_allocations ||
        GetAspect == usm_system_allocations)
        GTEST_SKIP_("This device aspect has not been implemented yet.");

    auto Aspect = DPCTLDevice_HasAspect(OpenCL_cpu, GetAspect);
    auto D = reinterpret_cast<device *>(OpenCL_cpu);
    auto HasAspect = D->has(cl::sycl::aspect(GetAspect));
    EXPECT_TRUE(Aspect == HasAspect);
}

TEST_P(TestDPCTLSyclDeviceInterfaceAspects, CheckOCLGPU_HasAspect)
{
    if (!OpenCL_gpu)
        GTEST_SKIP_("Skipping as no OpenCL CPU device found.");

    DPCTLSyclAspectType GetAspect = GetParam();

    if (GetAspect == emulated ||
        // GetAspect == image ||
        GetAspect == usm_device_allocations ||
        GetAspect == usm_host_allocations ||
        GetAspect == usm_atomic_host_allocations ||
        GetAspect == usm_shared_allocations ||
        GetAspect == usm_atomic_shared_allocations ||
        GetAspect == usm_system_allocations)
        GTEST_SKIP_("This device aspect has not been implemented yet.");

    auto Aspect = DPCTLDevice_HasAspect(OpenCL_gpu, GetAspect);
    auto D = reinterpret_cast<device *>(OpenCL_gpu);
    auto HasAspect = D->has(cl::sycl::aspect(GetAspect));
    EXPECT_TRUE(Aspect == HasAspect);
}

TEST_P(TestDPCTLSyclDeviceInterfaceAspects, CheckLevel0GPU_HasAspect)
{
    if (!OpenCL_Level0_gpu)
        GTEST_SKIP_("Skipping as no OpenCL CPU device found.");

    DPCTLSyclAspectType GetAspect = GetParam();

    if (GetAspect == emulated ||
        // GetAspect == image ||
        GetAspect == usm_device_allocations ||
        GetAspect == usm_host_allocations ||
        GetAspect == usm_atomic_host_allocations ||
        GetAspect == usm_shared_allocations ||
        GetAspect == usm_atomic_shared_allocations ||
        GetAspect == usm_system_allocations)
        GTEST_SKIP_("This device aspect has not been implemented yet.");

    auto Aspect = DPCTLDevice_HasAspect(OpenCL_Level0_gpu, GetAspect);
    auto D = reinterpret_cast<device *>(OpenCL_Level0_gpu);
    auto HasAspect = D->has(cl::sycl::aspect(GetAspect));
    EXPECT_TRUE(Aspect == HasAspect);
}

INSTANTIATE_TEST_SUITE_P(DPCTLSyclDeviceInterfaceAspects,
                         TestDPCTLSyclDeviceInterfaceAspects,
                         ::testing::Values(cpu,
                                           gpu,
                                           accelerator,
                                           custom,
                                           emulated,
                                           host_debuggable,
                                           fp16,
                                           fp64,
                                           atomic64,
                                           //    image,
                                           online_compiler,
                                           online_linker,
                                           queue_profiling,
                                           usm_device_allocations,
                                           usm_host_allocations,
                                           usm_atomic_host_allocations,
                                           usm_shared_allocations,
                                           usm_atomic_shared_allocations,
                                           usm_system_allocations));
