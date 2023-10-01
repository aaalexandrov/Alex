use winit::{
    event::{VirtualKeyCode},
};

use winit_input_helper::WinitInputHelper;

use std::{sync::{Arc, Mutex}, env, time::{SystemTime, Duration}};

use vulkano::{
    memory::allocator::{AllocationCreateInfo, MemoryTypeFilter, MemoryAllocator},
    command_buffer::{allocator::{StandardCommandBufferAllocator}, AutoCommandBufferBuilder, CommandBufferUsage, RenderingAttachmentInfo, CopyBufferInfo},
    image::{ImageUsage, Image, view::{ImageView, ImageViewCreateInfo}, ImageCreateInfo, sampler::{Sampler, SamplerCreateInfo, SamplerMipmapMode, Filter}},
    pipeline::{
        graphics::{
            viewport::{Viewport, }, 
        }, Pipeline, PipelineBindPoint, 
    }, 
    render_pass::{AttachmentLoadOp, AttachmentStoreOp}, descriptor_set::{PersistentDescriptorSet, WriteDescriptorSet}, buffer::{Buffer, BufferCreateInfo, BufferUsage, }, format::Format, DeviceSize, swapchain::PresentMode, 
};

use glam::{Mat4, Vec3, Quat, uvec2, UVec2, Vec4Swizzles};

mod app;
use app::{App};

mod geom;

mod cam;
use cam::Camera;

mod model;
use model::Model;

mod solid;

mod font;
use font::{FixedFontData, FixedFont};

use std::f32::consts::PI;

mod cs {
    vulkano_shaders::shader! {
        ty: "compute",
        path: "src/gen.comp",
    }
}


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
    let (model_path, invert) = ("data/b17green.obj", true);
    //let (model_path, invert) = ("data/b17silver.obj", true);
    let mut model = Model::load_obj(model_path, usize::MAX);
    if invert {
        model.invert_triangle_order();
    }
    if model.normals.len() != model.positions.len() {
        model.generate_normals();
    }

    let uniform_buffer = Buffer::new_slice::<cs::UniformData>(
        app.renderer.allocator.clone(),
        BufferCreateInfo{
            usage: BufferUsage::UNIFORM_BUFFER | BufferUsage::TRANSFER_DST,
            ..Default::default()
        },
        AllocationCreateInfo{
            memory_type_filter: MemoryTypeFilter::PREFER_DEVICE,
            ..Default::default()
        },
        1
    ).unwrap();

    let positions_buffer = app.renderer.get_buffer_slice(BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, model.positions.as_slice());
    let normals_buffer = app.renderer.get_buffer_slice(BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, model.normals.as_slice());
    let triangles_buffer = app.renderer.get_buffer_slice(BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, model.triangles.as_slice());
    let bvh_buffer = app.renderer.get_buffer_write(BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, model.bvh.len() as DeviceSize, |bvh: &mut [cs::TreeNode]| {
        for i in 0..bvh.len() {
            let b = &model.bvh[i];
            bvh[i] = cs::TreeNode {
                box_min: b.bound.min.to_array(), 
                content_start: b.tri_start,
                box_max: b.bound.max.to_array(),
                content_end: b.tri_end,
                child: b.child_ind,
                _pad: [0; 2],
            }
        }
    });
    let material_buffer = app.renderer.get_buffer_write(BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, model.materials.len() as DeviceSize, |mat: &mut [cs::MaterialData]| {
        for i in 0..mat.len() {
            let m = &model.materials[i];
            mat[i] = cs::MaterialData {
                albedo: m.albedo,
                power: m.shininess,
            };
        }
    });
    let tri_material_indices_buffer = app.renderer.get_buffer_slice(BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, model.triangle_material_indices.as_slice());

    let compute_pipeline = app.renderer.load_compute_pipeline(cs::load(app.renderer.device.clone()).unwrap());
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
    let compute_descriptor_set = PersistentDescriptorSet::new(
        &app.renderer.descriptor_set_allocator,
        compute_pipeline.layout().set_layouts()[0].clone(),
        [
            WriteDescriptorSet::buffer(0, uniform_buffer.clone()),
            WriteDescriptorSet::image_view(1, storage_image_view),
            WriteDescriptorSet::buffer(2, positions_buffer.clone()),
            WriteDescriptorSet::buffer(3, normals_buffer.clone()),
            WriteDescriptorSet::buffer(4, triangles_buffer.clone()),
            WriteDescriptorSet::buffer(5, bvh_buffer.clone()),
            WriteDescriptorSet::buffer(6, material_buffer.clone()),
            WriteDescriptorSet::buffer(7, tri_material_indices_buffer.clone()),],
        []
    ).unwrap();

    let fullscreen_vertex_buffer = Buffer::from_iter(
        app.renderer.allocator.clone(),
        BufferCreateInfo {
            usage: BufferUsage::VERTEX_BUFFER,
            ..Default::default()
        },
        AllocationCreateInfo {
            memory_type_filter: MemoryTypeFilter::PREFER_DEVICE
                | MemoryTypeFilter::HOST_SEQUENTIAL_WRITE,
            ..Default::default()
        },
        solid::FULLSCREEN_QUAD_VERTICES,
    ).unwrap();
    let fullscreen_index_buffer = Buffer::from_iter(
        app.renderer.allocator.clone(),
        BufferCreateInfo {
            usage: BufferUsage::INDEX_BUFFER,
            ..Default::default()
        },
        AllocationCreateInfo {
            memory_type_filter: MemoryTypeFilter::PREFER_DEVICE
                | MemoryTypeFilter::HOST_SEQUENTIAL_WRITE,
            ..Default::default()
        },
        solid::QUAD_INDICES,
    ).unwrap();
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

        let window_extent = swapchain_image_view.image().extent();
        if window_extent[0] as f32 != viewport.extent[0] || window_extent[1] as f32 != viewport.extent[1] {
            setup_for_window_size(window_extent, &mut viewport, &mut camera)
        }

        let fps = 1.0 / delta.as_secs_f32();
        font.clear();
        font.add_str_pix(font.data.char_size * 2, UVec2::from_slice(&window_extent) * 2 / 3, format!("{fps:.2}").into(), solid::WHITE);

        let mut cmd_builder = AutoCommandBufferBuilder::primary(&cmd_buffer_allocator, app.renderer.queue.queue_family_index(), CommandBufferUsage::OneTimeSubmit).unwrap();
        cmd_builder
            .copy_buffer(CopyBufferInfo::buffers(app.renderer.get_buffer_data(BufferUsage::TRANSFER_SRC, get_uniform_contents(&camera, &model)), uniform_buffer.clone())).unwrap()
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

fn get_uniform_contents(camera: &Camera, model: &Model) -> cs::UniformData {
    cs::UniformData {
        view_proj: camera.view_proj().to_cols_array_2d(),
        background_color: [0.0, 0.0, 1.0, 1.0],
        camera_pos: camera.transform.w_axis.xyz().to_array(),
        num_tri_indices: model.triangles.len() as u32,
        num_vertices: model.positions.len() as u32,
        num_bvh_nodes: model.bvh.len() as u32,
        num_materials: (model.materials.len() as u32).into(),
        sun: cs::DirectionalLight { dir: [1.0 / f32::sqrt(3.0); 3].into(), color: [252.0 / 255.0, 229.0 / 255.0, 112.0 / 255.0].into(), ambient: [0.2; 3] },
    }
}

