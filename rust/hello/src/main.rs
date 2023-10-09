use gen::ModelsBuffers;
use winit::event::VirtualKeyCode;

use winit_input_helper::WinitInputHelper;

use std::{sync::{Arc, Mutex}, env, time::{SystemTime, Duration}};

use vulkano::{
    memory::allocator::{AllocationCreateInfo, MemoryAllocator},
    command_buffer::{allocator::StandardCommandBufferAllocator, AutoCommandBufferBuilder, CommandBufferUsage, RenderingAttachmentInfo, CopyBufferInfo},
    image::{ImageUsage, Image, view::{ImageView, ImageViewCreateInfo}, ImageCreateInfo, sampler::{Sampler, SamplerCreateInfo, SamplerMipmapMode, Filter}},
    pipeline::{
        graphics::viewport::Viewport, Pipeline, PipelineBindPoint, 
    }, 
    render_pass::{AttachmentLoadOp, AttachmentStoreOp}, descriptor_set::{PersistentDescriptorSet, WriteDescriptorSet}, buffer::{BufferUsage, Subbuffer, }, format::Format, DeviceSize, swapchain::PresentMode, 
};

use glam::{Mat4, Vec3, Quat, uvec2, UVec2, Vec4Swizzles};

mod app;
use app::App;

mod geom;
use geom::Box3;

mod cam;
use cam::Camera;

mod model;
use model::Model;

mod scene;
use scene::{Scene, SceneObject};

mod solid;

mod font;
use font::{FixedFontData, FixedFont};

mod gen;

use std::f32::consts::PI;

