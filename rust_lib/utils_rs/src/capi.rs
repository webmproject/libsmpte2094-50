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
// Easy to C-bind wrappers.
use crate::{ComponentMix, ControlPoint, DynamicMetadata, ToneMappingRule};
use std::ffi::{c_char, CString};

#[derive(Clone, Copy, Default)]
#[repr(C)]
pub struct ToneMappingRuleFfi {
    pub alternate_hdr_headroom_log2: f32,
    pub curve: *mut ControlPoint,
    pub curve_len: usize,
    pub curve_cap: usize,
    pub use_pchip_slope: bool,
    pub mix: ComponentMix,
}

impl ToneMappingRuleFfi {
    pub fn get_curve(&self) -> &[ControlPoint] {
        if self.curve.is_null() || self.curve_len == 0 {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(self.curve, self.curve_len) }
        }
    }
    pub fn add_point(&mut self, point: ControlPoint) {
        let mut vec = if self.curve.is_null() {
            Vec::new()
        } else {
            unsafe { Vec::from_raw_parts(self.curve, self.curve_len, self.curve_cap) }
        };
        vec.push(point);
        self.curve = vec.as_mut_ptr();
        self.curve_len = vec.len();
        self.curve_cap = vec.capacity();
        std::mem::forget(vec);
    }
}

#[derive(Clone, Copy, Default)]
#[repr(C)]
pub struct DynamicMetadataFfi {
    pub has_adaptive_tone_map_flag: bool,
    pub use_reference_white_tone_mapping_flag: bool,
    pub hdr_reference_white: f32,
    pub baseline_hdr_headroom_log2: f32,
    pub gain_application_space_chromaticities: [f32; 8],
    pub rules: *mut ToneMappingRuleFfi,
    pub rules_len: usize,
    pub rules_cap: usize,
}

impl DynamicMetadataFfi {
    pub fn get_rules(&self) -> &[ToneMappingRuleFfi] {
        if self.rules.is_null() || self.rules_len == 0 {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(self.rules, self.rules_len) }
        }
    }
    pub fn add_rule(&mut self, rule: ToneMappingRuleFfi) {
        let mut vec = if self.rules.is_null() {
            Vec::new()
        } else {
            unsafe { Vec::from_raw_parts(self.rules, self.rules_len, self.rules_cap) }
        };
        vec.push(rule);
        self.rules = vec.as_mut_ptr();
        self.rules_len = vec.len();
        self.rules_cap = vec.capacity();
        std::mem::forget(vec);
    }
}

#[derive(Clone, Copy, Default)]
#[repr(C)]
pub struct ToSt209450ResultFfi {
    pub success: bool,
    pub data: *mut u8,
    pub data_len: usize,
    pub data_cap: usize,
    pub error_message: *mut c_char,
}

impl ToSt209450ResultFfi {
    pub fn get_data(&self) -> &[u8] {
        if self.data.is_null() || self.data_len == 0 {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(self.data, self.data_len) }
        }
    }
    pub fn get_error_message(&self) -> &str {
        if self.error_message.is_null() {
            ""
        } else {
            unsafe { std::ffi::CStr::from_ptr(self.error_message) }.to_str().unwrap_or("")
        }
    }
}

#[derive(Clone, Copy, Default)]
#[repr(C)]
pub struct FromSt209450ResultFfi {
    pub success: bool,
    pub metadata: DynamicMetadataFfi,
    pub error_message: *mut c_char,
}

impl FromSt209450ResultFfi {
    pub fn get_error_message(&self) -> &str {
        if self.error_message.is_null() {
            ""
        } else {
            unsafe { std::ffi::CStr::from_ptr(self.error_message) }.to_str().unwrap_or("")
        }
    }
}

#[derive(Clone, Copy, Default)]
#[repr(C)]
pub struct SimpleResultFfi {
    pub success: bool,
    pub error_message: *mut c_char,
}

impl SimpleResultFfi {
    pub fn get_error_message(&self) -> &str {
        if self.error_message.is_null() {
            ""
        } else {
            unsafe { std::ffi::CStr::from_ptr(self.error_message) }.to_str().unwrap_or("")
        }
    }
}

impl From<ToneMappingRule> for ToneMappingRuleFfi {
    fn from(rule: ToneMappingRule) -> Self {
        let mut curve = rule.curve;
        let curve_ptr = curve.as_mut_ptr();
        let curve_len = curve.len();
        let curve_cap = curve.capacity();
        std::mem::forget(curve);

        Self {
            alternate_hdr_headroom_log2: rule.alternate_hdr_headroom_log2,
            curve: curve_ptr,
            curve_len,
            curve_cap,
            use_pchip_slope: rule.use_pchip_slope,
            mix: rule.mix,
        }
    }
}

