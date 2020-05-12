use sdl2::event::Event;
use sdl2::keyboard::{Keycode, Scancode};
use sdl2::rect::Rect;
use sdl2::video::SwapInterval;
use sdl2::pixels::Color;

//use rand::distributions::{Distribution, Uniform};

use nalgebra_glm as glm;
use glm::{Vec2, Vec3, Vec4, Mat4x4};
use nalgebra::{UnitQuaternion, Unit};
use approx::abs_diff_eq;

use std::f32;

mod geom;
mod scene;

use geom::{Ray, Triangle, Plane, Box3, Sphere};
use scene::{Intersectable, Scene, Solid, Reflective, Shader};

fn main() {
    let sdl_context = sdl2::init().unwrap();
    let video_subsystem = sdl_context.video().unwrap();

    let frame_buffer = ImageRGBA::new(800, 600);

    let window = video_subsystem.window("SDL Window", frame_buffer.width, frame_buffer.height).opengl().build().unwrap();

    let driver_index = sdl2::render::drivers().position(|d| d.name == "opengl").unwrap() as u32;
    let mut canvas = window.into_canvas()
        .index(driver_index)
        .build().unwrap();
    video_subsystem.gl_set_swap_interval(SwapInterval::VSync).unwrap();

    let canvas_creator = canvas.texture_creator();
    let mut texture = canvas_creator.create_texture_streaming(sdl2::pixels::PixelFormatEnum::RGBA8888, frame_buffer.width, frame_buffer.height).unwrap();

    let tri: Box<dyn Intersectable> = Box::new(Triangle::new(&Vec3::new(1.0, 0.0, -1.0), &Vec3::new(0.0, 0.0, 1.0), &Vec3::new(1.0, 0.0, 1.0)));
    let plane: Box<dyn Intersectable> = Box::new(Plane::new(&Vec3::new(0.0, 0.0, 1.0), 1.0));
    let box3: Box<dyn Intersectable> = Box::new(Box3::new(&Vec3::new(-1.0, -1.0, -1.0), &Vec3::new(-0.5, 0.5, 0.5)));
    let sphere: Box<dyn Intersectable> = Box::new(Sphere::new(&Vec3::new(0.5, 0.0, 0.0), 0.5));

    let tri_shader: Box<dyn Shader> = Box::new(Solid{ diffuse_color: Vec3::new(1.0, 0.0, 0.0) });
    let plane_shader: Box<dyn Shader> = Box::new(Solid{ diffuse_color: Vec3::new(0.0, 1.0, 0.0) });
    let box_shader: Box<dyn Shader> = Box::new(Solid{ diffuse_color: Vec3::new(1.0, 1.0, 0.0) });
    let sphere_shader: Box<dyn Shader> = Box::new(Reflective{ diffuse_color: Vec3::new(0.0, 1.0, 1.0), reflective_color: Vec3::new(0.5, 0.5, 0.5) });

    let mut scene = Scene::new();
    let tri_shader = scene.add_shader(tri_shader);
    let plane_shader = scene.add_shader(plane_shader);
    let box_shader = scene.add_shader(box_shader);
    let sphere_shader = scene.add_shader(sphere_shader);
    scene.add_object(tri, tri_shader);
    scene.add_object(plane, plane_shader);
    scene.add_object(box3, box_shader);
    scene.add_object(sphere, sphere_shader);

    let mut camera = Camera::new(
        &Vec2::new(frame_buffer.width as f32, frame_buffer.height as f32), 
        &Vec3::new(0.0, -3.0, 0.0), 
        &Vec3::new(0.0, 1.0, 0.0), 
        &Vec3::new(0.0, 0.0, 1.0), 
        f32::consts::FRAC_PI_2);

    const TILE_SIZE: u32 = 50;

    println!("Render scene starting...");
    let start = std::time::Instant::now();
    camera.render_scene(TILE_SIZE, &scene, &frame_buffer);
    let elapsed = start.elapsed();
    println!("Render scene finished in {} ms", elapsed.as_millis());

    let mut event_pump = sdl_context.event_pump().unwrap();

    let mut prev_loop = std::time::Instant::now();
    'running: loop {
        let dt = prev_loop.elapsed().as_secs_f32();
        prev_loop = std::time::Instant::now();
        let mut modified = false;
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit {..} | 
                Event::KeyDown{ keycode: Some(Keycode::Escape), .. } => {
                    break 'running;
                }
                _ => {}
            }
        }

        let mut translate = Vec3::zeros();
        let mut rotate = UnitQuaternion::identity();
        for scancode in event_pump.keyboard_state().pressed_scancodes() {
            match scancode {
                Scancode::W => {
                    translate += Vec3::new(0.0, 1.0, 0.0);
                }
                Scancode::S => {
                    translate += Vec3::new(0.0, -1.0, 0.0);
                }
                Scancode::A => {
                    translate += Vec3::new(-1.0, 0.0, 0.0);
                }
                Scancode::D => {
                    translate += Vec3::new(1.0, 0.0, 0.0);
                }
                Scancode::R => {
                    translate += Vec3::new(0.0, 0.0, 1.0);
                }
                Scancode::F => {
                    translate += Vec3::new(0.0, 0.0, -1.0);
                }
                Scancode::Q => {
                    rotate *= UnitQuaternion::from_axis_angle(&Unit::new_normalize(Vec3::new(0.0, 0.0, 1.0)), 1.0);
                }
                Scancode::E => {
                    rotate *= UnitQuaternion::from_axis_angle(&Unit::new_normalize(Vec3::new(0.0, 0.0, -1.0)), 1.0);
                }
                Scancode::T => {
                    rotate *= UnitQuaternion::from_axis_angle(&Unit::new_normalize(Vec3::new(1.0, 0.0, 0.0)), 1.0);
                }
                Scancode::G => {
                    rotate *= UnitQuaternion::from_axis_angle(&Unit::new_normalize(Vec3::new(-1.0, 0.0, 0.0)), 1.0);
                }
                Scancode::Z => {
                    rotate *= UnitQuaternion::from_axis_angle(&Unit::new_normalize(Vec3::new(0.0, -1.0, 0.0)), 1.0);
                }
                Scancode::C => {
                    rotate *= UnitQuaternion::from_axis_angle(&Unit::new_normalize(Vec3::new(0.0, 1.0, 0.0)), 1.0);
                }
                _ => {}
            }
        }
        if !abs_diff_eq!(translate, Vec3::zeros()) {
            camera.translate_local(&(translate * dt));
            modified = true;
        }
        if !abs_diff_eq!(rotate, UnitQuaternion::identity()) {
            camera.rotate_local(&UnitQuaternion::identity().slerp(&rotate, dt));
            modified = true;
        }

        if modified {
            camera.render_scene(TILE_SIZE, &scene, &frame_buffer);
        }
        texture.update(frame_buffer.rect(), &frame_buffer.pixels, frame_buffer.pitch()).unwrap();
        canvas.copy(&texture, frame_buffer.rect(), frame_buffer.rect()).unwrap();

        canvas.present();
    }
}

