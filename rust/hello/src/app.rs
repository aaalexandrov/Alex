use vulkano::{
    device::{Device, Queue, DeviceExtensions, DeviceCreateInfo, QueueCreateInfo, Features, QueueFlags, physical::{PhysicalDeviceType, PhysicalDevice}}, 
    swapchain::{Surface, Swapchain, SwapchainCreateInfo, acquire_next_image, SwapchainPresentInfo, PresentMode}, 
    image::{view::ImageView, ImageUsage, Image}, 
    instance::{InstanceExtensions, Instance, InstanceCreateInfo, InstanceCreateFlags}, 
    memory::allocator::{MemoryAllocator, StandardMemoryAllocator, MemoryTypeFilter, AllocationCreateInfo}, 
    VulkanLibrary, Version, command_buffer::PrimaryAutoCommandBuffer, sync::{GpuFuture, self}, Validated, VulkanError, shader::ShaderModule, 
    pipeline::{ComputePipeline, GraphicsPipeline, PipelineShaderStageCreateInfo, layout::PipelineDescriptorSetLayoutCreateInfo, compute::ComputePipelineCreateInfo, PipelineLayout, 
        graphics::{subpass::PipelineRenderingCreateInfo, GraphicsPipelineCreateInfo, vertex_input::{VertexInputState, Vertex, VertexDefinition}, input_assembly::{InputAssemblyState, PrimitiveTopology}, 
        viewport::ViewportState, rasterization::RasterizationState, multisample::MultisampleState, color_blend::ColorBlendState}, PartialStateMode}, format::Format, 
        descriptor_set::allocator::{StandardDescriptorSetAllocator}, buffer::{Buffer, BufferContents, Subbuffer, BufferCreateInfo, BufferUsage}, DeviceSize};
use std::sync::Arc;

use winit::{
    event_loop::EventLoop,
    window::{WindowBuilder, Window},
};

use winit_input_helper::WinitInputHelper;


pub struct Renderer {
    pub device: Arc<Device>,
    pub queue: Arc<Queue>,
    pub allocator: Box<dyn MemoryAllocator>,
    pub descriptor_set_allocator: StandardDescriptorSetAllocator,
}

impl Renderer {
    pub fn new<QueuePred>(instance_extensions: &InstanceExtensions, device_extensions: &DeviceExtensions, queue_pred: QueuePred, device_index: usize) -> Renderer 
        where QueuePred: FnMut(Arc<PhysicalDevice>, usize) -> bool {
        let library = VulkanLibrary::new().unwrap();
    
        let instance = Instance::new(
            library,
            InstanceCreateInfo { flags: InstanceCreateFlags::ENUMERATE_PORTABILITY, enabled_extensions: *instance_extensions, ..Default::default() }
        ).unwrap();

        let (physical_device, device_extensions, queue_family) = Renderer::select_physical_device_and_queue_family(&instance, &device_extensions, queue_pred, device_index)
            .expect("Np physical device with queue support found");
    
        println!("Using device {} (type: {:?})", physical_device.properties().device_name, physical_device.properties().device_type);
    
        let (device, mut queues) = Device::new(
            physical_device,
            DeviceCreateInfo { queue_create_infos: vec![
                QueueCreateInfo { queue_family_index: queue_family, ..Default::default() }], 
            enabled_extensions: device_extensions, 
            enabled_features: Features{dynamic_rendering: true, ..Features::empty()},
            ..Default::default() 
        }).unwrap();
    
        let queue = queues.next().unwrap();
    
        let allocator = Box::new(StandardMemoryAllocator::new_default(device.clone()));

        let descriptor_set_allocator = StandardDescriptorSetAllocator::new(device.clone());

        Renderer {
            device,
            queue,
            allocator,
            descriptor_set_allocator,
        }
    }

