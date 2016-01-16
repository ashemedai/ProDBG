#![cfg(any(target_os="linux",
    target_os="freebsd",
    target_os="dragonfly",
    target_os="netbsd",
    target_os="openbsd"))]

extern crate x11_dl;

use {Scale, Key, KeyRepeat};
use key_handler::KeyHandler;
use self::x11_dl::keysym::*;

use libc::{c_void, c_char, c_uchar};
use std::ffi::{CString};
use std::ptr;
use std::mem;

#[link(name = "X11")]
extern {
    fn mfb_open(name: *const c_char, width: u32, height: u32, scale: i32) -> *mut c_void;
    fn mfb_close(window: *mut c_void);
    fn mfb_update(window: *mut c_void, buffer: *const c_uchar);
    fn mfb_set_position(window: *mut c_void, x: i32, y: i32);
    fn mfb_set_key_callback(window: *mut c_void, target: *mut c_void, cb: unsafe extern fn(*mut c_void, i32, i32));
    fn mfb_should_close(window: *mut c_void) -> i32;
    fn mfb_get_screen_size() -> u32;
}

pub struct Window {
    window_handle: *mut c_void,
    key_handler: KeyHandler,
}

#[allow(non_upper_case_globals)]
unsafe extern "C" fn key_callback(window: *mut c_void, key: i32, s: i32) {
    let win: *mut Window = mem::transmute(window);

    let state = s == 1;

    match key as u32 {
        XK_0 => (*win).key_handler.set_key_state(Key::Key0, state),
        XK_1 => (*win).key_handler.set_key_state(Key::Key1, state),
        XK_2 => (*win).key_handler.set_key_state(Key::Key2, state),
        XK_3 => (*win).key_handler.set_key_state(Key::Key3, state),
        XK_4 => (*win).key_handler.set_key_state(Key::Key4, state),
        XK_5 => (*win).key_handler.set_key_state(Key::Key5, state),
        XK_6 => (*win).key_handler.set_key_state(Key::Key6, state),
        XK_7 => (*win).key_handler.set_key_state(Key::Key7, state),
        XK_8 => (*win).key_handler.set_key_state(Key::Key8, state),
        XK_9 => (*win).key_handler.set_key_state(Key::Key9, state),
        XK_a => (*win).key_handler.set_key_state(Key::A, state),
        XK_b => (*win).key_handler.set_key_state(Key::B, state),
        XK_c => (*win).key_handler.set_key_state(Key::C, state),
        XK_d => (*win).key_handler.set_key_state(Key::D, state),
        XK_e => (*win).key_handler.set_key_state(Key::E, state),
        XK_f => (*win).key_handler.set_key_state(Key::F, state),
        XK_g => (*win).key_handler.set_key_state(Key::G, state),
        XK_h => (*win).key_handler.set_key_state(Key::H, state),
        XK_i => (*win).key_handler.set_key_state(Key::I, state),
        XK_j => (*win).key_handler.set_key_state(Key::J, state),
        XK_k => (*win).key_handler.set_key_state(Key::K, state),
        XK_l => (*win).key_handler.set_key_state(Key::L, state),
        XK_m => (*win).key_handler.set_key_state(Key::M, state),
        XK_n => (*win).key_handler.set_key_state(Key::N, state),
        XK_o => (*win).key_handler.set_key_state(Key::O, state),
        XK_p => (*win).key_handler.set_key_state(Key::P, state),
        XK_q => (*win).key_handler.set_key_state(Key::Q, state),
        XK_r => (*win).key_handler.set_key_state(Key::R, state),
        XK_s => (*win).key_handler.set_key_state(Key::S, state),
        XK_t => (*win).key_handler.set_key_state(Key::T, state),
        XK_u => (*win).key_handler.set_key_state(Key::U, state),
        XK_v => (*win).key_handler.set_key_state(Key::V, state),
        XK_w => (*win).key_handler.set_key_state(Key::W, state),
        XK_x => (*win).key_handler.set_key_state(Key::X, state),
        XK_y => (*win).key_handler.set_key_state(Key::Y, state),
        XK_z => (*win).key_handler.set_key_state(Key::Z, state),
        XK_F1 => (*win).key_handler.set_key_state(Key::F1, state),
        XK_F2 => (*win).key_handler.set_key_state(Key::F2, state),
        XK_F3 => (*win).key_handler.set_key_state(Key::F3, state),
        XK_F4 => (*win).key_handler.set_key_state(Key::F4, state),
        XK_F5 => (*win).key_handler.set_key_state(Key::F5, state),
        XK_F6 => (*win).key_handler.set_key_state(Key::F6, state),
        XK_F7 => (*win).key_handler.set_key_state(Key::F7, state),
        XK_F8 => (*win).key_handler.set_key_state(Key::F8, state),
        XK_F9 => (*win).key_handler.set_key_state(Key::F9, state),
        XK_F10 => (*win).key_handler.set_key_state(Key::F10, state),
        XK_F11 => (*win).key_handler.set_key_state(Key::F11, state),
        XK_F12 => (*win).key_handler.set_key_state(Key::F12, state),
        XK_Down => (*win).key_handler.set_key_state(Key::Down, state),
        XK_Left => (*win).key_handler.set_key_state(Key::Left, state),
        XK_Right => (*win).key_handler.set_key_state(Key::Right, state),
        XK_Up => (*win).key_handler.set_key_state(Key::Up, state),
        XK_Escape => (*win).key_handler.set_key_state(Key::Escape, state),
        XK_apostrophe => (*win).key_handler.set_key_state(Key::Apostrophe, state),
        XK_grave => (*win).key_handler.set_key_state(Key::Backquote, state),
        XK_backslash => (*win).key_handler.set_key_state(Key::Backslash, state),
        XK_comma => (*win).key_handler.set_key_state(Key::Comma, state),
        XK_equal => (*win).key_handler.set_key_state(Key::Equal, state),
        XK_bracketleft => (*win).key_handler.set_key_state(Key::LeftBracket, state),
        XK_minus => (*win).key_handler.set_key_state(Key::Minus, state),
        //XP_period => (*win).key_handler.set_key_state(Key::Period, state),
        XK_braceright => (*win).key_handler.set_key_state(Key::RightBracket, state),
        XK_semicolon => (*win).key_handler.set_key_state(Key::Semicolon, state),
        XK_slash => (*win).key_handler.set_key_state(Key::Slash, state),
        //XK_backslash => (*win).key_handler.set_key_state(Key::Backspace, state),
        XK_Delete => (*win).key_handler.set_key_state(Key::Delete, state),
        XK_End => (*win).key_handler.set_key_state(Key::End, state),
        XK_Return => (*win).key_handler.set_key_state(Key::Enter, state),
        XK_Home => (*win).key_handler.set_key_state(Key::Home, state),
        XK_Insert => (*win).key_handler.set_key_state(Key::Insert, state),
        XK_Menu => (*win).key_handler.set_key_state(Key::Menu, state),
        XK_Page_Down => (*win).key_handler.set_key_state(Key::PageDown, state),
        XK_Page_Up => (*win).key_handler.set_key_state(Key::PageUp, state),
        XK_Pause => (*win).key_handler.set_key_state(Key::Pause, state),
        XK_space => (*win).key_handler.set_key_state(Key::Space, state),
        XK_Tab => (*win).key_handler.set_key_state(Key::Tab, state),
        XK_Num_Lock => (*win).key_handler.set_key_state(Key::NumLock, state),
        XK_Caps_Lock => (*win).key_handler.set_key_state(Key::CapsLock, state),
        XK_Scroll_Lock => (*win).key_handler.set_key_state(Key::ScrollLock, state),
        XK_Shift_L => (*win).key_handler.set_key_state(Key::LeftShift, state),
        XK_Shift_R => (*win).key_handler.set_key_state(Key::RightShift, state),
        XK_Control_L => (*win).key_handler.set_key_state(Key::LeftCtrl, state),
        XK_Control_R => (*win).key_handler.set_key_state(Key::RightCtrl, state),
        XK_KP_0 => (*win).key_handler.set_key_state(Key::NumPad0, state),
        XK_KP_1 => (*win).key_handler.set_key_state(Key::NumPad1, state),
        XK_KP_2 => (*win).key_handler.set_key_state(Key::NumPad2, state),
        XK_KP_3 => (*win).key_handler.set_key_state(Key::NumPad3, state),
        XK_KP_4 => (*win).key_handler.set_key_state(Key::NumPad4, state),
        XK_KP_5 => (*win).key_handler.set_key_state(Key::NumPad5, state),
        XK_KP_6 => (*win).key_handler.set_key_state(Key::NumPad6, state),
        XK_KP_7 => (*win).key_handler.set_key_state(Key::NumPad7, state),
        XK_KP_8 => (*win).key_handler.set_key_state(Key::NumPad8, state),
        XK_KP_9 => (*win).key_handler.set_key_state(Key::NumPad9, state),
        XK_KP_Decimal => (*win).key_handler.set_key_state(Key::NumPadDot, state),
        XK_KP_Divide => (*win).key_handler.set_key_state(Key::NumPadSlash, state),
        XK_KP_Multiply => (*win).key_handler.set_key_state(Key::NumPadAsterisk, state),
        XK_KP_Subtract => (*win).key_handler.set_key_state(Key::NumPadMinus, state),
        XK_KP_Add => (*win).key_handler.set_key_state(Key::NumPadPlus, state),
        XK_KP_Enter => (*win).key_handler.set_key_state(Key::NumPadEnter, state),
        XK_Super_L => (*win).key_handler.set_key_state(Key::LeftSuper, state),
        XK_Super_R => (*win).key_handler.set_key_state(Key::RightSuper, state),
    	_ => (),
    }
}

