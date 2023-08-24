use tobj;

use std::vec::Vec;

use glam::{Vec3, vec3};
use super::geom::{Box3, max_elem_index};

pub struct BoundNode {
    pub bound: Box3,
    pub tri_start: u32,
    pub tri_end: u32,
    pub child_ind: [u32; 2],
}

pub struct Model {
    pub vertices: Vec::<f32>,
    pub triangles: Vec::<u32>,
    pub bvh: Vec::<BoundNode>,
}

impl Model {
    pub fn load_obj(path: &str, model_index: usize) -> Model {
        let (obj, _) = tobj::load_obj(path, &tobj::GPU_LOAD_OPTIONS).unwrap();

        let mut model = Model { 
            vertices: Vec::<f32>::new(), 
            triangles: Vec::<u32>::new(), 
            bvh: Vec::<BoundNode>::new(),
        };

        let indices = if model_index == usize::MAX { 0..obj.len() } else { model_index..(model_index + 1) };
        for i in indices {
            let mesh = &obj[i].mesh;
            let base_triangle = (model.vertices.len() / 3) as u32;
            model.triangles.extend(mesh.indices.iter().map(|x| x + base_triangle));
            model.vertices.extend_from_slice(&mesh.positions);
        }

        model
    }

    fn build_bvh(&mut self) {
        assert_eq!(self.bvh.len(), 0);
        self.create_bvh_node(0, self.triangles.len() as u32);
    }

    fn vertex(&self, index: u32) -> Vec3 {
        vec3(self.vertices[(index * 3 + 0) as usize], self.vertices[(index * 3 + 1) as usize], self.vertices[(index * 3 + 2) as usize])
    }

    fn create_bvh_node(&mut self, tris_begin: u32, tris_end: u32) -> Option<u32> {
        assert_eq!((tris_end - tris_begin) % 3, 0);
        if tris_begin >= tris_end {
            return None;
        }
        let mut bound = Box3::new_empty();
        for t in tris_begin..tris_end {
            bound = bound.union_point(&self.vertex(self.triangles[t as usize]));
        }

        let dim = max_elem_index(bound.size());



        return Some(u32::MAX);
    }
}