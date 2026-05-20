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

#ifndef LIBSMPTE2094_50_TESTS_TEST_HELPER_H_
#define LIBSMPTE2094_50_TESTS_TEST_HELPER_H_

#include <cmath>
#include <cstddef>

#if defined(BAZEL_BUILD)
#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#else
#include "gmock/gmock-matchers.h"
#endif

namespace smpte2094_50 {

// Extra tolerance added to all float comparisons.
inline constexpr float kExtraTolerance = 1e-6f;

// Component mix matcher.
MATCHER_P(ComponentMixEq, rhs, "") {
  // It should be 1/50000, but because of the approximation of the float as a
  // fraction, we need to add a bit of tolerance.
  constexpr float kMixTolerance = 3.0f / 50000 + kExtraTolerance;
  return testing::ExplainMatchResult(
             testing::Pointwise(testing::FloatNear(kMixTolerance), rhs.rgb),
             arg.rgb, result_listener) &&
         testing::ExplainMatchResult(testing::FloatNear(rhs.max, kMixTolerance),
                                     arg.max, result_listener) &&
         testing::ExplainMatchResult(testing::FloatNear(rhs.min, kMixTolerance),
                                     arg.min, result_listener) &&
         testing::ExplainMatchResult(
             testing::FloatNear(rhs.component, kMixTolerance), arg.component,
             result_listener);
}

// Tone mapping rule matcher.
MATCHER(ToneMappingRuleEq, "") {
  auto& lhs = std::get<0>(arg);
  auto& rhs = std::get<1>(arg);
  if (!testing::ExplainMatchResult(
          testing::FloatNear(rhs.alternate_hdr_headroom_log2,
                             1.0f / 10000 + kExtraTolerance),
          lhs.alternate_hdr_headroom_log2, result_listener)) {
    *result_listener << " at alternate_hdr_headroom_log2";
    return false;
  }
  if (lhs.use_pchip_slope != rhs.use_pchip_slope) {
    *result_listener << "use_pchip_slope mismatch";
    return false;
  }
  if (!testing::ExplainMatchResult(ComponentMixEq(rhs.mix), lhs.mix,
                                   result_listener)) {
    return false;
  }
  if (lhs.curve.size() != rhs.curve.size()) {
    *result_listener << "curve size mismatch";
    return false;
  }
  for (size_t i = 0; i < lhs.curve.size(); ++i) {
    if (!testing::ExplainMatchResult(
            testing::FloatNear(rhs.curve[i].x, 1.0f / 1000 + kExtraTolerance),
            lhs.curve[i].x, result_listener)) {
      *result_listener << " at curve[" << i << "].x";
      return false;
    }
    if (!testing::ExplainMatchResult(
            testing::FloatNear(rhs.curve[i].y, 1.0f / 10000 + kExtraTolerance),
            lhs.curve[i].y, result_listener)) {
      *result_listener << " at curve[" << i << "].y";
      return false;
    }
    if (!lhs.use_pchip_slope) {
      static const float kPi = std::acos(-1.0f);
      if (!testing::ExplainMatchResult(
              testing::FloatNear(std::atan(rhs.curve[i].m),
                                 kPi / 36000 + kExtraTolerance),
              std::atan(lhs.curve[i].m), result_listener)) {
        *result_listener << " at curve[" << i << "].m";
        return false;
      }
    }
  }
  return true;
}

// Main SMPTE 2094-50 matcher.
MATCHER_P(DynamicMetadataEq, other, "") {
  if (arg.has_adaptive_tone_map_flag != other.has_adaptive_tone_map_flag) {
    *result_listener << "has_adaptive_tone_map_flag mismatch";
    return false;
  }

  if (!testing::ExplainMatchResult(
          testing::FloatNear(other.hdr_reference_white,
                             1.0f / 5 + kExtraTolerance),
          arg.hdr_reference_white, result_listener)) {
    *result_listener << " at hdr_reference_white";
    return false;
  }

  // Early exit: do not test the rest of the struct.
  if (!arg.has_adaptive_tone_map_flag) return true;

  if (!testing::ExplainMatchResult(
          testing::FloatNear(other.baseline_hdr_headroom_log2,
                             1.0f / 10000 + kExtraTolerance),
          arg.baseline_hdr_headroom_log2, result_listener)) {
    *result_listener << " at baseline_hdr_headroom_log2";
    return false;
  }
  if (!testing::ExplainMatchResult(
          testing::Pointwise(testing::FloatNear(1.0f / 50000 + kExtraTolerance),
                             other.gain_application_space_chromaticities),
          arg.gain_application_space_chromaticities, result_listener)) {
    return false;
  }
  return testing::ExplainMatchResult(
      testing::Pointwise(ToneMappingRuleEq(), other.rules), arg.rules,
      result_listener);
}

}  // namespace smpte2094_50

#endif  // LIBSMPTE2094_50_TESTS_TEST_HELPER_H_
