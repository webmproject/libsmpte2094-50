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

#include "third_party/absl/status/status.h"
#include "third_party/libsmpte2094_50/include/smpte2094_50/smpte2094_50.h"
#include "third_party/libsmpte2094_50/include/smpte2094_50/utils.h"
#include "third_party/libsmpte2094_50/tests/test_helper.h"
#if defined(BAZEL_BUILD)
#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#else
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#endif

namespace smpte2094_50
{
namespace
{

TEST(MetadataTest_St209450Roundtrip, St209450Roundtrip)
{
    DynamicMetadata metadata;
    metadata.hdr_reference_white = 203;
    metadata.has_adaptive_tone_map_flag = true;
    metadata.baseline_hdr_headroom_log2 = 0.1f;
    metadata.use_reference_white_tone_mapping_flag = false;
    metadata.gain_application_space_chromaticities = DynamicMetadata::kChromaticitiesSrgb;

    metadata.rules.push_back(ToneMappingRule { 1.5f,
                                               { { 0.0f, 0.0f, 0.1f }, { 0.5f, 0.2f, 0.5f }, { 1.0f, 1.0f, 0.9f } },
                                               false,
                                               { { 0.1f, 0.15f, 0.2f }, 0.4f, 1.0f - 0.1f - 0.15f - 0.2f - 0.4f, 0.f } });

    auto st209450_data_or = ToSt209450(metadata);
    ASSERT_TRUE(st209450_data_or.ok());
    std::string st209450_data = *st209450_data_or;

    auto metadata_rt_or = FromSt209450(st209450_data);
    ASSERT_TRUE(metadata_rt_or.ok());
    DynamicMetadata metadata_rt = *metadata_rt_or;

    EXPECT_TRUE(testing::Matches(DynamicMetadataEq(metadata))(metadata_rt));
}

TEST(MetadataTest_St209450Roundtrip, St209450Roundtrip2)
{
    DynamicMetadata metadata;
    metadata.hdr_reference_white = 500; // Non default reference white.
    metadata.has_adaptive_tone_map_flag = true;
    metadata.baseline_hdr_headroom_log2 = 1.5f;
    metadata.use_reference_white_tone_mapping_flag = false;
    metadata.gain_application_space_chromaticities = DynamicMetadata::kChromaticitiesSrgb;

    metadata.rules.push_back(ToneMappingRule { 1.0f,
                                               { { 0.0f, 0.0f, 0.1f }, { 0.5f, -0.2f, 0.5f }, { 1.0f, -1.0f, 0.9f } },
                                               false,
                                               { { 0.1f, 0.15f, 0.2f }, 0.4f, 1.0f - 0.1f - 0.15f - 0.2f - 0.4f, 0.f } });

    auto st209450_data_or = ToSt209450(metadata);
    ASSERT_TRUE(st209450_data_or.ok());
    std::string st209450_data = *st209450_data_or;

    auto metadata_rt_or = FromSt209450(st209450_data);
    ASSERT_TRUE(metadata_rt_or.ok());
    DynamicMetadata metadata_rt = *metadata_rt_or;

    EXPECT_TRUE(testing::Matches(DynamicMetadataEq(metadata))(metadata_rt));
}

TEST(MetadataTest_St209450Roundtrip, InvalidMetadataWrongSign)
{
    DynamicMetadata metadata;
    metadata.hdr_reference_white = 500;
    metadata.has_adaptive_tone_map_flag = true;
    metadata.baseline_hdr_headroom_log2 = 1.5f;
    metadata.use_reference_white_tone_mapping_flag = false;
    metadata.gain_application_space_chromaticities = DynamicMetadata::kChromaticitiesSrgb;

    metadata.rules.push_back(ToneMappingRule { 1.0f,
                                               { { 0.0f, 0.0f, 0.1f }, { 0.5f, -0.2f, 0.5f }, { 1.0f, 1.0f, 0.9f } },
                                               false,
                                               { { 0.1f, 0.15f, 0.2f }, 0.4f, 1.0f - 0.1f - 0.15f - 0.2f - 0.4f, 0.f } });

    auto status_or = ToSt209450(metadata);
    EXPECT_FALSE(status_or.ok());
    EXPECT_EQ(status_or.status().code(), absl::StatusCode::kInvalidArgument);
    EXPECT_TRUE(std::string(status_or.status().message()).find("Sign of y does not match") != std::string::npos);
}

TEST(MetadataTest, FailsOnCurveSize)
{
    constexpr int kNumPoints = 33;
    DynamicMetadata metadata;
    metadata.hdr_reference_white = 500;
    metadata.has_adaptive_tone_map_flag = true;
    metadata.baseline_hdr_headroom_log2 = 1.5f;
    metadata.use_reference_white_tone_mapping_flag = false;
    metadata.gain_application_space_chromaticities = DynamicMetadata::kChromaticitiesSrgb;
    std::vector<ControlPoint> identity_curve(kNumPoints);
    for (int i = 0; i < kNumPoints; ++i) { // 34 points.
        const float x = i / static_cast<float>(kNumPoints - 1);
        identity_curve[i] = { x, 0.0f, 0.0f };
    }
    metadata.rules.push_back(ToneMappingRule { 1.0f, identity_curve, true, { { 0.5f, 0.5f, 0.0f }, 0.0f, 0.0f, 0.f } });

    {
        auto status_or = ToSt209450(metadata);
        EXPECT_FALSE(status_or.ok());
        EXPECT_EQ(status_or.status().code(), absl::StatusCode::kInvalidArgument);
        EXPECT_TRUE(std::string(status_or.status().message()).find("curve size 33 is > 32") != std::string::npos);
    }

    metadata.rules[0].curve.clear();
    {
        auto status_or = ToSt209450(metadata);
        EXPECT_FALSE(status_or.ok());
        EXPECT_EQ(status_or.status().code(), absl::StatusCode::kInvalidArgument);
        EXPECT_TRUE(std::string(status_or.status().message()).find("curve size 0 is > 32 or == 0") != std::string::npos);
    }
}

TEST(MetadataTest, PopulateUsingRwtm)
{
    DynamicMetadata metadata;
    metadata.baseline_hdr_headroom_log2 = 2.0f;
    PopulateUsingRwtm(metadata);

    EXPECT_EQ(metadata.rules.size(), 2);
    EXPECT_EQ(metadata.gain_application_space_chromaticities, DynamicMetadata::kChromaticitiesRec2020);

    // Rule 0 should have alternate_hdr_headroom_log2 = 0.
    EXPECT_FLOAT_EQ(metadata.rules[0].alternate_hdr_headroom_log2, 0.0f);
    EXPECT_EQ(metadata.rules[0].curve.size(), 8);
    EXPECT_NEAR(metadata.rules[0].curve[0].x, 1.0f, 1e-6);

    // Rule 1 should have alternate_hdr_headroom_log2 > 0.
    EXPECT_GT(metadata.rules[1].alternate_hdr_headroom_log2, 0.0f);
}

TEST(MetadataTest, FromSt209450RwtmFlag)
{
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
    DynamicMetadata expected_metadata;
    expected_metadata.hdr_reference_white = 203;
    expected_metadata.has_adaptive_tone_map_flag = true;
    expected_metadata.baseline_hdr_headroom_log2 = 2.0f;
    expected_metadata.use_reference_white_tone_mapping_flag = false;
    expected_metadata.gain_application_space_chromaticities = DynamicMetadata::kChromaticitiesRec2020;
    expected_metadata.rules.push_back(ToneMappingRule { 0.0f,
                                                        { { 1.0f, -0.82291f, 0.0f },
                                                          { 1.18363f, -0.8795f, -0.46552f },
                                                          { 1.44891f, -1.0167f, -0.53445f },
                                                          { 1.79583f, -1.19658f, -0.49583f },
                                                          { 2.22441f, -1.39531f, -0.43254f },
                                                          { 2.73462f, -1.59948f, -0.37056f },
                                                          { 3.32649f, -1.80214f, -0.31706f },
                                                          { 4.0f, -2.0f, -0.27288f } },
                                                        false,
                                                        { { 0.0f, 0.0f, 0.0f }, 1.0f, 0.0f, 0.0f } });
    expected_metadata.rules.push_back(ToneMappingRule { 1.23023f,
                                                        { { 1.0f, 0.0f, 0.0f },
                                                          { 1.27549f, -0.03869f, -0.22983f },
                                                          { 1.60201f, -0.12702f, -0.2943f },
                                                          { 1.97956f, -0.24036f, -0.30047f },
                                                          { 2.40813f, -0.36637f, -0.28607f },
                                                          { 2.88772f, -0.49863f, -0.26543f },
                                                          { 3.41835f, -0.6337f, -0.24415f },
                                                          { 4.0f, -0.76977f, -0.22434f } },
                                                        false,
                                                        { { 0.0f, 0.0f, 0.0f }, 1.0f, 0.0f, 0.0f } });

    EXPECT_TRUE(testing::Matches(DynamicMetadataEq(expected_metadata))(metadata));
}

} // namespace
} // namespace smpte2094_50