impl Window {
    pub fn new(name: &str, width: usize, height: usize, scale: Scale) -> Result<Window, &str> {
        let n = match CString::new(name) {
            Err(_) => { 
                println!("Unable to convert {} to c_string", name);
                return Err("Unable to set correct name"); 
            }
            Ok(n) => n,
        };

        unsafe {
            let handle = mfb_open(n.as_ptr(), width as u32, height as u32, Self::get_scale_factor(width, height, scale));

            if handle == ptr::null_mut() {
                return Err("Unable to open Window");
            }

            Ok(Window { 
                window_handle: handle,
                key_handler: KeyHandler::new(),
            })
        }
    }

    pub fn update(&mut self, buffer: &[u32]) {
        self.key_handler.update();

        unsafe {
            mfb_update(self.window_handle, buffer.as_ptr() as *const u8);
            mfb_set_key_callback(self.window_handle, mem::transmute(self), key_callback);
        }
    }

    #[inline]
    pub fn set_position(&mut self, x: isize, y: isize) {
        unsafe { mfb_set_position(self.window_handle, x as i32, y as i32) }
    }

    #[inline]
    pub fn get_keys(&self) -> Option<Vec<Key>> {
        self.key_handler.get_keys()
    }

    #[inline]
    pub fn get_keys_pressed(&self, repeat: KeyRepeat) -> Option<Vec<Key>> {
        self.key_handler.get_keys_pressed(repeat)
    }

