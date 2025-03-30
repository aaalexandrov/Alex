    struct Uniforms {
        worldViewProj: mat4x4f,
    };

    @group(0) @binding(0) var<uniform> uni: Uniforms;
    @group(0) @binding(1) var texSampler: sampler;
    @group(0) @binding(2) var tex0: texture_2d<f32>;
   
    struct VSIn {
        @location(0) pos: vec3f, 
        @location(1) color: vec3f, 
        @location(2) uv: vec2f,
    };

    struct VSOut {
        @builtin(position) pos: vec4f,
        @location(0) color: vec4f,
        @location(1) uv: vec2f,
    };

    @vertex
    fn vs_main(vsIn: VSIn) -> VSOut {
        var vsOut: VSOut;
        vsOut.pos = uni.worldViewProj * vec4f(vsIn.pos, 1.0);
        vsOut.color = vec4f(vsIn.color, 1.0);
        vsOut.uv = vsIn.uv;
        return vsOut;
    }

    @fragment
    fn fs_main(vsOut: VSOut) -> @location(0) vec4f {
        var tex: vec4f = textureSample(tex0, texSampler, vsOut.uv);
        return vsOut.color * tex;
    }
