# Copyright 2026 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

cmake_minimum_required(VERSION 3.23)

project(libsmpte2094_50 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SMPTE2094_ENABLE_TESTS "Enable tests." ON)
option(SMPTE2094_ENABLE_WERROR "Enable -Werror." ON)

set(RUST_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rust_lib")

include(FetchContent)

# Install dependencies.
set(SMPTE2094_ABSL_GIT_TAG 20260107.1)
set(SMPTE2094_CORROSION_GIT_TAG v0.6.1)
set(SMPTE2094_CBINDGEN_GIT_TAG v0.29.2)

FetchContent_Declare(
  abseil-cpp
  EXCLUDE_FROM_ALL
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG ${SMPTE2094_ABSL_GIT_TAG}
  GIT_SHALLOW ON)
FetchContent_MakeAvailable(abseil-cpp)

FetchContent_Declare(
  Corrosion
  EXCLUDE_FROM_ALL
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG ${SMPTE2094_CORROSION_GIT_TAG}
  GIT_SHALLOW ON)
FetchContent_MakeAvailable(Corrosion)

# Install cbindgen.
find_program(
  CBINDGEN cbindgen
  HINTS "$ENV{HOME}/.cargo/bin"
  HINTS ${CMAKE_BINARY_DIR})

if(CBINDGEN)
  add_executable(cbindgen IMPORTED GLOBAL)
  set_property(TARGET cbindgen PROPERTY IMPORTED_LOCATION ${CBINDGEN})
endif()

if(NOT TARGET cbindgen)
  FetchContent_Declare(
    cbindgen_repo
    EXCLUDE_FROM_ALL
    GIT_REPOSITORY https://github.com/mozilla/cbindgen.git
    GIT_TAG ${SMPTE2094_CBINDGEN_GIT_TAG}
    GIT_SHALLOW ON)
  FetchContent_MakeAvailable(cbindgen_repo)

  corrosion_import_crate(MANIFEST_PATH ${cbindgen_repo_SOURCE_DIR}/Cargo.toml
                         PROFILE release)

  set(CBINDGEN $<TARGET_FILE:cbindgen>)
endif()

set(GENERATED_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated_headers")
set(TARGET_INCLUDE_SUBDIR "${GENERATED_HEADERS_DIR}/libsmpte2094_50")
file(MAKE_DIRECTORY ${TARGET_INCLUDE_SUBDIR})

add_custom_target(
  smpte2094_50_generated_headers
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/rust_lib
    ${CMAKE_CURRENT_BINARY_DIR}/cbindgen_src
  COMMAND sed -i.bak "s/unsafe(no_mangle)/no_mangle/g"
          ${CMAKE_CURRENT_BINARY_DIR}/cbindgen_src/pchip_rs/src/capi.rs
  COMMAND sed -i.bak "s/unsafe(no_mangle)/no_mangle/g"
          ${CMAKE_CURRENT_BINARY_DIR}/cbindgen_src/utils_rs/src/capi.rs
  COMMAND
    ${CBINDGEN} --config
    ${CMAKE_CURRENT_BINARY_DIR}/cbindgen_src/pchip_rs/cbindgen.toml --crate
    pchip_rs --output ${TARGET_INCLUDE_SUBDIR}/pchip_rs.h
    ${CMAKE_CURRENT_BINARY_DIR}/cbindgen_src/pchip_rs
  COMMAND
    ${CBINDGEN} --config
    ${CMAKE_CURRENT_BINARY_DIR}/cbindgen_src/utils_rs/cbindgen.toml --crate
    utils_rs --output ${TARGET_INCLUDE_SUBDIR}/utils_rs.h
    ${CMAKE_CURRENT_BINARY_DIR}/cbindgen_src/utils_rs
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM)

# Compile the Rust Library.
corrosion_import_crate(MANIFEST_PATH
                       ${CMAKE_CURRENT_SOURCE_DIR}/rust_lib/Cargo.toml)

add_custom_target(
  generate_include_links ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${CMAKE_CURRENT_BINARY_DIR}/virtual_includes/libsmpte2094_50"
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink
    "${CMAKE_CURRENT_SOURCE_DIR}/include/smpte2094_50"
    "${CMAKE_CURRENT_BINARY_DIR}/virtual_includes/libsmpte2094_50/smpte2094_50"
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink
    "${CMAKE_CURRENT_BINARY_DIR}/_deps/abseil-cpp-src/absl"
    "${CMAKE_CURRENT_BINARY_DIR}/virtual_includes/absl")

# -----------------------------------------------------------------------------
# Library Definition
# -----------------------------------------------------------------------------
add_library(smpte2094_50 INTERFACE include/smpte2094_50/smpte2094_50.h)
target_include_directories(
  smpte2094_50 INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                         $<INSTALL_INTERFACE:include>)

add_library(smpte2094_50_utils src/utils.cc include/smpte2094_50/utils.h)
add_dependencies(smpte2094_50_utils smpte2094_50_generated_headers
                 generate_include_links)

target_link_libraries(
  smpte2094_50_utils
  PUBLIC absl::status
         absl::statusor
         absl::strings
         absl::span
         smpte2094_50
         pchip_rs
         utils_rs # Use native Corrosion imported static libraries directly
)
target_include_directories(
  smpte2094_50_utils
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${GENERATED_HEADERS_DIR}>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/virtual_includes>
         $<INSTALL_INTERFACE:include>)

if(SMPTE2094_ENABLE_WERROR)
  # Warnings as errors
  if(MSVC)
    target_compile_options(smpte2094_50_utils INTERFACE /WX)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES
                                                  "GNU")
    target_compile_options(smpte2094_50_utils INTERFACE -Werror)
  else()
    message(FATAL_ERROR "libsmpte2094-50: Unknown compiler, bailing out")
  endif()
endif()

if(SMPTE2094_ENABLE_TESTS)
  enable_testing() # Allow ctest to be called from top-level directory.
  add_subdirectory(tests)
endif()