fn main() {
    const RENDER_SIZE: UVec2 = uvec2(1024, 768);

    let device_index: usize = if let Some(num) = env::args().nth(1) { num.parse().unwrap() } else { 0 };

    let mut app = App::new("Rayz", [RENDER_SIZE.x, RENDER_SIZE.y], device_index);

    let cmd_buffer_allocator = StandardCommandBufferAllocator::new(app.renderer.device.clone(), Default::default());
    let mut upload_builder = AutoCommandBufferBuilder::primary(&cmd_buffer_allocator, app.renderer.queue.queue_family_index(), CommandBufferUsage::OneTimeSubmit).unwrap();

    let swapchain_format = app.swapchain_image_views[0].format();
    let mut font = FixedFont::new(FixedFontData::from_file("data/font_10x20.png", &app.renderer, swapchain_format, &mut upload_builder));

    let cam_transform_default = Mat4::from_translation(Vec3::new(0.0, 0.0, -10.0)) * Mat4::from_rotation_z(PI);
    let mut camera = Camera {
        transform: cam_transform_default, 
        projection: Mat4::IDENTITY,
    };

    //let (model_path, invert) = ("data/cessna.obj", true);
    //let (model_path, invert) = ("data/diamond.obj", false);
    //let (model_path, invert) = ("data/magnolia.obj", true);
    //let (model_path, invert) = ("data/airboat.obj", true);
    //let (model_path, invert) = ("data/b17green.obj", true);
    //let (model_path, invert) = ("data/b17silver.obj", true);
    
    const SCENE_SIZE: f32 = 1000.0;
    const MIN_SCENE_SIZE: f32 = 1.0;
    let mut scene = Scene::new(Box3::new(Vec3::splat(-SCENE_SIZE), Vec3::splat(SCENE_SIZE)), MIN_SCENE_SIZE);
    {
        let model = load_model("data/b17green.obj", true);
        let obj = SceneObject::new(model.clone());
        scene.set_transform(obj.clone(), Some(Mat4::from_rotation_y(PI/2.0)));

        let model = load_model("data/b17silver.obj", true);
        let obj = SceneObject::new(model.clone());
        scene.set_transform(obj.clone(), Some(Mat4::from_scale_rotation_translation(Vec3::splat(10.0), Quat::IDENTITY, Vec3::new(0.0, -50.0, 0.0))));
    }

    let models_buffers = get_models_buffers(&app, &scene, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST);

    let uniform_buffer = app.renderer.get_buffer::<gen::UniformData>(BufferUsage::UNIFORM_BUFFER | BufferUsage::TRANSFER_DST, 1);
    
    let compute_pipeline = app.renderer.load_compute_pipeline(gen::load(app.renderer.device.clone()).unwrap());
    let solid_pipeline = solid::load_pipeline(&app.renderer, false, swapchain_format);

    let mut viewport = Viewport {
        offset: [0.0, 0.0],
        extent: [0.0, 0.0],
        depth_range: 0.0..=1.0,
    };

    setup_for_window_size(app.swapchain_image_views[0].image().extent(), &mut viewport, &mut camera);
    
    let sampler_linear = Sampler::new(
        app.renderer.device.clone(),
        SamplerCreateInfo {
            mag_filter: Filter::Linear,
            min_filter: Filter::Linear,
            mipmap_mode: SamplerMipmapMode::Linear,
            ..SamplerCreateInfo::default()
        }
    ).unwrap();
    let storage_image = create_storage_image(app.renderer.allocator.clone(), [RENDER_SIZE.x, RENDER_SIZE.y, 1]);
    let storage_image_view = ImageView::new(
        storage_image.clone(), 
        ImageViewCreateInfo {
            usage: ImageUsage::STORAGE,
            ..ImageViewCreateInfo::from_image(&storage_image)
        }).unwrap();
    let storage_image_view_sampled = ImageView::new(
        storage_image.clone(), 
        ImageViewCreateInfo {
            usage: ImageUsage::SAMPLED,
            ..ImageViewCreateInfo::from_image(&storage_image)
        }).unwrap();

    let fullscreen_vertex_buffer = app.renderer.get_buffer_slice(BufferUsage::VERTEX_BUFFER, &solid::FULLSCREEN_QUAD_VERTICES);
    let fullscreen_index_buffer = app.renderer.get_buffer_slice(BufferUsage::INDEX_BUFFER, &solid::QUAD_INDICES);
    let fullscreen_uniform_buffer = font.data.uniform_buffer.clone();

    let solid_descriptor_set = PersistentDescriptorSet::new(
        &app.renderer.descriptor_set_allocator,
        solid_pipeline.layout().set_layouts()[0].clone(),
        [
            WriteDescriptorSet::buffer(0, fullscreen_uniform_buffer),
            WriteDescriptorSet::sampler(1, sampler_linear),
            WriteDescriptorSet::image_view(2, storage_image_view_sampled),
            //WriteDescriptorSet::image_view(2, font.data.texture_view.clone()),
        ],
        []
    ).unwrap();

    let upload_buffer = upload_builder.build().unwrap();

    let mut do_update = true;
    let mut poll_time = SystemTime::now();
    let start_time = Arc::new(poll_time);
    let frames = Arc::new(Mutex::new(0u64));

    let frames_clone = frames.clone();
    app.on_exit = Box::new(move || {
        let run_time = SystemTime::now().duration_since(*start_time).unwrap().as_secs_f64();
        let frames = *frames_clone.lock().unwrap();
        let fps = frames as f64 / run_time;
        println!("{fps:.2} fps - {frames} frames in {run_time:.2} seconds");
    });

    app.run(upload_buffer, move |app, swapchain_image_view| {
        *frames.lock().unwrap() += 1;
        let now = SystemTime::now();
        let delta = now.duration_since(poll_time).unwrap();
        poll_time = now;

        process_camera_input(&mut app.input, &mut camera, delta);
        if app.input.key_released(VirtualKeyCode::X) {
            camera.transform = cam_transform_default;
        }
        if app.input.key_released(VirtualKeyCode::V) {
            app.present_mode = if app.present_mode.is_none() { Some(PresentMode::Immediate) } else { None };
        }
        if app.input.key_released(VirtualKeyCode::P) {
            do_update = !do_update;
        }

        let window_extent = swapchain_image_view.image().extent();
        if window_extent[0] as f32 != viewport.extent[0] || window_extent[1] as f32 != viewport.extent[1] {
            setup_for_window_size(window_extent, &mut viewport, &mut camera)
        }

        let fps = 1.0 / delta.as_secs_f32();
        font.clear();
        font.add_str_pix(font.data.char_size * 2, UVec2::from_slice(&window_extent) * 2 / 3, format!("{fps:.2}").into(), solid::WHITE);

        if do_update {
            update_scene(&mut scene, delta);
        }
        let (model_inst_buffer, model_nodes_buffer) = get_scene_buffers(&app, &scene, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST);

        let compute_descriptor_set = PersistentDescriptorSet::new(
            &app.renderer.descriptor_set_allocator,
            compute_pipeline.layout().set_layouts()[0].clone(),
            [
                WriteDescriptorSet::buffer(0, uniform_buffer.clone()),
                WriteDescriptorSet::image_view(1, storage_image_view.clone()),
                WriteDescriptorSet::buffer(2, models_buffers.positions.clone()),
                WriteDescriptorSet::buffer(3, models_buffers.normals.clone()),
                WriteDescriptorSet::buffer(4, models_buffers.triangles.clone()),
                WriteDescriptorSet::buffer(5, models_buffers.bvh.clone()),
                WriteDescriptorSet::buffer(6, models_buffers.materials.clone()),
                WriteDescriptorSet::buffer(7, models_buffers.tri_material_indices.clone()),
                WriteDescriptorSet::buffer(8, model_inst_buffer.clone()),
                WriteDescriptorSet::buffer(9, model_nodes_buffer.clone())],
            []
        ).unwrap();

        let mut cmd_builder = AutoCommandBufferBuilder::primary(&cmd_buffer_allocator, app.renderer.queue.queue_family_index(), CommandBufferUsage::OneTimeSubmit).unwrap();
        cmd_builder
            .copy_buffer(CopyBufferInfo::buffers(app.renderer.get_buffer_data(
                BufferUsage::TRANSFER_SRC, 
                get_uniform_contents(&camera, &models_buffers, model_inst_buffer.len() as u32, model_nodes_buffer.len() as u32)),
                uniform_buffer.clone())).unwrap()
            .bind_pipeline_compute(compute_pipeline.clone()).unwrap()
            .bind_descriptor_sets(PipelineBindPoint::Compute, compute_pipeline.layout().clone(), 0, compute_descriptor_set.clone()).unwrap()
            .dispatch([div_ceil(storage_image.extent()[0], 8), div_ceil(storage_image.extent()[1], 8), 1]).unwrap()

            .begin_rendering(vulkano::command_buffer::RenderingInfo { color_attachments: vec![Some(RenderingAttachmentInfo{
                load_op: AttachmentLoadOp::Clear,
                store_op: AttachmentStoreOp::Store,
                clear_value: Some([0.0, 0.0, 1.0, 1.0].into()),
                ..RenderingAttachmentInfo::image_view(swapchain_image_view.clone())
                })], ..Default::default() }).unwrap()
            .set_viewport(
                0, 
                [viewport.clone()].into_iter().collect()
            ).unwrap();
    
        cmd_builder
            .bind_pipeline_graphics(solid_pipeline.clone()).unwrap()
            .bind_descriptor_sets(PipelineBindPoint::Graphics, solid_pipeline.layout().clone(), 0, solid_descriptor_set.clone()).unwrap()
            .bind_vertex_buffers(0, fullscreen_vertex_buffer.clone()).unwrap()
            .bind_index_buffer(fullscreen_index_buffer.clone()).unwrap()
            .draw_indexed(fullscreen_index_buffer.len() as u32, 1, 0, 0, 0).unwrap();

        font.render(&app.renderer, &mut cmd_builder);

        cmd_builder
            .end_rendering().unwrap();

        let cmd_buffer = cmd_builder.build().unwrap();
        
        cmd_buffer
    })
}

