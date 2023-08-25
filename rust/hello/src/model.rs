use tobj;

use std::{vec::Vec, cmp::Ordering};

use glam::{Vec3, vec3, UVec3, uvec3};
use super::geom::{Box3, max_elem_index};

#[repr(C)]
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

        model.build_bvh();

        model
    }

    pub fn invert_triangle_order(&mut self) {
        self.triangles = self.triangles.chunks_exact(3).map(|t| [t[1], t[0], t[2]]).flatten().collect();
    }

    fn vertex(&self, index: u32) -> Vec3 {
        vec3(
            self.vertices[(index * 3 + 0) as usize], 
            self.vertices[(index * 3 + 1) as usize], 
            self.vertices[(index * 3 + 2) as usize])
    }

    fn triangle_bound(&self, tri: &UVec3) -> Box3 {
        Box3::from_min_and_size(self.vertex(tri[0]), Vec3::ZERO)
            .union_point(self.vertex(tri[1]))
            .union_point(self.vertex(tri[2]))
    }

    fn build_bvh(&mut self) {
        assert_eq!(self.bvh.len(), 0);

        let mut tris = self.triangles
            .chunks_exact(3)
            .map(|t| UVec3::from_slice(t))
            .collect::<Vec<_>>();

        self.create_bvh_node(&mut tris, 0, 8);

        self.triangles = tris
            .iter()
            .flat_map(|t| t.to_array())
            .collect();
    }

    fn create_bvh_node(&mut self, tris: &mut [UVec3], tri_start: u32, num_leaf_tris: u32) -> Option<u32> {
        if tris.is_empty() {
            return None;
        }
        let mut bound = Box3::EMPTY;
        for tri in tris.iter() {
            bound = bound.union(&self.triangle_bound(tri));
        }

        let bound_index = self.bvh.len();
        self.bvh.push(BoundNode { bound, tri_start, tri_end: tri_start, child_ind: [u32::MAX; 2] });

        if tris.len() <= num_leaf_tris as usize {
            self.bvh[bound_index].tri_end = tri_start + tris.len() as u32;
            return Some(bound_index as u32);
        }

        let dim = max_elem_index(bound.size()) as usize;

        tris.select_nth_unstable_by(tris.len() / 2, |a, b| {
            let bound_a = self.triangle_bound(a);
            let bound_b = self.triangle_bound(b);

            let mut ord = bound_a.min[dim].partial_cmp(&bound_b.min[dim]).unwrap();
            if ord == Ordering::Equal {
                ord = bound_a.max[dim].partial_cmp(&bound_b.max[dim]).unwrap();
            }
            ord
        });

        let (left, right) = tris.split_at_mut(tris.len() / 2);


        self.bvh[bound_index].child_ind = [
            self.create_bvh_node(left, tri_start, num_leaf_tris).unwrap_or(u32::MAX),
            self.create_bvh_node(right, tri_start + left.len() as u32, num_leaf_tris).unwrap_or(u32::MAX)];

        return Some(bound_index as u32);
    }
}