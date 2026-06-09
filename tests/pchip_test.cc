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

#include <vector>

#include "absl/types/span.h"
#include "gtest/gtest.h"
#include "smpte2094_50/pchip.h"

namespace smpte2094_50 {
namespace {

constexpr float kEpsilon = 1e-6f;

TEST(PchipSlopesTest, MonotonicIncreasing) {
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 2.0f};
  auto res = ComputePchipSlopes(x, y);
  ASSERT_TRUE(res.ok());
  const auto& d = *res;

  ASSERT_EQ(d.size(), 3);
  EXPECT_NEAR(d[0], 1.0f, kEpsilon);
  EXPECT_NEAR(d[1], 1.0f, kEpsilon);
  EXPECT_NEAR(d[2], 1.0f, kEpsilon);
}

TEST(PchipSlopesTest, LocalExtrema) {
  // Slope should be zero at local max/min to ensure monotonicity.
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 0.0f};
  auto res = ComputePchipSlopes(x, y);
  ASSERT_TRUE(res.ok());
  const auto& d = *res;

  ASSERT_EQ(d.size(), 3);
  EXPECT_NEAR(d[1], 0.0f, kEpsilon);
}

TEST(PchipSlopesTest, DuplicatePoints) {
  // Case with zero-length interval in the middle.
  std::vector<float> x = {0.0f, 1.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 1.0f, 2.0f};
  auto res = ComputePchipSlopes(x, y);
  EXPECT_TRUE(res.ok());
}

TEST(PchipSlopesTest, Plateau) {
  // Case with a horizontal plateau.
  // Should not happen in SMPTE 2095-50 which has the constraints that
  // if x_i = x_{i+1} then y_i = y_{i+1}.
  std::vector<float> x = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f};
  std::vector<float> y = {0.0f, 1.0f, 1.0f, 1.0f, 2.0f};
  auto res = ComputePchipSlopes(x, y);
  ASSERT_TRUE(res.ok());
  const auto& d = *res;

  ASSERT_EQ(d.size(), 5);
  EXPECT_NEAR(d[0], 1.5f, kEpsilon);
  EXPECT_NEAR(d[1], 0.0f, kEpsilon);
  EXPECT_NEAR(d[2], 0.0f, kEpsilon);
  EXPECT_NEAR(d[3], 0.0f, kEpsilon);
  EXPECT_NEAR(d[4], 1.5f, kEpsilon);
}

TEST(PchipSlopesTest, DuplicatePointsHorizontal) {
  // Case where the segment after duplicate points is horizontal.
  std::vector<float> x = {0.0f, 1.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 2.0f, 2.0f};
  auto res = ComputePchipSlopes(x, y);
  EXPECT_TRUE(res.ok());
}

TEST(PchipSlopesTest, SmallNumberPoints) {
  // Zero points
  std::vector<float> x0 = {};
  std::vector<float> y0 = {};
  auto res0 = ComputePchipSlopes(x0, y0);
  ASSERT_TRUE(res0.ok());
  ASSERT_EQ(res0->size(), 0);

  // Single point
  std::vector<float> x1 = {1.0f};
  std::vector<float> y1 = {5.0f};
  auto res1 = ComputePchipSlopes(x1, y1);
  ASSERT_TRUE(res1.ok());
  auto d1 = *res1;
  ASSERT_EQ(d1.size(), 1);
  EXPECT_FLOAT_EQ(d1[0], 0.0f);

  // Two points
  std::vector<float> x2 = {0.0f, 2.0f};
  std::vector<float> y2 = {0.0f, 4.0f};
  auto res2 = ComputePchipSlopes(x2, y2);
  ASSERT_TRUE(res2.ok());
  auto d2 = *res2;
  ASSERT_EQ(d2.size(), 2);
  EXPECT_FLOAT_EQ(d2[0], 2.0f);
  EXPECT_FLOAT_EQ(d2[1], 2.0f);
}

TEST(PchipSlopesTest, BoundaryConditions) {
  // Verify three-point finite difference at boundaries.
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 4.0f};
  auto res = ComputePchipSlopes(x, y);
  ASSERT_TRUE(res.ok());
  auto d = *res;

  ASSERT_EQ(d.size(), 3);
  EXPECT_NEAR(d[0], 0.0f, kEpsilon);
  EXPECT_NEAR(d[1], 1.5f, kEpsilon);
  EXPECT_NEAR(d[2], 4.0f, kEpsilon);
}

TEST(PchipSlopesTest, UnsortedX) {
  // PCHIP interpolation requires the x values to be strictly increasing.
  std::vector<float> x = {1.0f, 0.0f, 2.0f};
  std::vector<float> y = {1.0f, 0.0f, 2.0f};
  auto res = ComputePchipSlopes(x, y);
  EXPECT_FALSE(res.ok());
}

TEST(PchipInterpolatorTest, Interpolation) {
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 0.0f};
  auto res = PchipInterpolator::Create(x, y);
  ASSERT_TRUE(res.ok());
  auto& interp = *res;

  EXPECT_NEAR(interp.Interpolate(0.0f), 0.0f, kEpsilon);
  EXPECT_NEAR(interp.Interpolate(1.0f), 1.0f, kEpsilon);
  EXPECT_NEAR(interp.Interpolate(2.0f), 0.0f, kEpsilon);
  EXPECT_NEAR(interp.Interpolate(0.5f), 0.75f, kEpsilon);
}

TEST(PchipInterpolatorTest, Extrapolation) {
  std::vector<float> x = {1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f};
  auto res = PchipInterpolator::Create(x, y);
  ASSERT_TRUE(res.ok());
  auto& interp = *res;

  // Left extrapolation (clamped to y[0])
  EXPECT_EQ(interp.Interpolate(0.0f), 0.0f);

  // Right extrapolation (clamped to y[n-1])
  EXPECT_EQ(interp.Interpolate(4.0f), 1.0f);
}

