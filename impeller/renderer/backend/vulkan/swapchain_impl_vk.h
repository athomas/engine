// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <memory>
#include <variant>

#include "flutter/fml/macros.h"
#include "impeller/renderer/backend/vulkan/vk.h"
#include "vulkan/vulkan_enums.hpp"

namespace impeller {

class Context;
class SwapchainImageVK;
class Surface;
struct FrameSynchronizer;

//------------------------------------------------------------------------------
/// @brief      An instance of a swapchain that does NOT adapt to going out of
///             date with the underlying surface. Errors will be indicated when
///             the next drawable is acquired from this implementation of the
///             swapchain. If the error is due the swapchain going out of date,
///             the caller must recreate another instance by optionally
///             stealing this implementations guts.
///
class SwapchainImplVK final
    : public std::enable_shared_from_this<SwapchainImplVK> {
 public:
  static std::shared_ptr<SwapchainImplVK> Create(
      const std::shared_ptr<Context>& context,
      vk::UniqueSurfaceKHR surface,
      vk::SwapchainKHR old_swapchain = VK_NULL_HANDLE,
      vk::SurfaceTransformFlagBitsKHR last_transform =
          vk::SurfaceTransformFlagBitsKHR::eIdentity);

  ~SwapchainImplVK();

  bool IsValid() const;

  struct AcquireResult {
    std::unique_ptr<Surface> surface;
    bool out_of_date = false;

    AcquireResult(bool p_out_of_date = false) : out_of_date(p_out_of_date) {}

    AcquireResult(std::unique_ptr<Surface> p_surface)
        : surface(std::move(p_surface)) {}
  };

  AcquireResult AcquireNextDrawable();

  vk::Format GetSurfaceFormat() const;

  vk::SurfaceTransformFlagBitsKHR GetLastTransform() const;

  std::shared_ptr<Context> GetContext() const;

  std::pair<vk::UniqueSurfaceKHR, vk::UniqueSwapchainKHR> DestroySwapchain();

 private:
  std::weak_ptr<Context> context_;
  vk::UniqueSurfaceKHR surface_;
  vk::Queue present_queue_ = {};
  vk::Format surface_format_ = vk::Format::eUndefined;
  vk::UniqueSwapchainKHR swapchain_;
  std::vector<std::shared_ptr<SwapchainImageVK>> images_;
  std::vector<std::unique_ptr<FrameSynchronizer>> synchronizers_;
  size_t current_frame_ = 0u;
  bool is_valid_ = false;
  size_t current_transform_poll_count_ = 0u;
  vk::SurfaceTransformFlagBitsKHR transform_if_changed_discard_swapchain_;

  SwapchainImplVK(const std::shared_ptr<Context>& context,
                  vk::UniqueSurfaceKHR surface,
                  vk::SwapchainKHR old_swapchain,
                  vk::SurfaceTransformFlagBitsKHR last_transform);

  bool Present(const std::shared_ptr<SwapchainImageVK>& image, uint32_t index);

  void WaitIdle() const;

  SwapchainImplVK(const SwapchainImplVK&) = delete;

  SwapchainImplVK& operator=(const SwapchainImplVK&) = delete;
};

}  // namespace impeller
