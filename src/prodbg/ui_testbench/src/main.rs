extern crate core;
extern crate libc;
extern crate minifb;

use minifb::{Window, Key, Scale, WindowOptions, MouseMode, MouseButton};
use libc::{c_void, c_int, c_float};

use core::plugin_handler::*;
use std::ptr;

const WIDTH: usize = 1280;
const HEIGHT: usize = 1024;

fn main() {
    let mut window = match Window::new("Noise Test - Press ESC to exit", WIDTH, HEIGHT, 
                                       WindowOptions { 
                                           resize: true,
                                           scale: Scale::X1,
                                           ..WindowOptions::default()
                                       }) {
        Ok(win) => win,
        Err(err) => {
            println!("Unable to create window {}", err);
            return;
        }
    };

    let search_paths = vec!["../../..", "t2-output/macosx-clang-debug-default", "target/debug"];

    let mut plugin_handler = PluginHandler::new(search_paths, Some("t2-output"));

    plugin_handler.add_plugin("bitmap_memory");
    plugin_handler.create_view_instance(&"Bitmap View".to_string());

    unsafe {
        bgfx_create();
        bgfx_create_window(window.get_window_handle() as *mut c_void, WIDTH as i32, HEIGHT as i32);  
    }

    while window.is_open() && !window.is_key_down(Key::Escape) {
        match plugin_handler.watch_recv.try_recv() {
            Ok(file) => {
                plugin_handler.reload_plugin(file.path.as_ref().unwrap());
            }
            _ => (),
        }

        unsafe { 
            bgfx_pre_update();

            for instance in &plugin_handler.view_instances {
                bgfx_imgui_set_window_pos(0.0, 0.0);
                bgfx_imgui_set_window_size(bgfx_get_screen_width(), bgfx_get_screen_height());

                bgfx_imgui_begin(1);

                let plugin_funcs = instance.plugin_type.plugin_funcs as *mut CViewPlugin; 
                ((*plugin_funcs).update)(instance.user_data, bgfx_get_ui_funcs(), ptr::null(), ptr::null());

                bgfx_imgui_end();
            }

            bgfx_post_update();

            window.get_mouse_pos(MouseMode::Clamp).map(|mouse| {
                prodbg_set_mouse_pos(mouse.0, mouse.1);
                prodbg_set_mouse_state(0, window.get_mouse_down(MouseButton::Left) as c_int);
            });
        }


        window.update();
    }

    unsafe {
        bgfx_destroy();
    }
}

///
/// 
///
///

extern {
    fn bgfx_pre_update();
    fn bgfx_post_update();
    fn bgfx_create();
    fn bgfx_create_window(window: *const c_void, width: c_int, height: c_int);
    fn bgfx_destroy();

    fn prodbg_set_mouse_pos(x: f32, y: f32);
    fn prodbg_set_mouse_state(mouse: c_int, state: c_int);

    fn bgfx_get_ui_funcs() -> *const c_void;

    fn bgfx_imgui_begin(show: c_int);
    fn bgfx_imgui_end();

    fn bgfx_imgui_set_window_pos(x: c_float, y: c_float);
    fn bgfx_imgui_set_window_size(x: c_float, y: c_float);

    fn bgfx_get_screen_width() -> f32;
    fn bgfx_get_screen_height() -> f32;
}

