#version 430 core

layout(local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

uniform sampler2D u_gbuffer_pos;
uniform sampler2D u_gbuffer_lighting;

layout(binding = 0, rgba32f) uniform image3D imgOutput;

void main() {
	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
	vec2 uv = vec2(gl_GlobalInvocationID.x / 1280, gl_GlobalInvocationID.y / 720);
	vec4 pos = texture(u_gbuffer_pos, uv);
	vec4 light = texture(u_gbuffer_lighting, uv);
	light.w = 1.0f;

	ivec3 sample_pos = ivec3(pos.xyz);
	
	imageStore(imgOutput,sample_pos, light);
}