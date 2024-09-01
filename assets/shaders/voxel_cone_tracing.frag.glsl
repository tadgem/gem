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

uniform sampler2D   u_position_map;
uniform sampler2D   u_normal_map;
uniform sampler3D   u_voxel_map; // x = metallic, y = roughness, z = AO
uniform vec3		u_aabb_min;
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

// Traces a diffuse voxel cone.
vec3 traceDiffuseVoxelCone(const vec3 from, vec3 direction) {
	direction = normalize(direction);

	const float CONE_SPREAD = 0.325;

	vec4 acc = vec4(0.0f);

	// Controls bleeding from close surfaces.
	// Low values look rather bad if using shadow cone tracing.
	// Might be a better choice to use shadow maps and lower this value.
	float dist = 0.1953125;

	// Trace.
	while (dist < SQRT2 && acc.a < 1) {
		vec3 c = from + dist * direction;
		c = scaleAndBias(from + dist * direction);
		float l = (1 + CONE_SPREAD * dist / VOXEL_SIZE);
		float level = log2(l);
		float ll = (level + 1) * (level + 1);
		vec4 voxel = texture(u_voxel_map, c);
		acc += 0.075 * ll * voxel * pow(1 - voxel.a, 2);
		dist += ll * VOXEL_SIZE * 2;
	}
	return pow(acc.rgb * 2.0, vec3(1.5));
}

vec3 indirectDiffuseLight(vec3 position, vec3 normal, vec3 diffuse) {
	const float ANGLE_MIX = 0.5f; // Angle mix (1.0f => orthogonal direction, 0.0f => direction of normal).

	const float w[3] = { 1.0, 1.0, 1.0 }; // Cone weights.

	// Find a base for the side cones with the normal as one of its base vectors.
	const vec3 ortho = normalize(orthogonal(normal));
	const vec3 ortho2 = normalize(cross(ortho, normal));

	// Find base vectors for the corner cones too.
	const vec3 corner = 0.5f * (ortho + ortho2);
	const vec3 corner2 = 0.5f * (ortho - ortho2);

	// Find start position of trace (start with a bit of offset).
	const vec3 N_OFFSET = normal * (1 + 4 * ISQRT2) * VOXEL_SIZE;
	const vec3 C_ORIGIN = position + N_OFFSET;

	// Accumulate indirect diffuse light.
	vec3 acc = vec3(0);

	// We offset forward in normal direction, and backward in cone direction.
	// Backward in cone direction improves GI, and forward direction removes
	// artifacts.
	const float CONE_OFFSET = -0.01;

	// Trace front cone
	acc += w[0] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * normal, normal);

	// Trace 4 side cones.
	const vec3 s1 = mix(normal, ortho, ANGLE_MIX);
	const vec3 s2 = mix(normal, -ortho, ANGLE_MIX);
	const vec3 s3 = mix(normal, ortho2, ANGLE_MIX);
	const vec3 s4 = mix(normal, -ortho2, ANGLE_MIX);

	acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * ortho, s1);
	acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * ortho, s2);
	acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * ortho2, s3);
	acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * ortho2, s4);

	// Trace 4 corner cones.
	const vec3 c1 = mix(normal, corner, ANGLE_MIX);
	const vec3 c2 = mix(normal, -corner, ANGLE_MIX);
	const vec3 c3 = mix(normal, corner2, ANGLE_MIX);
	const vec3 c4 = mix(normal, -corner2, ANGLE_MIX);

	acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * corner, c1);
	acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * corner, c2);
	acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * corner2, c3);
	acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * corner2, c4);

	// Return result.
	return 1.0 * acc * (diffuse + vec3(0.001f));
}


void main()
{
	vec3 diffuse = vec3(1.0);
	vec3 position = texture(u_position_map, aUV).xyz;
	vec3 normal = texture(u_normal_map, aUV).xyz;
	vec3 normalized_n = normalize(normal);
	FragColor = vec4(indirectDiffuseLight(position - u_aabb_min, normalized_n, diffuse), 1.0);
}