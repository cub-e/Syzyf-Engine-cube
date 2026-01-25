#version 460

in VS_OUT {
	vec3 worldPos;
	vec3 viewPos;
	vec3 normal;
	vec3 tangent;
	vec2 texcoords;
} ps_in;

#include "shared/shared.h"
#include "shared/uniforms.h"

uniform samplerCube cubemap;

out vec4 fragColor;

void main() {
	vec3 cameraDir = normalize(ps_in.worldPos - Global_CameraWorldPos);
	vec3 reflectedDir = reflect(cameraDir, normalize(ps_in.normal));

	fragColor = vec4(texture(cubemap, reflectedDir).rgb, 1.0);
}
