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

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "third_party/absl/strings/string_view.h"
#include "third_party/absl/types/span.h"
#include "third_party/libsmpte2094_50/utils_rs.h"

namespace utils_rs
{

using ControlPoint = utils_ffi::ControlPoint;

template <typename T>
struct SpanWrapper
{
    absl::Span<const T> span_;
    absl::Span<const T> to_span() const { return span_; }
    const T * data() const { return span_.data(); }
    size_t size() const { return span_.size(); }
};

class ToneMappingRule : public utils_ffi::ToneMappingRuleFfi
{
public:
    bool owned_;

    ToneMappingRule() : owned_(true)
    {
        alternate_hdr_headroom_log2 = 0.0f;
        curve = nullptr;
        curve_len = 0;
        curve_cap = 0;
        use_pchip_slope = false;
        mix = utils_ffi::ComponentMix {};
    }

    ToneMappingRule(const utils_ffi::ToneMappingRuleFfi & ffi, bool owned = false) : ToneMappingRuleFfi(ffi), owned_(owned) {}

    ~ToneMappingRule()
    {
        if (owned_) {
            ::utils_ffi::tone_mapping_rule_free(*this);
        }
        curve = nullptr;
        curve_len = 0;
        curve_cap = 0;
    }

    ToneMappingRule(ToneMappingRule && other) noexcept : utils_ffi::ToneMappingRuleFfi(other), owned_(other.owned_)
    {
        other.owned_ = false;
        other.curve = nullptr;
        other.curve_len = 0;
        other.curve_cap = 0;
    }

    ToneMappingRule & operator=(ToneMappingRule && other) noexcept
    {
        if (this != &other) {
            if (owned_) {
                ::utils_ffi::tone_mapping_rule_free(*this);
            }
            utils_ffi::ToneMappingRuleFfi::operator=(other);
            owned_ = other.owned_;
            other.owned_ = false;
            other.curve = nullptr;
            other.curve_len = 0;
            other.curve_cap = 0;
        }
        return *this;
    }

    ToneMappingRule(const ToneMappingRule & other) = delete;
    ToneMappingRule & operator=(const ToneMappingRule & other) = delete;

    void add_point(ControlPoint point) { ::utils_ffi::tone_mapping_rule_add_point_ffi(this, point); }

    SpanWrapper<ControlPoint> get_curve() const { return SpanWrapper<ControlPoint> { absl::MakeConstSpan(curve, curve_len) }; }
};

class DynamicMetadata : public utils_ffi::DynamicMetadataFfi
{
public:
    bool owned_;
    mutable std::vector<ToneMappingRule> rules_;

    DynamicMetadata() : owned_(true)
    {
        has_adaptive_tone_map_flag = false;
        use_reference_white_tone_mapping_flag = false;
        hdr_reference_white = 0.0f;
        baseline_hdr_headroom_log2 = 0.0f;
        for (int i = 0; i < 8; ++i)
            gain_application_space_chromaticities[i] = 0.0f;
        rules = nullptr;
        rules_len = 0;
        rules_cap = 0;
    }

    DynamicMetadata(const utils_ffi::DynamicMetadataFfi & ffi, bool owned = false)
        : utils_ffi::DynamicMetadataFfi(ffi), owned_(owned)
    {
    }

    ~DynamicMetadata()
    {
        if (owned_) {
            ::utils_ffi::dynamic_metadata_free(*this);
        }
        rules = nullptr;
        rules_len = 0;
        rules_cap = 0;
    }

    DynamicMetadata(DynamicMetadata && other) noexcept : utils_ffi::DynamicMetadataFfi(other), owned_(other.owned_)
    {
        other.owned_ = false;
        other.rules = nullptr;
        other.rules_len = 0;
        other.rules_cap = 0;
    }

    DynamicMetadata & operator=(DynamicMetadata && other) noexcept
    {
        if (this != &other) {
            if (owned_) {
                ::utils_ffi::dynamic_metadata_free(*this);
            }
            utils_ffi::DynamicMetadataFfi::operator=(other);
            owned_ = other.owned_;
            other.owned_ = false;
            other.rules = nullptr;
            other.rules_len = 0;
            other.rules_cap = 0;
        }
        return *this;
    }

    DynamicMetadata(const DynamicMetadata & other) = delete;
    DynamicMetadata & operator=(const DynamicMetadata & other) = delete;

    void add_rule(ToneMappingRule & rule)
    {
        ::utils_ffi::dynamic_metadata_add_rule_ffi(this, rule);
        rule.owned_ = false;
    }

    SpanWrapper<ToneMappingRule> get_rules() const
    {
        rules_.clear();
        rules_.reserve(rules_len);
        for (int i = 0; i < rules_len; ++i) {
            rules_.emplace_back(rules[i], false);
        }
        return SpanWrapper<ToneMappingRule> { absl::MakeConstSpan(rules_.data(), rules_.size()) };
    }
};

class ToSt209450Result : public utils_ffi::ToSt209450ResultFfi
{
public:
    ToSt209450Result()
    {
        success = false;
        data = nullptr;
        data_len = 0;
        data_cap = 0;
        error_message = nullptr;
    }

    ToSt209450Result(const utils_ffi::ToSt209450ResultFfi & ffi) : utils_ffi::ToSt209450ResultFfi(ffi) {}

