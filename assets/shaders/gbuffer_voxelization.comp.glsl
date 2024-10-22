#version 430 core

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
uniform vec2	  u_input_resolution;
layout(binding = 0, rgba16f) uniform image3D imgOutput;
layout(binding = 1, rgba16f) uniform image3D imgOutput1;
layout(binding = 2, rgba16f) uniform image3D imgOutput2;
layout(binding = 3, rgba16f) uniform image3D imgOutput3;
layout(binding = 4, rgba16f) uniform image3D imgOutput4;
layout(binding = 5, rgba16f) uniform image3D imgOutput5;

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

ivec3 get_texel_from_pos(vec3 position, vec3 resolution)
{
	vec3 aabb_dim = u_aabb.max - u_aabb.min;
	vec3 unit = vec3((aabb_dim.x / resolution.x), (aabb_dim.y / resolution.y) , (aabb_dim.z / resolution.z));

	/// <summary>
	/// 0,0,0 is aabb.min
	/// </summary>
	vec3 new_pos = position - u_aabb.min;
	int x = int(new_pos.x / unit.x) ;
	int y = int(new_pos.y / unit.y) ;
	int z = int(new_pos.z / unit.z) ;

	return ivec3(x, y, z);

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
	vec4 current = imageLoad(imgOutput, sample_pos);
	vec4 val = mix(light, current, 0.5);
	if(isnan(val.x) || isnan(val.y) || isnan(val.z))
	{
		return;
	}

	imageStore(imgOutput,sample_pos, val);
}