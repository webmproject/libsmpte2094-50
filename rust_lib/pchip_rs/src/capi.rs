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
use crate::{
    create_subsampled_pchip, pchip_slopes, GainCurve, PchipInterpolator, ReverseInterpolateResult,
};
use std::ffi::{c_char, CString};

#[derive(Clone, Copy)]
#[repr(C)]
pub struct PchipSlopesResultFfi {
    pub slopes: *mut f32,
    pub slopes_len: usize,
    pub success: bool,
    pub error_message: *mut c_char,
}

impl PchipSlopesResultFfi {
    pub fn get_slopes(&self) -> &[f32] {
        if self.slopes.is_null() || self.slopes_len == 0 {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(self.slopes, self.slopes_len) }
        }
    }
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct PchipInterpolatorFfi {
    pub x: *mut f32,
    pub x_len: usize,
    pub y: *mut f32,
    pub y_len: usize,
    pub slopes: *mut f32,
    pub slopes_len: usize,
}

impl PchipInterpolatorFfi {
    pub fn x(&self) -> &[f32] {
        if self.x.is_null() || self.x_len == 0 {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(self.x, self.x_len) }
        }
    }
    pub fn y(&self) -> &[f32] {
        if self.y.is_null() || self.y_len == 0 {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(self.y, self.y_len) }
        }
    }
    pub fn slopes(&self) -> &[f32] {
        if self.slopes.is_null() || self.slopes_len == 0 {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(self.slopes, self.slopes_len) }
        }
    }
}

impl From<PchipInterpolator> for PchipInterpolatorFfi {
    fn from(interp: PchipInterpolator) -> Self {
        let mut x = interp.x.into_boxed_slice();
        let x_ptr = x.as_mut_ptr();
        let x_len = x.len();
        std::mem::forget(x);

        let mut y = interp.y.into_boxed_slice();
        let y_ptr = y.as_mut_ptr();
        let y_len = y.len();
        std::mem::forget(y);

        let mut slopes = interp.slopes.into_boxed_slice();
        let slopes_ptr = slopes.as_mut_ptr();
        let slopes_len = slopes.len();
        std::mem::forget(slopes);

        Self { x: x_ptr, x_len, y: y_ptr, y_len, slopes: slopes_ptr, slopes_len }
    }
}

