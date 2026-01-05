#version 460

in vec3 pNormal;

out vec4 fragColor;

void main() {
	fragColor = vec4(1.0, 1.0, 1.0, 1.0);

	float shadow = clamp(0.0, 1.0, dot(normalize(pNormal), vec3(0.0, 0.0, -1.0)) * 3.6);

	fragColor *= mix(0.15, 1.01, pow(shadow - 1.0, 3.0) + 1.0);

	gl_FragDepth = 0.9999;
}