#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in mat4 iTransform;

layout(location = 0) out vec3 oUVW;

uniform mat4	u_view_projection;
uniform ivec3	u_texture_resolution;
uniform ivec3	u_voxel_group_resolution;

ivec3 get_uv_from_invocation( int idx, ivec3 limits ) {
    int tmp = idx;
    int z = tmp / (limits.x * limits.y);
    tmp -= (z * limits.x * limits.y);
    int y = tmp / limits.x;
    int x = tmp % limits.x;
    return ivec3( x , y , z);
}

void main()
{
	ivec3 base_uv = get_uv_from_invocation(gl_InstanceID, u_texture_resolution / u_voxel_group_resolution) * u_voxel_group_resolution;
    ivec3 offset_uv = get_uv_from_invocation(gl_VertexID / 24, u_voxel_group_resolution);
    ivec3 uv = base_uv + offset_uv;

    vec3 uvw = vec3(float(uv.x / u_texture_resolution.x), float(uv.x / u_texture_resolution.y), float(uv.x / u_texture_resolution.z)); 
	oUVW = uvw;

	vec3 worldPos = vec3(iTransform * vec4(aPos, 1.0));
	gl_Position = u_view_projection * vec4(worldPos, 1.0);
}