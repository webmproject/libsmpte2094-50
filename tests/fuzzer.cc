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

#include <cstdlib>
#include <iostream>
#include <string_view>

#include "absl/status/statusor.h"
#include "smpte2094_50/smpte2094_50.h"
#include "smpte2094_50/utils.h"
#include "test_helper.h"
#include "testing/base/public/gmock.h"
#include "testing/fuzzing/fuzztest.h"

namespace smpte2094_50 {
namespace {

// TODO(vrabaud) add a serialization fuzz test with a DynamicMetadata
// generator.
void DynamicMetadataSerializationFuzzTest(const std::string_view buffer) {
  // Generate a random DynamicMetadata instance by reading from a random
  // buffer.
  absl::StatusOr<DynamicMetadata> metadata_init = FromSt209450(buffer);
  if (!metadata_init.ok()) return;
  const DynamicMetadata metadata = metadata_init.value();

  // Serialize the metadata.
  absl::StatusOr<std::string> serialized = ToSt209450(metadata);
  if (!serialized.ok()) {
    std::cerr << "Serialization failed: " << serialized.status() << "\n";
    std::abort();
  }

  // Deserialize the serialized data.
  absl::StatusOr<DynamicMetadata> deserialized = FromSt209450(*serialized);
  if (!deserialized.ok()) {
    std::cerr << "Deserialization failed: " << deserialized.status() << "\n";
    std::abort();
  }

  if (IsValid(metadata_init.value())) {
    EXPECT_TRUE(IsValid(deserialized.value()));
  }

  // Verify they match.
  EXPECT_THAT(metadata, DynamicMetadataEq(*deserialized));
}

// Register the fuzz test.
FUZZ_TEST(Smpte2094Fuzzer, DynamicMetadataSerializationFuzzTest);

}  // namespace
}  // namespace smpte2094_50
