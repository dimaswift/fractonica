@ctype mat4 mat44_t

// --- COMPUTE SHADER ---
@cs cs_mandelbrot

layout(binding=0) uniform cs_params {
    vec2 center;
    float zoom;
    int width;
    int height;
};

// 'writeonly' means we only write to it. format 'rgba8' is standard.
layout(binding=0, rgba8) writeonly uniform image2D dest_tex;

layout(local_size_x=16, local_size_y=16, local_size_z=1) in;

void main() {
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;

    // Safety check
    if (x >= width || y >= height) return;

    // Map pixel to complex plane
    float aspect = float(width) / float(height);
    vec2 uv = vec2(float(x)/width, float(y)/height);
    vec2 c = center + (uv - 0.5) * vec2(aspect, 1.0) * (1.0/zoom);

    // Mandelbrot math
    vec2 z = vec2(0.0);
    int iter = 0;
    const int max_iter = 10;
    for(iter=0; iter<max_iter; iter++) {
        if(dot(z, z) > 4.0) break;
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
    }

    // Color mapping
    float t = float(iter) / float(max_iter);
    vec4 color = vec4(t, t*0.5, 1.0-t, 1.0);
    if (iter == max_iter) color = vec4(0.0, 0.0, 0.0, 1.0);

    // Write to texture
    imageStore(dest_tex, ivec2(x, y), color);
}
@end
@program mandelbrot cs_mandelbrot

// --- DISPLAY SHADER (Full Screen Quad) ---
@vs vs_display
in vec2 pos;       // Position (-1 to 1)
in vec2 texcoord0; // UVs (0 to 1)
out vec2 uv;

void main() {
    gl_Position = vec4(pos, 1.0, 1.0);
    uv = texcoord0;
}
@end

@fs fs_display
layout(binding=0) uniform texture2D tex;
layout(binding=0) uniform sampler smp;
in vec2 uv;
out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(tex, smp), uv);
}
@end

@program display vs_display fs_display