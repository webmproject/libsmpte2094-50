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

#ifndef THIRD_PARTY_LIBSMPTE2094_50_TESTS_PCHIP_FFI_H_
#define THIRD_PARTY_LIBSMPTE2094_50_TESTS_PCHIP_FFI_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "third_party/absl/types/span.h"
#include "third_party/libsmpte2094_50/pchip_rs.h"

namespace pchip_rs {

using ReverseInterpolateResult = pchip_ffi::ReverseInterpolateResult;

template <typename T>
struct SpanWrapper {
  absl::Span<const T> span_;
  absl::Span<const T> to_span() const { return span_; }
  const T* data() const { return span_.data(); }
  size_t size() const { return span_.size(); }
};

class PchipSlopesResult : public pchip_ffi::PchipSlopesResultFfi {
 public:
  PchipSlopesResult() = default;
  PchipSlopesResult(const pchip_ffi::PchipSlopesResultFfi& ffi)
      : PchipSlopesResultFfi(ffi) {}
  ~PchipSlopesResult() {
    ::pchip_ffi::pchip_slopes_result_free(*this);
    slopes = nullptr;
    slopes_len = 0;
    success = false;
    error_message = nullptr;
  }

  SpanWrapper<float> get_slopes() const {
    return SpanWrapper<float>{absl::MakeConstSpan(slopes, slopes_len)};
  }
};

class PchipInterpolatorResult;

class PchipInterpolator : public pchip_ffi::PchipInterpolatorFfi {
 public:
  PchipInterpolator() = default;
  PchipInterpolator(const pchip_ffi::PchipInterpolatorFfi& ffi)
      : PchipInterpolatorFfi(ffi) {}

  static PchipInterpolatorResult create_ffi(absl::Span<const float> x,
                                            absl::Span<const float> y);

  float interpolate(float xi) const {
    return ::pchip_ffi::pchip_interpolator_interpolate_ffi(this, xi);
  }

  ReverseInterpolateResult reverse_interpolate(float yi) const {
    return ::pchip_ffi::pchip_interpolator_reverse_interpolate_ffi(this, yi);
  }

  SpanWrapper<float> x() const {
    return SpanWrapper<float>{
        absl::MakeConstSpan(PchipInterpolatorFfi::x, x_len)};
  }

  SpanWrapper<float> y() const {
    return SpanWrapper<float>{
        absl::MakeConstSpan(PchipInterpolatorFfi::y, y_len)};
  }

  SpanWrapper<float> slopes() const {
    return SpanWrapper<float>{
        absl::MakeConstSpan(PchipInterpolatorFfi::slopes, slopes_len)};
  }
};

class PchipInterpolatorResult {
 public:
  PchipInterpolator interp;
  bool success;
  char* error_message;

  PchipInterpolatorResult() = default;
  PchipInterpolatorResult(const pchip_ffi::PchipInterpolatorResultFfi& ffi) {
    interp = PchipInterpolator(ffi.interp);
    success = ffi.success;
    error_message = ffi.error_message;
  }
};

inline PchipInterpolatorResult PchipInterpolator::create_ffi(
    absl::Span<const float> x, absl::Span<const float> y) {
  pchip_ffi::PchipInterpolatorResultFfi ffi_res =
      ::pchip_ffi::pchip_interpolator_create_ffi(x.data(), x.size(), y.data(),
                                                 y.size());
  return PchipInterpolatorResult(ffi_res);
}

class GainCurveResult;

class GainCurve : public pchip_ffi::GainCurveFfi {
 public:
  GainCurve() = default;
  GainCurve(const pchip_ffi::GainCurveFfi& ffi) : GainCurveFfi(ffi) {}

  static GainCurveResult create_ffi(absl::Span<const float> x,
                                    absl::Span<const float> y);

  float interpolate(float xi) const {
    return ::pchip_ffi::gain_curve_interpolate_ffi(this, xi);
  }

  ReverseInterpolateResult reverse_interpolate(float yi) const {
    return ::pchip_ffi::gain_curve_reverse_interpolate_ffi(this, yi);
  }
};

class GainCurveResult {
 public:
  GainCurve curve;
  bool success;
  char* error_message;

  GainCurveResult() = default;
  GainCurveResult(const pchip_ffi::GainCurveResultFfi& ffi) {
    curve = GainCurve(ffi.curve);
    success = ffi.success;
    error_message = ffi.error_message;
  }
};

inline GainCurveResult GainCurve::create_ffi(absl::Span<const float> x,
                                             absl::Span<const float> y) {
  pchip_ffi::GainCurveResultFfi ffi_res = ::pchip_ffi::gain_curve_create_ffi(
      x.data(), x.size(), y.data(), y.size());
  return GainCurveResult(ffi_res);
}

inline PchipSlopesResult pchip_slopes_ffi(absl::Span<const float> x,
                                          absl::Span<const float> y) {
  pchip_ffi::PchipSlopesResultFfi ffi_res =
      ::pchip_ffi::pchip_slopes_ffi(x.data(), x.size(), y.data(), y.size());
  return PchipSlopesResult(ffi_res);
}

inline PchipInterpolatorResult create_subsampled_pchip_ffi(
    absl::Span<const float> x, absl::Span<const float> y,
    size_t num_control_points) {
  pchip_ffi::PchipInterpolatorResultFfi ffi_res =
      ::pchip_ffi::create_subsampled_pchip_ffi(x.data(), x.size(), y.data(),
                                               y.size(), num_control_points);
  return PchipInterpolatorResult(ffi_res);
}

}  // namespace pchip_rs

#endif  // THIRD_PARTY_LIBSMPTE2094_50_TESTS_PCHIP_FFI_H_
