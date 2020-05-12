use std::{f32, mem};
use nalgebra_glm as glm;
pub use glm::{Vec3};
use approx::abs_diff_eq;

pub struct Interval {
    pub min: f32,
    pub max: f32,
}

impl Interval {
    pub fn new(min: f32, max: f32) -> Interval {
        Interval{ min, max }
    }

    pub fn empty() -> Interval {
        Interval { min: f32::INFINITY, max: -f32::INFINITY }
    }

    pub fn all() -> Interval {
        Interval { min: -f32::INFINITY, max: f32::INFINITY }
    }

    pub fn is_empty(&self) -> bool {
        !(self.min <= self.max)
    }

    pub fn intersection(&self, other: &Interval) -> Interval {
        Interval::new(f32::max(self.min, other.min), f32::min(self.max, other.max))
    }

    pub fn union(&self, other: &Interval) -> Interval {
        Interval::new(f32::min(self.min, other.min), f32::max(self.max, other.max))
    }
}

pub struct Ray {
    pub origin: Vec3,
    pub direction: Vec3,
}

impl Ray {
    pub fn new(origin: &Vec3, direction: &Vec3) -> Ray {
        assert!(abs_diff_eq!(glm::length(&direction), 1.0, epsilon = 0.00001));
        Ray { origin: *origin, direction: *direction }
    }

    pub fn point(&self, t: f32) -> Vec3 {
        self.origin + t * self.direction
    }

    pub fn intersect_plane(&self, plane: &Plane) -> f32 {
        let dot_dir_norm = glm::dot(&self.direction, &plane.normal);
        if abs_diff_eq!(dot_dir_norm, 0.0) {
            return std::f32::NAN;
        }

        (-plane.d - glm::dot(&self.origin, &plane.normal)) / dot_dir_norm
    }

    pub fn intersect_tri(&self, tri: &Triangle) -> f32 {
        let plane = tri.get_plane();
        let t = self.intersect_plane(&plane);
        if t.is_nan() {
            return t;
        }

        let p = self.point(t);
        //let ep = plane.eval(&p);
        //assert!(abs_diff_eq!(&ep, &0.0, epsilon =0.00001f32));

        let v0 = tri.points[1] - tri.points[0];
        let v1 = tri.points[2] - tri.points[0];
        let v2 = p - tri.points[0];
        let d00 = glm::dot(&v0, &v0);
        let d01 = glm::dot(&v0, &v1);
        let d11 = glm::dot(&v1, &v1);
        let d20 = glm::dot(&v2, &v0);
        let d21 = glm::dot(&v2, &v1);
        let denom = d00 * d11 - d01 * d01;
        let b1 = (d11 * d20 - d01 * d21) / denom;
        let b2 = (d00 * d21 - d01 * d20) / denom;
        let bary = Vec3::new(1.0 - b1 - b2, b1, b2);

        if glm::all(&glm::greater_than_equal(&bary, &glm::zero::<Vec3>())) {
            t
        } else {
            std::f32::NAN
        }
    }

    pub fn intersect_box(&self, box3: &Box3) -> Interval {
        let mut int = Interval::all();

        for d in 0..3 {
            if abs_diff_eq!(self.direction[d], 0.0) {
                if self.origin[d] < box3.min[d] || box3.max[d] < self.origin[d] {
                    return Interval::empty();
                }
                continue;
            }
            let mut dim_int = Interval::new((box3.min[d] - self.origin[d]) / self.direction[d], (box3.max[d] - self.origin[d]) / self.direction[d]);
            if self.direction[d] < 0.0 {
                mem::swap(&mut dim_int.min, &mut dim_int.max);
            }
            int = int.intersection(&dim_int);
            if int.is_empty() {
                return Interval::empty();
            }
        }

        int
    }

    pub fn intersect_sphere(&self, sphere: &Sphere) -> Interval {
        let co = self.origin - sphere.center;
        let (x0, x1) = solve_quadratic(1.0, 2.0 * Vec3::dot(&co, &self.direction), Vec3::dot(&co, &co) - (sphere.radius * sphere.radius));
        if x0.is_nan() {
            Interval::empty()
        } else {
            Interval::new(x0, x1)
        }
    }
}

pub struct Plane {
    pub normal: Vec3,
    pub d: f32,
}

impl Plane {
    pub fn new(normal: &Vec3, d: f32) -> Plane {
        Plane { normal: glm::normalize(normal), d }
    }

    pub fn from_point(normal: &Vec3, point: &Vec3) -> Plane {
        Plane { normal: glm::normalize(normal), d: -glm::dot(normal, point) }
    }
   
    pub fn eval(&self, point: &Vec3) -> f32 {
        glm::dot(&self.normal, point) + self.d
    }
}

pub struct Triangle {
    pub points: [Vec3; 3]
}

impl Triangle {
    pub fn new(p0: &Vec3, p1: &Vec3, p2: &Vec3) -> Triangle {
        Triangle { points: [*p0, *p1, *p2] }
    }

    pub fn get_point(&self, bary: &Vec3) -> Vec3 {
        self.points[0] * bary[0] + self.points[1] * bary[1] + self.points[2] * bary[2]
    }

    pub fn get_normal(&self) -> Vec3 {
        let e0 = self.points[0] - self.points[2];
        let e1 = self.points[1] - self.points[0];
        glm::cross(&e0, &e1)
    }

    pub fn get_plane(&self) -> Plane {
        Plane::from_point(&self.get_normal(), &self.points[0])
    }
}

fn vec3_cardinal(dim: usize, e: f32) -> Vec3 {
    let mut v = Vec3::zeros();
    v[dim] = e;
    v
}

pub struct Box3 {
    pub min: Vec3,
    pub max: Vec3,
}

impl Box3 {
    pub fn new(min: &Vec3, max: &Vec3) -> Box3 {
        Box3{  
            min: *min, 
            max: *max,
        }
    }

    pub fn get_size(&self) -> Vec3 {
        self.max - self.min
    }

    pub fn get_center(&self) -> Vec3 {
        0.5 * (self.min + self.max)
    }

    pub fn get_normal(&self, pos: &Vec3) -> Vec3 {
        let ratio = (pos - self.get_center()).component_div(&self.get_size());
        let mut dim = 0;
        for d in 1..3 {
            if ratio[d].abs() > ratio[dim].abs() {
                dim = d;
            }
        }
        vec3_cardinal(dim, ratio[dim].signum())
    }
}

pub struct Sphere {
    pub center: Vec3,
    pub radius: f32,
}

impl Sphere {
    pub fn new(center: &Vec3, radius: f32) -> Sphere {
        Sphere {
            center: *center,
            radius,
        }
    }

    pub fn get_normal(&self, pos: &Vec3) -> Vec3 {
        glm::normalize(&(pos - self.center))
    }
}

fn solve_quadratic(a: f32, b: f32, c: f32) -> (f32, f32) {
    assert!(!(abs_diff_eq!(a, 0.0) && abs_diff_eq!(b, 0.0)));
    if abs_diff_eq!(a, 0.0) {
        let x = -c/b;
        return (x, x);
    }

    let d = b*b - 4.0*a*c;
    if d < 0.0 {
        return (f32::NAN, f32::NAN);
    }
    let sqrt_d = d.sqrt();
    let x0 = (-b - sqrt_d) / (2.0 * a);
    let x1 = (-b + sqrt_d) / (2.0 * a);
    (x0, x1)
}