#version 460

in vec3 pNormal;
in vec2 pUVCoords;

uniform sampler2D colorTex;
uniform vec3 uColor;

out vec4 fragColor;

void main() {
	fragColor = texture(colorTex, pUVCoords) * vec4(uColor, 1.0);
}