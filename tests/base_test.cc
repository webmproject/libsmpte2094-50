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

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/match.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "smpte2094_50/smpte2094_50.h"
#include "smpte2094_50/utils.h"
#include "test_helper.h"

namespace smpte2094_50 {
namespace {

TEST(MetadataTest_St209450Roundtrip, St209450Roundtrip) {
  DynamicMetadata metadata = {
      .hdr_reference_white = 203,
      .has_adaptive_tone_map_flag = true,
      .baseline_hdr_headroom_log2 = 0.1f,
      .use_reference_white_tone_mapping_flag = false,
      .gain_application_space_chromaticities =
          DynamicMetadata::kChromaticitiesSrgb,
  };

  metadata.rules.push_back({
      .alternate_hdr_headroom_log2 = 1.5f,
      .curve = {{.x = 0.0, .y = 0.0, .m = 0.1f},
                {.x = 0.5, .y = 0.2, .m = 0.5f},
                {.x = 1.0, .y = 1.0, .m = 0.9f}},
      .use_pchip_slope = false,
      .mix = {.rgb = {0.1f, 0.15f, 0.2f},
              .max = 0.4f,
              .min = 1.0f - 0.1f - 0.15f - 0.2f - 0.4f,
              .component = 0.f},
  });

  auto st209450_data_or = ToSt209450(metadata);
  ASSERT_TRUE(st209450_data_or.ok());
  std::string st209450_data = *st209450_data_or;

  auto metadata_rt_or = FromSt209450(st209450_data);
  ASSERT_TRUE(metadata_rt_or.ok());
  DynamicMetadata metadata_rt = *metadata_rt_or;

  EXPECT_THAT(metadata_rt, DynamicMetadataEq(metadata));
}

TEST(MetadataTest_St209450Roundtrip, St209450Roundtrip2) {
  DynamicMetadata metadata = {
      .hdr_reference_white = 500,  // Non default reference white.
      .has_adaptive_tone_map_flag = true,
      .baseline_hdr_headroom_log2 = 1.5f,
      .use_reference_white_tone_mapping_flag = false,
      .gain_application_space_chromaticities =
          DynamicMetadata::kChromaticitiesSrgb,
  };

  metadata.rules.push_back({
      .alternate_hdr_headroom_log2 = 1.0f,
      .curve = {{.x = 0.0, .y = 0.0, .m = 0.1f},
                {.x = 0.5, .y = -0.2, .m = 0.5f},  // Negative gain.
                {.x = 1.0, .y = -1.0, .m = 0.9f}},
      .use_pchip_slope = false,
      .mix = {.rgb = {0.1f, 0.15f, 0.2f},
              .max = 0.4f,
              .min = 1.0f - 0.1f - 0.15f - 0.2f - 0.4f,
              .component = 0.f},
  });

  auto st209450_data_or = ToSt209450(metadata);
  ASSERT_TRUE(st209450_data_or.ok());
  std::string st209450_data = *st209450_data_or;

  auto metadata_rt_or = FromSt209450(st209450_data);
  ASSERT_TRUE(metadata_rt_or.ok());
  DynamicMetadata metadata_rt = *metadata_rt_or;

  EXPECT_THAT(metadata_rt, DynamicMetadataEq(metadata));
}

TEST(MetadataTest_St209450Roundtrip, InvalidMetadataWrongSign) {
  DynamicMetadata metadata = {
      .hdr_reference_white = 500,
      .has_adaptive_tone_map_flag = true,
      .baseline_hdr_headroom_log2 = 1.5f,
      .use_reference_white_tone_mapping_flag = false,
      .gain_application_space_chromaticities =
          DynamicMetadata::kChromaticitiesSrgb,
  };

  metadata.rules.push_back({
      .alternate_hdr_headroom_log2 = 1.0f,
      .curve = {{.x = 0.0, .y = 0.0, .m = 0.1f},
                {.x = 0.5, .y = -0.2, .m = 0.5f},
                {.x = 1.0, .y = 1.0, .m = 0.9f}},  // Wrong sign.
      .use_pchip_slope = false,
      .mix = {.rgb = {0.1f, 0.15f, 0.2f},
              .max = 0.4f,
              .min = 1.0f - 0.1f - 0.15f - 0.2f - 0.4f,
              .component = 0.f},
  });

  auto status_or = ToSt209450(metadata);
  EXPECT_FALSE(status_or.ok());
  EXPECT_EQ(status_or.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_TRUE(absl::StrContains(status_or.status().message(),
                                "Sign of y does not match"));
}

TEST(MetadataTest, FailsOnCurveSize) {
  constexpr int kNumPoints = 33;
  DynamicMetadata metadata = {
      .hdr_reference_white = 500,
      .has_adaptive_tone_map_flag = true,
      .baseline_hdr_headroom_log2 = 1.5f,
      .use_reference_white_tone_mapping_flag = false,
      .gain_application_space_chromaticities =
          DynamicMetadata::kChromaticitiesSrgb,
  };
  std::vector<ControlPoint> identity_curve(kNumPoints);
  for (int i = 0; i < kNumPoints; ++i) {  // 34 points.
    const float x = i / static_cast<float>(kNumPoints - 1);
    identity_curve[i] = {.x = x, .y = 0.0f};
  }
  metadata.rules.push_back({
      .alternate_hdr_headroom_log2 = 1.0f,
      .curve = identity_curve,
      .use_pchip_slope = true,
      .mix = {.rgb = {0.5f, 0.5f, 0.0f},
              .max = 0.0f,
              .min = 0.0f,
              .component = 0.f},
  });

  {
    auto status_or = ToSt209450(metadata);
    EXPECT_FALSE(status_or.ok());
    EXPECT_EQ(status_or.status().code(), absl::StatusCode::kInvalidArgument);
    EXPECT_TRUE(absl::StrContains(status_or.status().message(),
                                  "curve size 33 is > 32"));
  }

  metadata.rules[0].curve.clear();
  {
    auto status_or = ToSt209450(metadata);
    EXPECT_FALSE(status_or.ok());
    EXPECT_EQ(status_or.status().code(), absl::StatusCode::kInvalidArgument);
    EXPECT_TRUE(absl::StrContains(status_or.status().message(),
                                  "curve size 0 is > 32 or == 0"));
  }
}

TEST(MetadataTest, PopulateUsingRwtm) {
  DynamicMetadata metadata;
  metadata.baseline_hdr_headroom_log2 = 2.0f;
  PopulateUsingRwtm(metadata);

  EXPECT_EQ(metadata.rules.size(), 2);
  EXPECT_EQ(metadata.gain_application_space_chromaticities,
            DynamicMetadata::kChromaticitiesRec2020);

  // Rule 0 should have alternate_hdr_headroom_log2 = 0.
  EXPECT_FLOAT_EQ(metadata.rules[0].alternate_hdr_headroom_log2, 0.0f);
  EXPECT_EQ(metadata.rules[0].curve.size(), 8);
  EXPECT_NEAR(metadata.rules[0].curve[0].x, 1.0f, 1e-6);

  // Rule 1 should have alternate_hdr_headroom_log2 > 0.
  EXPECT_GT(metadata.rules[1].alternate_hdr_headroom_log2, 0.0f);
}

TEST(MetadataTest, FromSt209450RwtmFlag) {
  // Construct a bitstream with use_reference_white_tone_mapping_flag = 1.
  // application_version=0 (3), minimum_application_version=0 (3), reserved=0
  // (2) -> 0x00 has_custom_hdr_reference_white_flag=0 (1),
  // has_adaptive_tone_map_flag=1 (1), reserved=0 (6) -> 0x40
  // baseline_hdr_headroom_log2 = 2.0 -> 20000 (16 bits) -> 0x4E20
  // use_reference_white_tone_mapping_flag=1 (1), reserved=0 (7) -> 0x80
  std::string data("\x00\x40\x4E\x20\x80", 5);

  auto metadata_or = FromSt209450(data);
  ASSERT_TRUE(metadata_or.ok());
  DynamicMetadata metadata = *metadata_or;
  EXPECT_TRUE(metadata.use_reference_white_tone_mapping_flag);
  EXPECT_EQ(metadata.rules.size(), 2);

  // Ground truth generated by hdrscope.
  DynamicMetadata expected_metadata = {
      .hdr_reference_white = 203,
      .has_adaptive_tone_map_flag = true,
      .baseline_hdr_headroom_log2 = 2.0f,
      .use_reference_white_tone_mapping_flag = false,
      .gain_application_space_chromaticities =
          DynamicMetadata::kChromaticitiesRec2020,
  };
  expected_metadata.rules.push_back({
      .alternate_hdr_headroom_log2 = 0.0f,
      .curve = {{.x = 1.0f, .y = -0.82291f, .m = 0.0f},
                {.x = 1.18363f, .y = -0.8795f, .m = -0.46552f},
                {.x = 1.44891f, .y = -1.0167f, .m = -0.53445f},
                {.x = 1.79583f, .y = -1.19658f, .m = -0.49583f},
                {.x = 2.22441f, .y = -1.39531f, .m = -0.43254f},
                {.x = 2.73462f, .y = -1.59948f, .m = -0.37056f},
                {.x = 3.32649f, .y = -1.80214f, .m = -0.31706f},
                {.x = 4.0f, .y = -2.0f, .m = -0.27288f}},
      .use_pchip_slope = false,
      .mix = {.rgb = {0.0f, 0.0f, 0.0f},
              .max = 1.0f,
              .min = 0.0f,
              .component = 0.0f},
  });
  expected_metadata.rules.push_back({
      .alternate_hdr_headroom_log2 = 1.23023f,
      .curve = {{.x = 1.0f, .y = 0.0f, .m = 0.0f},
                {.x = 1.27549f, .y = -0.03869f, .m = -0.22983f},
                {.x = 1.60201f, .y = -0.12702f, .m = -0.2943f},
                {.x = 1.97956f, .y = -0.24036f, .m = -0.30047f},
                {.x = 2.40813f, .y = -0.36637f, .m = -0.28607f},
                {.x = 2.88772f, .y = -0.49863f, .m = -0.26543f},
                {.x = 3.41835f, .y = -0.6337f, .m = -0.24415f},
                {.x = 4.0f, .y = -0.76977f, .m = -0.22434f}},
      .use_pchip_slope = false,
      .mix = {.rgb = {0.0f, 0.0f, 0.0f},
              .max = 1.0f,
              .min = 0.0f,
              .component = 0.0f},
  });

  EXPECT_THAT(metadata, DynamicMetadataEq(expected_metadata));
}

}  // namespace
}  // namespace smpte2094_50
