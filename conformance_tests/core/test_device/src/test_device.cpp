/*
 *
 * Copyright (C) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"

#include "utils/utils.hpp"
#include "test_harness/test_harness.hpp"
#include "logging/logging.hpp"

#include <chrono>
#include <thread>

namespace lzt = level_zero_tests;

#include <level_zero/ze_api.h>

namespace {

TEST(zeDeviceGetTests,
     GivenZeroCountWhenRetrievingDevicesThenValidCountReturned) {
  lzt::get_ze_device_count();
}

TEST(zeDeviceGetTests,
     GivenValidCountWhenRetrievingDevicesThenNotNullDevicesAreReturned) {

  auto device_count = lzt::get_ze_device_count();

  ASSERT_GT(device_count, 0);

  auto devices = lzt::get_ze_devices(device_count);
  for (auto device : devices) {
    EXPECT_NE(nullptr, device);
  }
}

TEST(zeDeviceGetSubDevicesTest,
     GivenZeroCountWhenRetrievingSubDevicesThenValidCountIsReturned) {
  lzt::get_ze_sub_device_count(lzt::zeDevice::get_instance()->get_device());
}

TEST(zeDeviceGetSubDevicesTest,
     GivenValidCountWhenRetrievingSubDevicesThenNotNullSubDeviceReturned) {

  std::vector<ze_device_handle_t> sub_devices =
      lzt::get_ze_sub_devices(lzt::zeDevice::get_instance()->get_device());

  for (auto sub_device : sub_devices) {
    EXPECT_NE(nullptr, sub_device);
  }
}

TEST(zeDeviceGetDevicePropertiesTests,
     GivenValidDeviceWhenRetrievingPropertiesThenValidPropertiesAreReturned) {

  auto devices = lzt::get_ze_devices();
  for (auto device : devices) {
    auto properties = lzt::get_device_properties(device);
    EXPECT_EQ(ZE_DEVICE_TYPE_GPU, properties.type);
  }
}

TEST(
    zeDeviceGetComputePropertiesTests,
    GivenValidDeviceWhenRetrievingComputePropertiesThenValidPropertiesAreReturned) {

  auto devices = lzt::get_ze_devices();
  ASSERT_GT(devices.size(), 0);
  for (auto device : devices) {
    ze_device_compute_properties_t properties =
        lzt::get_compute_properties(device);

    EXPECT_GT(properties.maxTotalGroupSize, 0);
    EXPECT_GT(properties.maxGroupSizeX, 0);
    EXPECT_GT(properties.maxGroupSizeY, 0);
    EXPECT_GT(properties.maxGroupSizeZ, 0);
    EXPECT_GT(properties.maxGroupCountX, 0);
    EXPECT_GT(properties.maxGroupCountY, 0);
    EXPECT_GT(properties.maxGroupCountZ, 0);
    EXPECT_GT(properties.maxSharedLocalMemory, 0);
    EXPECT_GT(properties.numSubGroupSizes, 0);
    for (uint32_t i = 0; i < properties.numSubGroupSizes; ++i) {
      EXPECT_NE(0, properties.subGroupSizes[i]);
    }
  }
}

TEST(
    zeDeviceGetMemoryPropertiesTests,
    GivenValidCountPointerWhenRetrievingMemoryPropertiesThenValidCountReturned) {
  auto devices = lzt::get_ze_devices();
  ASSERT_GT(devices.size(), 0);
  for (auto device : devices) {
    lzt::get_memory_properties_count(device);
  }
}

TEST(
    zeDeviceGetMemoryPropertiesTests,
    GivenValidDeviceWhenRetrievingMemoryPropertiesThenValidPropertiesAreReturned) {

  auto devices = lzt::get_ze_devices();
  ASSERT_GT(devices.size(), 0);

  for (auto device : devices) {
    uint32_t count = lzt::get_memory_properties_count(device);
    uint32_t count_out = count;

    ASSERT_GT(count, 0) << "no memory properties found";
    std::vector<ze_device_memory_properties_t> properties =
        lzt::get_memory_properties(device);

    for (uint32_t i = 0; i < count_out; ++i) {
      EXPECT_EQ(count_out, count);
      EXPECT_GT(properties[i].maxBusWidth, 0);
      EXPECT_GT(properties[i].totalSize, 0);
    }
  }
}

TEST(
    zeDeviceGetMemoryAccessTests,
    GivenValidDeviceWhenRetrievingMemoryAccessPropertiesThenValidPropertiesReturned) {
  auto devices = lzt::get_ze_devices();

  ASSERT_GT(devices.size(), 0);
  for (auto device : devices) {
    lzt::get_memory_access_properties(device);
  }
}

TEST(
    zeDeviceGetCachePropertiesTests,
    GivenValidDeviceWhenRetrievingCachePropertiesThenValidPropertiesAreReturned) {
  auto devices = lzt::get_ze_devices();

  ASSERT_GT(devices.size(), 0);
  for (auto device : devices) {
    ze_device_cache_properties_t properties = lzt::get_cache_properties(device);
  }
}

TEST(
    zeDeviceGetImagePropertiesTests,
    GivenValidDeviceWhenRetrievingImagePropertiesThenValidPropertiesAreReturned) {
  auto devices = lzt::get_ze_devices();

  ASSERT_GT(devices.size(), 0);
  for (auto device : devices) {
    ze_device_image_properties_t properties = lzt::get_image_properties(device);
    EXPECT_TRUE(properties.supported);
    EXPECT_GT(properties.maxImageDims1D, 0);
    EXPECT_GT(properties.maxImageDims2D, 0);
    EXPECT_GT(properties.maxImageDims3D, 0);
    EXPECT_GT(properties.maxImageArraySlices, 0);
  }
}

TEST(zeDeviceGetP2PPropertiesTests,
     GivenValidDevicesWhenRetrievingP2PThenValidPropertiesAreReturned) {
  auto drivers = lzt::get_all_driver_handles();

  ASSERT_GT(drivers.size(), 0)
      << "no drivers found for peer to peer device test";

  std::vector<ze_device_handle_t> devices;
  for (auto driver : drivers) {
    devices = lzt::get_ze_devices(driver);

    if (devices.size() >= 2)
      break;
  }
  if (lzt::get_ze_device_count() < 2) {
    SUCCEED();
    LOG_INFO << "WARNING:  Exiting as multiple devices do not exist";
    return;
  }

  lzt::get_p2p_properties(devices[0], devices[1]);
}

TEST(zeDeviceCanAccessPeerTests,
     GivenValidDevicesWhenRetrievingCanAccessPropertyThenCapabilityIsReturned) {
  auto drivers = lzt::get_all_driver_handles();
  ASSERT_GT(drivers.size(), 0)
      << "no drivers found for peer to peer device test";

  std::vector<ze_device_handle_t> devices;
  for (auto driver : drivers) {
    devices = lzt::get_ze_devices(driver);

    if (devices.size() >= 1)
      break;
  }

  if (lzt::get_ze_device_count() < 2) {
    SUCCEED();
    LOG_INFO << "WARNING:  Exiting as multiple devices do not exist";
    return;
  }

  ze_bool_t a2b, b2a;
  a2b = lzt::can_access_peer(devices[0], devices[1]);
  b2a = lzt::can_access_peer(devices[1], devices[0]);

  EXPECT_EQ(a2b, b2a);
}

TEST(
    zeDeviceGetKernelPropertiesTests,
    GivenValidDeviceWhenRetrievingKernelPropertiesThenValidPropertiesReturned) {

  auto devices = lzt::get_ze_devices();
  ASSERT_GT(devices.size(), 0);

  for (auto device : devices) {
    auto properties = lzt::get_kernel_properties(device);

    LOG_DEBUG << "SPIR-V version supported "
              << ZE_MAJOR_VERSION(properties.spirvVersionSupported) << "."
              << ZE_MINOR_VERSION(properties.spirvVersionSupported);
    LOG_DEBUG << "nativeKernelSupported: " << properties.nativeKernelSupported;
    LOG_DEBUG << "16-bit Floating Point Supported: "
              << lzt::to_string(properties.fp16Supported);
    LOG_DEBUG << "64-bit Floating Point Supported: "
              << lzt::to_string(properties.fp64Supported);
    LOG_DEBUG << "64-bit Atomics Supported: "
              << lzt::to_string(properties.int64AtomicsSupported);
    LOG_DEBUG << "4 Component Dot Product Supported: "
              << lzt::to_string(properties.dp4aSupported);
    LOG_DEBUG << "Half-Precision FP Capabilities: ";
    LOG_DEBUG << "\t" << properties.halfFpCapabilities;
    LOG_DEBUG << "Single-Precision Capabilities: ";
    LOG_DEBUG << "\t" << properties.singleFpCapabilities;
    LOG_DEBUG << "Double-Precision FP Capabilities: ";
    LOG_DEBUG << "\t" << properties.doubleFpCapabilities;
    LOG_DEBUG << "Max Argument Size: " << properties.maxArgumentsSize;
    LOG_DEBUG << "Print Buffer Size: " << properties.printfBufferSize;
  }
}

class zeSetCacheConfigTests
    : public ::testing::Test,
      public ::testing::WithParamInterface<ze_cache_config_t> {};

TEST_P(zeSetCacheConfigTests,
       GivenConfigFlagWhenSettingIntermediateCacheConfigThenSuccessIsReturned) {
  lzt::set_last_level_cache_config(lzt::zeDevice::get_instance()->get_device(),
                                   GetParam());
}

INSTANTIATE_TEST_CASE_P(SetLastLevelCacheConfigParemeterizedTest,
                        zeSetCacheConfigTests,
                        ::testing::Values(ZE_CACHE_CONFIG_DEFAULT,
                                          ZE_CACHE_CONFIG_LARGE_SLM,
                                          ZE_CACHE_CONFIG_LARGE_DATA));

typedef struct DeviceHandlesBySku_ {
  uint32_t vendorId;
  uint32_t deviceId;
  std::vector<ze_device_handle_t> deviceHandlesForSku;
} DeviceHandlesBySku_t;

// Return false if uuuid's are NOT equal.
bool areDeviceUuidsEqual(ze_device_uuid_t uuid1, ze_device_uuid_t uuid2) {
  if (std::memcmp(&uuid1, &uuid2, sizeof(ze_device_uuid_t))) {
    return false;
  }
  return true;
}

class DevicePropertiesTest : public ::testing::Test {
public:
  std::vector<DeviceHandlesBySku_t *> deviceHandlesAllSkus;

  DeviceHandlesBySku_t *findDeviceHandlesBySku(uint32_t vendorId,
                                               uint32_t deviceId);
  void addDeviceHandleBySku(uint32_t vendorId, uint32_t deviceId,
                            ze_device_handle_t handle);
  void populateDevicesBySku();
  void freeDevicesBySku();

  DevicePropertiesTest() { populateDevicesBySku(); }

  ~DevicePropertiesTest() { freeDevicesBySku(); }
};

DeviceHandlesBySku_t *
DevicePropertiesTest::findDeviceHandlesBySku(uint32_t vendorId,
                                             uint32_t deviceId) {
  DeviceHandlesBySku_t *foundSkuHandles = nullptr;

  for (DeviceHandlesBySku_t *iterSkuHandles :
       DevicePropertiesTest::deviceHandlesAllSkus) {
    if ((iterSkuHandles->vendorId == vendorId) &&
        (iterSkuHandles->deviceId == deviceId)) {
      foundSkuHandles = iterSkuHandles;
      break;
    }
  }
  return foundSkuHandles;
}

void DevicePropertiesTest::addDeviceHandleBySku(uint32_t vendorId,
                                                uint32_t deviceId,
                                                ze_device_handle_t handle) {
  DeviceHandlesBySku_t *skuHandles = findDeviceHandlesBySku(vendorId, deviceId);
  if (skuHandles == nullptr) {
    skuHandles = new DeviceHandlesBySku_t;
    skuHandles->vendorId = vendorId;
    skuHandles->deviceId = deviceId;
    DevicePropertiesTest::deviceHandlesAllSkus.push_back(skuHandles);
  }
  skuHandles->deviceHandlesForSku.push_back(handle);
}

void DevicePropertiesTest::populateDevicesBySku() {
  for (ze_device_handle_t deviceHandle : lzt::get_ze_devices()) {
    ze_device_properties_t deviceProperties =
        lzt::get_device_properties(deviceHandle);
    if (deviceProperties.type == ZE_DEVICE_TYPE_GPU) {
      addDeviceHandleBySku(deviceProperties.vendorId, deviceProperties.deviceId,
                           deviceHandle);
    }
  }
}

void DevicePropertiesTest::freeDevicesBySku() {
  for (DeviceHandlesBySku_t *iterSkuHandles :
       DevicePropertiesTest::deviceHandlesAllSkus) {
    delete iterSkuHandles;
  }
}

TEST_F(DevicePropertiesTest,
       GivenMultipleRootDevicesWhenSKUsMatcheThenDevicePropertiesMatch) {
  if (lzt::get_ze_device_count() < 2) {
    SUCCEED();
    LOG_INFO << "WARNING:  Exiting as multiple devices do not exist";
    return;
  }
  for (DeviceHandlesBySku_t *iterSkuHandles :
       DevicePropertiesTest::deviceHandlesAllSkus) {

    ze_device_handle_t firstDeviceHandle =
        iterSkuHandles->deviceHandlesForSku.front();
    ze_device_properties_t firstDeviceProperties =
        lzt::get_device_properties(firstDeviceHandle);
    EXPECT_FALSE(firstDeviceProperties.isSubdevice);
    EXPECT_GT(firstDeviceProperties.maxCommandQueues, 0);
    EXPECT_GT(firstDeviceProperties.numAsyncComputeEngines, 0);
    EXPECT_GT(firstDeviceProperties.numAsyncCopyEngines, 0);

    bool first_iteration = true;
    for (ze_device_handle_t iterDeviceHandle :
         iterSkuHandles->deviceHandlesForSku) {
      ze_device_properties_t iterDeviceProperties =
          lzt::get_device_properties(iterDeviceHandle);

      EXPECT_EQ(firstDeviceProperties.type, iterDeviceProperties.type);
      EXPECT_EQ(firstDeviceProperties.vendorId, iterDeviceProperties.vendorId);
      EXPECT_EQ(firstDeviceProperties.deviceId, iterDeviceProperties.deviceId);
      if (first_iteration) {
        EXPECT_TRUE(areDeviceUuidsEqual(firstDeviceProperties.uuid,
                                        iterDeviceProperties.uuid));
        first_iteration = false;
      } else {
        EXPECT_FALSE(areDeviceUuidsEqual(firstDeviceProperties.uuid,
                                         iterDeviceProperties.uuid));
      }
      EXPECT_FALSE(iterDeviceProperties.isSubdevice);
      EXPECT_EQ(firstDeviceProperties.coreClockRate,
                iterDeviceProperties.coreClockRate);
      EXPECT_EQ(firstDeviceProperties.unifiedMemorySupported,
                iterDeviceProperties.unifiedMemorySupported);
      EXPECT_EQ(firstDeviceProperties.onDemandPageFaultsSupported,
                iterDeviceProperties.onDemandPageFaultsSupported);
      EXPECT_EQ(firstDeviceProperties.maxCommandQueues,
                iterDeviceProperties.maxCommandQueues);
      EXPECT_EQ(firstDeviceProperties.numAsyncComputeEngines,
                iterDeviceProperties.numAsyncComputeEngines);
      EXPECT_EQ(firstDeviceProperties.numAsyncCopyEngines,
                iterDeviceProperties.numAsyncCopyEngines);
      EXPECT_EQ(firstDeviceProperties.maxCommandQueuePriority,
                iterDeviceProperties.maxCommandQueuePriority);
      EXPECT_EQ(firstDeviceProperties.numThreadsPerEU,
                iterDeviceProperties.numThreadsPerEU);
      EXPECT_EQ(firstDeviceProperties.physicalEUSimdWidth,
                iterDeviceProperties.physicalEUSimdWidth);
      EXPECT_EQ(firstDeviceProperties.numEUsPerSubslice,
                iterDeviceProperties.numEUsPerSubslice);
      EXPECT_EQ(firstDeviceProperties.numSubslicesPerSlice,
                iterDeviceProperties.numSubslicesPerSlice);
      EXPECT_EQ(firstDeviceProperties.numSlices,
                iterDeviceProperties.numSlices);
    }
  }
}

TEST_F(DevicePropertiesTest,
       GivenMultipleRootDevicesWhenSKUsMatchThenComputePropertiesMatch) {
  if (lzt::get_ze_device_count() < 2) {
    SUCCEED();
    LOG_INFO << "WARNING:  Exiting as multiple devices do not exist";
    return;
  }
  for (DeviceHandlesBySku_t *iterSkuHandles :
       DevicePropertiesTest::deviceHandlesAllSkus) {

    ze_device_handle_t firstDeviceHandle =
        iterSkuHandles->deviceHandlesForSku.front();
    ze_device_compute_properties_t firstDeviceProperties =
        lzt::get_compute_properties(firstDeviceHandle);

    EXPECT_GT(firstDeviceProperties.maxTotalGroupSize, 0);
    EXPECT_GT(firstDeviceProperties.maxGroupSizeX, 0);
    EXPECT_GT(firstDeviceProperties.maxGroupSizeY, 0);
    EXPECT_GT(firstDeviceProperties.maxGroupSizeZ, 0);
    EXPECT_GT(firstDeviceProperties.maxGroupCountX, 0);
    EXPECT_GT(firstDeviceProperties.maxGroupCountY, 0);
    EXPECT_GT(firstDeviceProperties.maxGroupCountZ, 0);
    EXPECT_GT(firstDeviceProperties.maxSharedLocalMemory, 0);
    EXPECT_GT(firstDeviceProperties.numSubGroupSizes, 0);
    for (uint32_t i = 0; i < firstDeviceProperties.numSubGroupSizes; ++i) {
      EXPECT_NE(0, firstDeviceProperties.subGroupSizes[i]);
    }

    for (ze_device_handle_t iterDeviceHandle :
         iterSkuHandles->deviceHandlesForSku) {
      ze_device_compute_properties_t iterDeviceProperties =
          lzt::get_compute_properties(iterDeviceHandle);

      EXPECT_EQ(firstDeviceProperties.maxTotalGroupSize,
                iterDeviceProperties.maxTotalGroupSize);
      EXPECT_EQ(firstDeviceProperties.maxGroupSizeX,
                iterDeviceProperties.maxGroupSizeX);
      EXPECT_EQ(firstDeviceProperties.maxGroupSizeY,
                iterDeviceProperties.maxGroupSizeY);
      EXPECT_EQ(firstDeviceProperties.maxGroupSizeZ,
                iterDeviceProperties.maxGroupSizeZ);
      EXPECT_EQ(firstDeviceProperties.maxGroupCountX,
                iterDeviceProperties.maxGroupCountX);
      EXPECT_EQ(firstDeviceProperties.maxGroupCountY,
                iterDeviceProperties.maxGroupCountY);
      EXPECT_EQ(firstDeviceProperties.maxGroupCountZ,
                iterDeviceProperties.maxGroupCountZ);
      EXPECT_EQ(firstDeviceProperties.maxSharedLocalMemory,
                iterDeviceProperties.maxSharedLocalMemory);
      EXPECT_EQ(firstDeviceProperties.numSubGroupSizes,
                iterDeviceProperties.numSubGroupSizes);
      for (uint32_t i = 0; i < firstDeviceProperties.numSubGroupSizes; ++i) {
        EXPECT_EQ(firstDeviceProperties.subGroupSizes[i],
                  iterDeviceProperties.subGroupSizes[i]);
        ;
      }
    }
  }
}

TEST_F(DevicePropertiesTest,
       GivenMultipleRootDevicesWhenSKUsMatchThenMemoryPropertiesMatch) {
  if (lzt::get_ze_device_count() < 2) {
    SUCCEED();
    LOG_INFO << "WARNING:  Exiting as multiple devices do not exist";
    return;
  }
  for (DeviceHandlesBySku_t *iterSkuHandles :
       DevicePropertiesTest::deviceHandlesAllSkus) {

    ze_device_handle_t firstDeviceHandle =
        iterSkuHandles->deviceHandlesForSku.front();
    uint32_t firstDevicePropertiesCount =
        lzt::get_memory_properties_count(firstDeviceHandle);
    std::vector<ze_device_memory_properties_t> firstDeviceProperties =
        lzt::get_memory_properties(firstDeviceHandle);

    EXPECT_EQ(firstDevicePropertiesCount, firstDeviceProperties.size());

    for (uint32_t i = 0; i < firstDevicePropertiesCount; i++) {
      EXPECT_GT(firstDeviceProperties[i].maxClockRate, 0);
      EXPECT_GT(firstDeviceProperties[i].maxBusWidth, 0);
      EXPECT_GT(firstDeviceProperties[i].totalSize, 0);
    }

    for (ze_device_handle_t iterDeviceHandle :
         iterSkuHandles->deviceHandlesForSku) {
      uint32_t iterDevicePropertiesCount =
          lzt::get_memory_properties_count(iterDeviceHandle);
      std::vector<ze_device_memory_properties_t> iterDeviceProperties =
          lzt::get_memory_properties(iterDeviceHandle);

      EXPECT_EQ(firstDevicePropertiesCount, iterDevicePropertiesCount);

      for (uint32_t i = 0; i < firstDevicePropertiesCount; i++) {
        EXPECT_EQ(iterDeviceProperties[i].maxClockRate,
                  firstDeviceProperties[i].maxClockRate);
        EXPECT_EQ(iterDeviceProperties[i].maxBusWidth,
                  firstDeviceProperties[i].maxBusWidth);
        EXPECT_EQ(iterDeviceProperties[i].totalSize,
                  firstDeviceProperties[i].totalSize);
      }
    }
  }
}

TEST_F(DevicePropertiesTest,
       GivenMultipleRootDevicesWhenSKUsMatchThenMemoryAccessPropertiesMatch) {
  if (lzt::get_ze_device_count() < 2) {
    SUCCEED();
    LOG_INFO << "WARNING:  Exiting as multiple devices do not exist";
    return;
  }
  for (DeviceHandlesBySku_t *iterSkuHandles :
       DevicePropertiesTest::deviceHandlesAllSkus) {

    ze_device_handle_t firstDeviceHandle =
        iterSkuHandles->deviceHandlesForSku.front();
    ze_device_memory_access_properties_t firstDeviceProperties =
        lzt::get_memory_access_properties(firstDeviceHandle);

    for (ze_device_handle_t iterDeviceHandle :
         iterSkuHandles->deviceHandlesForSku) {
      ze_device_memory_access_properties_t iterDeviceProperties =
          lzt::get_memory_access_properties(iterDeviceHandle);

      EXPECT_EQ(iterDeviceProperties.hostAllocCapabilities,
                firstDeviceProperties.hostAllocCapabilities);
      EXPECT_EQ(iterDeviceProperties.deviceAllocCapabilities,
                firstDeviceProperties.deviceAllocCapabilities);
      EXPECT_EQ(iterDeviceProperties.sharedSingleDeviceAllocCapabilities,
                firstDeviceProperties.sharedSingleDeviceAllocCapabilities);
      EXPECT_EQ(iterDeviceProperties.sharedCrossDeviceAllocCapabilities,
                firstDeviceProperties.sharedCrossDeviceAllocCapabilities);
      EXPECT_EQ(iterDeviceProperties.sharedSystemAllocCapabilities,
                firstDeviceProperties.sharedSystemAllocCapabilities);
    }
  }
}

TEST_F(DevicePropertiesTest,
       GivenMultipleRootDevicesWhenSKUsMatchThenCachePropertiesMatch) {
  if (lzt::get_ze_device_count() < 2) {
    SUCCEED();
    LOG_INFO << "WARNING:  Exiting as multiple devices do not exist";
    return;
  }
  for (DeviceHandlesBySku_t *iterSkuHandles :
       DevicePropertiesTest::deviceHandlesAllSkus) {

    ze_device_handle_t firstDeviceHandle =
        iterSkuHandles->deviceHandlesForSku.front();
    ze_device_cache_properties_t firstDeviceProperties =
        lzt::get_cache_properties(firstDeviceHandle);

    for (ze_device_handle_t iterDeviceHandle :
         iterSkuHandles->deviceHandlesForSku) {
      ze_device_cache_properties_t iterDeviceProperties =
          lzt::get_cache_properties(iterDeviceHandle);

      EXPECT_EQ(iterDeviceProperties.intermediateCacheControlSupported,
                firstDeviceProperties.intermediateCacheControlSupported);
      EXPECT_EQ(iterDeviceProperties.intermediateCacheSize,
                firstDeviceProperties.intermediateCacheSize);
      EXPECT_EQ(iterDeviceProperties.intermediateCachelineSize,
                firstDeviceProperties.intermediateCachelineSize);
      EXPECT_EQ(iterDeviceProperties.lastLevelCacheSizeControlSupported,
                firstDeviceProperties.lastLevelCacheSizeControlSupported);
      EXPECT_EQ(iterDeviceProperties.lastLevelCacheSize,
                firstDeviceProperties.lastLevelCacheSize);
      EXPECT_EQ(iterDeviceProperties.lastLevelCachelineSize,
                firstDeviceProperties.lastLevelCachelineSize);
    }
  }
}

TEST_F(DevicePropertiesTest,
       GivenMultipleRootDevicesWhenSKUsMatchThenPeerAccessPropertiesMatch) {
  if (lzt::get_ze_device_count() < 2) {
    SUCCEED();
    LOG_INFO << "WARNING:  Exiting as multiple devices do not exist";
    return;
  }
  for (DeviceHandlesBySku_t *iterSkuHandles :
       DevicePropertiesTest::deviceHandlesAllSkus) {

    ze_device_handle_t firstDeviceHandle =
        iterSkuHandles->deviceHandlesForSku.front();

    EXPECT_TRUE(lzt::can_access_peer(firstDeviceHandle, firstDeviceHandle));

    for (ze_device_handle_t iterDeviceHandle :
         iterSkuHandles->deviceHandlesForSku) {
      ze_device_p2p_properties_t iterDeviceProperties =
          lzt::get_p2p_properties(firstDeviceHandle, iterDeviceHandle);
      ze_bool_t a2b, b2a;
      a2b = lzt::can_access_peer(firstDeviceHandle, iterDeviceHandle);
      b2a = lzt::can_access_peer(iterDeviceHandle, firstDeviceHandle);
      EXPECT_EQ(a2b, b2a);
    }
  }
}

TEST_F(DevicePropertiesTest,
       GivenMultipleRootDevicesWhenSKUsMatchThenSubDeviceCountsMatch) {
  if (lzt::get_ze_device_count() < 2) {
    SUCCEED();
    LOG_INFO << "WARNING:  Exiting as multiple devices do not exist";
    return;
  }
  for (DeviceHandlesBySku_t *iterSkuHandles :
       DevicePropertiesTest::deviceHandlesAllSkus) {

    ze_device_handle_t firstDeviceHandle =
        iterSkuHandles->deviceHandlesForSku.front();
    uint32_t firstDeviceSubDeviceCount =
        lzt::get_ze_sub_device_count(firstDeviceHandle);

    for (ze_device_handle_t iterDeviceHandle :
         iterSkuHandles->deviceHandlesForSku) {
      uint32_t iterDeviceSubDeviceCount =
          lzt::get_ze_sub_device_count(iterDeviceHandle);
      EXPECT_EQ(iterDeviceSubDeviceCount, firstDeviceSubDeviceCount);
    }
  }
}

// Return false if uuuid's are NOT equal.
bool areNativeKernelUuidsEqual(ze_native_kernel_uuid_t *uuid1,
                               ze_native_kernel_uuid_t *uuid2) {
  if (std::memcmp(uuid1->id, uuid2->id, ZE_MAX_NATIVE_KERNEL_UUID_SIZE)) {
    return false;
  }
  return true;
}

TEST_F(DevicePropertiesTest,
       GivenMultipleRootDevicesWhenSKUsMatchThenKernelPropertiesMatch) {
  if (lzt::get_ze_device_count() < 2) {
    SUCCEED();
    LOG_INFO << "WARNING:  Exiting as multiple devices do not exist";
    return;
  }
  for (DeviceHandlesBySku_t *iterSkuHandles :
       DevicePropertiesTest::deviceHandlesAllSkus) {

    ze_device_handle_t firstDeviceHandle =
        iterSkuHandles->deviceHandlesForSku.front();
    ze_device_kernel_properties_t firstDeviceProperties =
        lzt::get_kernel_properties(firstDeviceHandle);

    for (ze_device_handle_t iterDeviceHandle :
         iterSkuHandles->deviceHandlesForSku) {
      ze_device_kernel_properties_t iterDeviceProperties =
          lzt::get_kernel_properties(iterDeviceHandle);

      EXPECT_EQ(iterDeviceProperties.spirvVersionSupported,
                firstDeviceProperties.spirvVersionSupported);

      EXPECT_TRUE(areNativeKernelUuidsEqual(
          &iterDeviceProperties.nativeKernelSupported,
          &firstDeviceProperties.nativeKernelSupported));

      EXPECT_EQ(iterDeviceProperties.fp16Supported,
                firstDeviceProperties.fp16Supported);

      EXPECT_EQ(iterDeviceProperties.fp64Supported,
                firstDeviceProperties.fp64Supported);

      EXPECT_EQ(iterDeviceProperties.int64AtomicsSupported,
                firstDeviceProperties.int64AtomicsSupported);

      EXPECT_EQ(iterDeviceProperties.int64AtomicsSupported,
                firstDeviceProperties.int64AtomicsSupported);

      EXPECT_EQ(iterDeviceProperties.dp4aSupported,
                firstDeviceProperties.dp4aSupported);

      EXPECT_EQ(iterDeviceProperties.halfFpCapabilities,
                firstDeviceProperties.halfFpCapabilities);

      EXPECT_EQ(iterDeviceProperties.singleFpCapabilities,
                firstDeviceProperties.singleFpCapabilities);

      EXPECT_EQ(iterDeviceProperties.doubleFpCapabilities,
                firstDeviceProperties.doubleFpCapabilities);

      EXPECT_EQ(iterDeviceProperties.maxArgumentsSize,
                firstDeviceProperties.maxArgumentsSize);

      EXPECT_EQ(iterDeviceProperties.printfBufferSize,
                firstDeviceProperties.printfBufferSize);
    }
  }
}
} // namespace
