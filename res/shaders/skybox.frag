#version 460

in vec3 pTexCoords;

out vec4 FragColor;

uniform samplerCube skyboxTexture;

void main() {
    FragColor = texture(skyboxTexture, pTexCoords);
}