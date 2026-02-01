#version 460

in VARYINGS {
	vec3 normal;
	vec3 worldPos;
  vec3 viewPos;
  float height;
	flat uint instanceID;
} ps_in;

#include "shared/shared.h"
#include "shared/uniforms.h"

#define SHADING_PBR

#include "shared/shading.h"
#include "shared/light.h"

uniform samplerCube Builtin_EnvIrradianceMap;
uniform samplerCube Builtin_EnvPrefilterMap;
uniform sampler2D Builtin_BRDFConvolutionMap;

out vec4 fragColor;

void main() {
  Material mat;
  mat.albedo = vec3(1.0, 0.9, 0.087);
  mat.metallic = 0.0;
  mat.roughness = 0.3;

  float ao = pow(ps_in.height * 0.5, 0.7);
  vec3 aoColor = vec3(0.365, 0.0, 0.729);
  mat.albedo = mix(mat.albedo, aoColor, ao);

	vec3 V = normalize(Global_CameraWorldPos - ps_in.worldPos);
  vec3 N = normalize(ps_in.normal);

  if (!gl_FrontFacing) {
    N = -N;
  }

	vec3 R = reflect(-V, N);

	vec3 F0 = vec3(0.04); 
  F0 = mix(F0, mat.albedo, mat.metallic);

  fragColor = vec4(
    shade(
      mat,
      ps_in.worldPos,
      N,
      vec3(0, 0, 0)
    ),
    1.0
  );

	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, mat.roughness);

	vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - mat.metallic;

	vec3 irradiance = texture(Builtin_EnvIrradianceMap, N).rgb;
  vec3 diffuse = irradiance * mat.albedo.rgb;

	const float MAX_REFLECTION_LOD = 7.0;
    vec3 prefilteredColor = textureLod(Builtin_EnvPrefilterMap, R, mat.roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf = texture(Builtin_BRDFConvolutionMap, vec2(max(dot(N, V), 0.0), mat.roughness)).rg;
	
	if (isnan(brdf.x)) {
		brdf = vec2(0, 0);
	}

  vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

	vec3 ambient = (kD * diffuse + specular);

	fragColor.xyz += ambient;
}
