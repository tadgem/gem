#version 450

layout(location = 0) in vec2 aUV;
out vec4 color;

uniform sampler2D lighting_pass;
uniform sampler2D cone_tracing_pass;
uniform sampler2D ssr_pass;

uniform float u_exposure;

const float GAMMA = 2.2;
vec3 standard_gamma_tone_mapping(vec3 color, float exposure)
{
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-color.rgb * exposure);
    // gamma correction 
    return pow(mapped, vec3(1.0 / GAMMA));
}

vec3 filmic_tone_mapping(vec3 color)
{
	color = max(vec3(0.), color - vec3(0.004));
	color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
	return color;
}

vec3 Uncharted2ToneMapping(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	float exposure = 2.;
	color *= exposure;
	color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
	color /= white;
	color = pow(color, vec3(1. / GAMMA));
	return color;
}

void main(void)
{
    vec4 direct = texture(lighting_pass, aUV);
    vec4 indirect = texture(cone_tracing_pass, aUV);
    vec4 ssr = texture(ssr_pass, aUV);
    vec4 final_color = direct + (direct * indirect) + ssr;

    // gamma correction 
    //color = vec4(filmic_tone_mapping(final_color.rgb), 1.0);
	color = mix(direct, indirect + ssr, (length(indirect) / 2.0)) * 2.0;
} 