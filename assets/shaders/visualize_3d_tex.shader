#version 450
#vert

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 oUVW;

struct AABB
{
	vec3 min;
	vec3 max;
};

uniform mat4	u_view_projection;
uniform mat4	u_model;
uniform ivec3	u_texture_resolution;
uniform ivec3	u_voxel_group_resolution;
uniform AABB	u_aabb;


layout(std430, binding=1) buffer ssbo_instance_matrices { //GL_VERTEX_BUFFER?/ GL_STATIC_DRAW
	mat4 u_instance_matrices[];
};

vec3 get_uv_from_invocation( int idx, ivec3 limits ) {
    int tmp = idx;
    int z = tmp / (limits.x * limits.y);
    int y = (tmp / limits.x) % limits.y;
    int x = tmp % limits.x;
    return vec3( float(x) , float(y), float(z));
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


void main()
{
	vec3 base_uv = get_uv_from_invocation(gl_InstanceID, u_texture_resolution / u_voxel_group_resolution) * u_voxel_group_resolution;
    vec3 offset_uv = get_uv_from_invocation(gl_VertexID / 24, u_voxel_group_resolution);
    vec3 uv = base_uv + offset_uv;
	vec3 unit = (u_aabb.max - u_aabb.min) / u_texture_resolution;
    // UV needs to be in range 0-1 for texture sampling
    vec3 uvw = vec3(float(uv.x / u_texture_resolution.x), float(uv.y / u_texture_resolution.y), float(uv.z / u_texture_resolution.z));
	oUVW = uvw;

	const int INSTANCE_INDEX = gl_InstanceID;

	vec4 worldPos = u_instance_matrices[INSTANCE_INDEX] * vec4(aPos, 1.0);

	gl_Position = u_view_projection * worldPos;
}
#frag

out vec4 FragColor;

layout(location = 0) in vec3 oUVW;

uniform sampler3D u_volume;

void main()
{
	vec4 col = texture(u_volume, oUVW);
	if (col.w < 0.3)
	{
		discard;
	}
	FragColor = texture(u_volume, oUVW) * 2.0f;
}