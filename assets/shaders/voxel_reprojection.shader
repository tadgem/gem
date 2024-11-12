#version 430 core
#compute

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

struct AABB
{
	vec3 min;
	vec3 max;
};

layout(binding = 0, rgba16f) uniform image3D u_current_grid;
layout(binding = 1, rgba16f) uniform image3D u_history_grid;

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

ivec3 get_texel_from_pos(vec3 position, vec3 resolution, AABB bb)
{
	vec3 aabb_dim = bb.max - bb.min;
	vec3 unit = vec3((aabb_dim.x / resolution.x), (aabb_dim.y / resolution.y) , (aabb_dim.z / resolution.z));

	vec3 new_pos = position - bb.min;
	int x = int(new_pos.x / unit.x) ;
	int y = int(new_pos.y / unit.y) ;
	int z = int(new_pos.z / unit.z) ;

	return ivec3(x, y, z);
}

vec3 get_pos_from_texel(ivec3 texel, vec3 resolution, AABB bb)
{
	vec3 aabb_dim = bb.max - bb.min;
	vec3 unit = vec3((aabb_dim.x / resolution.x), (aabb_dim.y / resolution.y) , (aabb_dim.z / resolution.z));

	return bb.min + (texel * unit);
}

void main() {
	ivec3 	pix 					= ivec3(gl_GlobalInvocationID.xyz);
	vec3 	uv 						= pix / u_resolution;
	vec3 	last_world_position 	= get_pos_from_texel(pix, u_resolution, u_previous_aabb);

	if(!is_in_aabb(last_world_position, u_current_aabb))
	{
		imageStore(u_current_grid, pix, vec4(0.0));
	}

	vec3 	new_world_position		= get_pos_from_texel(pix, u_resolution, u_current_aabb);

	ivec3 last_uv_coord 			= get_texel_from_pos(last_world_position, u_resolution, u_previous_aabb);
	ivec3 current_uv_coord 			= get_texel_from_pos(new_world_position, u_resolution, u_current_aabb);
	vec4 history_colour 					= imageLoad(u_history_grid, last_uv_coord);
	vec4 current_colour = imageLoad(u_current_grid, current_uv_coord);

	imageStore(u_current_grid, current_uv_coord, mix(current_colour, history_colour, 0.5));
}