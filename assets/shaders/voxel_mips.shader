#version 430 core
#compute

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;


layout(binding = 0, rgba16f) uniform image3D current_mip;
layout(binding = 1, rgba16f) uniform image3D last_mip;

uniform vec3 u_current_resolution;

void main() {
	ivec3 pix = ivec3(gl_GlobalInvocationID.xyz);
	vec3 uv = pix / u_current_resolution;
	// sample 8 texels matching current mip coord in previous mip
	ivec3 center = ivec3(gl_GlobalInvocationID.xyz) * 2;
	vec4 colour = vec4(0.0);

	ivec3 lod_pix1 = center;
	colour  += imageLoad(last_mip, lod_pix1);
	
	ivec3 lod_pix2 = center + ivec3(1,0,0);
	colour  +=  imageLoad(last_mip, lod_pix2);
	
	ivec3 lod_pix3 = center + ivec3(1,1,0);
	colour  +=  imageLoad(last_mip, lod_pix3);
	
	ivec3 lod_pix4 = center + ivec3(1,1,1);
	colour  +=  imageLoad(last_mip, lod_pix4);

	ivec3 lod_pix5 = center + ivec3(0,1,1);
	colour  +=  imageLoad(last_mip, lod_pix5);
		
	ivec3 lod_pix6 = center + ivec3(0,0,1);
	colour  +=  imageLoad(last_mip, lod_pix6);

	ivec3 lod_pix7 = center + ivec3(0,1,0);
	colour  +=  imageLoad(last_mip, lod_pix7);

	ivec3 lod_pix8 = center + ivec3(1,0,1);
	colour  +=  imageLoad(last_mip, lod_pix8);

	ivec3 lod_pix9 = center + ivec3(-1,0,0);
	colour  +=  imageLoad(last_mip, lod_pix9);
	
	ivec3 lod_pix10 = center + ivec3(-1,-1,0);
	colour  +=  imageLoad(last_mip, lod_pix10);
	
	ivec3 lod_pix11 = center + ivec3(-1,-1,-1);
	colour  +=  imageLoad(last_mip, lod_pix11);

	ivec3 lod_pix12 = center + ivec3(0,-1,-1);
	colour  += imageLoad(last_mip, lod_pix12);
		
	ivec3 lod_pix13 = center + ivec3(0,0,-1);
	colour  +=  imageLoad(last_mip, lod_pix13);

	ivec3 lod_pix14 = center + ivec3(0,-1,0);
	colour  +=  imageLoad(last_mip, lod_pix14);

	ivec3 lod_pix15 = center + ivec3(-1,0,-1);
	colour  +=  imageLoad(last_mip, lod_pix15);

	imageStore(current_mip, pix, colour * 0.1);
}