    #[inline]
    pub fn is_key_down(&self, key: Key) -> bool {
        self.key_handler.is_key_down(key)
    }

    #[inline]
    pub fn set_key_repeat_delay(&mut self, delay: f32) {
        self.key_handler.set_key_repeat_delay(delay)
    }

    #[inline]
    pub fn set_key_repeat_rate(&mut self, rate: f32) {
        self.key_handler.set_key_repeat_rate(rate)
    }

    #[inline]
    pub fn is_key_pressed(&self, key: Key, repeat: KeyRepeat) -> bool {
        self.key_handler.is_key_pressed(key, repeat)
    }

    #[inline]
    pub fn is_open(&self) -> bool {
        unsafe { mfb_should_close(self.window_handle) == 1 }
    }

    unsafe fn get_scale_factor(width: usize, height: usize, scale: Scale) -> i32 {
        let factor: i32 = match scale {
            Scale::X1 => 1,
            Scale::X2 => 2,
            Scale::X4 => 4,
            Scale::FitScreen => {
                let wh: u32 = mfb_get_screen_size();
                let screen_x = (wh >> 16) as i32; 
                let screen_y = (wh & 0xffff) as i32; 

                println!("{} - {}", screen_x, screen_y);

                let mut scale = 1i32;

                loop {
                    let w = width as i32 * (scale + 1);
                    let h = height as i32 * (scale + 1);

                    if w > screen_x || h > screen_y {
                        break;
                    }

                    scale *= 2;
                }

                if scale >= 4 {
                	4
                } else {
                	scale
				}
            }

            _ => {
            	println!("Scale above 4 not support currently, defaults to 4");
            	4
            }
        };

        return factor;
    }
}

impl Drop for Window {
    fn drop(&mut self) {
        unsafe {
            mfb_close(self.window_handle);
        }
    }
}

