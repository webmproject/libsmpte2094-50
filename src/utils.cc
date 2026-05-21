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

#include "smpte2094_50/utils.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "smpte2094_50/smpte2094_50.h"
#include "smpte2094_50/utils.h"
#include "utils_ffi.h"

namespace smpte2094_50 {

namespace {

utils_rs::DynamicMetadata ToRustMetadata(const DynamicMetadata& cpp) {
  utils_rs::DynamicMetadata rust;
  rust.has_adaptive_tone_map_flag = cpp.has_adaptive_tone_map_flag;
  rust.use_reference_white_tone_mapping_flag =
      cpp.use_reference_white_tone_mapping_flag;
  rust.hdr_reference_white = cpp.hdr_reference_white;
  rust.baseline_hdr_headroom_log2 = cpp.baseline_hdr_headroom_log2;
  for (int i = 0; i < 8; ++i) {
    rust.gain_application_space_chromaticities[i] =
        cpp.gain_application_space_chromaticities[i];
  }

  for (const auto& cpp_rule : cpp.rules) {
    utils_rs::ToneMappingRule rs_rule;
    rs_rule.alternate_hdr_headroom_log2 = cpp_rule.alternate_hdr_headroom_log2;
    rs_rule.use_pchip_slope = cpp_rule.use_pchip_slope;

    rs_rule.mix.rgb[0] = cpp_rule.mix.rgb[0];
    rs_rule.mix.rgb[1] = cpp_rule.mix.rgb[1];
    rs_rule.mix.rgb[2] = cpp_rule.mix.rgb[2];
    rs_rule.mix.max = cpp_rule.mix.max;
    rs_rule.mix.min = cpp_rule.mix.min;
    rs_rule.mix.component = cpp_rule.mix.component;

    for (const auto& cpp_pt : cpp_rule.curve) {
      utils_rs::ControlPoint rs_pt;
      rs_pt.x = cpp_pt.x;
      rs_pt.y = cpp_pt.y;
      rs_pt.m = cpp_pt.m;
      rs_rule.add_point(rs_pt);
    }
    rust.add_rule(rs_rule);
  }
  return rust;
}

void FromRustMetadata(const utils_rs::DynamicMetadata& rust,
                      DynamicMetadata& cpp) {
  cpp.has_adaptive_tone_map_flag = rust.has_adaptive_tone_map_flag;
  cpp.use_reference_white_tone_mapping_flag =
      rust.use_reference_white_tone_mapping_flag;
  cpp.hdr_reference_white = rust.hdr_reference_white;
  cpp.baseline_hdr_headroom_log2 = rust.baseline_hdr_headroom_log2;
  for (int i = 0; i < 8; ++i) {
    cpp.gain_application_space_chromaticities[i] =
        rust.gain_application_space_chromaticities[i];
  }

  cpp.rules.clear();
  cpp.rules.reserve(rust.get_rules().size());
  for (const auto& rs_rule : rust.get_rules().to_span()) {
    ToneMappingRule cpp_rule;
    cpp_rule.alternate_hdr_headroom_log2 = rs_rule.alternate_hdr_headroom_log2;
    cpp_rule.use_pchip_slope = rs_rule.use_pchip_slope;

    cpp_rule.mix.rgb[0] = rs_rule.mix.rgb[0];
    cpp_rule.mix.rgb[1] = rs_rule.mix.rgb[1];
    cpp_rule.mix.rgb[2] = rs_rule.mix.rgb[2];
    cpp_rule.mix.max = rs_rule.mix.max;
    cpp_rule.mix.min = rs_rule.mix.min;
    cpp_rule.mix.component = rs_rule.mix.component;

    for (const auto& rs_pt : rs_rule.get_curve().to_span()) {
      ControlPoint cpp_pt;
      cpp_pt.x = rs_pt.x;
      cpp_pt.y = rs_pt.y;
      cpp_pt.m = rs_pt.m;
      cpp_rule.curve.push_back(cpp_pt);
    }
    cpp.rules.push_back(cpp_rule);
  }
}

}  // namespace

absl::StatusOr<std::string> ToSt209450(const DynamicMetadata& metadata) {
  utils_rs::DynamicMetadata rs_metadata = ToRustMetadata(metadata);
  const ::utils_rs::ToSt209450Result result =
      utils_rs::to_st209450_ffi(rs_metadata);
  if (!result.success) {
    return absl::InvalidArgumentError(result.get_error_message());
  }
  const auto& bytes = result.get_data();
  return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

absl::StatusOr<DynamicMetadata> FromSt209450(absl::string_view data) {
  const ::utils_rs::FromSt209450Result result =
      utils_rs::from_st209450_ffi(absl::MakeConstSpan(
          reinterpret_cast<const uint8_t*>(data.data()), data.size()));
  if (!result.success) {
    return absl::InvalidArgumentError(result.get_error_message());
  }
  DynamicMetadata metadata;
  FromRustMetadata(result.metadata, metadata);
  return metadata;
}

void PopulateUsingRwtm(DynamicMetadata& metadata) {
  utils_rs::DynamicMetadata rs_metadata = ToRustMetadata(metadata);
  utils_rs::dynamic_metadata_populate_using_rwtm(rs_metadata);
  FromRustMetadata(rs_metadata, metadata);
}

absl::Status PopulatePchipSlopes(DynamicMetadata& metadata) {
  utils_rs::DynamicMetadata rs_metadata = ToRustMetadata(metadata);
  const auto result =
      utils_rs::dynamic_metadata_populate_pchip_slopes_ffi(rs_metadata);
  if (!result.success) {
    return absl::InvalidArgumentError(result.get_error_message());
  }
  FromRustMetadata(rs_metadata, metadata);
  return absl::OkStatus();
}

absl::Status PopulateImplicitParameters(DynamicMetadata& metadata) {
  utils_rs::DynamicMetadata rs_metadata = ToRustMetadata(metadata);
  const auto result = utils_rs::populate_implicit_parameters_ffi(rs_metadata);
  if (!result.success) {
    return absl::InvalidArgumentError(result.get_error_message());
  }
  FromRustMetadata(rs_metadata, metadata);
  return absl::OkStatus();
}

bool IsValid(const DynamicMetadata& metadata) {
  utils_rs::DynamicMetadata rs_metadata = ToRustMetadata(metadata);
  return utils_rs::is_valid_ffi(rs_metadata);
}

}  // namespace smpte2094_50
