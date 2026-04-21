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

uniform vec3 warmColor;
uniform vec3 coldColor;
uniform float warmStrength;
uniform float coldStrength;
uniform vec3 baseColor;

uniform vec3 lightDir;

out vec4 fragColor;

void main() {
    vec3 N = normalize(ps_in.normal);
    
    vec3 L = normalize(-lightDir); 
    
    float NdotL = dot(N, L);
    float intensity = (1.0 + NdotL) * 0.5;

    vec3 cool = coldColor + (baseColor * coldStrength);
    vec3 warm = warmColor + (baseColor * warmStrength);

    vec3 finalColor = mix(cool, warm, intensity);

    fragColor = vec4(finalColor, 1.0);
}
