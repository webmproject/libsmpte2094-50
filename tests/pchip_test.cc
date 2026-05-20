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

#include "third_party/absl/types/span.h"
#if defined(BAZEL_BUILD)
#include "testing/base/public/gunit.h"
#include "third_party/libsmpte2094_50/pchip_rs.h"
#else
#include "gtest/gtest.h"
#include "pchip_ffi.h"
#endif

namespace smpte2094_50 {
namespace {

constexpr float kEpsilon = 1e-6f;

TEST(PchipSlopesTest, MonotonicIncreasing) {
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 2.0f};
  auto res = pchip_rs::pchip_slopes_ffi(absl::MakeConstSpan(x),
                                        absl::MakeConstSpan(y));
  ASSERT_TRUE(res.success);
  const auto d = res.get_slopes().to_span();

  ASSERT_EQ(d.size(), 3);
  EXPECT_NEAR(d[0], 1.0f, kEpsilon);
  EXPECT_NEAR(d[1], 1.0f, kEpsilon);
  EXPECT_NEAR(d[2], 1.0f, kEpsilon);
}

TEST(PchipSlopesTest, LocalExtrema) {
  // Slope should be zero at local max/min to ensure monotonicity.
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 0.0f};
  auto res = pchip_rs::pchip_slopes_ffi(absl::MakeConstSpan(x),
                                        absl::MakeConstSpan(y));
  ASSERT_TRUE(res.success);
  const auto d = res.get_slopes().to_span();

  ASSERT_EQ(d.size(), 3);
  EXPECT_NEAR(d[1], 0.0f, kEpsilon);
}

TEST(PchipSlopesTest, DuplicatePoints) {
  // Case with zero-length interval in the middle.
  std::vector<float> x = {0.0f, 1.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 1.0f, 2.0f};
  auto res = pchip_rs::pchip_slopes_ffi(absl::MakeConstSpan(x),
                                        absl::MakeConstSpan(y));
  EXPECT_TRUE(res.success);
}

TEST(PchipSlopesTest, Plateau) {
  // Case with a horizontal plateau.
  // Should not happen in SMPTE 2095-50 which has the constraints that
  // if x_i = x_{i+1} then y_i = y_{i+1}.
  std::vector<float> x = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f};
  std::vector<float> y = {0.0f, 1.0f, 1.0f, 1.0f, 2.0f};
  auto res = pchip_rs::pchip_slopes_ffi(absl::MakeConstSpan(x),
                                        absl::MakeConstSpan(y));
  ASSERT_TRUE(res.success);
  const auto d = res.get_slopes().to_span();

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
  auto res = pchip_rs::pchip_slopes_ffi(absl::MakeConstSpan(x),
                                        absl::MakeConstSpan(y));
  EXPECT_TRUE(res.success);
}

TEST(PchipSlopesTest, SmallNumberPoints) {
  // Zero points
  std::vector<float> x0 = {};
  std::vector<float> y0 = {};
  auto res0 = pchip_rs::pchip_slopes_ffi(absl::MakeConstSpan(x0),
                                         absl::MakeConstSpan(y0));
  ASSERT_TRUE(res0.success);
  ASSERT_EQ(res0.get_slopes().size(), 0);

  // Single point
  std::vector<float> x1 = {1.0f};
  std::vector<float> y1 = {5.0f};
  auto res1 = pchip_rs::pchip_slopes_ffi(absl::MakeConstSpan(x1),
                                         absl::MakeConstSpan(y1));
  ASSERT_TRUE(res1.success);
  auto d1 = res1.get_slopes().to_span();
  ASSERT_EQ(d1.size(), 1);
  EXPECT_FLOAT_EQ(d1[0], 0.0f);

  // Two points
  std::vector<float> x2 = {0.0f, 2.0f};
  std::vector<float> y2 = {0.0f, 4.0f};
  auto res2 = pchip_rs::pchip_slopes_ffi(absl::MakeConstSpan(x2),
                                         absl::MakeConstSpan(y2));
  ASSERT_TRUE(res2.success);
  auto d2 = res2.get_slopes().to_span();
  ASSERT_EQ(d2.size(), 2);
  EXPECT_FLOAT_EQ(d2[0], 2.0f);
  EXPECT_FLOAT_EQ(d2[1], 2.0f);
}