pub struct ImageRGBA {
    width: u32,
    height: u32,
    pixels: Box<[u8]>,
}

impl ImageRGBA {
    pub fn new(width: u32, height: u32) -> ImageRGBA {
        let pix = vec![0; (width * height * 4) as usize];
        ImageRGBA{ width, height, pixels: pix.into_boxed_slice() }
    }

    pub fn rect(&self) -> Rect {
        Rect::new(0, 0, self.width, self.height)
    }

    pub fn pitch(&self) -> usize {
        (self.width * 4) as usize
    }

    pub fn get_pixel(&self, x: u32, y: u32) -> Color {
        let ind = ((y * self.width + x) * 4) as usize;
        Color::RGBA(self.pixels[ind + 3], self.pixels[ind + 2], self.pixels[ind + 1], self.pixels[ind + 0])
    }

    pub fn set_pixel(&mut self, x: u32, y: u32, color: Color) {
        let ind = ((y * self.width + x) * 4) as usize;
        self.pixels[ind + 0] = color.a;
        self.pixels[ind + 1] = color.b;
        self.pixels[ind + 2] = color.g;
        self.pixels[ind + 3] = color.r;
    }

    pub fn set_pixel_unchecked(&self, x: u32, y: u32, color: Color) {
        let ind = ((y * self.width + x) * 4) as usize;
        unsafe {
            let px: *mut [u8; 4] = std::mem::transmute::<&u8, *mut [u8; 4]>(&self.pixels[ind]);
            (*px)[0] = color.a;
            (*px)[1] = color.b;
            (*px)[2] = color.g;
            (*px)[3] = color.r;
        }
    }
}

