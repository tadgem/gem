#version 430 core
#compute

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;


layout(binding = 0, rgba16f) uniform image3D current_voxel_map;

void main() {
	ivec3 pix = ivec3(gl_GlobalInvocationID.xyz);
	imageStore(current_voxel_map, pix, vec4(0.0));
}