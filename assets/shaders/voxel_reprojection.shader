#version 430 core
#compute

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

struct AABB
{
	vec3 min;
	vec3 max;
};

layout(binding = 0, rgba16f) uniform image3D ;

uniform vec3 u_current_resolution;
uniform vec3 u_previous_aabb;
uniform vec3 u_current_aabb;

vec3 transform_point(vec3 point, AABB previous, AABB current)
{
	return vec3(0.0);
}

void main() {
	ivec3 pix = ivec3(gl_GlobalInvocationID.xyz);
	vec3 uv = pix / u_current_resolution;
}