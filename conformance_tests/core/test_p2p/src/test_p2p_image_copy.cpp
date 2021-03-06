/*
 *
 * Copyright (C) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"

#include "test_harness/test_harness.hpp"
#include "logging/logging.hpp"

namespace lzt = level_zero_tests;

#include <level_zero/ze_api.h>

namespace {

class P2PImageCopy : public ::testing::Test {
protected:
  void SetUp() override {

    auto devices = lzt::get_ze_devices(lzt::get_default_driver());
    if (devices.size() < 2) {
      LOG_INFO << "WARNING:  Less than 2 devices found, cannot run test";
      SUCCEED();
      skip = true;
      return;
    }
    if (!lzt::can_access_peer(devices[0], devices[1])) {
      LOG_INFO
          << "dev0 and dev1 fail zeDeviceCanAccessPeer check, cannot run test";
      SUCCEED();
      skip = true;
      return;
    }

    dev0 = devices[0];
    dev1 = devices[1];
    auto img_prop = lzt::get_image_properties(dev0);
    if (img_prop.supported == false) {
      LOG_INFO << "device does not support images, cannot run test";
      SUCCEED();
      skip = true;
      return;
    }

    img_prop = lzt::get_image_properties(dev1);
    if (img_prop.supported == false) {
      LOG_INFO << "device does not support images, cannot run test";
      SUCCEED();
      skip = true;
      return;
    }

    input_png = lzt::ImagePNG32Bit("test_input.png");
    img_width = input_png.width();
    img_height = input_png.height();
    output_png = lzt::ImagePNG32Bit(img_width, img_height);

    ze_image_desc_t img_desc = {
        ZE_IMAGE_DESC_VERSION_CURRENT,
        ZE_IMAGE_FLAG_PROGRAM_READ,
        ZE_IMAGE_TYPE_2D,
        {ZE_IMAGE_FORMAT_LAYOUT_8_8_8_8, ZE_IMAGE_FORMAT_TYPE_UNORM,
         ZE_IMAGE_FORMAT_SWIZZLE_R, ZE_IMAGE_FORMAT_SWIZZLE_G,
         ZE_IMAGE_FORMAT_SWIZZLE_B, ZE_IMAGE_FORMAT_SWIZZLE_A},
        img_width,
        img_height,
        1,
        0,
        0};

    lzt::create_ze_image(dev0, img_dev0, &img_desc);
    lzt::create_ze_image(dev1, img_dev1, &img_desc);

    command_list_dev0 = lzt::create_command_list(dev0);
    command_list_dev1 = lzt::create_command_list(dev1);
    command_q_dev0 = lzt::create_command_queue();
    command_q_dev1 = lzt::create_command_queue();

    ze_event_pool_desc_t ep_desc = {ZE_EVENT_POOL_DESC_VERSION_CURRENT,
                                    ZE_EVENT_POOL_FLAG_DEFAULT, 10};
    ep = lzt::create_event_pool(ep_desc, devices);
  }

  void TearDown() override {
    if (skip)
      return;
    lzt::destroy_ze_image(img_dev0);
    lzt::destroy_ze_image(img_dev1);
    lzt::destroy_command_list(command_list_dev0);
    lzt::destroy_command_list(command_list_dev1);
    lzt::destroy_command_queue(command_q_dev0);
    lzt::destroy_command_queue(command_q_dev1);
    lzt::destroy_event_pool(ep);
  }

  lzt::ImagePNG32Bit input_png;
  lzt::ImagePNG32Bit output_png;
  uint32_t img_width;
  uint32_t img_height;
  ze_device_handle_t dev0, dev1;
  ze_image_handle_t img_dev0, img_dev1;
  ze_command_list_handle_t command_list_dev0;
  ze_command_list_handle_t command_list_dev1;
  ze_command_queue_handle_t command_q_dev0;
  ze_command_queue_handle_t command_q_dev1;
  ze_event_pool_handle_t ep;
  bool skip = false;
};

TEST_F(P2PImageCopy,
       GivenImageOnDeviceWhenCopiedToOtherDeviceThenResultIsCorrect) {
  if (skip)
    return;

  ze_event_desc_t event_desc = {ZE_EVENT_DESC_VERSION_CURRENT, 0,
                                ZE_EVENT_SCOPE_FLAG_HOST};
  auto event1 = lzt::create_event(ep, event_desc);
  event_desc = {ZE_EVENT_DESC_VERSION_CURRENT, 1, ZE_EVENT_SCOPE_FLAG_HOST};
  auto event2 = lzt::create_event(ep, event_desc);

  // Load image to dev0
  lzt::append_image_copy_from_mem(command_list_dev0, img_dev0,
                                  input_png.raw_data(), event1);
  lzt::append_wait_on_events(command_list_dev0, 1, &event1);

  // copy to dev 1
  lzt::append_image_copy(command_list_dev0, img_dev1, img_dev0, event2);
  lzt::append_wait_on_events(command_list_dev0, 1, &event2);
  lzt::close_command_list(command_list_dev0);
  lzt::execute_command_lists(command_q_dev0, 1, &command_list_dev0, nullptr);
  lzt::synchronize(command_q_dev0, UINT32_MAX);

  // Copyback to host
  lzt::append_image_copy_to_mem(command_list_dev1, output_png.raw_data(),
                                img_dev1, nullptr);
  lzt::close_command_list(command_list_dev1);
  lzt::execute_command_lists(command_q_dev1, 1, &command_list_dev1, nullptr);
  lzt::synchronize(command_q_dev1, UINT32_MAX);

  // compare results
  EXPECT_EQ(input_png, output_png);

  lzt::destroy_event(event1);
  lzt::destroy_event(event2);
}

TEST_F(P2PImageCopy,
       GivenImageOnDeviceWhenRegionCopiedToOtherDeviceThenResultIsCorrect) {
  if (skip)
    return;

  ze_event_desc_t event_desc = {ZE_EVENT_DESC_VERSION_CURRENT, 0,
                                ZE_EVENT_SCOPE_FLAG_HOST};
  auto event1 = lzt::create_event(ep, event_desc);
  event_desc = {ZE_EVENT_DESC_VERSION_CURRENT, 1, ZE_EVENT_SCOPE_FLAG_HOST};
  auto event2 = lzt::create_event(ep, event_desc);
  event_desc = {ZE_EVENT_DESC_VERSION_CURRENT, 2, ZE_EVENT_SCOPE_FLAG_HOST};
  auto event3 = lzt::create_event(ep, event_desc);

  // Load image to dev0
  lzt::append_image_copy_from_mem(command_list_dev0, img_dev0,
                                  input_png.raw_data(), event1);
  lzt::append_wait_on_events(command_list_dev0, 1, &event1);

  // copy to dev 1
  ze_image_region_t region = {0, 0, 0, img_width / 2, img_height / 2, 1};
  lzt::append_image_copy_region(command_list_dev0, img_dev1, img_dev0, &region,
                                &region, event2);
  lzt::append_wait_on_events(command_list_dev0, 1, &event2);
  region = {img_width / 2, img_height / 2, 0, img_width, img_height, 1};
  lzt::append_image_copy_region(command_list_dev0, img_dev1, img_dev0, &region,
                                &region, event3);
  lzt::append_wait_on_events(command_list_dev0, 1, &event3);

  lzt::close_command_list(command_list_dev0);
  lzt::execute_command_lists(command_q_dev0, 1, &command_list_dev0, nullptr);
  lzt::synchronize(command_q_dev0, UINT32_MAX);

  // Copyback to host
  lzt::append_image_copy_to_mem(command_list_dev1, output_png.raw_data(),
                                img_dev1, nullptr);
  lzt::close_command_list(command_list_dev1);
  lzt::execute_command_lists(command_q_dev1, 1, &command_list_dev1, nullptr);
  lzt::synchronize(command_q_dev1, UINT32_MAX);

  // compare results
  EXPECT_EQ(input_png, output_png);

  lzt::destroy_event(event1);
  lzt::destroy_event(event2);
  lzt::destroy_event(event3);
}

class P2PImageCopyMemory
    : public P2PImageCopy,
      public ::testing::WithParamInterface<ze_memory_type_t> {};

TEST_P(P2PImageCopyMemory,
       GivenImageOnDeviceWhenCopiedToMemoryOnOtherDeviceThenResultIsCorrect) {
  if (skip)
    return;

  ze_event_desc_t event_desc = {ZE_EVENT_DESC_VERSION_CURRENT, 0,
                                ZE_EVENT_SCOPE_FLAG_HOST};
  auto event1 = lzt::create_event(ep, event_desc);
  event_desc = {ZE_EVENT_DESC_VERSION_CURRENT, 1, ZE_EVENT_SCOPE_FLAG_HOST};
  auto event2 = lzt::create_event(ep, event_desc);

  void *target_mem;
  size_t mem_size = img_height * img_width * sizeof(uint32_t);
  if (GetParam() == ZE_MEMORY_TYPE_DEVICE) {
    ze_device_mem_alloc_flag_t d_flags = ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT;
    ze_host_mem_alloc_flag_t h_flags = ZE_HOST_MEM_ALLOC_FLAG_DEFAULT;
    target_mem = lzt::allocate_device_memory(mem_size, 1, d_flags, dev1,
                                             lzt::get_default_driver());
  } else {
    ze_device_mem_alloc_flag_t d_flags = ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT;
    ze_host_mem_alloc_flag_t h_flags = ZE_HOST_MEM_ALLOC_FLAG_DEFAULT;
    target_mem =
        lzt::allocate_shared_memory(mem_size, 1, d_flags, h_flags, dev1);
  }

  // Load image to dev0
  lzt::append_image_copy_from_mem(command_list_dev0, img_dev0,
                                  input_png.raw_data(), event1);
  lzt::append_wait_on_events(command_list_dev0, 1, &event1);

  // copy to dev 1 memory
  lzt::append_image_copy_to_mem(command_list_dev0, target_mem, img_dev0,
                                event2);
  lzt::append_wait_on_events(command_list_dev0, 1, &event2);
  lzt::close_command_list(command_list_dev0);
  lzt::execute_command_lists(command_q_dev0, 1, &command_list_dev0, nullptr);
  lzt::synchronize(command_q_dev0, UINT32_MAX);

  // Copyback to host
  lzt::append_memory_copy(command_list_dev1, output_png.raw_data(), target_mem,
                          mem_size);
  lzt::close_command_list(command_list_dev1);
  lzt::execute_command_lists(command_q_dev1, 1, &command_list_dev1, nullptr);
  lzt::synchronize(command_q_dev1, UINT32_MAX);

  // compare results
  EXPECT_EQ(input_png, output_png);

  lzt::destroy_event(event1);
  lzt::destroy_event(event2);
  lzt::free_memory(target_mem);
}

TEST_P(P2PImageCopyMemory,
       GivenImageOnDeviceWhenCopiedFromMemoryOnOtherDeviceThenResultIsCorrect) {
  if (skip)
    return;

  ze_event_desc_t event_desc = {ZE_EVENT_DESC_VERSION_CURRENT, 0,
                                ZE_EVENT_SCOPE_FLAG_HOST};
  auto event1 = lzt::create_event(ep, event_desc);
  event_desc = {ZE_EVENT_DESC_VERSION_CURRENT, 1, ZE_EVENT_SCOPE_FLAG_HOST};
  auto event2 = lzt::create_event(ep, event_desc);

  void *target_mem;
  size_t mem_size = img_height * img_width * sizeof(uint32_t);
  if (GetParam() == ZE_MEMORY_TYPE_DEVICE) {
    ze_device_mem_alloc_flag_t d_flags = ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT;
    ze_host_mem_alloc_flag_t h_flags = ZE_HOST_MEM_ALLOC_FLAG_DEFAULT;
    target_mem = lzt::allocate_device_memory(mem_size, 1, d_flags, dev1,
                                             lzt::get_default_driver());
  } else {
    ze_device_mem_alloc_flag_t d_flags = ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT;
    ze_host_mem_alloc_flag_t h_flags = ZE_HOST_MEM_ALLOC_FLAG_DEFAULT;
    target_mem =
        lzt::allocate_shared_memory(mem_size, 1, d_flags, h_flags, dev1);
  }

  // Load image to dev0
  lzt::append_image_copy_from_mem(command_list_dev0, img_dev0,
                                  input_png.raw_data(), event1);
  lzt::append_wait_on_events(command_list_dev0, 1, &event1);
  lzt::close_command_list(command_list_dev0);
  lzt::execute_command_lists(command_q_dev0, 1, &command_list_dev0, nullptr);
  lzt::synchronize(command_q_dev0, UINT32_MAX);

  // on dev1, copy from dev 0
  lzt::append_image_copy_from_mem(command_list_dev1, img_dev0, target_mem,
                                  event2);
  lzt::append_wait_on_events(command_list_dev0, 1, &event2);

  // Copyback to host
  lzt::append_memory_copy(command_list_dev1, output_png.raw_data(), target_mem,
                          mem_size);
  lzt::close_command_list(command_list_dev1);
  lzt::execute_command_lists(command_q_dev1, 1, &command_list_dev1, nullptr);
  lzt::synchronize(command_q_dev1, UINT32_MAX);

  // compare results
  EXPECT_EQ(input_png, output_png);

  lzt::destroy_event(event1);
  lzt::destroy_event(event2);
  lzt::free_memory(target_mem);
}

INSTANTIATE_TEST_CASE_P(P2PImageMemory, P2PImageCopyMemory,
                        testing::Values(ZE_MEMORY_TYPE_DEVICE,
                                        ZE_MEMORY_TYPE_SHARED));

} // namespace