fn to_sdl_color(clr: &scene::Color) -> Color {
    Color::RGB((clr.x * 255.0) as u8, (clr.y * 255.0) as u8, (clr.z * 255.0) as u8)
}

fn vec3_to_vec4(v3: &Vec3, w: f32) -> Vec4 {
    Vec4::new(v3.x, v3.y, v3.z, w)
}

pub struct Camera {
    transform: Mat4x4,
    plane_dist: f32,
    pixel_size: Vec2,
}

impl Camera {
    pub fn new(pixel_size: &Vec2, pos: &Vec3, forward: &Vec3, up: &Vec3, hfov: f32) -> Camera {
        let transform = Mat4x4::from_columns(&[
            glm::vec3_to_vec4(&glm::cross(forward, up)),
            glm::vec3_to_vec4(forward),
            glm::vec3_to_vec4(up),
            glm::vec3_to_vec4(pos),
        ]);

        let plane_dist = (pixel_size.x / 2.0) / (hfov / 2.0).tan();

        Camera { 
            transform, 
            plane_dist, 
            pixel_size: *pixel_size,
        }
    }

    pub fn get_pixel_ray(&self, pixel_pos: &Vec2) -> Ray {
        assert!(0.0 <= pixel_pos.x && pixel_pos.x <= self.pixel_size.x);
        assert!(0.0 <= pixel_pos.y && pixel_pos.y <= self.pixel_size.y);

        let local_dir = Vec4::new(pixel_pos.x - self.pixel_size.x / 2.0, self.plane_dist, self.pixel_size.y / 2.0 - pixel_pos.y, 0.0);

        Ray::new(&self.transform.column(3).xyz(), &glm::normalize(&((self.transform * local_dir).xyz())))
    }

    pub fn render_rect(&self, rect: &Rect, scene: &Scene, frame_buffer: &ImageRGBA) {
        assert!(0 <= rect.left() && rect.right() as u32 <= frame_buffer.width);
        assert!(0 <= rect.top() && rect.bottom() as u32 <= frame_buffer.height);

        for y in rect.top() as u32..rect.bottom() as u32 {
            for x in rect.left() as u32..rect.right() as u32 {
                let ray = self.get_pixel_ray(&Vec2::new(x as f32 + 0.5, y as f32 + 0.5));
                let color = to_sdl_color(&scene.shade_ray(&ray, Option::None, 1));
                frame_buffer.set_pixel_unchecked(x, y, color);
            }
        }

    }

    pub fn render_scene(&self, tile_size: u32, scene: &Scene, frame_buffer: &ImageRGBA) {
        assert_eq!(self.pixel_size.x, frame_buffer.width as f32);
        assert_eq!(self.pixel_size.y, frame_buffer.height as f32);

        let rect = Rect::new(0, 0, frame_buffer.width, frame_buffer.height);
        rayon::scope(|s| {
            for y in (rect.top()..rect.bottom()).step_by(tile_size as usize) {
                for x in (rect.left()..rect.right()).step_by(tile_size as usize) {
                    let tile = Rect::new(x, y, tile_size, tile_size);
                    let tile_valid = rect.intersection(tile).unwrap();
                    s.spawn(move|_| {
                        self.render_rect(&tile_valid, scene, frame_buffer);
                    });
                }
            }
        });
    }

    pub fn translate_local(&mut self, delta: &Vec3) {
        let local_delta = (self.transform * vec3_to_vec4(delta, 1.0)).xyz();
        self.transform.set_column(3, &vec3_to_vec4(&local_delta, 0.0));
    }

    pub fn rotate_local(&mut self, rot: &UnitQuaternion<f32>) {
        let rot_mat = rot.to_homogeneous();
        self.transform = self.transform * rot_mat;
    }
}