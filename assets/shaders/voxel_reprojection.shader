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

bool is_in_grid(ivec3 point, ivec3 resolution)
{
	if(point.x >= resolution.x) return false;
	if(point.y >= resolution.y) return false;
	if(point.z >= resolution.z) return false;

	if(point.x < 0) return false;
	if(point.y < 0) return false;
	if(point.z < 0) return false;

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

	vec3 aabb_dim 	= u_current_aabb.max - u_current_aabb.min;
	vec3 unit 		= vec3((aabb_dim.x / u_resolution.x), (aabb_dim.y / u_resolution.y) , (aabb_dim.z / u_resolution.z));

	// get the current world position in the current texel grid
	vec3 current_world_position 	= get_pos_from_texel(pix, u_resolution, u_current_aabb);
	// get a direction pointing from last bb to current bb
	vec3 reprojection_dir 			= u_current_aabb.min - u_previous_aabb.min;
	vec3 reprojection_texel_offset 	= reprojection_dir / unit;
	ivec3 last_grid_offset = ivec3(reprojection_texel_offset);
	ivec3 last_texel = pix - last_grid_offset;

	if(!is_in_grid(last_texel, ivec3(u_resolution)))
	{
		imageStore(u_current_grid, pix, vec4(0.0));
	}

	ivec3 current_uv_coord 				= pix;
	vec4 history_colour 				= imageLoad(u_history_grid, last_texel);

	imageStore(u_current_grid, current_uv_coord,  history_colour);
}