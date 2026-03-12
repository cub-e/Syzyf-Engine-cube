#version 460

#include "shared/shared.h"
#include "shared/uniforms.h"

layout (IN_POSITION) in vec3 vPos;
layout (IN_NORMAL) in vec3 vNormal;
layout (IN_TANGENT) in vec4 vTangent;
layout (IN_UV1) in vec2 vUVCoords;
layout (IN_UV2) in vec2 vUVCoords2;
layout (IN_JOINTS) in vec4 vJoints;
layout (IN_WEIGHTS) in vec4 vWeights;

out VS_OUT {
	vec3 worldPos;
	vec3 viewPos;
	vec3 normal;
	vec4 tangent;
	vec2 texcoords;
  vec2 texcoords2;
} vs_out;

//from learnopengl
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 inverseBindMatrices[MAX_BONES];

void main() {

  vec4 totalPosition = vec4(0.0f);
  for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
    int joint = int(vJoints[i]);

    if (joint == -1) {
      continue;
    }
    if (joint >= MAX_BONES) {
      totalPosition = vec4(vPos, 1.0f);
      break;
    }
    vec4 localPosition = inverseBindMatrices[joint] * vec4(vPos, 1.0f);
    totalPosition += localPosition * vWeights[i];
    vec3 localNormal = mat3(inverseBindMatrices[joint]) * vNormal;
  }

	gl_Position = Object_MVPMatrix * totalPosition;

	vs_out.worldPos = (Object_ModelMatrix * vec4(vPos, 1.0)).xyz;
	vs_out.viewPos = (Global_ViewMatrix * (Object_ModelMatrix * vec4(vPos, 1.0))).xyz;
	vs_out.normal = Object_NormalModelMatrix * vNormal;
	vs_out.tangent.xyz = Object_NormalModelMatrix * vTangent.xyz;
  vs_out.tangent.w = vTangent.w;
	vs_out.texcoords = vUVCoords;
  vs_out.texcoords2 = vUVCoords2;
}
