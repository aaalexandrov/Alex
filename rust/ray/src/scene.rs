use std::f32;
use std::vec::Vec;
use nalgebra_glm as glm;
use crate::geom::{Ray, Plane, Triangle, Box3, Sphere, Vec3};

pub type Color = Vec3;

pub trait Shader: Sync {
    fn shade(&self, ray: &Ray, ray_t: f32, depth: u32, obj: &SceneObject, scene: &Scene) -> Color;
}

fn shade_diffuse(diffuse_color: &Color, ray: &Ray, ray_t: f32, _depth: u32, obj: &SceneObject, scene: &Scene) -> Color {
    let shader_data = &scene.shader_data;
    let pos = ray.point(ray_t);
    let normal = obj.primitive.get_normal(&pos, &ray.direction);
    let nl = Vec3::dot(&normal, &shader_data.sun_dir);
    let mut shadow_nl = f32::max(nl, 0.0);
    if nl > 0.0 {
        if let (_, Some(_obj)) = scene.cast_ray(&Ray::new(&pos, &shader_data.sun_dir), Option::Some(obj)) {
            shadow_nl = 0.0;
        }
    }
    let color = shader_data.ambient_color + shadow_nl * shader_data.sun_color;

    diffuse_color.component_mul(&color)
}

fn get_reflected(ray: &Ray, ray_t: f32, depth: u32, obj: &SceneObject, scene: &Scene) -> Color {
    if depth == 0 {
        return scene.shader_data.sky_color;
    }
    let pos = ray.point(ray_t);
    let normal = obj.primitive.get_normal(&pos, &ray.direction);
    let reflected_dir = normal * (-2.0 * Vec3::dot(&normal, &ray.direction)) + ray.direction;
    let reflected_ray = Ray::new(&pos, &glm::normalize(&reflected_dir));
    scene.shade_ray(&reflected_ray, Option::Some(obj), depth - 1)
}

pub struct Solid {
    pub diffuse_color: Color,
}

impl Shader for Solid {
    fn shade(&self, ray: &Ray, ray_t: f32, depth: u32, obj: &SceneObject, scene: &Scene) -> Color {
        shade_diffuse(&self.diffuse_color, ray, ray_t, depth, obj, scene)
    }
}

pub struct Reflective {
    pub diffuse_color: Color,
    pub reflective_color: Color,
}

impl Shader for Reflective {
    fn shade(&self, ray: &Ray, ray_t: f32, depth: u32, obj: &SceneObject, scene: &Scene) -> Color {
        let diffuse = shade_diffuse(&self.diffuse_color, ray, ray_t, depth, obj, scene);
        let reflected = get_reflected(ray, ray_t, depth, obj, scene);
        glm::lerp_vec(&diffuse, &reflected, &self.reflective_color)
    }
}

pub trait Intersectable: Sync {
    fn intersect_ray(&self, ray: &Ray) -> f32;
    fn get_normal(&self, pos: &Vec3, dir: &Vec3) -> Vec3;
}

impl Intersectable for Plane {
    fn intersect_ray(&self, ray: &Ray) -> f32 {
        let t = ray.intersect_plane(self);
        if t >= 0.0 {
            t
        } else {
            f32::NAN
        }
    }

    fn get_normal(&self, _pos: &Vec3, dir: &Vec3) -> Vec3 {
        if Vec3::dot(&self.normal, &dir) >= 0.0 {
            -self.normal
        } else {
            self.normal
        }
    }
}

impl Intersectable for Triangle {
    fn intersect_ray(&self, ray: &Ray) -> f32 {
        let t = ray.intersect_tri(self);
        if t >= 0.0 {
            t
        } else {
            f32::NAN
        }
    }

    fn get_normal(&self, _pos: &Vec3, dir: &Vec3) -> Vec3 {
        let normal = self.get_normal().normalize();
        if Vec3::dot(&normal, &dir) >= 0.0 {
            -normal
        } else {
            normal
        }
    }
}

impl Intersectable for Box3 {
    fn intersect_ray(&self, ray: &Ray) -> f32 {
        let int = ray.intersect_box(self);
        if int.is_empty() || int.max < 0.0 {
            return f32::NAN;
        }
        if int.min >= 0.0 { 
            int.min
        } else {
            int.max
        }
    }

    fn get_normal(&self, pos: &Vec3, _dir: &Vec3) -> Vec3 {
        self.get_normal(pos)
    }
}

impl Intersectable for Sphere {
    fn intersect_ray(&self, ray: &Ray) -> f32 {
        let int = ray.intersect_sphere(self);
        if int.is_empty() || int.max < 0.0 {
            return f32::NAN;
        }
        if int.min >= 0.0 { 
            int.min
        } else {
            int.max
        }
    }


    fn get_normal(&self, pos: &Vec3, _dir: &Vec3) -> Vec3 {
        self.get_normal(pos)
    }
}

pub struct ShaderData {
    pub sun_color: Color,
    pub sun_dir: Vec3,
    pub sky_color: Color,
    pub ambient_color: Color,
}

pub struct SceneObject {
    pub primitive: Box<dyn Intersectable>,
    pub shader: &'static dyn Shader,
}

pub struct Scene {
    pub objects: Vec<SceneObject>,
    pub shaders: Vec<Box<dyn Shader>>,
    pub shader_data: ShaderData,
}

impl Scene {
    pub fn new() -> Scene {
        Scene {
            objects: Vec::new(),
            shaders: Vec::new(),
            shader_data: ShaderData {
                sun_color: Color::new(0.8, 0.8, 0.5),
                sun_dir: glm::normalize(&Vec3::new(0.5, 0.3, 1.0)),
                sky_color: Color::new(0.3, 0.3, 1.0),
                ambient_color: Color::new(0.2, 0.2, 0.2),
            }
        }
    }

    pub fn add_shader(&mut self, shader: Box<dyn Shader>) -> usize {
        self.shaders.push(shader);
        self.shaders.len() - 1
    }

    pub fn add_object(&mut self, int: Box<dyn Intersectable>, shader_index: usize) {
        let shader: &'static dyn Shader = unsafe {
            std::mem::transmute(self.shaders[shader_index].as_ref())
        };
        self.objects.push(SceneObject {
            primitive: int,
            shader,
        });
    }

    pub fn shade(&self, ray: &Ray, t_ray: f32, depth: u32, obj: &SceneObject) -> Color {
        let shader = obj.shader;
        shader.shade(ray, t_ray, depth, obj, self)
    }

    pub fn cast_ray(&self, ray: &Ray, ignore: Option<&SceneObject>) -> (f32, Option<&SceneObject>) {
        let mut t_min = f32::INFINITY;
        let mut obj_min = Option::<&SceneObject>::None;
        let ptr_ignore: *const SceneObject = if let Some(obj) = ignore { obj } else { std::ptr::null() };
        for obj in self.objects.as_slice() {
            if ptr_ignore == obj {
                continue;
            }
            let t = obj.primitive.intersect_ray(&ray);
            if t < t_min {
                t_min = t;
                obj_min = Option::<&SceneObject>::Some(obj);
            }
        }

        (t_min, obj_min)
    }

    pub fn shade_ray(&self, ray: &Ray, ignore: Option<&SceneObject>, depth: u32) -> Color {
        let (t_min, obj_min) = self.cast_ray(&ray, ignore);
        if let Some(obj) = obj_min {
            self.shade(&ray, t_min, depth, obj)
        } else {
            self.shader_data.sky_color
        }
    }
}