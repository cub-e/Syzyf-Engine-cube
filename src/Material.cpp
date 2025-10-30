#include <Material.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

void Material::Bind() {
	glUseProgram(this->shader->GetHandle());

	auto& uniforms = this->shader->GetUniforms();

	int textureIndex = 0;

	for (unsigned int i = 0; i < uniforms.VariableCount(); i++) {
		int offset = uniforms[i].offset;
		switch (uniforms[i].type) {
		case UniformType::Float1:
			glUniform1f(i, GetValue<float>(i));
			break;
		case UniformType::Float2:
			glUniform2fv(i, 1, &GetValue<glm::vec2>(i)[0]);
			break;
		case UniformType::Float3:
			glUniform3fv(i, 1, &GetValue<glm::vec3>(i)[0]);
			break;
		case UniformType::Float4:
			glUniform4fv(i, 1, &GetValue<glm::vec4>(i)[0]);
			break;
		case UniformType::Uint1:
			glUniform1ui(i, GetValue<unsigned int>(i));
			break;
		case UniformType::Uint2:
			glUniform2uiv(i, 1, &GetValue<glm::uvec2>(i)[0]);
			break;
		case UniformType::Uint3:
			glUniform3uiv(i, 1, &GetValue<glm::uvec3>(i)[0]);
			break;
		case UniformType::Matrix3x3:
			glUniformMatrix3fv(i, 1, false, &GetValue<glm::mat3>(i)[0][0]);
			break;
		case UniformType::Matrix4x4:
			glUniformMatrix4fv(i, 1, false, &GetValue<glm::mat4>(i)[0][0]);
			break;
		case UniformType::Sampler2D:
		{
			Texture2D* imageTex = GetValue<Texture2D>(i);

			GLuint imageTexHandle = 0;

			if (imageTex) {
				if (imageTex->IsDirty()) {
					imageTex->Update();
				}
				
				imageTexHandle = imageTex->GetHandle();
			}
			
			glActiveTexture(GL_TEXTURE0 + textureIndex);
			glBindTexture(GL_TEXTURE_2D, imageTexHandle);
			glUniform1i(i, textureIndex);

			textureIndex++;

			break;
		}
		case UniformType::Cubemap:
		{
			Cubemap* cubeTex = GetValue<Cubemap>(i);

			GLuint cubeTexHandle = 0;

			if (cubeTex) {
				if (cubeTex->IsDirty()) {
					cubeTex->Update();
				}
				
				cubeTexHandle = cubeTex->GetHandle();
			}
			
			glActiveTexture(GL_TEXTURE0 + textureIndex);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexHandle);
			glUniform1i(i, textureIndex);

			textureIndex++;

			break;
		}
		}
	}
}

Material::Material(ShaderProgram* shader):
shader(shader) {
	unsigned int uniformBufferSize = shader->GetUniforms().GetBufferSize();
	
	this->dataBuffer = (void*) new char[uniformBufferSize];
	memset(this->dataBuffer, 0, uniformBufferSize);
}

const ShaderProgram* Material::GetShader() const {
	return this->shader;
}
