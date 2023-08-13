use winit::{
    event::{Event, WindowEvent},
    event_loop::EventLoop,
    window::WindowBuilder,
};

use std::sync::Arc;

use vulkano::{
    VulkanLibrary, Version, VulkanError, Validated,
    sync::{self, GpuFuture},
    swapchain::{Surface, Swapchain, SwapchainCreateInfo, acquire_next_image, SwapchainPresentInfo},
    instance::{Instance, InstanceCreateFlags, InstanceCreateInfo},
    device::{Device, DeviceCreateInfo, DeviceExtensions, Features, QueueFlags, QueueCreateInfo},
    memory::allocator::{StandardMemoryAllocator},
    command_buffer::{allocator::{StandardCommandBufferAllocator}, AutoCommandBufferBuilder, CommandBufferUsage, RenderingAttachmentInfo},
    image::{ImageUsage, Image, view::ImageView},
    pipeline::{
        graphics::{
            viewport::{Viewport,}
        }
    }, 
    render_pass::{AttachmentLoadOp, AttachmentStoreOp}, 
};

fn main() {
    let event_loop = EventLoop::new();

    let library = VulkanLibrary::new().unwrap();

    let surface_extensions = Surface::required_extensions(&event_loop);

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
                image_usage: ImageUsage::COLOR_ATTACHMENT, 
                composite_alpha: surface_caps.supported_composite_alpha.into_iter().next().unwrap(), 
                ..Default::default() }
        ).unwrap()
    };

    let allocator = StandardMemoryAllocator::new_default(device.clone());

    let mut viewport = Viewport {
        offset: [0.0, 0.0],
        extent: [0.0, 0.0],
        depth_range: 0.0..=1.0,
    };

    let mut swapchain_image_views = setup_for_window_size(&swapchain_images, &mut viewport);

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
                    .begin_rendering(vulkano::command_buffer::RenderingInfo { color_attachments: vec![Some(RenderingAttachmentInfo{
                        load_op: AttachmentLoadOp::Clear,
                        store_op: AttachmentStoreOp::Store,
                        clear_value: Some([0.0, 0.0, 1.0, 1.0].into()),
                        ..RenderingAttachmentInfo::image_view(swapchain_image_views[image_index as usize].clone())
                        })], ..Default::default() }).unwrap()

                    // real rendering goes here

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

fn setup_for_window_size(images: &[Arc<Image>], viewport: &mut Viewport) -> Vec<Arc<ImageView>> {
    let extent = images[0].extent();
    viewport.extent = [extent[0] as f32, extent[1] as f32];

    images
        .iter()
        .map(
            |image| ImageView::new_default(image.clone()).unwrap())
        .collect::<Vec<_>>()
}