fn load_model(path: &str, invert: bool) -> Arc<Model> {
    let mut model = Model::load_obj(path, usize::MAX);
    if invert {
        model.invert_triangle_order();
    }
    if model.normals.len() != model.positions.len() {
        model.generate_normals();
    }
    Arc::new(model)
}

fn div_ceil(num: u32, denom: u32) -> u32 {
    (num + denom - 1) / denom
}

fn setup_for_window_size(extent: [u32; 3], viewport: &mut Viewport, camera: &mut Camera) {
    viewport.extent = [extent[0] as f32, extent[1] as f32];

    camera.projection = Mat4::perspective_lh(PI / 3.0, viewport.extent[0] / viewport.extent[1], 0.1, 100.0);
}

fn process_camera_input(input: &mut WinitInputHelper, camera: &mut Camera, delta: Duration) {
    let mut dpos = 12 as f32 * delta.as_secs_f32();
    let mut drot = PI / 1.5 * delta.as_secs_f32();

    if input.held_shift() {
        dpos *= 10.0;
        drot *= 3.0;
    }

    let mut delta_pos = Vec3::ZERO;
    let mut delta_rot = Quat::IDENTITY;

    if input.key_held(VirtualKeyCode::A) {
        delta_pos.x -= dpos;
    }
    if input.key_held(VirtualKeyCode::D) {
        delta_pos.x += dpos;
    }
    if input.key_held(VirtualKeyCode::R) {
        delta_pos.y -= dpos;
    }
    if input.key_held(VirtualKeyCode::F) {
        delta_pos.y += dpos;
    }
    if input.key_held(VirtualKeyCode::W) {
        delta_pos.z += dpos;
    }
    if input.key_held(VirtualKeyCode::S) {
        delta_pos.z -= dpos;
    }

    if input.key_held(VirtualKeyCode::Q) {
        delta_rot *= Quat::from_axis_angle(Vec3::Y, -drot);
    }
    if input.key_held(VirtualKeyCode::E) {
        delta_rot *= Quat::from_axis_angle(Vec3::Y, drot);
    }
    if input.key_held(VirtualKeyCode::T) {
        delta_rot *= Quat::from_axis_angle(Vec3::X, drot);
    }
    if input.key_held(VirtualKeyCode::G) {
        delta_rot *= Quat::from_axis_angle(Vec3::X, -drot);
    }
    if input.key_held(VirtualKeyCode::Z) {
        delta_rot *= Quat::from_axis_angle(Vec3::Z, -drot);
    }
    if input.key_held(VirtualKeyCode::C) {
        delta_rot *= Quat::from_axis_angle(Vec3::Z, drot);
    }

    camera.modify_transform(camera.transform.transform_vector3(delta_pos), delta_rot);
}

