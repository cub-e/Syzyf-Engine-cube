layout (IN_POSITION) in vec3 vPos;
layout (IN_NORMAL) in vec3 vNormal;
layout (IN_UV1) in vec2 vUVCoords;

out vec3 pNormal;
out vec2 pUVCoords;

layout (std430, binding = 0) buffer in_Offsets {
	float offsets[];
};

void main() {
	vec3 pos = vPos.xyz;
	pos.x = -1.0 + ((pos.x + 1 + (offsets[gl_InstanceID] * 2)) / 10);
	gl_Position = vec4(pos.xyz, 1.0);

	pNormal = normalize((Object_MVPMatrix * vec4(vNormal.xyz, 0.0)).xyz);
	pUVCoords = vUVCoords;
}