impl From<&PchipInterpolatorFfi> for PchipInterpolator {
    fn from(ffi: &PchipInterpolatorFfi) -> Self {
        let x = if ffi.x.is_null() || ffi.x_len == 0 {
            Vec::new()
        } else {
            unsafe { std::slice::from_raw_parts(ffi.x, ffi.x_len) }.to_vec()
        };
        let y = if ffi.y.is_null() || ffi.y_len == 0 {
            Vec::new()
        } else {
            unsafe { std::slice::from_raw_parts(ffi.y, ffi.y_len) }.to_vec()
        };
        let slopes = if ffi.slopes.is_null() || ffi.slopes_len == 0 {
            Vec::new()
        } else {
            unsafe { std::slice::from_raw_parts(ffi.slopes, ffi.slopes_len) }.to_vec()
        };
        Self { x, y, slopes }
    }
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct PchipInterpolatorResultFfi {
    pub interp: PchipInterpolatorFfi,
    pub success: bool,
    pub error_message: *mut c_char,
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct GainCurveFfi {
    pub interp: PchipInterpolatorFfi,
    pub y_min: f32,
}

impl From<GainCurve> for GainCurveFfi {
    fn from(curve: GainCurve) -> Self {
        Self { interp: PchipInterpolatorFfi::from(curve.interp), y_min: curve.y_min }
    }
}

impl From<&GainCurveFfi> for GainCurve {
    fn from(ffi: &GainCurveFfi) -> Self {
        Self { interp: PchipInterpolator::from(&ffi.interp), y_min: ffi.y_min }
    }
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct GainCurveResultFfi {
    pub curve: GainCurveFfi,
    pub success: bool,
    pub error_message: *mut c_char,
}

#[unsafe(no_mangle)]
pub extern "C" fn pchip_slopes_ffi(
    x: *const f32,
    x_len: usize,
    y: *const f32,
    y_len: usize,
) -> PchipSlopesResultFfi {
    let x_slice = if x.is_null() || x_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(x, x_len) }
    };
    let y_slice = if y.is_null() || y_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(y, y_len) }
    };
    match pchip_slopes(x_slice, y_slice) {
        Ok(slopes) => {
            let mut b = slopes.into_boxed_slice();
            let ptr = b.as_mut_ptr();
            let len = b.len();
            std::mem::forget(b);
            PchipSlopesResultFfi {
                slopes: ptr,
                slopes_len: len,
                success: true,
                error_message: std::ptr::null_mut(),
            }
        }
        Err(e) => PchipSlopesResultFfi {
            slopes: std::ptr::null_mut(),
            slopes_len: 0,
            success: false,
            error_message: CString::new(e).unwrap().into_raw(),
        },
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn create_subsampled_pchip_ffi(
    x: *const f32,
    x_len: usize,
    y: *const f32,
    y_len: usize,
    num_control_points: usize,
) -> PchipInterpolatorResultFfi {
    let x_slice = if x.is_null() || x_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(x, x_len) }
    };
    let y_slice = if y.is_null() || y_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(y, y_len) }
    };
    match create_subsampled_pchip(x_slice, y_slice, num_control_points) {
        Ok(interp) => PchipInterpolatorResultFfi {
            interp: PchipInterpolatorFfi::from(interp),
            success: true,
            error_message: std::ptr::null_mut(),
        },
        Err(e) => PchipInterpolatorResultFfi {
            interp: PchipInterpolatorFfi {
                x: std::ptr::null_mut(),
                x_len: 0,
                y: std::ptr::null_mut(),
                y_len: 0,
                slopes: std::ptr::null_mut(),
                slopes_len: 0,
            },
            success: false,
            error_message: CString::new(e).unwrap().into_raw(),
        },
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn pchip_interpolator_create_ffi(
    x: *const f32,
    x_len: usize,
    y: *const f32,
    y_len: usize,
) -> PchipInterpolatorResultFfi {
    let x_slice = if x.is_null() || x_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(x, x_len) }
    };
    let y_slice = if y.is_null() || y_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(y, y_len) }
    };
    match PchipInterpolator::create(x_slice.to_vec(), y_slice.to_vec()) {
        Ok(interp) => PchipInterpolatorResultFfi {
            interp: PchipInterpolatorFfi::from(interp),
            success: true,
            error_message: std::ptr::null_mut(),
        },
        Err(e) => PchipInterpolatorResultFfi {
            interp: PchipInterpolatorFfi {
                x: std::ptr::null_mut(),
                x_len: 0,
                y: std::ptr::null_mut(),
                y_len: 0,
                slopes: std::ptr::null_mut(),
                slopes_len: 0,
            },
            success: false,
            error_message: CString::new(e).unwrap().into_raw(),
        },
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn pchip_interpolator_create_with_slopes_ffi(
    x: *const f32,
    x_len: usize,
    y: *const f32,
    y_len: usize,
    slopes: *const f32,
    slopes_len: usize,
) -> PchipInterpolatorFfi {
    let x_slice = if x.is_null() || x_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(x, x_len) }
    };
    let y_slice = if y.is_null() || y_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(y, y_len) }
    };
    let slopes_slice = if slopes.is_null() || slopes_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(slopes, slopes_len) }
    };
    let interp = PchipInterpolator::create_with_slopes(
        x_slice.to_vec(),
        y_slice.to_vec(),
        slopes_slice.to_vec(),
    );
    PchipInterpolatorFfi::from(interp)
}

#[unsafe(no_mangle)]
pub extern "C" fn pchip_interpolator_interpolate_ffi(
    interp: &PchipInterpolatorFfi,
    xi: f32,
) -> f32 {
    let rust_interp = PchipInterpolator::from(interp);
    rust_interp.interpolate(xi)
}

#[unsafe(no_mangle)]
pub extern "C" fn pchip_interpolator_reverse_interpolate_ffi(
    interp: &PchipInterpolatorFfi,
    yi: f32,
) -> ReverseInterpolateResult {
    let rust_interp = PchipInterpolator::from(interp);
    rust_interp.reverse_interpolate(yi)
}

