const std = @import("std");
const vk = @import("vk.zig");
const glfw = @import("glfw");

const BaseDispatch = vk.BaseWrapper(.{
    .enumerateInstanceLayerProperties = true,
    .createInstance = true,
    .getInstanceProcAddr = true,
});
const InstDispatch = vk.InstanceWrapper(.{
    .destroyInstance = true,
    .destroySurfaceKHR = true,
    .getPhysicalDeviceSurfaceFormatsKHR = true,
    .getPhysicalDeviceSurfacePresentModesKHR = true,
    .getPhysicalDeviceSurfaceSupportKHR = true,
    .enumeratePhysicalDevices = true,
    .getPhysicalDeviceProperties = true,
    .getPhysicalDeviceQueueFamilyProperties = true,
    .getPhysicalDeviceMemoryProperties = true,
    .enumerateDeviceExtensionProperties = true,
    .getDeviceProcAddr = true,
    .createDevice = true,
});
const DevDispatch = vk.DeviceWrapper(.{
    .destroyDevice = true,
    .getDeviceQueue = true,
});

const RequiredDeviceExtensions = [_][*:0]const u8{
    vk.extension_info.khr_swapchain.name,
};

const QueueFamilies = struct {
    universal: u32,
    present: u32,
};

pub const Vk = struct {
    alloc: std.mem.Allocator,
    allocCB: ?*const vk.AllocationCallbacks,

    baseFn: BaseDispatch,
    instFn: InstDispatch,
    devFn: DevDispatch,

    instance: vk.Instance,
    surface: vk.SurfaceKHR,
    physDevice: vk.PhysicalDevice,
    queueFamilies: QueueFamilies,
    device: vk.Device,
    universalQueue: vk.Queue,
    presentQueue: vk.Queue,
    memProps: vk.PhysicalDeviceMemoryProperties,

    pub fn init(appName: [:0]const u8, window: glfw.Window, alloc: std.mem.Allocator) !Vk {
        var self: Vk = undefined;

        self.alloc = alloc;
        self.allocCB = null;
        self.baseFn = try BaseDispatch.load(@as(vk.PfnGetInstanceProcAddr, @ptrCast(&glfw.getInstanceProcAddress)));

        const layers = try self.getInstanceLayers();
        defer self.alloc.free(layers);
        for (layers) |layerProps| {
            std.log.info("{s}, ver{}: {s}", .{ layerProps.layer_name, layerProps.spec_version, layerProps.description });
        }

        const reqExtensions = glfw.getRequiredInstanceExtensions() orelse {
            return error.ExtensionNotPresent;
        };
        self.instance = try self.baseFn.createInstance(&.{
            .p_application_info = &.{
                .p_application_name = appName,
                .application_version = vk.makeApiVersion(0, 0, 0, 0),
                .p_engine_name = appName,
                .engine_version = vk.makeApiVersion(0, 0, 0, 0),
                .api_version = vk.API_VERSION_1_2,
            },
            .enabled_layer_count = 0,
            .enabled_extension_count = @as(u32, @intCast(reqExtensions.len)),
            .pp_enabled_extension_names = reqExtensions.ptr,
        }, self.allocCB);
        self.instFn = try InstDispatch.load(self.instance, self.baseFn.dispatch.vkGetInstanceProcAddr);
        errdefer self.instFn.destroyInstance(self.instance, self.allocCB);

        if (glfw.createWindowSurface(self.instance, window, self.allocCB, &self.surface) != @intFromEnum(vk.Result.success)) {
            return error.ExtensionNotPresent;
        }
        errdefer self.instFn.destroySurfaceKHR(self.instance, self.surface, self.allocCB);

        var physDevices = try self.getPhysicalDevices();
        defer self.alloc.free(physDevices);
        var foundDevice = false;
        for (physDevices) |dev| {
            var devExts = try self.getPhysicalDeviceExtensions(dev);
            const devProps = self.instFn.getPhysicalDeviceProperties(dev);
            if (!checkExtensionsPresent(&RequiredDeviceExtensions, devExts)) {
                //std.log.info("Extensions not present on device {s}", .{devProps.device_name});
                continue;
            }
            if (!(self.checkSurfaceSupported(self.surface, dev) catch false)) {
                //std.log.info("Surface not supported on device {s}", .{devProps.device_name});
                continue;
            }
            const queues = self.getQueueIndices(dev) catch {
                //std.log.info("Could not find queue families on device {s}", .{devProps.device_name});
                continue;
            };
            foundDevice = true;
            self.physDevice = dev;
            self.queueFamilies = queues;
            std.log.info("\nDevice {s}, queues: universal {}, present {}", .{ devProps.device_name, queues.universal, queues.present });
            break;
        }
        if (!foundDevice)
            return error.InitializationFailed;

        var queues = std.ArrayList(vk.DeviceQueueCreateInfo).init(self.alloc);
        defer queues.deinit();
        var queuePriority = [1]f32{0.5};
        try queues.append(.{
            .queue_family_index = self.queueFamilies.universal,
            .queue_count = 1,
            .p_queue_priorities = &queuePriority,
        });
        if (self.queueFamilies.present != self.queueFamilies.universal) {
            try queues.append(.{
                .queue_family_index = self.queueFamilies.present,
                .queue_count = 1,
                .p_queue_priorities = &queuePriority,
            });
        }
        self.device = try self.instFn.createDevice(self.physDevice, &.{
            .queue_create_info_count = @as(u32, @intCast(queues.items.len)),
            .p_queue_create_infos = queues.items.ptr,
            .enabled_extension_count = @as(u32, @intCast(RequiredDeviceExtensions.len)),
            .pp_enabled_extension_names = @as([*]const [*:0]const u8, &RequiredDeviceExtensions),
        }, self.allocCB);
        self.devFn = try DevDispatch.load(self.device, self.instFn.dispatch.vkGetDeviceProcAddr);
        errdefer self.devFn.destroyDevice(self.device, self.allocCB);

        self.universalQueue = self.devFn.getDeviceQueue(self.device, self.queueFamilies.universal, 0);
        self.presentQueue = self.devFn.getDeviceQueue(self.device, self.queueFamilies.present, 0);

        self.memProps = self.instFn.getPhysicalDeviceMemoryProperties(self.physDevice);

        return self;
    }

    pub fn deinit(self: Vk) void {
        self.devFn.destroyDevice(self.device, self.allocCB);
        self.instFn.destroySurfaceKHR(self.instance, self.surface, self.allocCB);
        self.instFn.destroyInstance(self.instance, self.allocCB);
    }

    fn getPhysicalDevices(self: Vk) ![]vk.PhysicalDevice {
        var numDevices: u32 = undefined;
        _ = try self.instFn.enumeratePhysicalDevices(self.instance, &numDevices, null);
        var devices = try self.alloc.alloc(vk.PhysicalDevice, numDevices);
        errdefer self.alloc.free(devices);
        _ = try self.instFn.enumeratePhysicalDevices(self.instance, &numDevices, devices.ptr);
        return devices;
    }

    fn getPhysicalDeviceExtensions(self: Vk, physDevice: vk.PhysicalDevice) ![]vk.ExtensionProperties {
        var numExts: u32 = undefined;
        _ = try self.instFn.enumerateDeviceExtensionProperties(physDevice, null, &numExts, null);
        const extProps = try self.alloc.alloc(vk.ExtensionProperties, numExts);
        errdefer self.alloc.free(extProps);
        _ = try self.instFn.enumerateDeviceExtensionProperties(physDevice, null, &numExts, extProps.ptr);
        return extProps;
    }

    fn checkExtensionsPresent(reqExts: []const [*:0]const u8, exts: []vk.ExtensionProperties) bool {
        var missingExt = false;
        req: for (reqExts) |reqExt| {
            const reqLen = std.mem.indexOfSentinel(u8, 0, reqExt);
            for (exts) |ext| {
                const len = std.mem.indexOfScalar(u8, &ext.extension_name, 0).?;
                const extName = ext.extension_name[0..len];
                //std.log.info("\t{s}", .{extName});
                if (std.mem.eql(u8, extName, reqExt[0..reqLen]))
                    continue :req;
            }
            missingExt = true;
            break;
        }
        return !missingExt;
    }

    fn getInstanceLayers(self: Vk) ![]vk.LayerProperties {
        var numLayers: u32 = undefined;
        _ = try self.baseFn.enumerateInstanceLayerProperties(&numLayers, null);
        var layers = try self.alloc.alloc(vk.LayerProperties, numLayers);
        errdefer self.alloc.free(layers);
        _ = try self.baseFn.enumerateInstanceLayerProperties(&numLayers, layers.ptr);
        return layers;
    }

    fn checkSurfaceSupported(self: Vk, surface: vk.SurfaceKHR, physDevice: vk.PhysicalDevice) !bool {
        var surfaceFormats: u32 = undefined;
        _ = try self.instFn.getPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &surfaceFormats, null);
        var presentModes: u32 = undefined;
        _ = try self.instFn.getPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModes, null);
        return surfaceFormats > 0 and presentModes > 0;
    }

    fn getQueueIndices(self: Vk, physDevice: vk.PhysicalDevice) !QueueFamilies {
        var numFamilies: u32 = undefined;
        self.instFn.getPhysicalDeviceQueueFamilyProperties(physDevice, &numFamilies, null);
        var families = try self.alloc.alloc(vk.QueueFamilyProperties, numFamilies);
        defer self.alloc.free(families);
        self.instFn.getPhysicalDeviceQueueFamilyProperties(physDevice, &numFamilies, families.ptr);
        var universal: ?u32 = null;
        var present: ?u32 = null;
        for (families, 0..) |family, familyIndex| {
            if (universal == null and family.queue_flags.graphics_bit and family.queue_flags.compute_bit and family.queue_flags.transfer_bit)
                universal = @truncate(familyIndex);
            if (present == null and (self.instFn.getPhysicalDeviceSurfaceSupportKHR(physDevice, @truncate(familyIndex), self.surface) catch vk.FALSE) == vk.TRUE)
                present = @truncate(familyIndex);
        }
        return if (universal != null and present != null)
            .{ .universal = universal.?, .present = present.? }
        else
            error.FeatureNotPresent;
    }

    fn findMemoryTypeIndex(self: Vk, validTypesMask: u32, flags: vk.MemoryPropertyFlags) !u32 {
        for (0..self.memProps.memory_type_count) |i| {
            if ((validTypesMask & (@as(u32, 1) << @truncate(i))) != 0 and self.memProps.memory_types[i].property_flags.contains(flags))
                return @truncate(i);
        }
        return error.OutOfDeviceMemory;
    }
};
