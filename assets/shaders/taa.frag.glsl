#version 450

out vec4 FragColor;

layout (location = 0) in vec2 aUV;

uniform sampler2D u_current_light_buffer;
uniform sampler2D u_history_light_buffer;
uniform sampler2D u_velocity_buffer;

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
//    vec4 current_colour = texture(u_current_light_buffer, aUV);
//    vec2 velocity       = texture(u_velocity_buffer, aUV).xy / vec2(1920.0, 1080.0);
//    //vec2 velocity       = texture(u_velocity_buffer, aUV).xy;
//    vec4 history_colour = texture(u_history_light_buffer, aUV - velocity);
//
//    float modulationFactor = min(history_colour.w, 0.5);
//
//    vec2 off = 1.0 / vec2(1920.0, 1080.0);
//    vec3 antialiased = history_colour.xyz;
//    antialiased = mix(antialiased * antialiased, current_colour.xyz * current_colour.xyz, modulationFactor);
//    antialiased = sqrt(antialiased);
//
//    vec3 in1 = texture(u_current_light_buffer, aUV + vec2(+off.x, 0.0)).xyz;
//    vec3 in2 = texture(u_current_light_buffer, aUV + vec2(-off.x, 0.0)).xyz;
//    vec3 in3 = texture(u_current_light_buffer, aUV + vec2(0.0, +off.y)).xyz;
//    vec3 in4 = texture(u_current_light_buffer, aUV + vec2(0.0, -off.y)).xyz;
//    vec3 in5 = texture(u_current_light_buffer, aUV + vec2(+off.x, +off.y)).xyz;
//    vec3 in6 = texture(u_current_light_buffer, aUV + vec2(-off.x, +off.y)).xyz;
//    vec3 in7 = texture(u_current_light_buffer, aUV + vec2(+off.x, -off.y)).xyz;
//    vec3 in8 = texture(u_current_light_buffer, aUV + vec2(-off.x, -off.y)).xyz;
//
//    antialiased = encodePalYuv(antialiased);
//    vec3 in0 = encodePalYuv(current_colour.xyz);
//    in1 = encodePalYuv(in1);
//    in2 = encodePalYuv(in2);
//    in3 = encodePalYuv(in3);
//    in4 = encodePalYuv(in4);
//    in5 = encodePalYuv(in5);
//    in6 = encodePalYuv(in6);
//    in7 = encodePalYuv(in7);
//    in8 = encodePalYuv(in8);
//
//    vec3 minColor = min(min(min(in0, in1), min(in2, in3)), in4);
//    vec3 maxColor = max(max(max(in0, in1), max(in2, in3)), in4);
//    minColor = mix(minColor,
//       min(min(min(in5, in6), min(in7, in8)), minColor), 0.5);
//    maxColor = mix(maxColor,
//       max(max(max(in5, in6), max(in7, in8)), maxColor), 0.5);
//
//    vec3 preclamping = antialiased;
//    antialiased = clamp(antialiased, minColor, maxColor);
//    
//    modulationFactor = 1.0 / (1.0 / modulationFactor + 1.0);
//    
//    vec3 diff = antialiased - preclamping;
//    float clampAmount = dot(diff, diff);
//
//    modulationFactor += clampAmount * 4.0;
//    modulationFactor = clamp(modulationFactor, 0.05, 0.5);
//    
//    antialiased = decodePalYuv(antialiased);
//        
//    FragColor = vec4(antialiased, modulationFactor);

      vec4 current_colour = texture(u_current_light_buffer, aUV);
      const vec2 unit = vec2(1.0) / vec2(1920.0, 1080.0);
      vec2 velocity       = texture(u_velocity_buffer, aUV).xy / vec2(1920.0, 1080.0);
      if(velocity.x < unit.x)
      {
          velocity.x = 0.0;
      }
      if(velocity.y < unit.y)
      {
          velocity.y = 0.0;
      }
      vec4 history_colour = texture(u_history_light_buffer, aUV - velocity);

      vec3 minColor = vec3(9999.0); 
      vec3 maxColor = vec3(-9999.0);
 
      // Sample a 3x3 neighborhood to create a box in color space
      for(int x = -1; x <= 1; ++x)
      {
          for(int y = -1; y <= 1; ++y)
          {
              vec3 color = texture(u_current_light_buffer,aUV + (vec2(x, y) / vec2(1920, 1080))).xyz; // Sample neighbor
              minColor = min(minColor, color); // Take min and max
              maxColor = max(maxColor, color);
          }
      }
 
      // Clamp previous color to min/max bounding box
      vec4 previousColorClamped = vec4(clamp(history_colour.xyz, minColor, maxColor), 1.0);
 
      FragColor = mix(current_colour, previousColorClamped, 0.9);
}