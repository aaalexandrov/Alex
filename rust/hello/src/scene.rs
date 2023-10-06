use core::array;
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

    fn get_model_instance(&self) -> gen::ModelInstance {
        gen::ModelInstance {
            box_min: self.bound.min.to_array(),
            bvh_start: self.model.bvh_start_index,
            box_max: self.bound.max.to_array(),
            bvh_end: self.model.bvh_start_index + self.model.bvh.len() as u32,
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

    fn get_nodes_objects(&self, nodes: &mut [gen::TreeNode], mut next_node: usize, objects: &mut [gen::ModelInstance], mut next_object: usize) -> (usize, usize) {
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
            objects[next_object + i] = self.objects[i].read().unwrap().get_model_instance();
        }

        next_node += 1;
        next_object += self.objects.len();

        for i in 0..self.children.len() {
            if let Some(child) = &self.children[i] {
                nodes[node_index].child[i] = next_node as u32;
                (next_node, next_object) = child.get_nodes_objects(nodes, next_node, objects, next_object);
            }
        }
        (next_node, next_object)
    }
}

pub struct Scene {
    pub root: Box<SceneNode>,
    pub min_size: f32,
}

#[allow(dead_code)]
impl Scene {
    pub fn new(bound: Box3, min_size: f32) -> Scene {
        Scene {
            root: Box::new(SceneNode::new(bound)),
            min_size,
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
        if !o.bound.is_empty() {
            let prev_node = self.get_node(&o.bound);
            prev_node.remove_object(&obj);
        }
        if let Some(trans) = transform {
            o.transform = trans;
            o.bound = o.model.bvh[0].bound.get_transformed(trans);
            let node = self.get_node(&o.bound);
            drop(o);
            node.add_object(obj);
        } else {
            o.bound = Box3::EMPTY;
        }
    }

    pub fn count_nodes_objects(&self) -> (usize, usize) {
        self.root.count_nodes_objects()
    }

    pub fn get_nodes_objects(&self, nodes: &mut [gen::TreeNode], objects: &mut [gen::ModelInstance]) -> (usize, usize) {
        self.root.get_nodes_objects(nodes, 0, objects, 0)
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
