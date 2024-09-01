#version 450

layout(location = 0) in vec2 aUV;
out vec4 FragColor;

#define TSQRT2 2.828427
#define SQRT2 1.414213
#define ISQRT2 0.707106
#define DIST_FACTOR 1.1f /* Distance is multiplied by this when calculating attenuation. */
#define CONSTANT 1
#define LINEAR 0 /* Looks meh when using gamma correction. */
#define QUADRATIC 1
struct AABB
{
	vec3 min;
	vec3 max;
};


uniform sampler2D   u_position_map;
uniform sampler2D   u_normal_map;
uniform sampler3D   u_voxel_map; // x = metallic, y = roughness, z = AO
uniform AABB		u_aabb;
uniform vec3	  u_voxel_resolution;
#define VOXEL_SIZE (1/128.0)

vec3 scaleAndBias(const vec3 p) { return 0.5f * p + vec3(0.5f); }
// Returns an attenuation factor given a distance.
float attenuate(float dist) { dist *= DIST_FACTOR; return 1.0f / (CONSTANT + LINEAR * dist + QUADRATIC * dist * dist); }
// Returns a vector that is orthogonal to u.
vec3 orthogonal(vec3 u) {
	u = normalize(u);
	vec3 v = vec3(0.99146, 0.11664, 0.05832); // Pick any normalized vector.
	return abs(dot(u, v)) > 0.99999f ? cross(u, vec3(0, 1, 0)) : cross(u, v);
}

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


vec3 get_texel_from_pos(vec3 position, vec3 unit)
{
	vec3 clip_pos = position - u_aabb.min;
	int x = int(clip_pos.x / unit.x);
	int y = int(clip_pos.y / unit.y);
	int z = int(clip_pos.z / unit.z);
	return vec3(float(x/ u_voxel_resolution.x), float(y / u_voxel_resolution.y), float(z / u_voxel_resolution.z));
}

vec4 get_voxel_colour(vec3 position, vec3 unit)
{
	return texture(u_voxel_map, get_texel_from_pos(position, unit));
}

vec3 trace_cone(vec3 from, vec3 dir, vec3 unit)
{
	const int max_steps = 16;
	vec4 accum = vec4(0.0);
	vec3 pos = from;
	int steps = 0;
	while (accum.w < 0.99 && steps < max_steps)
	{
		pos += unit * dir;
		vec4 result = get_voxel_colour(pos, unit);
		accum += result;
		steps += 1;
	}
	return accum.xyz;
}

vec3 trace_cones(vec3 from, vec3 dir, vec3 unit)
{
	vec3 c1 = dir + vec3(0.1, 0.1, 0.1);
	vec3 c2 = dir + vec3(-0.1, 0.1, -0.1);
	vec3 c3 = dir + vec3(-0.1, 0.1, 0.1);
	vec3 c4 = dir + vec3(0.1, 0.1, -0.1);

	vec3 r1 = trace_cone(from, c1, unit);
	vec3 r2 = trace_cone(from, c2, unit);
	vec3 r3 = trace_cone(from, c3, unit);
	vec3 r4 = trace_cone(from, c4, unit);

	return (r1 + r2 + r3 + r4) / 4.0;
}



void main()
{
	vec3 aabb_dim = u_aabb.max - u_aabb.min;
	vec3 unit = vec3((aabb_dim.x / u_voxel_resolution.x), (aabb_dim.y / u_voxel_resolution.y), (aabb_dim.z / u_voxel_resolution.z));
	vec3 diffuse = vec3(1.0);
	vec3 position = texture(u_position_map, aUV).xyz;
	vec3 normal = texture(u_normal_map, aUV).xyz;
	vec3 normalized_n = normalize(normal);
	FragColor = vec4(trace_cones(position, normalized_n, unit), 1.0);
}