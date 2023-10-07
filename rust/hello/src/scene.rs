use core::array;
use std::collections::HashMap;
use by_address::ByAddress;
use std::vec::Vec;
use std::sync::{Arc, RwLock};
use glam::*;
use super::geom::*;
use super::model::*;
use super::gen;

pub struct SceneObject {
    pub model: Arc<Model>,
    pub transform: Mat4,
    pub bound: Box3,
}

impl SceneObject {
    pub fn new(model: Arc<Model>) -> Arc<RwLock<SceneObject>> {
        Arc::new(RwLock::new(SceneObject { 
            model, 
            transform: Mat4::IDENTITY, 
            bound: Box3::EMPTY,
        }))
    }

    fn get_model_instance(&self, bvh_start_index: usize) -> gen::ModelInstance {
        gen::ModelInstance {
            box_min: self.bound.min.to_array(),
            bvh_start: bvh_start_index as u32,
            box_max: self.bound.max.to_array(),
            bvh_end: (bvh_start_index + self.model.bvh.len()) as u32,
            transform: self.transform.to_cols_array_2d(),
            inv_transform: self.transform.inverse().to_cols_array_2d(),
        }
    }
}

pub struct SceneNode {
    pub bound: Box3,
    pub objects: Vec<Arc<RwLock<SceneObject>>>,
    pub children: [Option<Box<SceneNode>>; 2],
}

impl SceneNode {
    fn new(bound: Box3) -> SceneNode {
        SceneNode {
            bound,
            objects: Vec::new(),
            children: array::from_fn(|_| None),
        }
    }

    fn get_split_dim(&self) -> u32 {
        max_elem_index(self.bound.size())
    }

    fn get_bound_middle(&self, dim: u32) -> f32 {
        (self.bound.min[dim as usize] + self.bound.max[dim as usize]) / 2.0
    }

    fn add_object(&mut self, obj: Arc<RwLock<SceneObject>>) {
        assert!(self.objects.iter().position(|o| Arc::ptr_eq(o, &obj)).is_none());
        self.objects.push(obj);
    }

    fn remove_object(&mut self, obj: &Arc<RwLock<SceneObject>>) {
        let pos = self.objects.iter().position(|o| Arc::ptr_eq(o, &obj)).unwrap();
        self.objects.swap_remove(pos);
    }

    fn for_objects<EnumFn>(&self, func: &mut EnumFn)
        where EnumFn: FnMut(&Arc<RwLock<SceneObject>>) {
        for o in self.objects.iter() {
            func(o);
        }

        for child in self.children.iter().filter_map(|o| o.as_ref()) {
            child.for_objects(func);
        }
    }

    fn count_nodes_objects(&self) -> (usize, usize) {
        let (mut nodes, mut objects) = (1, self.objects.len());

        for child in self.children.iter().filter_map(|o| o.as_ref()) {
            let (child_nodes, child_objects) = child.count_nodes_objects();
            nodes += child_nodes;
            objects += child_objects;
        }
        (nodes, objects)
    }

    fn get_nodes_objects(&self, models_data: &ModelDataMap, nodes: &mut [gen::TreeNode], mut next_node: usize, objects: &mut [gen::ModelInstance], mut next_object: usize) -> (usize, usize) {
        let node_index = next_node;
        nodes[node_index] = gen::TreeNode {
            box_min: self.bound.min.to_array(),
            content_start: next_object as u32,
            box_max: self.bound.max.to_array(),
            content_end: (next_object + self.objects.len()) as u32,
            child: array::from_fn(|_| u32::MAX),
            _pad: array::from_fn(|_| 0),
        };
        for i in 0..self.objects.len() {
            let o = self.objects[i].read().unwrap();
            let bvh_start_index = models_data.get(&ByAddress(o.model.clone())).unwrap().bvh_start_index;
            objects[next_object + i] = o.get_model_instance(bvh_start_index);
        }

        next_node += 1;
        next_object += self.objects.len();

        for i in 0..self.children.len() {
            if let Some(child) = &self.children[i] {
                nodes[node_index].child[i] = next_node as u32;
                (next_node, next_object) = child.get_nodes_objects(models_data, nodes, next_node, objects, next_object);
            }
        }
        (next_node, next_object)
    }
}

#[derive(Clone, Copy)]
pub struct ModelData {
    pub bvh_start_index: usize,
    pub tri_start_index: usize,
    pub vert_start_index: usize,
    pub tri_mat_indices_start_index: usize,
    pub mat_start_index: usize,
}

type ModelDataMap = HashMap<ByAddress<Arc<Model>>, ModelData>;

pub struct Scene {
    pub root: Box<SceneNode>,
    pub min_size: f32,
    pub models: ModelDataMap,
    pub model_last_data: ModelData,
}

#[allow(dead_code)]
impl Scene {
    pub fn new(bound: Box3, min_size: f32) -> Scene {
        Scene {
            root: Box::new(SceneNode::new(bound)),
            min_size,
            models: ModelDataMap::new(),
            model_last_data: ModelData { 
                bvh_start_index: 0,
                tri_start_index: 0,
                vert_start_index: 0,
                tri_mat_indices_start_index: 0,
                mat_start_index: 0,
            },
        }
    }

