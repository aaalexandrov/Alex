use vulkano_shaders;
use vulkano::buffer::Subbuffer;

vulkano_shaders::shader! {
    ty: "compute",
    path: "src/gen.comp",
}

pub struct ModelsBuffers {
    pub positions: Subbuffer<[f32]>,
    pub normals: Subbuffer<[f32]>, 
    pub triangles: Subbuffer<[u32]>, 
    pub bvh: Subbuffer<[TreeNode]>, 
    pub tri_material_indices: Subbuffer<[u32]>, 
    pub materials: Subbuffer<[MaterialData]>,
}