#[unsafe(no_mangle)]
pub extern "C" fn gain_curve_create_ffi(
    x: *const f32,
    x_len: usize,
    y: *const f32,
    y_len: usize,
) -> GainCurveResultFfi {
    let x_slice = if x.is_null() || x_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(x, x_len) }
    };
    let y_slice = if y.is_null() || y_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(y, y_len) }
    };
    match GainCurve::create(x_slice.to_vec(), y_slice.to_vec()) {
        Ok(curve) => GainCurveResultFfi {
            curve: GainCurveFfi::from(curve),
            success: true,
            error_message: std::ptr::null_mut(),
        },
        Err(e) => GainCurveResultFfi {
            curve: GainCurveFfi {
                interp: PchipInterpolatorFfi {
                    x: std::ptr::null_mut(),
                    x_len: 0,
                    y: std::ptr::null_mut(),
                    y_len: 0,
                    slopes: std::ptr::null_mut(),
                    slopes_len: 0,
                },
                y_min: 0.0,
            },
            success: false,
            error_message: CString::new(e).unwrap().into_raw(),
        },
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gain_curve_create_with_slopes_ffi(
    x: *const f32,
    x_len: usize,
    y: *const f32,
    y_len: usize,
    slopes: *const f32,
    slopes_len: usize,
) -> GainCurveResultFfi {
    let x_slice = if x.is_null() || x_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(x, x_len) }
    };
    let y_slice = if y.is_null() || y_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(y, y_len) }
    };
    let slopes_slice = if slopes.is_null() || slopes_len == 0 {
        &[]
    } else {
        unsafe { std::slice::from_raw_parts(slopes, slopes_len) }
    };
    let curve =
        GainCurve::create_with_slopes(x_slice.to_vec(), y_slice.to_vec(), slopes_slice.to_vec());
    GainCurveResultFfi {
        curve: GainCurveFfi::from(curve),
        success: true,
        error_message: std::ptr::null_mut(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gain_curve_from_interpolator_ffi(interp: &PchipInterpolatorFfi) -> GainCurveFfi {
    let rust_interp = PchipInterpolator::from(interp);
    let curve = GainCurve::from_interpolator(rust_interp);
    GainCurveFfi::from(curve)
}

#[unsafe(no_mangle)]
pub extern "C" fn gain_curve_interpolate_ffi(curve: &GainCurveFfi, xi: f32) -> f32 {
    let rust_curve = GainCurve::from(curve);
    rust_curve.interpolate(xi)
}

#[unsafe(no_mangle)]
pub extern "C" fn gain_curve_reverse_interpolate_ffi(
    curve: &GainCurveFfi,
    yi: f32,
) -> ReverseInterpolateResult {
    let rust_curve = GainCurve::from(curve);
    rust_curve.reverse_interpolate(yi)
}

#[unsafe(no_mangle)]
pub extern "C" fn pchip_slopes_result_free(res: PchipSlopesResultFfi) {
    if !res.slopes.is_null() && res.slopes_len > 0 {
        unsafe {
            let _ = Box::from_raw(std::slice::from_raw_parts_mut(res.slopes, res.slopes_len));
        }
    }
    if !res.error_message.is_null() {
        unsafe {
            let _ = CString::from_raw(res.error_message);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn pchip_interpolator_free(interp: PchipInterpolatorFfi) {
    if !interp.x.is_null() && interp.x_len > 0 {
        unsafe {
            let _ = Box::from_raw(std::slice::from_raw_parts_mut(interp.x, interp.x_len));
        }
    }
    if !interp.y.is_null() && interp.y_len > 0 {
        unsafe {
            let _ = Box::from_raw(std::slice::from_raw_parts_mut(interp.y, interp.y_len));
        }
    }
    if !interp.slopes.is_null() && interp.slopes_len > 0 {
        unsafe {
            let _ = Box::from_raw(std::slice::from_raw_parts_mut(interp.slopes, interp.slopes_len));
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn pchip_interpolator_result_free(res: PchipInterpolatorResultFfi) {
    pchip_interpolator_free(res.interp);
    if !res.error_message.is_null() {
        unsafe {
            let _ = CString::from_raw(res.error_message);
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn gain_curve_free(curve: GainCurveFfi) {
    pchip_interpolator_free(curve.interp);
}

#[unsafe(no_mangle)]
pub extern "C" fn gain_curve_result_free(res: GainCurveResultFfi) {
    gain_curve_free(res.curve);
    if !res.error_message.is_null() {
        unsafe {
            let _ = CString::from_raw(res.error_message);
        }
    }
}
