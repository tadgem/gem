#version 430 core
#compute

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

struct AABB
{
	vec3 min;
	vec3 max;
};

layout(binding = 0, rgba16f) uniform image3D u_grid;

uniform vec3 u_resolution;
uniform AABB u_previous_aabb;
uniform AABB u_current_aabb;

bool is_in_aabb(vec3 point, AABB bb)
{
	if(point.x > bb.max.x) return false;
	if(point.y > bb.max.y) return false;
	if(point.z > bb.max.z) return false;
	if(point.x < bb.min.x) return false;
	if(point.y < bb.min.y) return false;
	if(point.z < bb.min.z) return false;
	return true;
}

vec3 transform_point(vec3 point, AABB previous, AABB current)
{
	return vec3(0.0);
}

void main() {
	ivec3 pix = ivec3(gl_GlobalInvocationID.xyz);
	vec3 uv = pix / u_resolution;

	vec3 prev_unit 		= (u_previous_aabb.max - u_previous_aabb.min) / u_resolution;
	vec3 last_world_position 	= u_previous_aabb.min + (prev_unit * pix);

	vec3 current_unit 	= (u_current_aabb.max - u_current_aabb.min) / u_resolution;
	vec3 new_world_position		= u_current_aabb.min + (current_unit * pix);

	if(!is_in_aabb(new_world_position, u_previous_aabb))
	{
		imageStore(u_grid, pix, vec4(0.0));
	}

	vec3 last_position_diff = last_world_position - u_previous_aabb.min;
	ivec3 last_uv_coord = ivec3(last_position_diff / prev_unit);
	vec4 colour = imageLoad(u_grid, last_uv_coord);
	imageStore(u_grid, pix, colour);
	// convert new position to a uv coord in the old grid
}