#version 430 core
#compute

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding=1) buffer ssbo_instance_matrices { //GL_VERTEX_BUFFER?/ GL_STATIC_DRAW
	mat4 u_instance_matrices[];
};

struct AABB
{
	vec3 min;
	vec3 max;
};

uniform vec3 u_resolution;
uniform vec3 u_voxel_unit;
uniform vec3 u_instance_resolution;
uniform AABB u_current_aabb;

//notes:
// num instances = voxel_resolution / u_instance_resolution
// each increment in position should be:
// u_voxel_unit * u_instance_resolution

mat4 translate(mat4 m, vec3 p)
{
	mat4 result = m;
	result[3] = m[0] * p[0] + m[1] * p[1] + m[2] * p[2] + m[3];
	return result;
}

mat4 scale(mat4 m, vec3 v)
{
	mat4 Result;
	Result[0] = m[0] * v[0];
	Result[1] = m[1] * v[1];
	Result[2] = m[2] * v[2];
	Result[3] = m[3];
	return Result;
}

float to_radians(float deg)
{
	return deg * 0.01745329251994329576923690768489;
}

vec4 angle_axis(float angle, vec3 axis)
{
	float a = angle;
	float s = sin(a * 0.5);
	return vec4(cos(a * 0.5), axis * s);
}

vec4 multiply_quaternions(vec4 a, vec4 b) {
	return vec4(
	a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,  // 1
	a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,  // i
	a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,  // j
	a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w   // k
	);
}

mat3 mat3_cast(vec4 q)
{
	mat3 Result = mat3(1.0);
	float qxx = q.x * q.x;
	float qyy = q.y * q.y;
	float qzz = q.z * q.z;
	float qxz = q.x * q.z;
	float qxy = q.x * q.y;
	float qyz = q.y * q.z;
	float qwx = q.w * q.x;
	float qwy = q.w * q.y;
	float qwz = q.w * q.z;

	Result[0][0] = 1.0 - 2.0 * (qyy +  qzz);
	Result[0][1] = 2.0 * (qxy + qwz);
	Result[0][2] = 2.0 * (qxz - qwy);

	Result[1][0] = 2.0 * (qxy - qwz);
	Result[1][1] = 1.0 - 2.0 * (qxx +  qzz);
	Result[1][2] = 2.0 * (qyz + qwx);

	Result[2][0] = 2.0 * (qxz + qwy);
	Result[2][1] = 2.0 * (qyz - qwx);
	Result[2][2] = 1.0 - 2.0 * (qxx +  qyy);
	return Result;
}

vec4 quat_from_euler(vec3 euler)
{
	vec3 rads = vec3(to_radians(euler.x), to_radians(euler.y), to_radians(euler.z));
	vec4 x_rot = angle_axis(rads.x, vec3(1.0, 0.0, 0.0));
	vec4 y_rot = angle_axis(rads.y, vec3(0.0, 1.0, 0.0));
	vec4 z_rot = angle_axis(rads.z, vec3(0.0, 0.0, 1.0));

	return multiply_quaternions(z_rot, multiply_quaternions(y_rot, x_rot));
}

mat4 get_model_matrix(vec3 pos, vec3 scalev)
{
	mat4 m = mat4(1.0);
	m = translate(m, pos);
	//vec3 euler = vec3(0.0001);
	//vec4 quat = quat_fewrom_euler(euler);
	//mat4 rot_mat = mat4(mat3_cast(quat));
	mat4 scale_mat = scale(m, scalev);
	return m *  scale_mat;
}

uint get_texel_index(ivec3 texel, ivec3 resolution)
{
	return texel.x + texel.y * resolution.x + texel.z * resolution.x * resolution.y;
}

ivec3 get_index_texel (int index, int xSize, int ySize, int zSize) {
	ivec3 result = ivec3(0, 0, 0);
	result.x = index % xSize;
	result.y = (index / xSize) % ySize;
	result.z = index / (xSize * ySize);
	return result;
}


void main() {
	uint 	index =	gl_GlobalInvocationID.x;
	vec3 	start_position = u_current_aabb.min;
	vec3	step_increment = (u_voxel_unit * u_instance_resolution);
	vec3	matrix_res = u_resolution / u_instance_resolution;

	ivec3 	texel = get_index_texel(int(index), int(matrix_res.x),int(matrix_res.y), int(matrix_res.z));
	vec3	final_position = start_position + (texel * step_increment);

	u_instance_matrices[index] = get_model_matrix(vec3(final_position.x, final_position.y, final_position.z), u_voxel_unit);
	// get instance X,Y,Z
}