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

struct DirLight
{
	vec3	direction;
	vec3	colour;
    mat4    light_space_matrix;
    float   intensity;
};

struct PointLight
{
	vec3	position;
	vec3	colour;
	float	radius;
    float   intensity;
};

const int NUM_POINT_LIGHTS = 16;
const float PI = 3.14159265359;
const float SHADOW_AMBIENT = 0.1;

uniform sampler2D   u_diffuse_map;
uniform sampler2D   u_position_map;
uniform sampler2D   u_normal_map;
uniform sampler2D   u_pbr_map; // x = metallic, y = roughness, z = AO
uniform sampler2D   u_dir_light_shadow_map; // x = metallic, y = roughness, z = AO
uniform vec3        u_dir_light_pos;
uniform vec3        u_cam_pos;
uniform DirLight    u_dir_light;
uniform PointLight  u_point_lights[NUM_POINT_LIGHTS];

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float blinnPhongAttenuation(float range, float dist)
{
    float l = 4.5 / range;
    float q = 75.0f / range;

    return 1.0 / (1.0 + l * dist + q * (dist * dist));
}

vec3 handle_dir_light(vec3 normal, float roughness, float metallic, vec3 albedo, vec3 V, vec3 F0, float shadow)
{
    vec3 lightDir = normalize(-u_dir_light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    float s = pow(max(dot(normal, lightDir), 0.0), roughness);
    vec3 H = normalize(V + lightDir);
    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, V, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = NdotL * albedo;
    // add to outgoing radiance Lo
    return ((kD * max((1.0 - shadow), SHADOW_AMBIENT)) * (diffuse / PI + specular)) * (u_dir_light.colour * u_dir_light.intensity) * NdotL;
}

float ShadowCalculation(vec3 normal, vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 
    float closestDepth = texture(u_dir_light_shadow_map, projCoords.xy).r; 
    float currentDepth = projCoords.z;  
    float bias = max(0.05 * (1.0 - dot(normal, u_dir_light.direction)), 0.005);  
    float shadow = (currentDepth) > closestDepth  ? 1.0 : 0.0;  
    return shadow;
}

float ShadowCalculationV2(vec3 f_normal, vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(u_dir_light_shadow_map, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(f_normal);
    vec3 frag_pos = texture(u_position_map, projCoords.xy).xyz;
    vec3 lightDir = normalize(u_dir_light_pos - frag_pos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_dir_light_shadow_map, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_dir_light_shadow_map, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}


void main()
{
   vec3 albedo = texture(u_diffuse_map, aUV).xyz;
   vec3 WorldPos = texture(u_position_map, aUV).xyz;
   vec3 N = texture(u_normal_map, aUV).xyz;
   vec3 V = normalize(u_cam_pos - WorldPos);
   vec3 pbr = texture(u_pbr_map, aUV).xyz;

   vec4 frag_pos_light_space = u_dir_light.light_space_matrix * vec4(WorldPos, 1.0);

   float metallic = pbr.x;
   float roughness = pbr.y;
   float ao = pbr.z;

   vec3 F0 = vec3(0.04);
   F0 = mix(F0, albedo, metallic);

   // reflectance equation
   vec3 Lo = vec3(0.0);

   float shadow = ShadowCalculationV2(N, frag_pos_light_space);

   Lo += handle_dir_light(N, roughness, metallic, albedo, V, F0, shadow);

   for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
   {
       vec3 LightDir = u_point_lights[i].position - WorldPos;
       // calculate per-light radiance
       vec3 L = normalize(LightDir);
       vec3 H = normalize(V + L);
       //vec3 H = normalize(L);

       float distance = length(LightDir);
       float attenuation = blinnPhongAttenuation(u_point_lights[i].radius, distance);
       attenuation *= attenuation;
       vec3 radiance = u_point_lights[i].colour;

       // Cook-Torrance BRDF
       float NDF = DistributionGGX(N, H, roughness);
       float G = GeometrySmith(N, V, L, roughness);
       vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

       vec3 numerator = NDF * G * F;
       float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
       vec3 specular = numerator / denominator;

       // kS is equal to Fresnel
       vec3 kS = F;
       // for energy conservation, the diffuse and specular light can't
       // be above 1.0 (unless the surface emits light); to preserve this
       // relationship the diffuse component (kD) should equal 1.0 - kS.
       vec3 kD = vec3(1.0) - kS;
       // multiply kD by the inverse metalness such that only non-metals 
       // have diffuse lighting, or a linear blend if partly metal (pure metals
       // have no diffuse light).
       kD *= 1.0 - metallic;

       // scale light by NdotL
       float NdotL = max(dot(N, L), 0.0);
       vec3 diffuse = NdotL * albedo;
       // add to outgoing radiance Lo
       Lo += (kD * attenuation * (diffuse / PI + specular)) * (radiance * u_point_lights[i].intensity)* NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
   }

   // ambient lighting (note that the next IBL tutorial will replace 
   // this ambient lighting with environment lighting).
   vec3 ambient = vec3(0.03) * albedo * ao;

   vec3 color = ambient + Lo;

   // HDR tonemapping
   color = color / (color + vec3(1.0));
   // gamma correct
   color = pow(color, vec3(1.0 / 2.2));

   FragColor = vec4(color, 1.0);

}