TEST(PchipSlopesTest, BoundaryConditions) {
  // Verify three-point finite difference at boundaries.
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 4.0f};
  auto res = pchip_rs::pchip_slopes_ffi(absl::MakeConstSpan(x),
                                        absl::MakeConstSpan(y));
  ASSERT_TRUE(res.success);
  auto d = res.get_slopes().to_span();

  ASSERT_EQ(d.size(), 3);
  EXPECT_NEAR(d[0], 0.0f, kEpsilon);
  EXPECT_NEAR(d[1], 1.5f, kEpsilon);
  EXPECT_NEAR(d[2], 4.0f, kEpsilon);
}

TEST(PchipSlopesTest, UnsortedX) {
  // PCHIP interpolation requires the x values to be strictly increasing.
  std::vector<float> x = {1.0f, 0.0f, 2.0f};
  std::vector<float> y = {1.0f, 0.0f, 2.0f};
  auto res = pchip_rs::pchip_slopes_ffi(absl::MakeConstSpan(x),
                                        absl::MakeConstSpan(y));
  EXPECT_FALSE(res.success);
}

TEST(PchipInterpolatorTest, Interpolation) {
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 0.0f};
  auto res = pchip_rs::PchipInterpolator::create_ffi(absl::MakeConstSpan(x),
                                                     absl::MakeConstSpan(y));
  ASSERT_TRUE(res.success);
  auto& interp = res.interp;

  EXPECT_NEAR(interp.interpolate(0.0f), 0.0f, kEpsilon);
  EXPECT_NEAR(interp.interpolate(1.0f), 1.0f, kEpsilon);
  EXPECT_NEAR(interp.interpolate(2.0f), 0.0f, kEpsilon);
  EXPECT_NEAR(interp.interpolate(0.5f), 0.75f, kEpsilon);
}

TEST(PchipInterpolatorTest, Extrapolation) {
  std::vector<float> x = {1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f};
  auto res = pchip_rs::PchipInterpolator::create_ffi(absl::MakeConstSpan(x),
                                                     absl::MakeConstSpan(y));
  ASSERT_TRUE(res.success);
  auto& interp = res.interp;

  // Left extrapolation (clamped to y[0])
  EXPECT_EQ(interp.interpolate(0.0f), 0.0f);

  // Right extrapolation (clamped to y[n-1])
  EXPECT_EQ(interp.interpolate(4.0f), 1.0f);
}

TEST(PchipInterpolatorTest, ReverseInterpolation) {
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 2.0f};
  auto res = pchip_rs::PchipInterpolator::create_ffi(absl::MakeConstSpan(x),
                                                     absl::MakeConstSpan(y));
  ASSERT_TRUE(res.success);
  auto& interp = res.interp;

  auto res1 = interp.reverse_interpolate(0.5f);
  EXPECT_TRUE(res1.success);
  EXPECT_NEAR(res1.xi, 0.5f, 5e-4f);

  auto res2 = interp.reverse_interpolate(0.0f);
  EXPECT_TRUE(res2.success);
  EXPECT_NEAR(res2.xi, 0.0f, kEpsilon);

  auto res3 = interp.reverse_interpolate(2.0f);
  EXPECT_TRUE(res3.success);
  EXPECT_NEAR(res3.xi, 2.0f, kEpsilon);

  EXPECT_FALSE(interp.reverse_interpolate(-1.0f).success);
  EXPECT_FALSE(interp.reverse_interpolate(3.0f).success);
}

