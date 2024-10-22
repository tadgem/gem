#version 450
#vert

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

layout(location = 0) out vec2 oUV;

void main()
{
    oUV = aUV;
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}

#frag

out vec4 FragColor;

layout (location = 0) in vec2 aUV;

uniform sampler2D u_current_light_buffer;
uniform sampler2D u_history_light_buffer;
uniform sampler2D u_velocity_buffer;
uniform vec2      u_resolution;
vec3 encodePalYuv(vec3 rgb)
{
    rgb = pow(rgb, vec3(2.0)); // gamma correction
    return vec3(
        dot(rgb, vec3(0.299, 0.587, 0.114)),
        dot(rgb, vec3(-0.14713, -0.28886, 0.436)),
        dot(rgb, vec3(0.615, -0.51499, -0.10001))
    );
}

vec3 decodePalYuv(vec3 yuv)
{
    vec3 rgb = vec3(
        dot(yuv, vec3(1., 0., 1.13983)),
        dot(yuv, vec3(1., -0.39465, -0.58060)),
        dot(yuv, vec3(1., 2.03211, 0.))
    );
    return pow(rgb, vec3(1.0 / 2.0)); // gamma correction
}

float mitchellNetravali(float x, float B, float C)
{
	float ax = abs(x);
    float ax3 = ax*ax*ax;
    if (ax < 1.) 
    {
        return ((12. - 9. * B - 6. * C) * ax * ax * ax +
                (-18. + 12. * B + 6. * C) * ax * ax + (6. - 2. * B)) / 6.;
    } 
    else if ((ax >= 1.) && (ax < 2.)) 
    {
        return ((-B - 6. * C) * ax * ax * ax +
                (6. * B + 30. * C) * ax * ax + (-12. * B - 48. * C) *
                ax + (8. * B + 24. * C)) / 6.;
    } 
    else
    {
        return 0.;
    }
}

void main()
{
      vec4 current_colour = texture(u_current_light_buffer, aUV);
      const vec2 unit = vec2(1.0) / u_resolution;
      vec2 velocity       = texture(u_velocity_buffer, aUV).xy / u_resolution;
      if(velocity.x < unit.x)
      {
          velocity.x = 0.0;
      }
      if(velocity.y < unit.y)
      {
          velocity.y = 0.0;
      }
      vec3 history_colour = texture(u_history_light_buffer, aUV - velocity).xyz;

      // Apply clamping on the history color.
      vec3 NearColor0 = texture(u_current_light_buffer, aUV + vec2(unit.x, 0)).xyz;
      vec3 NearColor1 = texture(u_current_light_buffer, aUV + vec2(0, unit.y)).xyz;
      vec3 NearColor2 = texture(u_current_light_buffer, aUV + vec2(-unit.x, 0)).xyz;
      vec3 NearColor3 = texture(u_current_light_buffer, aUV + vec2(0, -unit.y)).xyz;
    
      vec3 BoxMin = min(current_colour.xyz, min(NearColor0, min(NearColor1, min(NearColor2, NearColor3))));
      vec3 BoxMax = max(current_colour.xyz, max(NearColor0, max(NearColor1, max(NearColor2, NearColor3))));;
    
      history_colour = clamp(history_colour, BoxMin, BoxMax);
 
      // Clamp previous color to min/max bounding box
      vec4 previousColorClamped = vec4(history_colour.xyz, 1.0);
 
      FragColor = mix(current_colour, previousColorClamped, 0.95);
}