impl From<&ToneMappingRuleFfi> for ToneMappingRule {
    fn from(ffi: &ToneMappingRuleFfi) -> Self {
        let curve = if ffi.curve.is_null() || ffi.curve_len == 0 {
            Vec::new()
        } else {
            unsafe { std::slice::from_raw_parts(ffi.curve, ffi.curve_len) }.to_vec()
        };
        Self {
            alternate_hdr_headroom_log2: ffi.alternate_hdr_headroom_log2,
            curve,
            use_pchip_slope: ffi.use_pchip_slope,
            mix: ffi.mix,
        }
    }
}

impl From<DynamicMetadata> for DynamicMetadataFfi {
    fn from(meta: DynamicMetadata) -> Self {
        let mut rules: Vec<ToneMappingRuleFfi> =
            meta.rules.into_iter().map(ToneMappingRuleFfi::from).collect();
        let rules_ptr = rules.as_mut_ptr();
        let rules_len = rules.len();
        let rules_cap = rules.capacity();
        std::mem::forget(rules);

        Self {
            has_adaptive_tone_map_flag: meta.has_adaptive_tone_map_flag,
            use_reference_white_tone_mapping_flag: meta.use_reference_white_tone_mapping_flag,
            hdr_reference_white: meta.hdr_reference_white,
            baseline_hdr_headroom_log2: meta.baseline_hdr_headroom_log2,
            gain_application_space_chromaticities: meta.gain_application_space_chromaticities,
            rules: rules_ptr,
            rules_len,
            rules_cap,
        }
    }
}

