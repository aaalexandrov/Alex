use tobj;

use std::{vec::Vec, cmp::Ordering};

use glam::{Vec3, vec3, UVec4, uvec4};
use super::geom::{Box3, max_elem_index};

use ahash::{HashMap, HashMapExt};

pub struct Material {
    pub albedo: [f32; 3],
    pub shininess: f32,
}

pub struct BoundNode {
    pub bound: Box3,
    pub tri_start: u32,
    pub tri_end: u32,
    pub child_ind: [u32; 2],
}

pub struct Model {
    pub positions: Vec::<f32>,
    pub normals: Vec::<f32>,
    pub triangles: Vec::<u32>,
    pub bvh: Vec::<BoundNode>,
    pub bvh_start_index: u32,
    pub materials: Vec::<Material>,
    pub triangle_material_indices: Vec<u32>,
}

impl Model {
    pub fn load_obj(path: &str, model_index: usize) -> Model {
        let (obj, mat) = tobj::load_obj(path, &tobj::GPU_LOAD_OPTIONS).unwrap();

        let mut model = Model { 
            positions: Vec::<f32>::new(), 
            normals: Vec::<f32>::new(), 
            triangles: Vec::<u32>::new(), 
            bvh: Vec::<BoundNode>::new(),
            bvh_start_index: 0,
            materials: Vec::<Material>::new(),
            triangle_material_indices: Vec::<u32>::new(),
        };

        let indices = if model_index == usize::MAX { 0..obj.len() } else { model_index..(model_index + 1) };
        for i in indices {
            let mesh = &obj[i].mesh;
            let base_triangle = (model.positions.len() / 3) as u32;
            model.triangles.extend(mesh.indices.iter().map(|x| x + base_triangle));
            model.positions.extend_from_slice(&mesh.positions);
            model.normals.extend_from_slice(&mesh.normals);
            model.triangle_material_indices.extend(std::iter::repeat(
                mesh.material_id.unwrap_or(0) as u32
            ).take(mesh.indices.len() / 3));
        }

        if model.normals.len() != model.positions.len() {
            model.generate_normals();
        }

        if let Ok(materials) = mat {
            for m in materials.iter() {
                model.materials.push(Material {
                    albedo: m.diffuse.unwrap(),
                    shininess: m.shininess.unwrap_or(30.0),
                });
            }
        }

        if model.materials.is_empty() {
            model.materials.push(Material { 
                albedo: [0.8; 3], 
                shininess: 30.0,
            });
        }

        model.build_bvh();

        model
    }

    pub fn invert_triangle_order(&mut self) {
        self.triangles = self.triangles.chunks_exact(3).map(|t| [t[1], t[0], t[2]]).flatten().collect();
    }

    fn position(&self, index: u32) -> Vec3 {
        vec3(
            self.positions[(index * 3 + 0) as usize], 
            self.positions[(index * 3 + 1) as usize], 
            self.positions[(index * 3 + 2) as usize])
    }

    fn triangle_normal(&self, tri_index: u32) -> Vec3 {
        let i = (tri_index * 3) as usize;
        let v0 = self.position(self.triangles[i + 0]);
        let v1 = self.position(self.triangles[i + 1]);
        let v2 = self.position(self.triangles[i + 2]);
        let norm = (v0 - v1).cross(v2 - v0).normalize();
        norm
    }

    pub fn generate_normals(&mut self) {
        let mut vertex_triangles = HashMap::<u32, Vec<u32>>::new();
        for (pos_i, vert_i) in self.triangles.iter().enumerate() {
            if !vertex_triangles.contains_key(vert_i) {
                vertex_triangles.insert(*vert_i, Vec::new());
            }
            let tris = vertex_triangles.get_mut(vert_i).unwrap();
            tris.push((pos_i / 3) as u32);
        }

        self.normals.clear();

        for i in 0..self.positions.len() as u32 {
            let mut norm = Vec3::Z;
            if let Some(tris) = vertex_triangles.get(&i) {
                // todo: instead of average, calculate the containing cone & get its axis
                norm = tris.iter()
                    .map(|t| self.triangle_normal(*t))
                    .sum();
                norm = (norm / (tris.len() as f32)).normalize(); 
            }
            self.normals.extend_from_slice(&norm.to_array());

        }
    }

    fn triangle_bound(&self, tri: &UVec4) -> Box3 {
        Box3::from_min_and_size(self.position(tri[0]), Vec3::ZERO)
            .union_point(self.position(tri[1]))
            .union_point(self.position(tri[2]))
    }

    fn build_bvh(&mut self) {
        assert_eq!(self.bvh.len(), 0);

        let mut tris = self.triangles
            .chunks_exact(3)
            .zip(self.triangle_material_indices.iter().cloned())
            .map(|t| uvec4(t.0[0], t.0[1], t.0[2], t.1))
            .collect::<Vec<_>>();

        self.create_bvh_node(&mut tris, 0, 1);

        self.triangles = tris
            .iter()
            .flat_map(|t| [t[0], t[1], t[2]])
            .collect();

        self.triangle_material_indices = tris
            .iter()
            .map(|t| t[3])
            .collect();
    }

    fn create_bvh_node(&mut self, tris: &mut [UVec4], tri_start: u32, num_leaf_tris: u32) -> Option<u32> {
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