    ~ToSt209450Result()
    {
        ::utils_ffi::to_st209450_result_free(*this);
        data = nullptr;
        data_len = 0;
        data_cap = 0;
        error_message = nullptr;
    }

    ToSt209450Result(ToSt209450Result && other) noexcept : ToSt209450ResultFfi(other)
    {
        other.data = nullptr;
        other.data_len = 0;
        other.data_cap = 0;
        other.error_message = nullptr;
    }

    ToSt209450Result & operator=(ToSt209450Result && other) noexcept
    {
        if (this != &other) {
            ::utils_ffi::to_st209450_result_free(*this);
            ToSt209450ResultFfi::operator=(other);
            other.data = nullptr;
            other.data_len = 0;
            other.data_cap = 0;
            other.error_message = nullptr;
        }
        return *this;
    }

    ToSt209450Result(const ToSt209450Result & other) = delete;
    ToSt209450Result & operator=(const ToSt209450Result & other) = delete;

    SpanWrapper<uint8_t> get_data() const { return SpanWrapper<uint8_t> { absl::MakeConstSpan(data, data_len) }; }

    absl::string_view get_error_message() const { return error_message ? absl::string_view(error_message) : absl::string_view(); }
};

class FromSt209450Result : public utils_ffi::FromSt209450ResultFfi
{
public:
    FromSt209450Result()
    {
        success = false;
        metadata = utils_ffi::DynamicMetadataFfi {};
        error_message = nullptr;
    }

    FromSt209450Result(const utils_ffi::FromSt209450ResultFfi & ffi) : FromSt209450ResultFfi(ffi) {}

    ~FromSt209450Result()
    {
        ::utils_ffi::from_st209450_result_free(*this);
        metadata.rules = nullptr;
        metadata.rules_len = 0;
        metadata.rules_cap = 0;
        error_message = nullptr;
    }

    FromSt209450Result(FromSt209450Result && other) noexcept : FromSt209450ResultFfi(other)
    {
        other.metadata.rules = nullptr;
        other.metadata.rules_len = 0;
        other.metadata.rules_cap = 0;
        other.error_message = nullptr;
    }

    FromSt209450Result & operator=(FromSt209450Result && other) noexcept
    {
        if (this != &other) {
            ::utils_ffi::from_st209450_result_free(*this);
            FromSt209450ResultFfi::operator=(other);
            other.metadata.rules = nullptr;
            other.metadata.rules_len = 0;
            other.metadata.rules_cap = 0;
            other.error_message = nullptr;
        }
        return *this;
    }

    FromSt209450Result(const FromSt209450Result & other) = delete;
    FromSt209450Result & operator=(const FromSt209450Result & other) = delete;

    absl::string_view get_error_message() const { return error_message ? absl::string_view(error_message) : absl::string_view(); }
};

class SimpleResult : public utils_ffi::SimpleResultFfi
{
public:
    SimpleResult()
    {
        success = false;
        error_message = nullptr;
    }

    SimpleResult(const utils_ffi::SimpleResultFfi & ffi) : utils_ffi::SimpleResultFfi(ffi) {}

    ~SimpleResult()
    {
        ::utils_ffi::simple_result_free(*this);
        error_message = nullptr;
    }

    SimpleResult(SimpleResult && other) noexcept : SimpleResultFfi(other) { other.error_message = nullptr; }

    SimpleResult & operator=(SimpleResult && other) noexcept
    {
        if (this != &other) {
            ::utils_ffi::simple_result_free(*this);
            SimpleResultFfi::operator=(other);
            other.error_message = nullptr;
        }
        return *this;
    }

    SimpleResult(const SimpleResult & other) = delete;
    SimpleResult & operator=(const SimpleResult & other) = delete;

    absl::string_view get_error_message() const { return error_message ? absl::string_view(error_message) : absl::string_view(); }
};

inline ToSt209450Result to_st209450_ffi(const DynamicMetadata & metadata)
{
    utils_ffi::ToSt209450ResultFfi ffi_res = ::utils_ffi::to_st209450_ffi(&metadata);
    return ToSt209450Result(ffi_res);
}

inline FromSt209450Result from_st209450_ffi(absl::Span<const uint8_t> data)
{
    utils_ffi::FromSt209450ResultFfi ffi_res = ::utils_ffi::from_st209450_ffi(data.data(), data.size());
    return FromSt209450Result(ffi_res);
}

inline SimpleResult dynamic_metadata_populate_pchip_slopes_ffi(DynamicMetadata & metadata)
{
    utils_ffi::SimpleResultFfi ffi_res = ::utils_ffi::dynamic_metadata_populate_pchip_slopes_ffi(&metadata);
    return SimpleResult(ffi_res);
}

inline SimpleResult populate_implicit_parameters_ffi(DynamicMetadata & metadata)
{
    utils_ffi::SimpleResultFfi ffi_res = ::utils_ffi::populate_implicit_parameters_ffi(&metadata);
    return SimpleResult(ffi_res);
}

inline void dynamic_metadata_populate_using_rwtm(DynamicMetadata & metadata)
{
    ::utils_ffi::dynamic_metadata_populate_using_rwtm_ffi(&metadata);
}

inline bool is_valid_ffi(const DynamicMetadata & metadata)
{
    return ::utils_ffi::is_valid_ffi(&metadata);
}

} // namespace utils_rs

#endif // THIRD_PARTY_LIBSMPTE2094_50_SRC_UTILS_FFI_H_