    fn get_node(&mut self, bound: &Box3) -> &mut SceneNode {
        let mut node = self.root.as_mut();
        assert!(node.bound.contains(bound));
        loop {
            let size = node.bound.size();
            if size.cmple(Vec3::splat(self.min_size)).any() {
                return node;
            }
            let split_dim = node.get_split_dim();
            let split_val = node.get_bound_middle(split_dim);
            if bound.max[split_dim as usize] <= split_val {
                node = node.children[0].get_or_insert_with(|| {
                    let mut min_bound = node.bound;
                    min_bound.max[split_dim as usize] = split_val;
                    Box::new(SceneNode::new(min_bound))
                });
            } else if split_val <= bound.min[split_dim as usize] {
                node = node.children[1].get_or_insert_with(|| {
                    let mut max_bound = node.bound;
                    max_bound.min[split_dim as usize] = split_val;
                    Box::new(SceneNode::new(max_bound))
                });
            } else {
                return node;
            }
        }
    }

    pub fn set_transform(&mut self, obj: Arc<RwLock<SceneObject>>, transform: Option<Mat4>) {
        let mut o = obj.write().unwrap();
        let existing_obj = !o.bound.is_empty();
        if existing_obj {
            let prev_node = self.get_node(&o.bound);
            prev_node.remove_object(&obj);
        }
        if let Some(trans) = transform {
            if !existing_obj {
                self.add_model_data(&o.model);
            }
            o.transform = trans;
            o.bound = o.model.bvh[0].bound.get_transformed(trans);
            let node = self.get_node(&o.bound);
            drop(o);
            node.add_object(obj);
        } else {
            o.bound = Box3::EMPTY;
        }
    }

    fn add_model_data(&mut self, model: &Arc<Model>) {
        let model_key = ByAddress(model.clone());
        if !self.models.contains_key(&model_key) {
            self.models.insert(model_key, self.model_last_data);

            assert!(model.positions.len() > 0);
            assert_eq!(model.positions.len(), model.normals.len());
            self.model_last_data.bvh_start_index += model.bvh.len();
            self.model_last_data.tri_start_index += model.triangles.len();
            self.model_last_data.vert_start_index += model.positions.len();
            self.model_last_data.tri_mat_indices_start_index += model.triangle_material_indices.len();
            self.model_last_data.mat_start_index += model.materials.len();
        }
    }

    pub fn count_nodes_objects(&self) -> (usize, usize) {
        self.root.count_nodes_objects()
    }

    pub fn get_nodes_objects(&self, nodes: &mut [gen::TreeNode], objects: &mut [gen::ModelInstance]) -> (usize, usize) {
        self.root.get_nodes_objects(&self.models, nodes, 0, objects, 0)
    }

    pub fn get_models_data(&self, positions: &mut [f32], normals: &mut [f32], triangles: &mut [u32], bvh: &mut [gen::TreeNode], triangle_material_indices: &mut [u32], materials: &mut [gen::MaterialData]) {
        let mut models: Vec<_> = self.models.iter().collect();
        models.sort_by(|a, b| a.1.vert_start_index.cmp(&b.1.vert_start_index));
        for (ByAddress(model), data) in models {
            positions[data.vert_start_index..data.vert_start_index+model.positions.len()].clone_from_slice(&model.positions);
            normals[data.vert_start_index..data.vert_start_index+model.normals.len()].clone_from_slice(&model.normals);

            assert_eq!(data.vert_start_index % 3, 0);
            let vert_start_index = (data.vert_start_index / 3) as u32;
            for (i, tri) in model.triangles[..]
                .chunks_exact(3)
                .map(|t| UVec3::from_slice(t) + vert_start_index)
                .enumerate() {
                let tri_start_index = data.tri_start_index + 3 * i;
                tri.write_to_slice(&mut triangles[tri_start_index..tri_start_index+3]);
            }

            assert_eq!(data.tri_start_index % 3, 0);
            let tri_start_index = (data.tri_start_index / 3) as u32;
            for (i, b) in model.bvh.iter().enumerate() {
                bvh[data.bvh_start_index + i] = gen::TreeNode {
                    box_min: b.bound.min.to_array(), 
                    content_start: b.tri_start + tri_start_index,
                    box_max: b.bound.max.to_array(),
                    content_end: b.tri_end + tri_start_index,
                    child: b.child_ind.map(|c| if c < u32::MAX { c + data.bvh_start_index as u32 } else { u32::MAX }),
                    _pad: [0; 2],
                }
            }

            for (i, tri_mat_ind) in model.triangle_material_indices.iter().enumerate() {
                triangle_material_indices[data.tri_mat_indices_start_index + i] = tri_mat_ind + data.mat_start_index as u32;
            }

            for (i, m) in model.materials.iter().enumerate() {
                materials[data.mat_start_index + i] = gen::MaterialData {
                    albedo: m.albedo,
                    power: m.shininess,
                };
            }
        }
    }

    pub fn for_objects<EnumFn>(&self, mut func: EnumFn)
        where EnumFn: FnMut(&Arc<RwLock<SceneObject>>) {
        self.root.for_objects(&mut func);
    }

    pub fn collect_objects(&self) -> Vec<Arc<RwLock<SceneObject>>> {
        let mut objs = Vec::new();
        self.root.for_objects(&mut |o: &Arc<RwLock<SceneObject>>| objs.push(o.clone()));
        return objs;
    }
}
