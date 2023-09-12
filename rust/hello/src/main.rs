use winit::{
    event::{VirtualKeyCode},
};

use winit_input_helper::WinitInputHelper;

use std::{sync::Arc, env, time::{SystemTime}};

use vulkano::{
    memory::allocator::{AllocationCreateInfo, MemoryTypeFilter, MemoryAllocator},
    command_buffer::{allocator::{StandardCommandBufferAllocator}, AutoCommandBufferBuilder, CommandBufferUsage, RenderingAttachmentInfo, CopyBufferInfo},
    image::{ImageUsage, Image, view::{ImageView, ImageViewCreateInfo}, ImageCreateInfo, sampler::{Sampler, SamplerCreateInfo, SamplerMipmapMode, Filter}},
    pipeline::{
        graphics::{
            viewport::{Viewport, }, 
        }, Pipeline, PipelineBindPoint, 
    }, 
    render_pass::{AttachmentLoadOp, AttachmentStoreOp}, descriptor_set::{PersistentDescriptorSet, WriteDescriptorSet}, buffer::{Buffer, BufferCreateInfo, BufferUsage, BufferContents, Subbuffer}, format::Format, DeviceSize, padded::Padded, 
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
use font::FixedFont;

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

    let app = App::new("Rayz", [RENDER_SIZE.x, RENDER_SIZE.y], device_index);


    let font = FixedFont::from_file("data/font_10x20.png", &app.renderer);

    let mut camera = Camera{
        transform: Mat4::from_translation(Vec3::new(0.0, 0.0, -10.0)) * Mat4::from_rotation_z(PI), 
        projection: Mat4::IDENTITY,
    };

    let mut model = Model::load_obj("data/cessna.obj", usize::MAX);
    model.invert_triangle_order();

    let uniform_buffer = Buffer::new_slice::<cs::UniformData>(
        app.renderer.allocator.as_ref(),
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

    let vertices_buffer = Buffer::new_slice::<f32>(
        &*app.renderer.allocator,
        BufferCreateInfo{
            usage: BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST,
            ..Default::default()
        },
        AllocationCreateInfo{
            memory_type_filter: MemoryTypeFilter::PREFER_DEVICE,
            ..Default::default()
        },
        model.vertices.len() as DeviceSize
    ).unwrap();

    let triangles_buffer = Buffer::new_slice::<u32>(
        &*app.renderer.allocator,
        BufferCreateInfo{
            usage: BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST,
            ..Default::default()
        },
        AllocationCreateInfo{
            memory_type_filter: MemoryTypeFilter::PREFER_DEVICE,
            ..Default::default()
        },
        model.triangles.len() as DeviceSize
    ).unwrap();

    let bvh_buffer = Buffer::new_slice::<cs::BvhNode>(
        &*app.renderer.allocator,
        BufferCreateInfo{
            usage: BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST,
            ..Default::default()
        },
        AllocationCreateInfo{
            memory_type_filter: MemoryTypeFilter::PREFER_DEVICE,
            ..Default::default()
        },
        model.bvh.len() as DeviceSize
    ).unwrap();



    let compute_pipeline = app.renderer.load_compute_pipeline(cs::load(app.renderer.device.clone()).unwrap());
    let solid_pipeline = solid::load_pipeline(&app.renderer, app.swapchain_image_views[0].format());

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
    let storage_image = create_storage_image(&*app.renderer.allocator, [RENDER_SIZE.x, RENDER_SIZE.y, 1]);
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
            WriteDescriptorSet::buffer(2, vertices_buffer.clone()),
            WriteDescriptorSet::buffer(3, triangles_buffer.clone()),
            WriteDescriptorSet::buffer(4, bvh_buffer.clone()),],
        []
    ).unwrap();

    let fullscreen_vertex_buffer = Buffer::from_iter(
        app.renderer.allocator.as_ref(),
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
        app.renderer.allocator.as_ref(),
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
    let fullscreen_uniform_buffer = Buffer::from_data(
        app.renderer.allocator.as_ref(),
        BufferCreateInfo {
            usage: BufferUsage::UNIFORM_BUFFER,
            ..Default::default()
        },
        AllocationCreateInfo {
            memory_type_filter: MemoryTypeFilter::PREFER_DEVICE
                | MemoryTypeFilter::HOST_SEQUENTIAL_WRITE,
            ..Default::default()
        },
        solid::UniformData{ view_proj: Mat4::IDENTITY.to_cols_array_2d() }
    ).unwrap();


    let solid_descriptor_set = PersistentDescriptorSet::new(
        &app.renderer.descriptor_set_allocator,
        solid_pipeline.layout().set_layouts()[0].clone(),
        [
            WriteDescriptorSet::buffer(0, fullscreen_uniform_buffer),
            WriteDescriptorSet::sampler(1, sampler_linear),
            WriteDescriptorSet::image_view(2, storage_image_view_sampled),],
        []
    ).unwrap();

    let cmd_buffer_allocator = StandardCommandBufferAllocator::new(app.renderer.device.clone(), Default::default());

    let mut upload_builder = AutoCommandBufferBuilder::primary(&cmd_buffer_allocator, app.renderer.queue.queue_family_index(), CommandBufferUsage::OneTimeSubmit).unwrap();
    upload_builder
        .copy_buffer(CopyBufferInfo::buffers(
            get_staging_slice(&*app.renderer.allocator, model.vertices.as_slice()), 
            vertices_buffer.clone())).unwrap()
        .copy_buffer(CopyBufferInfo::buffers(
            get_staging_slice(&*app.renderer.allocator, model.triangles.as_slice()), 
            triangles_buffer.clone())).unwrap()
        .copy_buffer(CopyBufferInfo::buffers(
            get_staging_slice(&*app.renderer.allocator, model.bvh.iter().map(|b| 
                cs::BvhNode {
                    box_min: b.bound.min.to_array(), 
                    tri_start: b.tri_start,
                    box_max: b.bound.max.to_array(),
                    tri_end: b.tri_end,
                    child: b.child_ind,
                    _pad: [0; 2],
                }
            ).collect::<Vec<_>>().as_slice()), 
            bvh_buffer.clone())).unwrap();

    let upload_buffer = upload_builder.build().unwrap();

    let mut poll_time = SystemTime::now();

    app.run(upload_buffer, move |app, swapchain_image_view| {
        process_camera_input(&mut app.input, &mut camera, &mut poll_time);

        let window_extent = swapchain_image_view.image().extent();
        if window_extent[0] as f32 != viewport.extent[0] || window_extent[1] as f32 != viewport.extent[1] {
            setup_for_window_size(window_extent, &mut viewport, &mut camera)
        }

        let mut cmd_builder = AutoCommandBufferBuilder::primary(&cmd_buffer_allocator, app.renderer.queue.queue_family_index(), CommandBufferUsage::OneTimeSubmit).unwrap();
        cmd_builder
            .copy_buffer(CopyBufferInfo::buffers(get_staging_buffer(&*app.renderer.allocator, get_uniform_contents(&camera, &model)), uniform_buffer.clone())).unwrap()
            .bind_pipeline_compute(compute_pipeline.clone()).unwrap()
            .bind_descriptor_sets(PipelineBindPoint::Compute, compute_pipeline.layout().clone(), 0, compute_descriptor_set.clone()).unwrap()
            .dispatch([div_ceil(storage_image.extent()[0], 8), div_ceil(storage_image.extent()[1], 8), 1]).unwrap()

            .begin_rendering(vulkano::command_buffer::RenderingInfo { color_attachments: vec![Some(RenderingAttachmentInfo{
                load_op: AttachmentLoadOp::Clear,
                store_op: AttachmentStoreOp::Store,
                clear_value: Some([0.0, 0.0, 1.0, 1.0].into()),
                ..RenderingAttachmentInfo::image_view(swapchain_image_view.clone())
                })], ..Default::default() }).unwrap()

            .bind_pipeline_graphics(solid_pipeline.clone()).unwrap()
            .set_viewport(
                0, 
                [viewport.clone()].into_iter().collect()
            ).unwrap()
            .bind_descriptor_sets(PipelineBindPoint::Graphics, solid_pipeline.layout().clone(), 0, solid_descriptor_set.clone()).unwrap()
            .bind_vertex_buffers(0, fullscreen_vertex_buffer.clone()).unwrap()
            .bind_index_buffer(fullscreen_index_buffer.clone()).unwrap()
            .draw_indexed(fullscreen_index_buffer.len() as u32, 1, 0, 0, 0).unwrap()

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

fn process_camera_input(input: &mut WinitInputHelper, camera: &mut Camera, time: &mut SystemTime) {

    let now = SystemTime::now();
    let delta = now.duration_since(*time).unwrap();
    *time = now;

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

fn create_storage_image(mem_alloc: &(impl MemoryAllocator + ?Sized), extent: [u32; 3]) -> Arc<Image> {
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
        num_vertices: model.vertices.len() as u32,
        num_bvh_nodes: Padded(model.bvh.len() as u32),
        material: cs::MaterialData { albedo: [0.8; 3], refraction_index: 1.5, power: 30.0 },
        sun: Padded(cs::DirectionalLight { dir: Padded([1.0 / f32::sqrt(3.0); 3]), color: Padded([252.0 / 255.0, 229.0 / 255.0, 112.0 / 255.0]), ambient: [0.2; 3] }),
    }
}

fn get_staging_buffer<T: BufferContents>(mem_alloc: &(impl MemoryAllocator + ?Sized), data: T) -> Subbuffer<T> {
    Buffer::from_data(
        mem_alloc,
        BufferCreateInfo{
            usage: BufferUsage::TRANSFER_SRC,
            ..Default::default()
        },
        AllocationCreateInfo{
            memory_type_filter: MemoryTypeFilter::PREFER_HOST | MemoryTypeFilter::HOST_SEQUENTIAL_WRITE,
            ..Default::default()
        },
        data,
    ).unwrap()
}

fn get_staging_slice<T: BufferContents + Clone>(mem_alloc: &(impl MemoryAllocator + ?Sized), data: &[T]) -> Subbuffer<[T]> {
    let buffer = Buffer::new_slice(
        mem_alloc,
        BufferCreateInfo{
            usage: BufferUsage::TRANSFER_SRC,
            ..Default::default()
        },
        AllocationCreateInfo{
            memory_type_filter: MemoryTypeFilter::PREFER_HOST | MemoryTypeFilter::HOST_SEQUENTIAL_WRITE,
            ..Default::default()
        },
        data.len() as DeviceSize,
    ).unwrap();

    {
        let mut writer = buffer.write().unwrap();
        for i in 0..data.len() {
            writer[i] = data[i].clone();
        }
    }

    buffer
}
