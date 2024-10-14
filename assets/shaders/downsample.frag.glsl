#version 450

out vec4 FragColor;

layout (location = 0) in vec2 aUV;

uniform sampler2D u_prev_mip;

void main()
{
	vec2 unit = 1.0 / textureSize(u_prev_mip, 0);

	vec4 colour = vec4(0.0);

	// top left
	colour += texture(u_prev_mip, aUV + vec2(-unit.x, unit.y));
	// top center
	colour += texture(u_prev_mip, aUV + vec2(0.0, unit.y));
	// top center
	colour += texture(u_prev_mip, aUV + vec2(unit.x, unit.y));
	// center left
	colour += texture(u_prev_mip, aUV + vec2(-unit.x, 0.0));
	// center
	colour += texture(u_prev_mip, aUV);
	// center right
	colour += texture(u_prev_mip, aUV + vec2(unit.x, 0.0));
	// bottom left
	colour += texture(u_prev_mip, aUV + vec2(-unit.x, -unit.y));
	// bottom center
	colour += texture(u_prev_mip, aUV + vec2(0.0, -unit.y));
	// bottom center
	colour += texture(u_prev_mip, aUV + vec2(unit.x, -unit.y));

	FragColor = colour / 9.0;
}