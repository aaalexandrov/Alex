use std::sync::Arc;

use glam::*;

use vulkano::{pipeline::{graphics::vertex_input::Vertex, GraphicsPipeline}, format::Format, buffer::BufferContents};

use super::app::Renderer;

mod vs {
    vulkano_shaders::shader! {
        ty: "vertex",
        path: "src/solid.vert",
    }
}

mod fs {
    vulkano_shaders::shader! {
        ty: "fragment",
        path: "src/solid.frag",
    }
}

pub type UniformData = vs::UniformData;

#[repr(C)]
#[derive(BufferContents, Vertex, Clone, Copy)]
pub struct SolidVertex {
    #[format(R32G32B32_SFLOAT)]
    pub pos: Vec3,
    #[format(R32G32_SFLOAT)]
    pub tc: Vec2,
    #[format(R8G8B8A8_UNORM)]
    pub color: [u8; 4],
}

pub const WHITE: [u8; 4] = [255, 255, 255, 255];

pub const FULLSCREEN_QUAD_VERTICES: [SolidVertex; 4] = [
    SolidVertex{pos: vec3(-1.0, -1.0, 0.0), tc: vec2(0.0, 0.0), color: WHITE},
    SolidVertex{pos: vec3( 1.0, -1.0, 0.0), tc: vec2(1.0, 0.0), color: WHITE},
    SolidVertex{pos: vec3(-1.0,  1.0, 0.0), tc: vec2(0.0, 1.0), color: WHITE},
    SolidVertex{pos: vec3( 1.0,  1.0, 0.0), tc: vec2(1.0, 1.0), color: WHITE},
];

pub const QUAD_INDICES: [u32; 6] = [0, 1, 2, 2, 1, 3];

pub fn load_pipeline(renderer: &Renderer, alpha_blend: bool, attachment_format: Format) -> Arc<GraphicsPipeline> {
    renderer.load_graphics_pipeline::<SolidVertex>(
        vs::load(renderer.device.clone()).unwrap(), 
        fs::load(renderer.device.clone()).unwrap(), 
        [(0, alpha_blend.into())].into_iter().collect(),
        alpha_blend,
        &[attachment_format])
}
