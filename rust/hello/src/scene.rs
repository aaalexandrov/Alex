use std::Arc;
use glam::*;
use super::geom::*;
use super::model::*;

#[derive(Clone, Copy)]
pub struct ModelInstance {
    pub model: Arc<Model>;
    pub transform: Mat4;
    pub bound: Box3;
}

pub struct SceneNode {
    pub bound: Box3;
    pub models: Vec<ModelInstance>;
    pub children: [Option<Box<SceneNode>>; 2];
}

pub struct Scene {
    pub root: Box<SceneNode>;
}

impl SceneNode {
    pub fn new(bound: Box3) -> Box<SceneNode> {
        Box::new(SceneNode {
            bound,
            vec!(),
            [None; 2]
        })
    }
}

impl Scene {
    pub fn new(bound: Box3) -> Scene {
        Scene {
            root: SceneNode::new(bound),
        }
    }
}
