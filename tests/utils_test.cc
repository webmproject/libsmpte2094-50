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

#include <vector>

#include "gtest/gtest.h"
#include "smpte2094_50/smpte2094_50.h"

namespace smpte2094_50 {
namespace {

TEST(PopulatePchipSlopesTest, PopulatesSlopes) {
  DynamicMetadata metadata;
  ToneMappingRule rule;
  rule.use_pchip_slope = true;
  rule.curve = {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {2.0f, 0.0f, 0.0f}};
  metadata.rules.push_back(rule);

  EXPECT_TRUE(PopulatePchipSlopes(metadata).ok());

  EXPECT_FALSE(metadata.rules[0].use_pchip_slope);
  ASSERT_EQ(metadata.rules[0].curve.size(), 3);

  // Slopes for {0,1,2}, {0,1,0} are {2, 0, -2}
  EXPECT_NEAR(metadata.rules[0].curve[0].m, 2.0f, 1e-6f);
  EXPECT_NEAR(metadata.rules[0].curve[1].m, 0.0f, 1e-6f);
  EXPECT_NEAR(metadata.rules[0].curve[2].m, -2.0f, 1e-6f);
}

TEST(PopulateImplicitParametersTest, PopulatesRwtm) {
  DynamicMetadata metadata;
  metadata.baseline_hdr_headroom_log2 = 2.0f;
  metadata.use_reference_white_tone_mapping_flag = true;

  // Pre-existing rule (will be cleared).
  ToneMappingRule rule;
  rule.use_pchip_slope = true;
  rule.curve = {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {2.0f, 0.0f, 0.0f}};
  metadata.rules.push_back(rule);

  EXPECT_TRUE(PopulateImplicitParameters(metadata).ok());

  // If use_reference_white_tone_mapping_flag is true, it overrides everything.
  EXPECT_EQ(metadata.rules.size(), 2);
  EXPECT_FALSE(metadata.rules[0].use_pchip_slope);
  EXPECT_FALSE(metadata.rules[1].use_pchip_slope);
}

TEST(PopulateImplicitParametersTest, PopulatesPchip) {
  DynamicMetadata metadata;
  metadata.use_reference_white_tone_mapping_flag = false;

  ToneMappingRule rule;
  rule.use_pchip_slope = true;
  rule.curve = {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {2.0f, 0.0f, 0.0f}};
  metadata.rules.push_back(rule);

  EXPECT_TRUE(PopulateImplicitParameters(metadata).ok());

  EXPECT_EQ(metadata.rules.size(), 1);
  EXPECT_FALSE(metadata.rules[0].use_pchip_slope);
  EXPECT_NEAR(metadata.rules[0].curve[1].m, 0.0f, 1e-6f);
}

}  // namespace
}  // namespace smpte2094_50
