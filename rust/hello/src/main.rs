use winit::{
    event::{Event, WindowEvent},
    event_loop::EventLoop,
    window::WindowBuilder,
};

use std::sync::Arc;

use bytemuck::Pod;

use vulkano::{
    VulkanLibrary, Version, VulkanError, Validated,
    sync::{self, GpuFuture},
    swapchain::{Surface, Swapchain, SwapchainCreateInfo, acquire_next_image, SwapchainPresentInfo},
    instance::{Instance, InstanceCreateFlags, InstanceCreateInfo},
    device::{Device, DeviceCreateInfo, DeviceExtensions, Features, QueueFlags, QueueCreateInfo},
    memory::allocator::{StandardMemoryAllocator, AllocationCreateInfo, MemoryTypeFilter, MemoryAllocator},
    command_buffer::{allocator::{StandardCommandBufferAllocator}, AutoCommandBufferBuilder, CommandBufferUsage, RenderingAttachmentInfo},
    image::{ImageUsage, Image, view::{ImageView, ImageViewCreateInfo}, ImageCreateInfo, sampler::{Sampler, SamplerCreateInfo, SamplerMipmapMode, Filter}},
    pipeline::{
        graphics::{
            viewport::{Viewport, ViewportState,}, GraphicsPipelineCreateInfo, vertex_input::VertexInputState, input_assembly::{InputAssemblyState, PrimitiveTopology}, rasterization::RasterizationState, multisample::MultisampleState, color_blend::ColorBlendState, subpass::PipelineRenderingCreateInfo
        }, Pipeline, PipelineShaderStageCreateInfo, layout::PipelineDescriptorSetLayoutCreateInfo, ComputePipeline, compute::ComputePipelineCreateInfo, PipelineLayout, PipelineBindPoint, GraphicsPipeline, PartialStateMode
    }, 
    render_pass::{AttachmentLoadOp, AttachmentStoreOp}, shader::ShaderModule, descriptor_set::{allocator::StandardDescriptorSetAllocator, PersistentDescriptorSet, WriteDescriptorSet}, buffer::{Buffer, BufferCreateInfo, BufferUsage, BufferContents}, format::Format, 
};

mod cs {
    vulkano_shaders::shader! {
        ty: "compute",
        src: r"
            #version 450
            
            layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
            
            layout(set = 0, binding = 0) uniform Data {
                vec4 pix_value;
            };
            
            layout(set = 0, binding = 1, rgba8) uniform image2D Output;
            
            void main() {
                ivec2 pixCoord = ivec2(gl_GlobalInvocationID.xy);

                ivec2 sq = (pixCoord / 16);
                float w = (sq.x + sq.y) % 2;

                imageStore(Output, pixCoord, pix_value * w);
            }
        ",
    }
}

mod vs {
    vulkano_shaders::shader! {
        ty: "vertex",
        src: r"
            #version 450

            layout(location = 0) out vec2 tc;

            void main() {
                tc = vec2(
                    gl_VertexIndex % 2,
                    gl_VertexIndex / 2
                );
                gl_Position = vec4(
                    mix(-1, 1, tc.x),
                    mix(-1, 1, tc.y),
                    0,
                    1
                );
            }
        ",
    }
}

mod fs {
    vulkano_shaders::shader! {
        ty: "fragment",
        src: r"
            #version 450

            layout(location = 0) in vec2 tc;
            layout(location = 0) out vec4 color;

            layout (set = 0, binding = 0) uniform sampler samp;
            layout (set = 0, binding = 1) uniform texture2D tex;
            
            void main() {
                color = texture(sampler2D(tex, samp), tc);
            }
        ",
    }
}

