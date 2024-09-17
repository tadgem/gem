#version 450

out vec4 FragColor;

layout (location = 0) in vec2 aUV;

uniform sampler2D u_current_light_buffer;
uniform sampler2D u_history_light_buffer;

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

void main()
{
   vec4 current_colour = texture(u_current_light_buffer, aUV);
   vec4 history_colour = texture(u_history_light_buffer, aUV);

   float modulationFactor = min(history_colour.w, 0.5);

   vec2 off = 1.0 / vec2(1280.0, 720.0);
   vec3 antialiased = history_colour.xyz;
   antialiased = mix(antialiased * antialiased, current_colour.xyz * current_colour.xyz, modulationFactor);
   antialiased = sqrt(antialiased);

    vec3 in1 = texture(u_current_light_buffer, aUV + vec2(+off.x, 0.0)).xyz;
    vec3 in2 = texture(u_current_light_buffer, aUV + vec2(-off.x, 0.0)).xyz;
    vec3 in3 = texture(u_current_light_buffer, aUV + vec2(0.0, +off.y)).xyz;
    vec3 in4 = texture(u_current_light_buffer, aUV + vec2(0.0, -off.y)).xyz;
    vec3 in5 = texture(u_current_light_buffer, aUV + vec2(+off.x, +off.y)).xyz;
    vec3 in6 = texture(u_current_light_buffer, aUV + vec2(-off.x, +off.y)).xyz;
    vec3 in7 = texture(u_current_light_buffer, aUV + vec2(+off.x, -off.y)).xyz;
    vec3 in8 = texture(u_current_light_buffer, aUV + vec2(-off.x, -off.y)).xyz;

    antialiased = encodePalYuv(antialiased);
    vec3 in0 = encodePalYuv(current_colour.xyz);
    in1 = encodePalYuv(in1);
    in2 = encodePalYuv(in2);
    in3 = encodePalYuv(in3);
    in4 = encodePalYuv(in4);
    in5 = encodePalYuv(in5);
    in6 = encodePalYuv(in6);
    in7 = encodePalYuv(in7);
    in8 = encodePalYuv(in8);

    vec3 minColor = min(min(min(in0, in1), min(in2, in3)), in4);
    vec3 maxColor = max(max(max(in0, in1), max(in2, in3)), in4);
    minColor = mix(minColor,
       min(min(min(in5, in6), min(in7, in8)), minColor), 0.5);
    maxColor = mix(maxColor,
       max(max(max(in5, in6), max(in7, in8)), maxColor), 0.5);

    vec3 preclamping = antialiased;
    antialiased = clamp(antialiased, minColor, maxColor);
    
    modulationFactor = 1.0 / (1.0 / modulationFactor + 1.0);
    
    vec3 diff = antialiased - preclamping;
    float clampAmount = dot(diff, diff);

    modulationFactor += clampAmount * 4.0;
    modulationFactor = clamp(modulationFactor, 0.05, 0.5);
    
    antialiased = decodePalYuv(antialiased);
        
    FragColor = vec4(antialiased, modulationFactor);
}