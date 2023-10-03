use vulkano_shaders;

vulkano_shaders::shader! {
    ty: "compute",
    path: "src/gen.comp",
}
