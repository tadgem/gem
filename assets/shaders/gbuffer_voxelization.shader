#version 430 core
#compute

layout(local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

struct AABB
{
	vec3 min;
	vec3 max;
};

uniform sampler2D u_gbuffer_pos;
uniform sampler2D u_gbuffer_lighting;
uniform AABB	  u_aabb;
uniform vec3	  u_voxel_resolution;
uniform vec3	  u_voxel_unit;
uniform vec2	  u_input_resolution;

layout(binding = 0, rgba16f) uniform image3D imgOutput;

bool is_in_aabb(vec3 pos)
{
	if (pos.x < u_aabb.min.x) { return false; }
	if (pos.y < u_aabb.min.y) { return false; }
	if (pos.z < u_aabb.min.z) { return false; }

	if (pos.x > u_aabb.max.x) { return false; }
	if (pos.y > u_aabb.max.y) { return false; }
	if (pos.z > u_aabb.max.z) { return false; }

	return true;
}

float remap(float val, float from1, float to1, float from2, float to2)
{
	return (val - from1) / (to1 - from1) * (to2 - from2) + from2;
}

vec3 remap_vec3(vec3 val, vec3 from1, vec3 to1, vec3 from2, vec3 to2)
{
	return vec3(
		remap(val.x, from1.x, to1.x, from2.x, to2.x),
		remap(val.y, from1.y, to1.y, from2.y, to2.y),
		remap(val.z, from1.z, to1.z, from2.z, to2.z)
	);
}

ivec3 get_texel_from_pos(vec3 position, vec3 resolution)
{
	vec3 area = abs(u_aabb.max - u_aabb.min);
	vec3 center = u_aabb.min + (area * 0.5);
//
//	vec3 new_pos = position - u_aabb.min;
//	int x = int(new_pos.x / u_voxel_unit.x) ;
//	int y = int(new_pos.y / u_voxel_unit.y) ;
//	int z = int(new_pos.z / u_voxel_unit.z) ;

	vec3 normalized_offset = (position - center) / area;
	vec3 normalized_coords = vec3(0.5) + normalized_offset;

	return ivec3(normalized_coords * resolution);

}

void main() {
	ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = ivec2(u_input_resolution.x, u_input_resolution.y);
	if (pix.x >= size.x || pix.y >= size.y) {
		return;
	}
	vec2 uv = vec2(pix) / size;
	vec3 pos = texture(u_gbuffer_pos, uv).xyz;

	// is the pixel position within the bounding volume, if not do nothing
	if (!is_in_aabb(pos))
	{
		return;
	}

	// if the pixel is in volume, map position to from aabb to 3d texture
	// store pixel light value at this voxel

	vec4 light = texture(u_gbuffer_lighting, uv);
	light.w = 1.0;

	ivec3 sample_pos = get_texel_from_pos(pos.xyz, u_voxel_resolution);
	if(isnan(light.x) || isnan(light.y) || isnan(light.z))
	{
		return;
	}

	imageStore(imgOutput,sample_pos, light);
}