fn main() {
    let event_loop = EventLoop::new();

    let library = VulkanLibrary::new().unwrap();

    let surface_extensions = vulkano::instance::InstanceExtensions {
        //ext_swapchain_colorspace = true,
        ..Surface::required_extensions(&event_loop)
    };

    let instance = Instance::new(
        library,
        InstanceCreateInfo { flags: InstanceCreateFlags::ENUMERATE_PORTABILITY, enabled_extensions: surface_extensions, ..Default::default() }
    ).unwrap();

    let window = Arc::new(WindowBuilder::new()
        .with_title("Hello window")
        .with_inner_size(winit::dpi::LogicalSize::new(800.0, 600.0))
        .build(&event_loop)
        .unwrap());

    let surface = Surface::from_window(instance.clone(), window.clone()).unwrap();

    let mut device_extensions = DeviceExtensions {
        khr_swapchain: true,
        ..DeviceExtensions::empty()
    };

    // todo: filter physical devices properly
    let physical_device = instance.enumerate_physical_devices().unwrap().next()
        .expect("No physycal devices found");

    let queue_family = physical_device.queue_family_properties().iter().enumerate().position(|(_i, q)| q.queue_flags.contains(QueueFlags::GRAPHICS | QueueFlags::COMPUTE))
        .expect("No suitable queue family found") as u32;

    println!("Using device {} (type: {:?})", physical_device.properties().device_name, physical_device.properties().device_type);

    if physical_device.api_version() < Version::V1_3 {
        device_extensions.khr_dynamic_rendering = true;
    }

    let (device, mut queues) = Device::new(
        physical_device,
        DeviceCreateInfo { queue_create_infos: vec![
            QueueCreateInfo { queue_family_index: queue_family, ..Default::default() }], 
        enabled_extensions: device_extensions, 
        enabled_features: Features{dynamic_rendering: true, ..Features::empty()},
        ..Default::default() 
    }).unwrap();

    let queue = queues.next().unwrap();

    let memory_allocator = StandardMemoryAllocator::new_default(device.clone());

    // for (fmt, clr_space) in device.physical_device().surface_formats(&surface, Default::default()).unwrap() {
    //     println!("Surface format {fmt:?}, colorspace {clr_space:?}");
    // }

    #[repr(C)]
    #[derive(BufferContents)]
    struct DataBuffer {
        pix_value: [f32; 4],
    }

    let data_buffer = Buffer::from_data(
        &memory_allocator,
        BufferCreateInfo{
            usage: BufferUsage::UNIFORM_BUFFER,
            ..Default::default()
        },
        AllocationCreateInfo{
            memory_type_filter: MemoryTypeFilter::PREFER_DEVICE | MemoryTypeFilter::HOST_SEQUENTIAL_WRITE,
            ..Default::default()
        },
        DataBuffer{pix_value:[1.0, 1.0, 0.0, 1.0]}
    ).unwrap();

    let (mut swapchain, mut swapchain_images) = {
        let surface_caps = device.physical_device().surface_capabilities(&surface, Default::default()).unwrap();
        let image_format = device.physical_device().surface_formats(&surface, Default::default()).unwrap()[0].0;
        Swapchain::new(
            device.clone(),
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

    let descriptor_set_allocator = StandardDescriptorSetAllocator::new(device.clone());

    let compute_pipeline = load_compute_pipeline(device.clone(), cs::load(device.clone()).unwrap());
    let graphics_pipeline = load_graphics_pipeline(device.clone(), vs::load(device.clone()).unwrap(), fs::load(device.clone()).unwrap(), &[swapchain_images[0].format()]);

    let mut viewport = Viewport {
        offset: [0.0, 0.0],
        extent: [0.0, 0.0],
        depth_range: 0.0..=1.0,
    };

    let mut swapchain_image_views = setup_for_window_size(&swapchain_images, &mut viewport);
    
    let sampler_linear = Sampler::new(
        device.clone(),
        SamplerCreateInfo {
            mag_filter: Filter::Linear,
            min_filter: Filter::Linear,
            mipmap_mode: SamplerMipmapMode::Linear,
            ..SamplerCreateInfo::default()
        }
    ).unwrap();
    let mut storage_image = create_storage_image(&memory_allocator, [1024, 768, 1]);
    let mut storage_image_view = ImageView::new(
        storage_image.clone(), 
        ImageViewCreateInfo {
            usage: ImageUsage::STORAGE,
            ..ImageViewCreateInfo::from_image(&storage_image)
        }).unwrap();
    let mut storage_image_view_sampled = ImageView::new(
        storage_image.clone(), 
        ImageViewCreateInfo {
            usage: ImageUsage::SAMPLED,
            ..ImageViewCreateInfo::from_image(&storage_image)
        }).unwrap();
    let mut compute_descriptor_set = PersistentDescriptorSet::new(
        &descriptor_set_allocator,
        compute_pipeline.layout().set_layouts()[0].clone(),
        [WriteDescriptorSet::buffer(0, data_buffer),
        WriteDescriptorSet::image_view(1, storage_image_view),],
        []
    ).unwrap();
    let mut graphics_descriptor_set = PersistentDescriptorSet::new(
        &descriptor_set_allocator,
        graphics_pipeline.layout().set_layouts()[0].clone(),
        [WriteDescriptorSet::sampler(0, sampler_linear),
        WriteDescriptorSet::image_view(1, storage_image_view_sampled),],
        []
    ).unwrap();

    let cmd_buffer_allocator = StandardCommandBufferAllocator::new(device.clone(), Default::default());

    let mut recreate_swapchain = false;

    let mut previous_frame_end = Some(sync::now(device.clone()).boxed());


    event_loop.run(move |event, _, control_flow| {
        control_flow.set_wait();
        match event {
            Event::WindowEvent{
                event: WindowEvent::CloseRequested,
                window_id,
            } if window_id == window.id() => control_flow.set_exit(),
            Event::WindowEvent{
                event: WindowEvent::Resized(_),
                ..
            } => { recreate_swapchain = true; },
            Event::RedrawEventsCleared => {
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
                    swapchain_image_views = setup_for_window_size(&swapchain_images, &mut viewport);
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

                let mut cmd_builder = AutoCommandBufferBuilder::primary(&cmd_buffer_allocator, queue.queue_family_index(), CommandBufferUsage::OneTimeSubmit).unwrap();
                cmd_builder
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
                        .then_execute(queue.clone(), cmd_buffer).unwrap()
                        .then_swapchain_present(queue.clone(), SwapchainPresentInfo::swapchain_image_index(swapchain.clone(), image_index))
                        .then_signal_fence_and_flush();
                match future.map_err(Validated::unwrap) {
                    Ok(future) => {
                        previous_frame_end = Some(future.boxed());
                    },
                    Err(VulkanError::OutOfDate) => {
                        recreate_swapchain = true;
                        previous_frame_end = Some(sync::now(device.clone()).boxed());
                    },
                    Err(e) => {
                        println!("Failed to flush future {e}");
                        previous_frame_end = Some(sync::now(device.clone()).boxed());
                    },
                }
            },
            _ => ()
        }
    })
}

fn div_ceil(num: u32, denom: u32) -> u32 {
    (num + denom - 1) / denom
}

fn setup_for_window_size(images: &[Arc<Image>], viewport: &mut Viewport) -> Vec<Arc<ImageView>> {
    let extent = images[0].extent();
    viewport.extent = [extent[0] as f32, extent[1] as f32];

    images
        .iter()
        .map(
            |image| ImageView::new_default(image.clone()).unwrap())
        .collect::<Vec<_>>()
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