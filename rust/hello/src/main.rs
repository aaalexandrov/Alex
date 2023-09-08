use winit::{
    event::{VirtualKeyCode},
    event_loop::EventLoop,
    window::WindowBuilder,
};

use winit_input_helper::WinitInputHelper;

use std::{sync::Arc, env, time::{SystemTime}};

use vulkano::{
    VulkanLibrary, Version, VulkanError, Validated,
    sync::{self, GpuFuture},
    swapchain::{Surface, Swapchain, SwapchainCreateInfo, acquire_next_image, SwapchainPresentInfo},
    instance::{Instance, InstanceCreateFlags, InstanceCreateInfo, InstanceExtensions},
    device::{Device, DeviceCreateInfo, DeviceExtensions, Features, QueueFlags, QueueCreateInfo, physical::{PhysicalDevice, PhysicalDeviceType}},
    memory::allocator::{StandardMemoryAllocator, AllocationCreateInfo, MemoryTypeFilter, MemoryAllocator},
    command_buffer::{allocator::{StandardCommandBufferAllocator}, AutoCommandBufferBuilder, CommandBufferUsage, RenderingAttachmentInfo, CopyBufferInfo},
    image::{ImageUsage, Image, view::{ImageView, ImageViewCreateInfo}, ImageCreateInfo, sampler::{Sampler, SamplerCreateInfo, SamplerMipmapMode, Filter}},
    pipeline::{
        graphics::{
            viewport::{Viewport, ViewportState,}, GraphicsPipelineCreateInfo, vertex_input::VertexInputState, input_assembly::{InputAssemblyState, PrimitiveTopology}, rasterization::RasterizationState, multisample::MultisampleState, color_blend::ColorBlendState, subpass::PipelineRenderingCreateInfo
        }, Pipeline, PipelineShaderStageCreateInfo, layout::PipelineDescriptorSetLayoutCreateInfo, ComputePipeline, compute::ComputePipelineCreateInfo, PipelineLayout, PipelineBindPoint, GraphicsPipeline, PartialStateMode
    }, 
    render_pass::{AttachmentLoadOp, AttachmentStoreOp}, shader::ShaderModule, descriptor_set::{allocator::StandardDescriptorSetAllocator, PersistentDescriptorSet, WriteDescriptorSet}, buffer::{Buffer, BufferCreateInfo, BufferUsage, BufferContents, Subbuffer}, format::Format, DeviceSize, padded::Padded, 
};

use glam::{Mat4, Vec3, Quat, uvec2, UVec2, Vec4Swizzles};

mod app;
use app::Renderer;

mod geom;

mod cam;
use cam::Camera;

mod model;
use model::Model;

mod font;
use font::FixedFont;

use std::f32::consts::PI;

mod cs {
    vulkano_shaders::shader! {
        ty: "compute",
        path: "src/gen.comp",
    }
}

mod vs {
    vulkano_shaders::shader! {
        ty: "vertex",
        path: "src/fullscreen.vert",
    }
}

mod fs {
    vulkano_shaders::shader! {
        ty: "fragment",
        path: "src/fullscreen.frag",
    }
}

