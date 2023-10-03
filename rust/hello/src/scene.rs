use core::array;
use std::vec::Vec;
use std::sync::{Arc, RwLock};
use glam::*;
use super::geom::*;
use super::model::*;

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
}

pub struct Scene {
    pub root: Box<SceneNode>,
    pub min_size: f32,
}

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
                    let mut min_bound = *bound;
                    min_bound.max[split_dim as usize] = split_val;
                    Box::new(SceneNode::new(min_bound))
                });
            } else if split_val <= bound.min[split_dim as usize] {
                node = node.children[1].get_or_insert_with(|| {
                    let mut max_bound = *bound;
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

    
}