    pub fn load_compute_pipeline(&self, shader_module: Arc<ShaderModule>) -> Arc<ComputePipeline> {
        let cs = shader_module.entry_point("main").unwrap();
        let stage = PipelineShaderStageCreateInfo::new(cs);
        let layout = PipelineLayout::new(
            self.device.clone(),
            PipelineDescriptorSetLayoutCreateInfo::from_stages([&stage]).into_pipeline_layout_create_info(self.device.clone()).unwrap())
            .unwrap();
        ComputePipeline::new(self.device.clone(), None, ComputePipelineCreateInfo::stage_layout(stage, layout)).unwrap()
    }
    
    pub fn load_graphics_pipeline_vertex(&self, vs_module: Arc<ShaderModule>, fs_module: Arc<ShaderModule>, vertex_input_state: Option<VertexInputState>, attachment_formats: &[Format]) -> Arc<GraphicsPipeline> {
        let vs = vs_module.entry_point("main").unwrap();
        let fs = fs_module.entry_point("main").unwrap();
        let stages = [
            PipelineShaderStageCreateInfo::new(vs),
            PipelineShaderStageCreateInfo::new(fs),
        ];
        let layout = PipelineLayout::new(
            self.device.clone(),
            PipelineDescriptorSetLayoutCreateInfo::from_stages(&stages)
                .into_pipeline_layout_create_info(self.device.clone())
                .unwrap()
        ).unwrap();
    
        let subpass = PipelineRenderingCreateInfo{
            color_attachment_formats: attachment_formats.iter().map(|f| Some(*f)).collect(),
            ..Default::default()
        };
    
        GraphicsPipeline::new(
            self.device.clone(),
            None,
            GraphicsPipelineCreateInfo {
                stages: stages.into_iter().collect(),
                vertex_input_state,
                input_assembly_state: Some(InputAssemblyState {
                    topology: PartialStateMode::Fixed(PrimitiveTopology::TriangleList),
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
    
    pub fn load_graphics_pipeline<VertexStruct>(&self, vs_module: Arc<ShaderModule>, fs_module: Arc<ShaderModule>, attachment_formats: &[Format]) -> Arc<GraphicsPipeline> 
        where VertexStruct: Vertex {
        let vertex_input_state = [VertexStruct::per_vertex()]
            .definition(&vs_module.entry_point("main").unwrap().info().input_interface)
            .unwrap();
        self.load_graphics_pipeline_vertex(vs_module, fs_module, Some(vertex_input_state), attachment_formats)
    }

    fn get_memory_preference(usage: BufferUsage) -> MemoryTypeFilter {
        if usage == BufferUsage::TRANSFER_SRC { 
            MemoryTypeFilter::PREFER_HOST 
        } else { 
            MemoryTypeFilter::PREFER_DEVICE 
        }
    }

    pub fn get_buffer_data<T: BufferContents>(&self, usage: BufferUsage, data: T) -> Subbuffer<T> {
        Buffer::from_data(
            self.allocator.as_ref(),
            BufferCreateInfo{
                usage,
                ..Default::default()
            },
            AllocationCreateInfo{
                memory_type_filter: Self::get_memory_preference(usage) | MemoryTypeFilter::HOST_SEQUENTIAL_WRITE,
                ..Default::default()
            },
            data,
        ).unwrap()
    }
    
    pub fn get_buffer_write<T, WriteFunc>(&self, usage: BufferUsage, elements: DeviceSize, write_fn: WriteFunc) -> Subbuffer<[T]>
        where T: BufferContents + Sized, WriteFunc: Fn(&mut [T]) {
            let buffer = Buffer::new_slice(
                self.allocator.as_ref(),
                BufferCreateInfo{
                    usage,
                    ..Default::default()
                },
                AllocationCreateInfo{
                    memory_type_filter: Self::get_memory_preference(usage) | MemoryTypeFilter::HOST_SEQUENTIAL_WRITE,
                    ..Default::default()
                },
                elements,
            ).unwrap();
        
            {
                let mut writer = buffer.write().unwrap();
                write_fn(&mut *writer);
            }
        
            buffer
    }

    pub fn get_buffer_slice<T: BufferContents + Clone>(&self, usage: BufferUsage, data: &[T]) -> Subbuffer<[T]> {
        self.get_buffer_write(usage, data.len() as DeviceSize, |slice| {
            slice.clone_from_slice(data);
        })
    }

    fn select_physical_device_and_queue_family<QueuePred>(instance: &Arc<Instance>, device_extensions: &DeviceExtensions, mut queue_pred: QueuePred, device_index: usize) -> Option<(Arc<PhysicalDevice>, DeviceExtensions, u32)>
        where QueuePred: FnMut(Arc<PhysicalDevice>, usize) -> bool {
        let mut devices: Vec<_> = instance.enumerate_physical_devices().unwrap()
            .filter_map(|p| {
                let extensions = DeviceExtensions {
                    khr_dynamic_rendering: p.api_version() < Version::V1_3,
                    ..*device_extensions
                };
                if p.supported_extensions().contains(&extensions) {
                    Some((p, extensions))
                } else {
                    None
                }
            })
            .filter_map(|(p,extensions)| {
                let queue_index = p.queue_family_properties().iter().enumerate()
                    .position(|(i, q)| {
                        q.queue_flags.contains(QueueFlags::GRAPHICS | QueueFlags::COMPUTE) && queue_pred(p.clone(), i)
                    });
                if let Some(index) = queue_index {
                    Some((p, extensions, index as u32))
                } else {
                    None
                }
            }).collect();
            devices.sort_by_key(|(p,_,_)| {
                match p.properties().device_type {
                    PhysicalDeviceType::DiscreteGpu => 0,
                    PhysicalDeviceType::IntegratedGpu => 1,
                    PhysicalDeviceType::VirtualGpu => 2,
                    PhysicalDeviceType::Cpu => 3,
                    PhysicalDeviceType::Other => 4,
                    _ => 5,
                }
            });
            devices.into_iter().nth(device_index)
    }    
}

pub struct App {
    pub renderer: Renderer,
    pub event_loop: Option<EventLoop<()>>,
    pub input: WinitInputHelper,
    pub window: Arc<Window>,
    pub surface: Arc<Surface>,
    pub swapchain: Arc<Swapchain>,
    pub swapchain_image_views: Vec<Arc<ImageView>>,
    pub present_mode: Option<PresentMode>,
}

impl App {
    pub fn new(app_name: &str, win_size: [u32; 2], device_index: usize) -> App {
        let input = WinitInputHelper::new();
        let event_loop = Some(EventLoop::new());
        
        let window = Arc::new(WindowBuilder::new()
            .with_title(app_name)
            .with_inner_size(winit::dpi::PhysicalSize::new(win_size[0], win_size[1]))
            .build(event_loop.as_ref().unwrap())
            .unwrap());
    
        let surface_extensions = InstanceExtensions {
            ..Surface::required_extensions(event_loop.as_ref().unwrap())
        };
        let device_extensions = DeviceExtensions {
            khr_swapchain: true,
            ..DeviceExtensions::empty()
        };
    
        let mut surface : Option<Arc<Surface>> = None;
    
        let renderer = Renderer::new(&surface_extensions, &device_extensions, |p, i| {
            if surface.is_none() {
                surface = Some(Surface::from_window(p.instance().clone(), window.clone()).unwrap());
            }
            p.surface_support(i as u32, &surface.as_ref().unwrap()).unwrap_or(false)
        }, device_index);
    
        let surface = surface.unwrap();

        let (swapchain, swapchain_images) = {
            let surface_caps = renderer.device.physical_device().surface_capabilities(&surface, Default::default()).unwrap();
            let image_format = renderer.device.physical_device().surface_formats(&surface, Default::default()).unwrap()[0].0;
            Swapchain::new(
                renderer.device.clone(),
                surface.clone(),
                SwapchainCreateInfo { 
                    min_image_count: surface_caps.min_image_count.max(2), 
                    image_format, 
                    image_extent: window.inner_size().into(), 
                    image_usage: ImageUsage::COLOR_ATTACHMENT | ImageUsage::STORAGE, 
                    composite_alpha: surface_caps.supported_composite_alpha.into_iter().next().unwrap(), 
                    ..Default::default() }
            ).unwrap()
        };
    
        let swapchain_image_views = App::create_image_views(&swapchain_images);

        App {
            renderer,
            event_loop,
            input,
            window,
            surface,
            swapchain,
            swapchain_image_views,
            present_mode: None,
        }
    }

    pub fn run<LoopFn>(mut self, init_cmd_buffer: Arc<PrimaryAutoCommandBuffer>, mut loop_fn: LoopFn) -> !
        where LoopFn: 'static + FnMut(&mut Self, Arc<ImageView>) -> Arc<PrimaryAutoCommandBuffer> {

        let mut recreate_swapchain = false;

        let mut previous_frame_end: Option<Box<dyn GpuFuture>> = Some(sync::now(self.renderer.device.clone()).boxed()
            .then_execute(self.renderer.queue.clone(), init_cmd_buffer).unwrap()
            .then_signal_fence_and_flush().unwrap().boxed());


        let event_loop = self.event_loop.take().unwrap();
        event_loop.run(move |event, _, control_flow| {
            if !self.input.update(&event) {
                return;
            }
    
            if self.input.close_requested() || self.input.destroyed() {
                control_flow.set_exit();
                return;
            }
    
            // render
            let window_extent: [u32; 2] = self.window.inner_size().into();
            if window_extent.contains(&0) {
                // don't draw on empty window
                return;
            }
            previous_frame_end.as_mut().unwrap().cleanup_finished();
            if recreate_swapchain {
                let (new_swapchain, new_images) = self.swapchain.recreate(SwapchainCreateInfo { image_extent: window_extent, ..self.swapchain.create_info() })
                    .expect("Failed to recreate swapchain");
                self.swapchain = new_swapchain;
                self.swapchain_image_views = App::create_image_views(&new_images);
                recreate_swapchain = false;
            }
            let (image_index, suboptimal, acquire_future) = 
                match acquire_next_image(self.swapchain.clone(), None).map_err(Validated::unwrap) {
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
    
            let img_view = self.swapchain_image_views[image_index as usize].clone();
            let cmd_buffer = loop_fn(&mut self, img_view);
    
            let future = previous_frame_end
                    .take().unwrap()
                    .join(acquire_future)
                    .then_execute(self.renderer.queue.clone(), cmd_buffer).unwrap()
                    .then_swapchain_present(
                        self.renderer.queue.clone(), 
                        SwapchainPresentInfo {
                            present_mode: self.present_mode,
                            ..SwapchainPresentInfo::swapchain_image_index(self.swapchain.clone(), image_index)
                        }
                    )
                    .then_signal_fence_and_flush();
            match future.map_err(Validated::unwrap) {
                Ok(future) => {
                    previous_frame_end = Some(future.boxed());
                },
                Err(VulkanError::OutOfDate) => {
                    recreate_swapchain = true;
                    previous_frame_end = Some(sync::now(self.renderer.device.clone()).boxed());
                },
                Err(e) => {
                    println!("Failed to flush future {e}");
                    previous_frame_end = Some(sync::now(self.renderer.device.clone()).boxed());
                },
            }
        })
    }

    fn create_image_views(images: &[Arc<Image>]) -> Vec<Arc<ImageView>> {
        images
            .iter()
            .map(
                |image| ImageView::new_default(image.clone()).unwrap())
            .collect::<Vec<_>>()
    }
}