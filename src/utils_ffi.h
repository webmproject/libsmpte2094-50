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

#ifndef THIRD_PARTY_LIBSMPTE2094_50_SRC_UTILS_FFI_H_
#define THIRD_PARTY_LIBSMPTE2094_50_SRC_UTILS_FFI_H_

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "utils_rs_bridge/utils_ffi.h"

namespace utils_rs {

class ControlPoint {
 public:
  float x;
  float y;
  float m;

  ControlPoint(::rust::Box<utils_ffi::ControlPoint> p)
      : x(p->x()), y(p->y()), m(p->m()) {}
  ControlPoint(const utils_ffi::ControlPoint& p)
      : x(p.x()), y(p.y()), m(p.m()) {}
  ControlPoint() : x(0), y(0), m(0) {}

  ::rust::Box<utils_ffi::ControlPoint> to_ffi() const {
    return utils_ffi::control_point_create_ffi(x, y, m);
  }
};

class ComponentMix {
 public:
  std::array<float, 3> rgb;
  float max;
  float min;
  float component;

  ComponentMix(::rust::Box<utils_ffi::ComponentMix> m)
      : max(m->max()), min(m->min()), component(m->component()) {
    auto r = m->rgb();
    rgb[0] = r[0];
    rgb[1] = r[1];
    rgb[2] = r[2];
  }
  ComponentMix(const utils_ffi::ComponentMix& m)
      : max(m.max()), min(m.min()), component(m.component()) {
    auto r = m.rgb();
    rgb[0] = r[0];
    rgb[1] = r[1];
    rgb[2] = r[2];
  }
  ComponentMix() : rgb{0, 0, 0}, max(0), min(0), component(0) {}

  ::rust::Box<utils_ffi::ComponentMix> to_ffi() const {
    return utils_ffi::component_mix_create_ffi(rgb, max, min, component);
  }
};

template <typename T>
struct SpanWrapper {
  absl::Span<const T> span_;
  absl::Span<const T> to_span() const { return span_; }
  const T* data() const { return span_.data(); }
  size_t size() const { return span_.size(); }
};

class ToneMappingRule {
 public:
  float alternate_hdr_headroom_log2;
  bool use_pchip_slope;
  ComponentMix mix;
  std::vector<ControlPoint> curve;

  ToneMappingRule(::rust::Box<utils_ffi::ToneMappingRule> r)
      : alternate_hdr_headroom_log2(r->alternate_hdr_headroom_log2()),
        use_pchip_slope(r->use_pchip_slope()),
        mix(r->mix()) {
    auto ffi_curve = r->get_curve();
    curve.reserve(ffi_curve.size());
    for (const auto& p : ffi_curve) {
      curve.push_back(ControlPoint(p));
    }
  }
  ToneMappingRule(const utils_ffi::ToneMappingRule& r)
      : alternate_hdr_headroom_log2(r.alternate_hdr_headroom_log2()),
        use_pchip_slope(r.use_pchip_slope()),
        mix(r.mix()) {
    auto ffi_curve = r.get_curve();
    curve.reserve(ffi_curve.size());
    for (const auto& p : ffi_curve) {
      curve.push_back(ControlPoint(p));
    }
  }
  ToneMappingRule() : alternate_hdr_headroom_log2(0), use_pchip_slope(false) {}

  void add_point(const ControlPoint& point) { curve.push_back(point); }

  SpanWrapper<ControlPoint> get_curve() const {
    return {absl::MakeConstSpan(curve.data(), curve.size())};
  }

  ::rust::Box<utils_ffi::ToneMappingRule> to_ffi() const {
    auto r = utils_ffi::tone_mapping_rule_create_ffi();
    r->set_alternate_hdr_headroom_log2(alternate_hdr_headroom_log2);
    r->set_use_pchip_slope(use_pchip_slope);
    r->set_mix(mix.to_ffi());
    for (const auto& p : curve) {
      r->add_point(p.to_ffi());
    }
    return r;
  }
};

class DynamicMetadata {
 public:
  bool has_adaptive_tone_map_flag;
  bool use_reference_white_tone_mapping_flag;
  float hdr_reference_white;
  float baseline_hdr_headroom_log2;
  std::array<float, 8> gain_application_space_chromaticities;
  std::vector<ToneMappingRule> rules;

  DynamicMetadata(::rust::Box<utils_ffi::DynamicMetadata> m)
      : has_adaptive_tone_map_flag(m->has_adaptive_tone_map_flag()),
        use_reference_white_tone_mapping_flag(
            m->use_reference_white_tone_mapping_flag()),
        hdr_reference_white(m->hdr_reference_white()),
        baseline_hdr_headroom_log2(m->baseline_hdr_headroom_log2()) {
    auto chrom = m->gain_application_space_chromaticities();
    for (int i = 0; i < 8; ++i)
      gain_application_space_chromaticities[i] = chrom[i];
    auto ffi_rules = m->get_rules();
    rules.reserve(ffi_rules.size());
    for (const auto& r : ffi_rules) {
      rules.push_back(ToneMappingRule(r));
    }
  }
  DynamicMetadata(const utils_ffi::DynamicMetadata& m)
      : has_adaptive_tone_map_flag(m.has_adaptive_tone_map_flag()),
        use_reference_white_tone_mapping_flag(
            m.use_reference_white_tone_mapping_flag()),
        hdr_reference_white(m.hdr_reference_white()),
        baseline_hdr_headroom_log2(m.baseline_hdr_headroom_log2()) {
    auto chrom = m.gain_application_space_chromaticities();
    for (int i = 0; i < 8; ++i)
      gain_application_space_chromaticities[i] = chrom[i];
    auto ffi_rules = m.get_rules();
    rules.reserve(ffi_rules.size());
    for (const auto& r : ffi_rules) {
      rules.push_back(ToneMappingRule(r));
    }
  }
  DynamicMetadata()
      : has_adaptive_tone_map_flag(false),
        use_reference_white_tone_mapping_flag(false),
        hdr_reference_white(0),
        baseline_hdr_headroom_log2(0) {
    gain_application_space_chromaticities.fill(0.0f);
  }

