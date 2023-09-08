use vulkano::{
    device::{Device, Queue, DeviceExtensions, DeviceCreateInfo, QueueCreateInfo, Features, QueueFlags, physical::{PhysicalDeviceType, PhysicalDevice}}, 
    swapchain::{Surface, Swapchain}, 
    image::view::ImageView, 
    instance::{InstanceExtensions, Instance, InstanceCreateInfo, InstanceCreateFlags}, 
    memory::allocator::{MemoryAllocator, StandardMemoryAllocator}, 
    VulkanLibrary, Version};
use std::sync::Arc;

use winit::{
    event::{VirtualKeyCode},
    event_loop::EventLoop,
    window::{WindowBuilder, Window},
};

use winit_input_helper::WinitInputHelper;


pub struct Renderer {
    pub device: Arc<Device>,
    pub queue: Arc<Queue>,
    pub allocator: Box<dyn MemoryAllocator>,
}

pub struct App {
    renderer: Renderer,
    event_loop: EventLoop<()>,
    input: WinitInputHelper,
    window: Arc<Window>,
    surface: Arc<Surface>,
    swapchain: Arc<Swapchain>,
    swapchain_image_views: Vec<Arc<ImageView>>,
}

impl Renderer {
    pub fn new<QueuePred>(instance_extensions: &InstanceExtensions, device_extensions: &DeviceExtensions, mut queue_pred: QueuePred, device_index: usize) -> Renderer 
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
    
        let allocator: Box<dyn MemoryAllocator> = Box::new(StandardMemoryAllocator::new_default(device.clone()));
            
        Renderer {
            device,
            queue,
            allocator,
        }
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

impl App {

}