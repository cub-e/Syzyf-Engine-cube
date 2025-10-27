#version 460

layout (IN_POSITION) in vec3 vPos;

out vec3 pTexCoords;

void main() {
	mat4 view = mat4(mat3(Global_ViewMatrix));

	pTexCoords = vPos;

	vec4 pos = Global_ProjectionMatrix * view * vec4(vPos, 1.0);

	gl_Position = pos.xyww;
}