impl From<&DynamicMetadataFfi> for DynamicMetadata {
    fn from(ffi: &DynamicMetadataFfi) -> Self {
        let rules_ffi = if ffi.rules.is_null() || ffi.rules_len == 0 {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(ffi.rules, ffi.rules_len) }
        };
        let rules = rules_ffi.iter().map(ToneMappingRule::from).collect();
        Self {
            has_adaptive_tone_map_flag: ffi.has_adaptive_tone_map_flag,
            use_reference_white_tone_mapping_flag: ffi.use_reference_white_tone_mapping_flag,
            hdr_reference_white: ffi.hdr_reference_white,
            baseline_hdr_headroom_log2: ffi.baseline_hdr_headroom_log2,
            gain_application_space_chromaticities: ffi.gain_application_space_chromaticities,
            rules,
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn dynamic_metadata_free(metadata: DynamicMetadataFfi) {
    if !metadata.rules.is_null() {
        unsafe {
            let rules = Vec::from_raw_parts(metadata.rules, metadata.rules_len, metadata.rules_cap);
            for rule in rules {
                tone_mapping_rule_free(rule);
            }
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn tone_mapping_rule_free(rule: ToneMappingRuleFfi) {
    if !rule.curve.is_null() {
        unsafe {
            let _ = Vec::from_raw_parts(rule.curve, rule.curve_len, rule.curve_cap);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn to_st209450_result_free(res: ToSt209450ResultFfi) {
    if !res.data.is_null() {
        unsafe {
            let _ = Vec::from_raw_parts(res.data, res.data_len, res.data_cap);
        }
    }
    if !res.error_message.is_null() {
        unsafe {
            let _ = std::ffi::CString::from_raw(res.error_message);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn from_st209450_result_free(res: FromSt209450ResultFfi) {
    dynamic_metadata_free(res.metadata);
    if !res.error_message.is_null() {
        unsafe {
            let _ = std::ffi::CString::from_raw(res.error_message);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn simple_result_free(res: SimpleResultFfi) {
    if !res.error_message.is_null() {
        unsafe {
            let _ = std::ffi::CString::from_raw(res.error_message);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn to_st209450_ffi(metadata: &DynamicMetadataFfi) -> ToSt209450ResultFfi {
    let meta = DynamicMetadata::from(metadata);
    match meta.to_st2094_50() {
        Ok(data) => {
            let mut vec = data;
            let ptr = vec.as_mut_ptr();
            let len = vec.len();
            let cap = vec.capacity();
            std::mem::forget(vec);
            ToSt209450ResultFfi {
                success: true,
                data: ptr,
                data_len: len,
                data_cap: cap,
                error_message: std::ptr::null_mut(),
            }
        }
        Err(e) => ToSt209450ResultFfi {
            success: false,
            data: std::ptr::null_mut(),
            data_len: 0,
            data_cap: 0,
            error_message: CString::new(e).unwrap().into_raw(),
        },
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn is_valid_ffi(metadata: &DynamicMetadataFfi) -> bool {
    let meta = DynamicMetadata::from(metadata);
    meta.is_valid()
}

#[unsafe(no_mangle)]
pub extern "C" fn from_st209450_ffi(data: *const u8, data_len: usize) -> FromSt209450ResultFfi {
    let data_slice = if data.is_null() || data_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(data, data_len) }
    };
    match DynamicMetadata::from_st2094_50(data_slice) {
        Ok(metadata) => FromSt209450ResultFfi {
            success: true,
            metadata: DynamicMetadataFfi::from(metadata),
            error_message: std::ptr::null_mut(),
        },
        Err(e) => FromSt209450ResultFfi {
            success: false,
            metadata: DynamicMetadataFfi {
                has_adaptive_tone_map_flag: false,
                use_reference_white_tone_mapping_flag: false,
                hdr_reference_white: 0.0,
                baseline_hdr_headroom_log2: 0.0,
                gain_application_space_chromaticities: [0.0; 8],
                rules: std::ptr::null_mut(),
                rules_len: 0,
                rules_cap: 0,
            },
            error_message: CString::new(e).unwrap().into_raw(),
        },
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn populate_implicit_parameters_ffi(
    metadata: &mut DynamicMetadataFfi,
) -> SimpleResultFfi {
    let mut meta = DynamicMetadata::from(&*metadata);
    let res = meta.populate_implicit_parameters();
    let new_ffi = DynamicMetadataFfi::from(meta);
    dynamic_metadata_free(std::mem::replace(metadata, new_ffi));
    match res {
        Ok(_) => SimpleResultFfi { success: true, error_message: std::ptr::null_mut() },
        Err(e) => {
            SimpleResultFfi { success: false, error_message: CString::new(e).unwrap().into_raw() }
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn dynamic_metadata_populate_pchip_slopes_ffi(
    metadata: &mut DynamicMetadataFfi,
) -> SimpleResultFfi {
    let mut meta = DynamicMetadata::from(&*metadata);
    let res = meta.populate_pchip_slopes();
    let new_ffi = DynamicMetadataFfi::from(meta);
    dynamic_metadata_free(std::mem::replace(metadata, new_ffi));
    match res {
        Ok(_) => SimpleResultFfi { success: true, error_message: std::ptr::null_mut() },
        Err(e) => {
            SimpleResultFfi { success: false, error_message: CString::new(e).unwrap().into_raw() }
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn dynamic_metadata_populate_using_rwtm_ffi(metadata: &mut DynamicMetadataFfi) {
    let mut meta = DynamicMetadata::from(&*metadata);
    meta.populate_using_rwtm();
    let new_ffi = DynamicMetadataFfi::from(meta);
    dynamic_metadata_free(std::mem::replace(metadata, new_ffi));
}

#[unsafe(no_mangle)]
pub extern "C" fn dynamic_metadata_get_rules_ffi(
    metadata: &DynamicMetadataFfi,
) -> *const ToneMappingRuleFfi {
    metadata.rules
}

#[unsafe(no_mangle)]
pub extern "C" fn dynamic_metadata_get_rules_len_ffi(metadata: &DynamicMetadataFfi) -> usize {
    metadata.rules_len
}

#[unsafe(no_mangle)]
pub extern "C" fn dynamic_metadata_add_rule_ffi(
    metadata: &mut DynamicMetadataFfi,
    rule: ToneMappingRuleFfi,
) {
    metadata.add_rule(rule);
}

#[unsafe(no_mangle)]
pub extern "C" fn tone_mapping_rule_is_valid_ffi(rule: &ToneMappingRuleFfi) -> bool {
    let r = ToneMappingRule::from(rule);
    r.is_valid()
}

#[unsafe(no_mangle)]
pub extern "C" fn tone_mapping_rule_get_curve_ffi(
    rule: &ToneMappingRuleFfi,
) -> *const ControlPoint {
    rule.curve
}

#[unsafe(no_mangle)]
pub extern "C" fn tone_mapping_rule_get_curve_len_ffi(rule: &ToneMappingRuleFfi) -> usize {
    rule.curve_len
}

#[unsafe(no_mangle)]
pub extern "C" fn tone_mapping_rule_add_point_ffi(
    rule: &mut ToneMappingRuleFfi,
    point: ControlPoint,
) {
    rule.add_point(point);
}