  void add_rule(const ToneMappingRule& rule) { rules.push_back(rule); }

  SpanWrapper<ToneMappingRule> get_rules() const {
    return {absl::MakeConstSpan(rules.data(), rules.size())};
  }

  ::rust::Box<utils_ffi::DynamicMetadata> to_ffi() const {
    auto m = utils_ffi::dynamic_metadata_create_ffi();
    m->set_has_adaptive_tone_map_flag(has_adaptive_tone_map_flag);
    m->set_use_reference_white_tone_mapping_flag(
        use_reference_white_tone_mapping_flag);
    m->set_hdr_reference_white(hdr_reference_white);
    m->set_baseline_hdr_headroom_log2(baseline_hdr_headroom_log2);
    m->set_gain_application_space_chromaticities(
        gain_application_space_chromaticities);
    for (const auto& r : rules) {
      m->add_rule(r.to_ffi());
    }
    return m;
  }
};

class ToSt209450Result {
 public:
  bool success;
  ::rust::Box<utils_ffi::ToSt209450Result> res;

  ToSt209450Result(::rust::Box<utils_ffi::ToSt209450Result> r)
      : success(r->success()), res(std::move(r)) {}

  ToSt209450Result(const ToSt209450Result& other)
      : success(other.success), res(other.res->clone()) {}

  ToSt209450Result& operator=(const ToSt209450Result& other) {
    success = other.success;
    res = other.res->clone();
    return *this;
  }

  absl::Span<const uint8_t> get_data() const {
    auto data = res->get_data();
    return absl::MakeConstSpan(data.data(), data.size());
  }
  absl::string_view get_error_message() const {
    auto s = res->get_error_message();
    return {s.data(), s.size()};
  }
};

class FromSt209450Result {
 public:
  bool success;
  DynamicMetadata metadata;
  ::rust::Box<utils_ffi::FromSt209450Result> res;

  FromSt209450Result(::rust::Box<utils_ffi::FromSt209450Result> r)
      : success(r->success()), metadata(r->metadata()), res(std::move(r)) {}

  FromSt209450Result(const FromSt209450Result& other)
      : success(other.success),
        metadata(other.metadata),
        res(other.res->clone()) {}

  FromSt209450Result& operator=(const FromSt209450Result& other) {
    success = other.success;
    metadata = other.metadata;
    res = other.res->clone();
    return *this;
  }

  absl::string_view get_error_message() const {
    auto s = res->get_error_message();
    return {s.data(), s.size()};
  }
};

class SimpleResult {
 public:
  bool success;
  ::rust::Box<utils_ffi::SimpleResult> res;

  SimpleResult(::rust::Box<utils_ffi::SimpleResult> r)
      : success(r->success()), res(std::move(r)) {}

  SimpleResult(const SimpleResult& other)
      : success(other.success), res(other.res->clone()) {}

  SimpleResult& operator=(const SimpleResult& other) {
    success = other.success;
    res = other.res->clone();
    return *this;
  }

  absl::string_view get_error_message() const {
    auto s = res->get_error_message();
    return {s.data(), s.size()};
  }
};

inline ToSt209450Result to_st209450_ffi(const DynamicMetadata& metadata) {
  return ToSt209450Result(utils_ffi::to_st209450_ffi(*metadata.to_ffi()));
}

inline bool is_valid_ffi(const DynamicMetadata& metadata) {
  return utils_ffi::is_valid_ffi(*metadata.to_ffi());
}

inline FromSt209450Result from_st209450_ffi(absl::Span<const uint8_t> data) {
  return FromSt209450Result(utils_ffi::from_st209450_ffi(
      ::rust::Slice<const uint8_t>(data.data(), data.size())));
}

inline SimpleResult populate_implicit_parameters_ffi(
    DynamicMetadata& metadata) {
  auto ffi_m = metadata.to_ffi();
  auto res = utils_ffi::populate_implicit_parameters_ffi(*ffi_m);
  metadata = DynamicMetadata(*ffi_m);
  return SimpleResult(std::move(res));
}

inline SimpleResult dynamic_metadata_populate_pchip_slopes_ffi(
    DynamicMetadata& metadata) {
  auto ffi_m = metadata.to_ffi();
  auto res = utils_ffi::dynamic_metadata_populate_pchip_slopes_ffi(*ffi_m);
  metadata = DynamicMetadata(*ffi_m);
  return SimpleResult(std::move(res));
}

inline void dynamic_metadata_populate_using_rwtm(DynamicMetadata& metadata) {
  auto ffi_m = metadata.to_ffi();
  utils_ffi::dynamic_metadata_populate_using_rwtm_ffi(*ffi_m);
  metadata = DynamicMetadata(*ffi_m);
}

}  // namespace utils_rs

#endif  // THIRD_PARTY_LIBSMPTE2094_50_SRC_UTILS_FFI_H_
