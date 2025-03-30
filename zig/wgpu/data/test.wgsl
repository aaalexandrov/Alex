    struct VSOut {
        @builtin(position) pos: vec4f,
        @location(0) color: vec4f,
    };

    @vertex
    fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> VSOut {
        var vsOut: VSOut;
        if (in_vertex_index == 0) {
            vsOut.pos = vec4f(0, -0.5, 0, 1.0);
            vsOut.color = vec4f(1, 0, 0, 1.0);
        } else if (in_vertex_index == 1) {
            vsOut.pos = vec4f(-0.5, 0.5, 0, 1.0);
            vsOut.color = vec4f(0, 1, 0, 1.0);
        } else {
            vsOut.pos = vec4f(0.5, 0.5, 0, 1.0);
            vsOut.color = vec4f(0, 0, 1, 1.0);
        }
        return vsOut;
    }

    @fragment
    fn fs_main(vsOut: VSOut) -> @location(0) vec4f {
        return vsOut.color;
    }
