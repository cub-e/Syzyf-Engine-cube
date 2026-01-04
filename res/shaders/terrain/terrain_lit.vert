#version 460

#include "shared/shared.h"
#include "shared/uniforms.h"

layout (IN_POSITION) in vec3 vPos;
layout (IN_NORMAL) in vec3 vNormal;
layout (IN_TANGENT) in vec3 vTangent;
layout (IN_UV1) in vec2 vUVCoordsTexture;
layout (IN_UV2) in vec2 vUVCoordsHeightmap;

out VS_OUT {
	vec3 worldPos;
	vec3 viewPos;
	vec3 normal;
	vec3 tangent;
	vec2 texcoords;
	vec2 heightmapCoords;
} vs_out;

void main() {
	gl_Position = vec4(vPos, 1.0);

	vs_out.worldPos = vPos;
	vs_out.viewPos = vPos;
	vs_out.normal = vNormal;
	vs_out.tangent = vTangent;
	vs_out.texcoords = vUVCoordsTexture;
	vs_out.heightmapCoords = vUVCoordsHeightmap;
}