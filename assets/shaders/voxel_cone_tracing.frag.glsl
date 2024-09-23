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

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
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
	float x = clip_pos.x / unit.x;
	float y = clip_pos.y / unit.y;
	float z = clip_pos.z / unit.z;
	return vec3(x/ u_voxel_resolution.x, y / u_voxel_resolution.y, z / u_voxel_resolution.z);
}

vec4 get_voxel_colour(vec3 position, vec3 unit, float lod)
{
	return textureLod(u_voxel_map, get_texel_from_pos(position, unit), lod);
}

vec3 trace_ray(vec3 from, vec3 dir, vec3 unit)
{
	vec4 accum = vec4(0.0);
	vec3 pos = from;
	int steps = 0;
	const int MAX_LOD = 5;
	int lod = 5;
	while (accum.w < 0.99 && is_in_aabb(pos))
	{
		pos += unit * (lod + 1) * dir;
		vec4 result = get_voxel_colour(pos, unit, lod);
		if(result.w > 0.2 && lod > 0)
		{
			lod -= 1;
		}
		else if(result.w < 0.1)
		{
			lod += 1;
		}
		if(lod == 0)
		{
			accum += result * (1.0 - (lod / MAX_LOD));
		}
		steps += 1;
	}
	return accum.xyz;
}

float remap(float source, float sourceFrom, float sourceTo, float targetFrom, float targetTo)
{
	return targetFrom + (source-sourceFrom)*(targetTo-targetFrom)/(sourceTo-sourceFrom);
}

vec3 trace_cone(vec3 from, vec3 dir, vec3 unit)
{
	const int max_steps = 2000; // should probs be the longest axis of minimum mip dimension
	vec4 accum = vec4(0.0);
	vec3 pos = from;
	int steps = 0;
	const int MAX_LOD = 5;
	float lod = 0.0;
	while (accum.w < 1.0 && is_in_aabb(pos) && steps < max_steps)
	{
		pos += dir * (steps + 1);
		vec4 result = get_voxel_colour(pos, unit, lod);
		accum += result;
		lod += 0.5;
		steps += 1;
	}
	return accum.xyz;
}

vec3 trace_cones_v2(vec3 from, vec3 dir, vec3 unit)
{
	//float ANGLE_MIX = rand(from.xy + dir.xy);
	const float ANGLE_MIX = 0.5;

	const float w[3] = { 1.0, 1.0, 1.0 }; // Cone weights.

	const vec3 ortho = normalize(orthogonal(dir));
	const vec3 ortho2 = normalize(cross(ortho, dir));
	
	const vec3 corner1 = 0.5f * (ortho + ortho2);
	const vec3 corner1_2 = 0.5f * (ortho - ortho2);
	
	const vec3 corner2 = 0.25f * (ortho + ortho2);
	const vec3 corner2_2 = 0.25f * (ortho - ortho2);

	const vec3 corner3 = 0.75f * (ortho + ortho2);
	const vec3 corner3_2 = 0.75f * (ortho - ortho2);

	vec3 acc = vec3(0);

	acc += w[0] * trace_cone(from, dir, unit);

	const vec3 s1 = mix(dir, ortho, ANGLE_MIX);
	const vec3 s2 = mix(dir, -ortho, ANGLE_MIX);
	const vec3 s3 = mix(dir, ortho2, ANGLE_MIX);
	const vec3 s4 = mix(dir, -ortho2, ANGLE_MIX);

	acc += w[1] * trace_cone(from, s1, unit);
	acc += w[1] * trace_cone(from, s2, unit);
	acc += w[1] * trace_cone(from, s3, unit);
	acc += w[1] * trace_cone(from, s4, unit);

	const vec3 c1_1 = mix(dir, corner1, ANGLE_MIX);
	const vec3 c1_2 = mix(dir, -corner1, ANGLE_MIX);
	const vec3 c1_3 = mix(dir, corner1_2, ANGLE_MIX);
	const vec3 c1_4 = mix(dir, -corner1_2, ANGLE_MIX);

	acc += w[2] * trace_cone(from, c1_1, unit);
	acc += w[2] * trace_cone(from, c1_2, unit);
	acc += w[2] * trace_cone(from, c1_3, unit);
	acc += w[2] * trace_cone(from, c1_4, unit);

	return acc / 9.0; // num traces to get a more usable output for now;
}



void main()
{
	vec3 aabb_dim = u_aabb.max - u_aabb.min;
	vec3 unit = vec3((aabb_dim.x / u_voxel_resolution.x), (aabb_dim.y / u_voxel_resolution.y), (aabb_dim.z / u_voxel_resolution.z));
	vec3 diffuse = vec3(1.0);
	vec3 position = texture(u_position_map, aUV).xyz;
	vec3 normal = texture(u_normal_map, aUV).xyz;
	vec3 normalized_n = normalize(normal);
	vec3 v_diffuse = trace_cones_v2(position, normalized_n, unit);
	FragColor = vec4(v_diffuse, 1.0);
}