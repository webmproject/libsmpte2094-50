/**
 * Copyright 2026 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "smpte2094_50/pchip.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "pchip_ffi.h"

namespace smpte2094_50 {

absl::StatusOr<std::vector<float>> ComputePchipSlopes(
    absl::Span<const float> x, absl::Span<const float> y) {
  const ::pchip_rs::PchipSlopesResult result = pchip_rs::pchip_slopes_ffi(x, y);
  if (!result.success) {
    return absl::InvalidArgumentError(result.get_error_message());
  }
  absl::Span<const float> slopes_span = result.get_slopes();
  return std::vector<float>(slopes_span.begin(), slopes_span.end());
}

struct PchipInterpolator::Impl {
  pchip_rs::PchipInterpolator rust_interp;
};

absl::StatusOr<PchipInterpolator> PchipInterpolator::Create(
    absl::Span<const float> x, absl::Span<const float> y) {
  ::pchip_rs::PchipInterpolatorResult result =
      pchip_rs::PchipInterpolator::create_ffi(x, y);
  if (!result.success) {
    return absl::InvalidArgumentError(result.get_error_message());
  }
  return PchipInterpolator(
      std::make_unique<Impl>(Impl{std::move(result.interp)}));
}

PchipInterpolator::PchipInterpolator(std::unique_ptr<Impl> impl)
    : impl_(std::move(impl)) {}

PchipInterpolator::PchipInterpolator(const PchipInterpolator& other)
    : impl_(std::make_unique<Impl>(Impl{other.impl_->rust_interp})) {}

PchipInterpolator& PchipInterpolator::operator=(
    const PchipInterpolator& other) {
  if (this != &other) {
    impl_ = std::make_unique<Impl>(Impl{other.impl_->rust_interp});
  }
  return *this;
}

PchipInterpolator::PchipInterpolator(PchipInterpolator&& other) noexcept
    : impl_(std::move(other.impl_)) {}

PchipInterpolator& PchipInterpolator::operator=(
    PchipInterpolator&& other) noexcept {
  if (this != &other) {
    impl_ = std::move(other.impl_);
  }
  return *this;
}

PchipInterpolator::~PchipInterpolator() = default;

float PchipInterpolator::Interpolate(float x) const {
  return impl_->rust_interp.interpolate(x);
}

absl::StatusOr<float> PchipInterpolator::ReverseInterpolate(float y) const {
  ::pchip_rs::ReverseInterpolateResult result =
      impl_->rust_interp.reverse_interpolate(y);
  if (!result.success) {
    return absl::InvalidArgumentError("Reverse interpolation failed.");
  }
  return result.xi;
}

std::vector<float> PchipInterpolator::x() const {
  absl::Span<const float> s = impl_->rust_interp.x();
  return std::vector<float>(s.begin(), s.end());
}

std::vector<float> PchipInterpolator::y() const {
  absl::Span<const float> s = impl_->rust_interp.y();
  return std::vector<float>(s.begin(), s.end());
}

std::vector<float> PchipInterpolator::slopes() const {
  absl::Span<const float> s = impl_->rust_interp.slopes();
  return std::vector<float>(s.begin(), s.end());
}

struct GainCurve::Impl {
  pchip_rs::GainCurve rust_curve;
};

absl::StatusOr<GainCurve> GainCurve::Create(absl::Span<const float> x,
                                            absl::Span<const float> y) {
  ::pchip_rs::GainCurveResult result = pchip_rs::GainCurve::create_ffi(x, y);
  if (!result.success) {
    return absl::InvalidArgumentError(result.get_error_message());
  }
  return GainCurve(std::make_unique<Impl>(Impl{std::move(result.curve)}));
}

absl::StatusOr<GainCurve> GainCurve::Create(absl::Span<const float> x,
                                            absl::Span<const float> y,
                                            absl::Span<const float> slopes) {
  ::pchip_rs::GainCurveResult result =
      pchip_rs::GainCurve::create_with_slopes_ffi(x, y, slopes);
  if (!result.success) {
    return absl::InvalidArgumentError(result.get_error_message());
  }
  return GainCurve(std::make_unique<Impl>(Impl{std::move(result.curve)}));
}

GainCurve::GainCurve(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

GainCurve::GainCurve(const GainCurve& other)
    : impl_(std::make_unique<Impl>(Impl{other.impl_->rust_curve})) {}

GainCurve& GainCurve::operator=(const GainCurve& other) {
  if (this != &other) {
    impl_ = std::make_unique<Impl>(Impl{other.impl_->rust_curve});
  }
  return *this;
}

GainCurve::GainCurve(GainCurve&& other) noexcept
    : impl_(std::move(other.impl_)) {}

GainCurve& GainCurve::operator=(GainCurve&& other) noexcept {
  if (this != &other) {
    impl_ = std::move(other.impl_);
  }
  return *this;
}

GainCurve::~GainCurve() = default;

float GainCurve::Interpolate(float x) const {
  return impl_->rust_curve.interpolate(x);
}

absl::StatusOr<float> GainCurve::ReverseInterpolate(float y) const {
  ::pchip_rs::ReverseInterpolateResult result =
      impl_->rust_curve.reverse_interpolate(y);
  if (!result.success) {
    return absl::InvalidArgumentError("Reverse interpolation failed.");
  }
  return result.xi;
}

absl::StatusOr<PchipInterpolator> CreateSubsampledPchip(
    absl::Span<const float> x, absl::Span<const float> y,
    size_t num_control_points) {
  ::pchip_rs::PchipInterpolatorResult result =
      pchip_rs::create_subsampled_pchip_ffi(x, y, num_control_points);
  if (!result.success) {
    return absl::InvalidArgumentError(result.get_error_message());
  }
  return PchipInterpolator(std::make_unique<PchipInterpolator::Impl>(
      PchipInterpolator::Impl{std::move(result.interp)}));
}

}  // namespace smpte2094_50