TEST(PchipInterpolatorTest, DecreasingCurve) {
  std::vector<float> x = {0.0f, 1.0f, 2.0f};
  std::vector<float> y = {2.0f, 1.0f, 0.0f};
  auto res = pchip_rs::PchipInterpolator::create_ffi(absl::MakeConstSpan(x),
                                                     absl::MakeConstSpan(y));
  ASSERT_TRUE(res.success);
  auto& interp = res.interp;

  auto res1 = interp.reverse_interpolate(1.5f);
  EXPECT_TRUE(res1.success);
  EXPECT_NEAR(res1.xi, 0.5f, 5e-4f);

  auto res2 = interp.reverse_interpolate(0.0f);
  EXPECT_TRUE(res2.success);
  EXPECT_NEAR(res2.xi, 2.0f, kEpsilon);
}

TEST(GainCurveTest, Extrapolation) {
  std::vector<float> x = {1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f};
  auto res = pchip_rs::GainCurve::create_ffi(absl::MakeConstSpan(x),
                                             absl::MakeConstSpan(y));
  ASSERT_TRUE(res.success);
  auto& curve = res.curve;

  // Left extrapolation (clamped to y[0])
  EXPECT_EQ(curve.interpolate(0.0f), 0.0f);

  // Right extrapolation (logarithmic: y_n + log2(x_n / xi))
  // For xi = 4.0, x_n = 2.0, y_n = 1.0:
  // 1.0 + log2(2.0 / 4.0) = 1.0 + log2(0.5) = 1.0 - 1.0 = 0.0
  EXPECT_NEAR(curve.interpolate(4.0f), 0.0f, kEpsilon);

  // For xi = 8.0: 1.0 + log2(2/8) = 1.0 - 2.0 = -1.0
  EXPECT_NEAR(curve.interpolate(8.0f), -1.0f, kEpsilon);
}

TEST(GainCurveTest, ReverseInterpolation) {
  std::vector<float> x = {1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f};
  auto res = pchip_rs::GainCurve::create_ffi(absl::MakeConstSpan(x),
                                             absl::MakeConstSpan(y));
  ASSERT_TRUE(res.success);
  auto& curve = res.curve;

  // Test value within range
  auto res1 = curve.reverse_interpolate(0.5f);
  EXPECT_TRUE(res1.success);
  EXPECT_NEAR(res1.xi, 1.5f, 5e-4f);

  // Test value in log tail
  // yi = -1.0 => xi = x_n * exp2(y_n - yi) = 2.0 * exp2(1.0 - (-1.0)) = 2 * 4 =
  // 8
  auto res2 = curve.reverse_interpolate(-1.0f);
  EXPECT_TRUE(res2.success);
  EXPECT_NEAR(res2.xi, 8.0f, kEpsilon);

  // Test below range but y_back is NOT the minimum. (Hypothetically)
  std::vector<float> x2 = {1.0f, 2.0f, 3.0f};
  std::vector<float> y2 = {2.0f, 0.0f, 1.0f};
  auto res3 = pchip_rs::GainCurve::create_ffi(absl::MakeConstSpan(x2),
                                              absl::MakeConstSpan(y2));
  ASSERT_TRUE(res3.success);
  auto& curve2 = res3.curve;
  // y_min is 0.0, y_back is 1.0.
  // If yi = -1.0, it is < y_min and < y_back.
  // xi = 3.0 * exp2(1.0 - (-1.0)) = 3.0 * 4 = 12.0
  auto res4 = curve2.reverse_interpolate(-1.0f);
  EXPECT_TRUE(res4.success);
  EXPECT_NEAR(res4.xi, 12.0f, kEpsilon);
}

TEST(PchipTest, GetPiecewiseCubicEqualPoints) {
  std::vector<float> x = {0.0f, 1.0f, 1.0f, 2.0f};
  std::vector<float> y = {0.0f, 1.0f, 1.0f, 2.0f};
  auto res = pchip_rs::create_subsampled_pchip_ffi(absl::MakeConstSpan(x),
                                                   absl::MakeConstSpan(y), 3);
  ASSERT_TRUE(res.success);
  const auto& interp = res.interp;
  const auto x_cp = interp.x().to_span();
  const auto y_cp = interp.y().to_span();
  const auto m_cp = interp.slopes().to_span();

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