fn main() {
    //println!("Vec3 align: {}, size: {}", std::mem::align_of::<Vec3>(), std::mem::size_of::<Vec3>());

    let mut input = WinitInputHelper::new();
    let event_loop = EventLoop::new();

    const RENDER_SIZE: UVec2 = uvec2(1024, 768);

    let window = Arc::new(WindowBuilder::new()
        .with_title("Hello window")
        .with_inner_size(winit::dpi::LogicalSize::new(RENDER_SIZE.x as f32, RENDER_SIZE.y as f32))
        .build(&event_loop)
        .unwrap());

    let surface_extensions = InstanceExtensions {
        ..Surface::required_extensions(&event_loop)
    };
    let device_extensions = DeviceExtensions {
        khr_swapchain: true,
        ..DeviceExtensions::empty()
    };

    let mut surface : Option<Arc<Surface>> = None;

    let device_index: usize = if let Some(num) = env::args().nth(1) { num.parse().unwrap() } else { 0 };
    let renderer = Renderer::new(&surface_extensions, &device_extensions, |p, i| {
        if surface.is_none() {
            surface = Some(Surface::from_window(p.instance().clone(), window.clone()).unwrap());
        }
        p.surface_support(i as u32, &surface.as_ref().unwrap()).unwrap_or(false)
    }, device_index);


    let surface = surface.unwrap();

    // for (fmt, clr_space) in device.physical_device().surface_formats(&surface, Default::default()).unwrap() {
    //     println!("Surface format {fmt:?}, colorspace {clr_space:?}");
    // }

    let font = FixedFont::from_file("data/font_10x20.png");

    let mut camera = Camera{
        transform: Mat4::from_translation(Vec3::new(0.0, 0.0, -10.0)) * Mat4::from_rotation_z(PI), 
        projection: Mat4::IDENTITY,
    };

    let mut model = Model::load_obj("data/cessna.obj", usize::MAX);
    model.invert_triangle_order();

    let uniform_buffer = Buffer::new_slice::<cs::UniformData>(
        &*renderer.allocator,
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
        &*renderer.allocator,
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
        &*renderer.allocator,
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
        &*renderer.allocator,
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


    let (mut swapchain, mut swapchain_images) = {
        let surface_caps = renderer.device.physical_device().surface_capabilities(&surface, Default::default()).unwrap();
        let image_format = renderer.device.physical_device().surface_formats(&surface, Default::default()).unwrap()[0].0;
        Swapchain::new(
            renderer.device.clone(),
            surface,
            SwapchainCreateInfo { 
                min_image_count: surface_caps.min_image_count.max(2), 
                image_format, 
                image_extent: window.inner_size().into(), 
                image_usage: ImageUsage::COLOR_ATTACHMENT | ImageUsage::STORAGE, 
                composite_alpha: surface_caps.supported_composite_alpha.into_iter().next().unwrap(), 
                ..Default::default() }
        ).unwrap()
    };

    let descriptor_set_allocator = StandardDescriptorSetAllocator::new(renderer.device.clone());

    let compute_pipeline = load_compute_pipeline(renderer.device.clone(), cs::load(renderer.device.clone()).unwrap());
    let graphics_pipeline = load_graphics_pipeline(renderer.device.clone(), vs::load(renderer.device.clone()).unwrap(), fs::load(renderer.device.clone()).unwrap(), &[swapchain_images[0].format()]);

    let mut viewport = Viewport {
        offset: [0.0, 0.0],
        extent: [0.0, 0.0],
        depth_range: 0.0..=1.0,
    };

    let mut swapchain_image_views = setup_for_window_size(&swapchain_images, &mut viewport, &mut camera);
    
    let sampler_linear = Sampler::new(
        renderer.device.clone(),
        SamplerCreateInfo {
            mag_filter: Filter::Linear,
            min_filter: Filter::Linear,
            mipmap_mode: SamplerMipmapMode::Linear,
            ..SamplerCreateInfo::default()
        }
    ).unwrap();
    let storage_image = create_storage_image(&*renderer.allocator, [RENDER_SIZE.x, RENDER_SIZE.y, 1]);
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
        &descriptor_set_allocator,
        compute_pipeline.layout().set_layouts()[0].clone(),
        [
            WriteDescriptorSet::buffer(0, uniform_buffer.clone()),
            WriteDescriptorSet::image_view(1, storage_image_view),
            WriteDescriptorSet::buffer(2, vertices_buffer.clone()),
            WriteDescriptorSet::buffer(3, triangles_buffer.clone()),
            WriteDescriptorSet::buffer(4, bvh_buffer.clone()),],
        []
    ).unwrap();
    let graphics_descriptor_set = PersistentDescriptorSet::new(
        &descriptor_set_allocator,
        graphics_pipeline.layout().set_layouts()[0].clone(),
        [
            WriteDescriptorSet::sampler(0, sampler_linear),
            WriteDescriptorSet::image_view(1, storage_image_view_sampled),],
        []
    ).unwrap();

    let cmd_buffer_allocator = StandardCommandBufferAllocator::new(renderer.device.clone(), Default::default());

    let mut upload_builder = AutoCommandBufferBuilder::primary(&cmd_buffer_allocator, renderer.queue.queue_family_index(), CommandBufferUsage::OneTimeSubmit).unwrap();
    upload_builder
        .copy_buffer(CopyBufferInfo::buffers(
            get_staging_slice(&*renderer.allocator, model.vertices.as_slice()), 
            vertices_buffer.clone())).unwrap()
        .copy_buffer(CopyBufferInfo::buffers(
            get_staging_slice(&*renderer.allocator, model.triangles.as_slice()), 
            triangles_buffer.clone())).unwrap()
        .copy_buffer(CopyBufferInfo::buffers(
            get_staging_slice(&*renderer.allocator, model.bvh.iter().map(|b| 
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

    let mut previous_frame_end: Option<Box<dyn GpuFuture>> = Some(sync::now(renderer.device.clone()).boxed()
            .then_execute(renderer.queue.clone(), upload_buffer).unwrap()
            .then_signal_fence_and_flush().unwrap().boxed());

    let mut recreate_swapchain = false;

    let mut poll_time = SystemTime::now();

    event_loop.run(move |event, _, control_flow| {
        if !input.update(&event) {
            return;
        }

        if input.close_requested() || input.destroyed() {
            control_flow.set_exit();
            return;
        }

        process_camera_input(&mut input, &mut camera, &mut poll_time);

        // render
        let window_extent: [u32; 2] = window.inner_size().into();
        if window_extent.contains(&0) {
            // don't draw on empty window
            return;
        }
        previous_frame_end.as_mut().unwrap().cleanup_finished();
        if recreate_swapchain {
            let (new_swapchain, new_images) = swapchain.recreate(SwapchainCreateInfo { image_extent: window_extent, ..swapchain.create_info() })
                .expect("Failed to recreate swapchain");
            swapchain = new_swapchain;
            swapchain_images = new_images;
            swapchain_image_views = setup_for_window_size(&swapchain_images, &mut viewport, &mut camera);
            viewport.extent = [window_extent[0] as f32, window_extent[1] as f32];
            recreate_swapchain = false;
        }
        let (image_index, suboptimal, acquire_future) = 
            match acquire_next_image(swapchain.clone(), None).map_err(Validated::unwrap) {
                Ok(r) => r,
                Err(VulkanError::OutOfDate) => {
                    recreate_swapchain = true;
                    return;
                },
                Err(e) => panic!("Failed to acquire swapchain image {e}"),
            };
        if suboptimal {
            recreate_swapchain = true;
        }

        let mut cmd_builder = AutoCommandBufferBuilder::primary(&cmd_buffer_allocator, renderer.queue.queue_family_index(), CommandBufferUsage::OneTimeSubmit).unwrap();
        cmd_builder
            .copy_buffer(CopyBufferInfo::buffers(get_staging_buffer(&*renderer.allocator, get_uniform_contents(&camera, &model)), uniform_buffer.clone())).unwrap()
            .bind_pipeline_compute(compute_pipeline.clone()).unwrap()
            .bind_descriptor_sets(PipelineBindPoint::Compute, compute_pipeline.layout().clone(), 0, compute_descriptor_set.clone()).unwrap()
            .dispatch([div_ceil(storage_image.extent()[0], 8), div_ceil(storage_image.extent()[1], 8), 1]).unwrap()

            .begin_rendering(vulkano::command_buffer::RenderingInfo { color_attachments: vec![Some(RenderingAttachmentInfo{
                load_op: AttachmentLoadOp::Clear,
                store_op: AttachmentStoreOp::Store,
                clear_value: Some([0.0, 0.0, 1.0, 1.0].into()),
                ..RenderingAttachmentInfo::image_view(swapchain_image_views[image_index as usize].clone())
                })], ..Default::default() }).unwrap()

            .bind_pipeline_graphics(graphics_pipeline.clone()).unwrap()
            .set_viewport(
                0, 
                [viewport.clone()].into_iter().collect()
            ).unwrap()
            .bind_descriptor_sets(PipelineBindPoint::Graphics, graphics_pipeline.layout().clone(), 0, graphics_descriptor_set.clone()).unwrap()
            .draw(4, 1, 0, 0).unwrap()

            .end_rendering().unwrap();

        let cmd_buffer = cmd_builder.build().unwrap();

        let future = previous_frame_end
                .take().unwrap()
                .join(acquire_future)
                .then_execute(renderer.queue.clone(), cmd_buffer).unwrap()
                .then_swapchain_present(renderer.queue.clone(), SwapchainPresentInfo::swapchain_image_index(swapchain.clone(), image_index))
                .then_signal_fence_and_flush();
        match future.map_err(Validated::unwrap) {
            Ok(future) => {
                previous_frame_end = Some(future.boxed());
            },
            Err(VulkanError::OutOfDate) => {
                recreate_swapchain = true;
                previous_frame_end = Some(sync::now(renderer.device.clone()).boxed());
            },
            Err(e) => {
                println!("Failed to flush future {e}");
                previous_frame_end = Some(sync::now(renderer.device.clone()).boxed());
            },
        }
    })
}

fn div_ceil(num: u32, denom: u32) -> u32 {
    (num + denom - 1) / denom
}

fn setup_for_window_size(images: &[Arc<Image>], viewport: &mut Viewport, camera: &mut Camera) -> Vec<Arc<ImageView>> {
    let extent = images[0].extent();
    viewport.extent = [extent[0] as f32, extent[1] as f32];

    camera.projection = Mat4::perspective_lh(PI / 3.0, viewport.extent[0] / viewport.extent[1], 0.1, 100.0);

    images
        .iter()
        .map(
            |image| ImageView::new_default(image.clone()).unwrap())
        .collect::<Vec<_>>()
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
        AllocationCreateInfo {
            .. Default::default()
        }
    ).unwrap()
}

fn load_compute_pipeline(device: Arc<Device>, shader_module: Arc<ShaderModule>) -> Arc<ComputePipeline> {
    let cs = shader_module.entry_point("main").unwrap();
    let stage = PipelineShaderStageCreateInfo::new(cs);
    let layout = PipelineLayout::new(
        device.clone(),
        PipelineDescriptorSetLayoutCreateInfo::from_stages([&stage]).into_pipeline_layout_create_info(device.clone()).unwrap())
        .unwrap();
    ComputePipeline::new(device, None, ComputePipelineCreateInfo::stage_layout(stage, layout)).unwrap()
}

fn load_graphics_pipeline(device: Arc<Device>, vs_module: Arc<ShaderModule>, fs_module: Arc<ShaderModule>, attachment_formats: &[Format]) -> Arc<GraphicsPipeline> {
    let vs = vs_module.entry_point("main").unwrap();
    let fs = fs_module.entry_point("main").unwrap();
    let stages = [
        PipelineShaderStageCreateInfo::new(vs),
        PipelineShaderStageCreateInfo::new(fs),
    ];
    let layout = PipelineLayout::new(
        device.clone(),
        PipelineDescriptorSetLayoutCreateInfo::from_stages(&stages)
            .into_pipeline_layout_create_info(device.clone())
            .unwrap()
    ).unwrap();

    let subpass = PipelineRenderingCreateInfo{
        color_attachment_formats: attachment_formats.iter().map(|f| Some(*f)).collect(),
        ..Default::default()
    };

    GraphicsPipeline::new(
        device,
        None,
        GraphicsPipelineCreateInfo {
            stages: stages.into_iter().collect(),
            vertex_input_state: Some(VertexInputState::default()),
            input_assembly_state: Some(InputAssemblyState {
                topology: PartialStateMode::Fixed(PrimitiveTopology::TriangleStrip),
                ..InputAssemblyState::default()
            }),
            viewport_state: Some(ViewportState::viewport_dynamic_scissor_irrelevant()),
            rasterization_state: Some(RasterizationState::default()),
            multisample_state: Some(MultisampleState::default()),
            color_blend_state: Some(ColorBlendState::new(attachment_formats.len() as u32)),
            subpass: Some(subpass.into()),
            ..GraphicsPipelineCreateInfo::layout(layout)
        },
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
