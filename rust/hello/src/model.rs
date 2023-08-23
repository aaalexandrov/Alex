use tobj;

use std::vec::Vec;

pub struct Model {
    pub vertices: Vec::<f32>,
    pub triangles: Vec::<u32>,
}

impl Model {
    pub fn load_obj(path: &str, model_index: usize) -> Model {
        let (mut obj, _) = tobj::load_obj(path, &tobj::GPU_LOAD_OPTIONS).unwrap();
        let obj_mesh = obj.swap_remove(model_index).mesh;

        Model { 
            vertices: obj_mesh.positions, 
            triangles: obj_mesh.indices, 
        }
    }
}