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

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "pchip_rs_bridge/capi.h"

namespace pchip_rs {

class ReverseInterpolateResult {
 public:
  float xi;
  bool success;
  ::rust::Box<pchip_ffi::ReverseInterpolateResult> res;

  ReverseInterpolateResult(::rust::Box<pchip_ffi::ReverseInterpolateResult> r)
      : xi(r->xi()), success(r->success()), res(std::move(r)) {}

  ReverseInterpolateResult(const ReverseInterpolateResult& other)
      : xi(other.xi), success(other.success), res(other.res->clone()) {}

  ReverseInterpolateResult& operator=(const ReverseInterpolateResult& other) {
    xi = other.xi;
    success = other.success;
    res = other.res->clone();
    return *this;
  }
};

template <typename T>
struct SpanWrapper {
  absl::Span<const T> span_;
  absl::Span<const T> to_span() const { return span_; }
  const T* data() const { return span_.data(); }
  size_t size() const { return span_.size(); }
};

class PchipSlopesResult {
 public:
  bool success;
  ::rust::Box<pchip_ffi::PchipSlopesResult> res;

  PchipSlopesResult(::rust::Box<pchip_ffi::PchipSlopesResult> r)
      : success(r->success()), res(std::move(r)) {}

  PchipSlopesResult(const PchipSlopesResult& other)
      : success(other.success), res(other.res->clone()) {}

  PchipSlopesResult& operator=(const PchipSlopesResult& other) {
    success = other.success;
    res = other.res->clone();
    return *this;
  }

  SpanWrapper<float> get_slopes() const {
    auto slice = res->get_slopes();
    return {absl::MakeConstSpan(slice.data(), slice.size())};
  }
  absl::string_view get_error_message() const {
    auto s = res->get_error_message();
    return {s.data(), s.size()};
  }
};

class PchipInterpolatorResult;

class PchipInterpolator {
 public:
  ::rust::Box<pchip_ffi::PchipInterpolator> interp;

  PchipInterpolator(::rust::Box<pchip_ffi::PchipInterpolator> i)
      : interp(std::move(i)) {}

  PchipInterpolator(const PchipInterpolator& other)
      : interp(other.interp->clone()) {}

  PchipInterpolator& operator=(const PchipInterpolator& other) {
    interp = other.interp->clone();
    return *this;
  }

  static PchipInterpolatorResult create_ffi(absl::Span<const float> x,
                                            absl::Span<const float> y);

  float interpolate(float xi) const { return interp->interpolate(xi); }

  ReverseInterpolateResult reverse_interpolate(float yi) const {
    return ReverseInterpolateResult(interp->reverse_interpolate(yi));
  }

  SpanWrapper<float> x() const {
    auto slice = interp->x();
    return {absl::MakeConstSpan(slice.data(), slice.size())};
  }

  SpanWrapper<float> y() const {
    auto slice = interp->y();
    return {absl::MakeConstSpan(slice.data(), slice.size())};
  }

  SpanWrapper<float> slopes() const {
    auto slice = interp->slopes();
    return {absl::MakeConstSpan(slice.data(), slice.size())};
  }
};

class PchipInterpolatorResult {
 public:
  PchipInterpolator interp;
  bool success;
  ::rust::Box<pchip_ffi::PchipInterpolatorResult> res;

  PchipInterpolatorResult(::rust::Box<pchip_ffi::PchipInterpolatorResult> r)
      : interp(r->interp()), success(r->success()), res(std::move(r)) {}

  PchipInterpolatorResult(const PchipInterpolatorResult& other)
      : interp(other.interp), success(other.success), res(other.res->clone()) {}

  PchipInterpolatorResult& operator=(const PchipInterpolatorResult& other) {
    interp = other.interp;
    success = other.success;
    res = other.res->clone();
    return *this;
  }

  absl::string_view get_error_message() const {
    auto s = res->get_error_message();
    return {s.data(), s.size()};
  }
};

inline PchipInterpolatorResult PchipInterpolator::create_ffi(
    absl::Span<const float> x, absl::Span<const float> y) {
  return PchipInterpolatorResult(pchip_ffi::pchip_interpolator_create_ffi(
      ::rust::Slice<const float>(x.data(), x.size()),
      ::rust::Slice<const float>(y.data(), y.size())));
}

class GainCurveResult;

class GainCurve {
 public:
  ::rust::Box<pchip_ffi::GainCurve> curve;

  GainCurve(::rust::Box<pchip_ffi::GainCurve> c) : curve(std::move(c)) {}

  GainCurve(const GainCurve& other) : curve(other.curve->clone()) {}

  GainCurve& operator=(const GainCurve& other) {
    curve = other.curve->clone();
    return *this;
  }

  static GainCurveResult create_ffi(absl::Span<const float> x,
                                    absl::Span<const float> y);

  float interpolate(float xi) const { return curve->interpolate(xi); }

  ReverseInterpolateResult reverse_interpolate(float yi) const {
    return ReverseInterpolateResult(curve->reverse_interpolate(yi));
  }
};

class GainCurveResult {
 public:
  GainCurve curve;
  bool success;
  ::rust::Box<pchip_ffi::GainCurveResult> res;

  GainCurveResult(::rust::Box<pchip_ffi::GainCurveResult> r)
      : curve(r->curve()), success(r->success()), res(std::move(r)) {}

  GainCurveResult(const GainCurveResult& other)
      : curve(other.curve), success(other.success), res(other.res->clone()) {}

  GainCurveResult& operator=(const GainCurveResult& other) {
    curve = other.curve;
    success = other.success;
    res = other.res->clone();
    return *this;
  }

  absl::string_view get_error_message() const {
    auto s = res->get_error_message();
    return {s.data(), s.size()};
  }
};

inline GainCurveResult GainCurve::create_ffi(absl::Span<const float> x,
                                             absl::Span<const float> y) {
  return GainCurveResult(pchip_ffi::gain_curve_create_ffi(
      ::rust::Slice<const float>(x.data(), x.size()),
      ::rust::Slice<const float>(y.data(), y.size())));
}

inline PchipSlopesResult pchip_slopes_ffi(absl::Span<const float> x,
                                          absl::Span<const float> y) {
  return PchipSlopesResult(pchip_ffi::pchip_slopes_ffi(
      ::rust::Slice<const float>(x.data(), x.size()),
      ::rust::Slice<const float>(y.data(), y.size())));
}

inline PchipInterpolatorResult create_subsampled_pchip_ffi(
    absl::Span<const float> x, absl::Span<const float> y,
    size_t num_control_points) {
  return PchipInterpolatorResult(pchip_ffi::create_subsampled_pchip_ffi(
      ::rust::Slice<const float>(x.data(), x.size()),
      ::rust::Slice<const float>(y.data(), y.size()), num_control_points));
}

}  // namespace pchip_rs

#endif  // THIRD_PARTY_LIBSMPTE2094_50_TESTS_PCHIP_FFI_H_