fn create_storage_image(mem_alloc: Arc<dyn MemoryAllocator>, extent: [u32; 3]) -> Arc<Image> {
    Image::new(
        mem_alloc,
        ImageCreateInfo {
            format: Format::R8G8B8A8_UNORM,
            extent,
            usage: ImageUsage::STORAGE | ImageUsage::SAMPLED,
            ..Default::default()
        },
        AllocationCreateInfo::default()
    ).unwrap()
}

fn get_uniform_contents(camera: &Camera, models_buffers: &ModelsBuffers, num_model_instances: u32, num_model_nodes: u32) -> gen::UniformData {
    gen::UniformData {
        view_proj: camera.view_proj().to_cols_array_2d(),
        background_color: [0.0, 0.0, 1.0, 1.0],
        camera_pos: camera.transform.w_axis.xyz().to_array(),
        num_tri_indices: models_buffers.triangles.len() as u32,
        num_vertices: models_buffers.positions.len() as u32,
        num_bvh_nodes: models_buffers.bvh.len() as u32,
        num_materials: models_buffers.materials.len() as u32,
        num_model_instances,
        num_model_nodes: num_model_nodes.into(),
        sun: gen::DirectionalLight { dir: [1.0 / f32::sqrt(3.0); 3].into(), color: [252.0 / 255.0, 229.0 / 255.0, 112.0 / 255.0].into(), ambient: [0.2; 3] },
    }
}

fn get_models_buffers(app: &App, scene: &Scene, usage: BufferUsage) -> gen::ModelsBuffers {
    let buffers = gen::ModelsBuffers {
        positions: app.renderer.get_buffer(usage, scene.model_last_data.vert_start_index as DeviceSize),
        normals: app.renderer.get_buffer(usage, scene.model_last_data.vert_start_index as DeviceSize),
        triangles: app.renderer.get_buffer(usage, scene.model_last_data.tri_start_index as DeviceSize),
        bvh: app.renderer.get_buffer(usage, scene.model_last_data.bvh_start_index as DeviceSize),
        tri_material_indices: app.renderer.get_buffer(usage, scene.model_last_data.tri_mat_indices_start_index as DeviceSize),
        materials: app.renderer.get_buffer(usage, scene.model_last_data.mat_start_index as DeviceSize),
    };
    scene.get_models_data(
        &mut buffers.positions.write().unwrap(), 
        &mut buffers.normals.write().unwrap(),
        &mut buffers.triangles.write().unwrap(),
        &mut buffers.bvh.write().unwrap(),
        &mut buffers.tri_material_indices.write().unwrap(), 
        &mut buffers.materials.write().unwrap());
    buffers
}

fn get_scene_buffers(app: &App, scene: &Scene, usage: BufferUsage) -> (Subbuffer<[gen::ModelInstance]>, Subbuffer<[gen::TreeNode]>) {
    let (scene_nodes, scene_objects) = scene.count_nodes_objects();
    let model_inst_buffer = app.renderer.get_buffer::<gen::ModelInstance>(usage, scene_objects as DeviceSize);
    let model_nodes_buffer = app.renderer.get_buffer::<gen::TreeNode>(usage, scene_nodes as DeviceSize);
    {
        let mut write_model_inst = model_inst_buffer.write().unwrap();
        let mut write_model_nodes = model_nodes_buffer.write().unwrap();
        scene.get_nodes_objects(&mut write_model_nodes, &mut write_model_inst);
    }

    (model_inst_buffer, model_nodes_buffer)
}

fn update_scene(scene: &mut Scene, delta: Duration) {

    let rot = Mat4::from_rotation_y(PI / 8.0 * delta.as_secs_f32());

    let objs = scene.collect_objects();
    for obj in objs.into_iter() {
        let trans = obj.read().unwrap().transform;
        scene.set_transform(obj, Some(trans * rot));
    }
}