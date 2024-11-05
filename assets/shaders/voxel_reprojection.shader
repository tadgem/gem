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

ivec3 get_texel_from_pos(vec3 position, vec3 resolution, AABB bb)
{
	vec3 aabb_dim = bb.max - bb.min;
	vec3 unit = vec3((aabb_dim.x / resolution.x), (aabb_dim.y / resolution.y) , (aabb_dim.z / resolution.z));

	/// <summary>
	/// 0,0,0 is aabb.min
	/// </summary>
	vec3 new_pos = position - bb.min;
	int x = int(new_pos.x / unit.x) ;
	int y = int(new_pos.y / unit.y) ;
	int z = int(new_pos.z / unit.z) ;

	return ivec3(x, y, z);

}

void main() {
	ivec3 pix = ivec3(gl_GlobalInvocationID.xyz);
	vec3 uv = pix / u_resolution;

	if(distance(u_previous_aabb.min, u_current_aabb.min) < 0.5)
	{
		return;
	}

	vec3 prev_unit 		= (u_previous_aabb.max - u_previous_aabb.min) / u_resolution;
	vec3 last_world_position 	= u_previous_aabb.min + (prev_unit * pix);

	vec3 current_unit 	= (u_current_aabb.max - u_current_aabb.min) / u_resolution;
	vec3 new_world_position		= u_current_aabb.min + (current_unit * pix);

	if(!is_in_aabb(new_world_position, u_previous_aabb))
	{
		imageStore(u_grid, pix, vec4(0.0));
	}

	ivec3 last_uv_coord = get_texel_from_pos(last_world_position, u_resolution, u_previous_aabb);
	vec4 colour = imageLoad(u_grid, last_uv_coord);
	imageStore(u_grid, pix, colour);
	// convert new position to a uv coord in the old grid
}