TEST(PchipInterpolatorTest, ReverseInterpolation) {
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 2.0f};
  auto res = PchipInterpolator::Create(x, y);
  ASSERT_TRUE(res.ok());
  auto& interp = *res;

  auto res1 = interp.ReverseInterpolate(0.5f);
  EXPECT_TRUE(res1.ok());
  EXPECT_NEAR(*res1, 0.5f, 5e-4f);

  auto res2 = interp.ReverseInterpolate(0.0f);
  EXPECT_TRUE(res2.ok());
  EXPECT_NEAR(*res2, 0.0f, kEpsilon);

  auto res3 = interp.ReverseInterpolate(2.0f);
  EXPECT_TRUE(res3.ok());
  EXPECT_NEAR(*res3, 2.0f, kEpsilon);

  EXPECT_FALSE(interp.ReverseInterpolate(-1.0f).ok());
  EXPECT_FALSE(interp.ReverseInterpolate(3.0f).ok());
}

TEST(PchipInterpolatorTest, DecreasingCurve) {
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {2.0f, 1.0f, 0.0f};
  auto res = PchipInterpolator::Create(x, y);
  ASSERT_TRUE(res.ok());
  auto& interp = *res;

  auto res1 = interp.ReverseInterpolate(1.5f);
  EXPECT_TRUE(res1.ok());
  EXPECT_NEAR(*res1, 0.5f, 5e-4f);

  auto res2 = interp.ReverseInterpolate(0.0f);
  EXPECT_TRUE(res2.ok());
  EXPECT_NEAR(*res2, 2.0f, kEpsilon);
}

TEST(GainCurveTest, Extrapolation) {
  std::vector<float> x = {1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f};
  auto res = GainCurve::Create(x, y);
  ASSERT_TRUE(res.ok());
  auto& curve = *res;

  // Left extrapolation (clamped to y[0])
  EXPECT_EQ(curve.Interpolate(0.0f), 0.0f);

  // Right extrapolation (logarithmic: y_n + log2(x_n / xi))
  // For xi = 4.0, x_n = 2.0, y_n = 1.0:
  // 1.0 + log2(2.0 / 4.0) = 1.0 + log2(0.5) = 1.0 - 1.0 = 0.0
  EXPECT_NEAR(curve.Interpolate(4.0f), 0.0f, kEpsilon);

  // For xi = 8.0: 1.0 + log2(2/8) = 1.0 - 2.0 = -1.0
  EXPECT_NEAR(curve.Interpolate(8.0f), -1.0f, kEpsilon);
}

TEST(GainCurveTest, ReverseInterpolation) {
  std::vector<float> x = {1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f};
  auto res = GainCurve::Create(x, y);
  ASSERT_TRUE(res.ok());
  auto& curve = *res;

  // Test value within range
  auto res1 = curve.ReverseInterpolate(0.5f);
  EXPECT_TRUE(res1.ok());
  EXPECT_NEAR(*res1, 1.5f, 5e-4f);

  // Test value in log tail
  // yi = -1.0 => xi = x_n * exp2(y_n - yi) = 2.0 * exp2(1.0 - (-1.0)) = 2 * 4 =
  // 8
  auto res2 = curve.ReverseInterpolate(-1.0f);
  EXPECT_TRUE(res2.ok());
  EXPECT_NEAR(*res2, 8.0f, kEpsilon);

  // Test below range but y_back is NOT the minimum. (Hypothetically)
  std::vector<float> x2 = {1.0f, 2.0f, 3.0f};
  std::vector<float> y2 = {2.0f, 0.0f, 1.0f};
  auto res3 = GainCurve::Create(x2, y2);
  ASSERT_TRUE(res3.ok());
  auto& curve2 = *res3;
  // y_min is 0.0, y_back is 1.0.
  // If yi = -1.0, it is < y_min and < y_back.
  // xi = 3.0 * exp2(1.0 - (-1.0)) = 3.0 * 4 = 12.0
  auto res4 = curve2.ReverseInterpolate(-1.0f);
  EXPECT_TRUE(res4.ok());
  EXPECT_NEAR(*res4, 12.0f, kEpsilon);
}

TEST(PchipTest, GetPiecewiseCubicEqualPoints) {
  std::vector<float> x = {0.0f, 1.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 1.0f, 2.0f};
  auto res = CreateSubsampledPchip(x, y, 3);
  ASSERT_TRUE(res.ok());
  const auto& interp = *res;
  const auto x_cp = interp.x();
  const auto y_cp = interp.y();
  const auto m_cp = interp.slopes();

  ASSERT_EQ(x_cp.size(), 3);
  EXPECT_FLOAT_EQ(x_cp[0], 0.0f);
  EXPECT_FLOAT_EQ(x_cp[1], 1.0f);
  EXPECT_FLOAT_EQ(x_cp[2], 2.0f);

  EXPECT_FLOAT_EQ(y_cp[0], 0.0f);
  EXPECT_FLOAT_EQ(y_cp[1], 1.0f);
  EXPECT_FLOAT_EQ(y_cp[2], 2.0f);

  ASSERT_EQ(m_cp.size(), 3);
  EXPECT_NEAR(m_cp[0], 1.0f, kEpsilon);
  EXPECT_NEAR(m_cp[1], 1.0f, kEpsilon);
  EXPECT_NEAR(m_cp[2], 1.0f, kEpsilon);
}

}  // namespace
}  // namespace smpte2094_50
