#version 450

layout(location = 0) in vec2 aUV;
out vec4 color;

uniform sampler2D lighting_pass;
uniform sampler2D cone_tracing_pass;
uniform sampler2D ssr_pass;

uniform float u_brightness;
uniform float u_contrast;
uniform float u_saturation;


mat4 brightnessMatrix( float brightness )
{
    return mat4( 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 brightness, brightness, brightness, 1 );
}

mat4 contrastMatrix( float contrast )
{
	float t = ( 1.0 - contrast ) / 2.0;
    
    return mat4( contrast, 0, 0, 0,
                 0, contrast, 0, 0,
                 0, 0, contrast, 0,
                 t, t, t, 1 );

}

mat4 saturationMatrix( float saturation )
{
    vec3 luminance = vec3( 0.3086, 0.6094, 0.0820 );
    
    float oneMinusSat = 1.0 - saturation;
    
    vec3 red = vec3( luminance.x * oneMinusSat );
    red+= vec3( saturation, 0, 0 );
    
    vec3 green = vec3( luminance.y * oneMinusSat );
    green += vec3( 0, saturation, 0 );
    
    vec3 blue = vec3( luminance.z * oneMinusSat );
    blue += vec3( 0, 0, saturation );
    
    return mat4( red,     0,
                 green,   0,
                 blue,    0,
                 0, 0, 0, 1 );
}

void main(void)
{
    vec4 direct = texture(lighting_pass, aUV);
    vec4 indirect = texture(cone_tracing_pass, aUV);
    vec4 ssr = texture(ssr_pass, aUV);
    vec4 final_color = mix(direct, indirect + ssr, (length(indirect) / 2.0)) * 2.0;
 
    color =     brightnessMatrix( u_brightness ) *
        		contrastMatrix( u_contrast ) * 
        		saturationMatrix( u_saturation ) *
        		final_color;
} 