const std = @import("std");
const warn = std.debug.warn;

pub fn main() void {
    warn("Hi there!\n", .{});
    warn("defer_try(5): {}\n", .{defer_try(5)});

    var ii: i32 = 44;
    var opt: ?*i32 = null;

    warn("opt val: {}\n", .{opt});
    opt = &ii;
    var ptr: *i32 = opt.?;
    warn("opt val: {}, {}, {}\n", .{ opt, opt.?, ptr });

    warn("max(3, 5): {}\n", .{my_max(3, 5)});
    var f: f32 = my_max(3.0, 5.0);
    warn("max(3.0, 5.0): {}\n", .{f});
}

fn defer_try(_i: i32) i32 {
    var i = _i;
    defer {
        i = i + 2;
    }
    return i + 1;
}

fn my_max(a: var, b: @TypeOf(a)) @TypeOf(a) {
    return if (a > b) a else b;
}
