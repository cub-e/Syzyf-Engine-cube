#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include <UniformSpec.h>
#include <Shader.h>
#include <Texture.h>

template <ShaderLike T_ShaderProg>
class ShaderVariableStorage {
	friend class SceneGraphics;
private:
	struct BufferPair {
		void* bufferData;
		GLuint bufferHandle;
	};

	void* dataBuffer;
	BufferPair* uniformBuffers;
	BufferPair* storageBuffers;
	T_ShaderProg* shader;
	bool dirty;
public:
	ShaderVariableStorage(T_ShaderProg* shader);
	
	void Bind();

	template<Blittable T>
	T GetValue(const std::string& uniformName) const;
	template<Blittable T>
	T GetValue(unsigned int uniformIndex) const;

	template<TextureClass T>
	T* GetValue(const std::string& uniformName) const;
	template<TextureClass T>
	T* GetValue(unsigned int uniformIndex) const;
	
	template<Blittable T>
	void SetValue(const std::string& uniformName, const T& value);
	template<Blittable T>
	void SetValue(unsigned int uniformIndex, const T& value);

	template<TextureClass T>
	void SetValue(const std::string& uniformName, T* value);
	template<TextureClass T>
	void SetValue(unsigned int uniformIndex, T* value);

	template<typename T_BufferRep>
	T_BufferRep GetUniformBuffer(const std::string& uniformBufferName);
	template<typename T_BufferRep>
	T_BufferRep GetUniformBuffer(int uniformBufferIndex);

	template<typename T_BufferRep>
	void SetUniformBuffer(const std::string& uniformBufferName, const T_BufferRep& data);
	template<typename T_BufferRep>
	void SetUniformBuffer(int uniformBufferIndex, const T_BufferRep& data);

	GLuint GetStorageBuffer(const std::string& storageBufferName);
	GLuint GetStorageBuffer(int storageBufferIndex);

	void BindStorageBuffer(const std::string& storageBufferName, GLuint bufferHandle);
	void BindStorageBuffer(int storageBufferIndex, GLuint bufferHandle);

	const T_ShaderProg* GetShader() const;
};

typedef ShaderVariableStorage<ShaderProgram> Material;
typedef ShaderVariableStorage<ComputeShaderProgram> ComputeDispatchData;

template<Blittable T>
static inline bool IsUniformOfRightType(UniformSpec::UniformSpec::UniformType type) {
	return false;
}

template<TextureClass T>
static inline bool IsUniformOfRightType(UniformSpec::UniformSpec::UniformType type) {
	return false;
}

template <ShaderLike T_ShaderProg>
void ShaderVariableStorage<T_ShaderProg>::Bind() {
	glUseProgram(this->shader->GetHandle());

	const UniformSpec& uniforms = this->shader->GetUniforms();

	int samplerIndex = 0;
	int imageIndex = 0;

	for (unsigned int i = 0; i < uniforms.VariableCount(); i++) {
		int offset = uniforms[i].offset;

		switch (uniforms[i].type) {
		case UniformSpec::UniformType::Float1:
			glUniform1f(i, GetValue<float>(i));
			break;
		case UniformSpec::UniformType::Float2:
			glUniform2fv(i, 1, &GetValue<glm::vec2>(i)[0]);
			break;
		case UniformSpec::UniformType::Float3:
			glUniform3fv(i, 1, &GetValue<glm::vec3>(i)[0]);
			break;
		case UniformSpec::UniformType::Float4:
			glUniform4fv(i, 1, &GetValue<glm::vec4>(i)[0]);
			break;
		case UniformSpec::UniformType::Uint1:
			glUniform1ui(i, GetValue<unsigned int>(i));
			break;
		case UniformSpec::UniformType::Uint2:
			glUniform2uiv(i, 1, &GetValue<glm::uvec2>(i)[0]);
			break;
		case UniformSpec::UniformType::Uint3:
			glUniform3uiv(i, 1, &GetValue<glm::uvec3>(i)[0]);
			break;
		case UniformSpec::UniformType::Matrix3x3:
			glUniformMatrix3fv(i, 1, false, &GetValue<glm::mat3>(i)[0][0]);
			break;
		case UniformSpec::UniformType::Matrix4x4:
			glUniformMatrix4fv(i, 1, false, &GetValue<glm::mat4>(i)[0][0]);
			break;
		case UniformSpec::UniformType::Sampler2D:
		{
			Texture2D* imageTex = GetValue<Texture2D>(i);

			GLuint imageTexHandle = 0;

			if (imageTex) {
				if (imageTex->IsDirty()) {
					imageTex->Update();
				}
				
				imageTexHandle = imageTex->GetHandle();
			}
			
			glActiveTexture(GL_TEXTURE0 + samplerIndex);
			glBindTexture(GL_TEXTURE_2D, imageTexHandle);
			glUniform1i(i, samplerIndex);

			samplerIndex++;

			break;
		}
		case UniformSpec::UniformType::Cubemap:
		{
			Cubemap* cubeTex = GetValue<Cubemap>(i);

			GLuint cubeTexHandle = 0;

			if (cubeTex) {
				if (cubeTex->IsDirty()) {
					cubeTex->Update();
				}
				
				cubeTexHandle = cubeTex->GetHandle();
			}
			
			glActiveTexture(GL_TEXTURE0 + samplerIndex);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexHandle);
			glUniform1i(i, samplerIndex);

			samplerIndex++;

			break;
		}
		case UniformSpec::UniformType::Image2D:
		{
			Texture2D* imageTex = GetValue<Texture2D>(i);

			GLuint imageTexHandle = 0;

			if (imageTex) {
				if (imageTex->IsDirty()) {
					imageTex->Update();
				}
				
				imageTexHandle = imageTex->GetHandle();
			}
			
			glBindImageTexture(imageIndex, imageTexHandle, 0, false, 0, GL_READ_WRITE, Texture::TextureFormatToGL(imageTex->GetFormat()));

			imageIndex++;

			break;
		}
		}
	}

	for (unsigned int i = 0; i < uniforms.UniformBuffersCount(); i++) {
		auto uniformBufferSpec = uniforms.UniformBufferAt(i);
		auto uniformBufferData = uniformBuffers[i];

		glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferData.bufferHandle);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, uniformBufferSpec.size, uniformBufferData.bufferData);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferBase(GL_UNIFORM_BUFFER, uniformBufferSpec.binding, uniformBufferData.bufferHandle);
	}

	for (unsigned int i = 0; i < uniforms.StorageBuffersCount(); i++) {
		auto storageBufferData = storageBuffers[i];

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, storageBufferData.bufferHandle);
	}
}

template <ShaderLike T_ShaderProg>
ShaderVariableStorage<T_ShaderProg>::ShaderVariableStorage(T_ShaderProg* shader):
shader(shader) {
	unsigned int variableBufferSize = shader->GetUniforms().GetBufferSize();
	
	this->dataBuffer = (void*) new char[variableBufferSize];
	memset(this->dataBuffer, 0, variableBufferSize);

	int uniformBuffersCount = shader->GetUniforms().UniformBuffersCount();

	this->uniformBuffers = new BufferPair[uniformBuffersCount];
	GLuint uniformBufferHandles[uniformBuffersCount];

	glGenBuffers(uniformBuffersCount, uniformBufferHandles);

	for (int i = 0; i < uniformBuffersCount; i++) {
		GLuint bufferHandle = uniformBufferHandles[i];
		unsigned int bufferSize = shader->GetUniforms().UniformBufferAt(i).size;

		this->uniformBuffers[i].bufferData = (void*) new std::byte[bufferSize];
		memset(this->uniformBuffers[i].bufferData, 0, bufferSize);

		this->uniformBuffers[i].bufferHandle = bufferHandle;

		glBindBuffer(GL_UNIFORM_BUFFER, bufferHandle);
		glBufferData(GL_UNIFORM_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	int storageBuffersCount = shader->GetUniforms().StorageBuffersCount();

	this->storageBuffers = new BufferPair[storageBuffersCount];
	memset(this->storageBuffers, 0, storageBuffersCount * sizeof(BufferPair));
}

template <ShaderLike T_ShaderProg>
const T_ShaderProg* ShaderVariableStorage<T_ShaderProg>::GetShader() const {
	return this->shader;
}

template <ShaderLike T_ShaderProg>
template<Blittable T>
T ShaderVariableStorage<T_ShaderProg>::GetValue(const std::string& uniformName) const {
	return GetValue<T>(glGetUniformLocation(this->shader->GetHandle(), uniformName.c_str()));
}

template <ShaderLike T_ShaderProg>
template<TextureClass T>
T* ShaderVariableStorage<T_ShaderProg>::GetValue(const std::string& uniformName) const {
	return GetValue<T>(glGetUniformLocation(this->shader->GetHandle(), uniformName.c_str()));
}

template <ShaderLike T_ShaderProg>
template<Blittable T>
T ShaderVariableStorage<T_ShaderProg>::GetValue(unsigned int uniformIndex) const {
	if (uniformIndex < 0 || uniformIndex >= this->shader->GetUniforms().VariableCount()) {
		return T{};
	}

	if (!IsUniformOfRightType<T>(this->shader->GetUniforms()[uniformIndex].type)) {
		return T{};
	}

	return *((T*) ((char*) this->dataBuffer + this->shader->GetUniforms()[uniformIndex].offset));
}

template <ShaderLike T_ShaderProg>
template<TextureClass T>
T* ShaderVariableStorage<T_ShaderProg>::GetValue(unsigned int uniformIndex) const {
	if (uniformIndex < 0 || uniformIndex >= this->shader->GetUniforms().VariableCount()) {
		return nullptr;
	}

	if (!IsUniformOfRightType<T>(this->shader->GetUniforms()[uniformIndex].type)) {
		return nullptr;
	}

	return *((T**) ((char*) this->dataBuffer + this->shader->GetUniforms()[uniformIndex].offset));
}

template <ShaderLike T_ShaderProg>
template<Blittable T>
void ShaderVariableStorage<T_ShaderProg>::SetValue(const std::string& uniformName, const T& value) {
	SetValue<T>(glGetUniformLocation(this->shader->GetHandle(), uniformName.c_str()), value);
}

template <ShaderLike T_ShaderProg>
template<TextureClass T>
void ShaderVariableStorage<T_ShaderProg>::SetValue(const std::string& uniformName, T* value) {
	SetValue<T>(glGetUniformLocation(this->shader->GetHandle(), uniformName.c_str()), value);
}

template <ShaderLike T_ShaderProg>
template<Blittable T>
void ShaderVariableStorage<T_ShaderProg>::SetValue(unsigned int uniformIndex, const T& value) {
	if (uniformIndex < 0 || uniformIndex >= this->shader->GetUniforms().VariableCount()) {
		return;
	}

	if (!IsUniformOfRightType<T>(this->shader->GetUniforms()[uniformIndex].type)) {
		return;
	}

	*((T*) ((char*) this->dataBuffer + this->shader->GetUniforms()[uniformIndex].offset)) = value;
}

template <ShaderLike T_ShaderProg>
template<TextureClass T>
void ShaderVariableStorage<T_ShaderProg>::SetValue(unsigned int uniformIndex, T* value) {
	if (uniformIndex < 0 || uniformIndex >= this->shader->GetUniforms().VariableCount()) {
		return;
	}

	if (!IsUniformOfRightType<T>(this->shader->GetUniforms()[uniformIndex].type)) {
		return;
	}

	*((T**) ((char*) this->dataBuffer + this->shader->GetUniforms()[uniformIndex].offset)) = value;
}

template <ShaderLike T_ShaderProg>
template<typename T_BufferRep>
T_BufferRep ShaderVariableStorage<T_ShaderProg>::GetUniformBuffer(const std::string& uniformBufferName) {
	UniformSpec spec = this->shader->GetUniforms();

	for (int i = 0; i < spec.UniformBuffersCount(); i++) {
		if (spec.UniformBufferAt(i).name == uniformBufferName) {
			return GetUniformBuffer(spec.UniformBufferAt(i).binding);
		}
	}

	return T_BufferRep{};
}

template <ShaderLike T_ShaderProg>
template<typename T_BufferRep>
T_BufferRep ShaderVariableStorage<T_ShaderProg>::GetUniformBuffer(int uniformBufferIndex) {
	T_BufferRep result;
	
	if (uniformBufferIndex < 0 || uniformBufferIndex >= this->shader->GetUniforms().UniformBuffersCount()) {
		void* storageBuffer = this->uniformBuffers[uniformBufferIndex].bufferData;
		UniformSpec::UniformBufferSpec bufferSpec = this->shader->GetUniforms().UniformBufferAt(uniformBufferIndex);
	
		memcpy(&result, storageBuffer, sizeof(T_ShaderProg) < bufferSpec.size ? sizeof(T_ShaderProg) : bufferSpec.size);
	}

	return result;
}

template <ShaderLike T_ShaderProg>
template<typename T_BufferRep>
void ShaderVariableStorage<T_ShaderProg>::SetUniformBuffer(const std::string& uniformBufferName, const T_BufferRep& data) {
	UniformSpec spec = this->shader->GetUniforms();

	for (int i = 0; i < spec.UniformBuffersCount(); i++) {
		if (spec.UniformBufferAt(i).name == uniformBufferName) {
			SetUniformBuffer(spec.UniformBufferAt(i).binding, data);

			return;
		}
	}
}

template <ShaderLike T_ShaderProg>
template<typename T_BufferRep>
void ShaderVariableStorage<T_ShaderProg>::SetUniformBuffer(int uniformBufferIndex, const T_BufferRep& data) {
	if (uniformBufferIndex < 0 || uniformBufferIndex >= this->shader->GetUniforms().UniformBuffersCount()) {
		return;
	}

	void* storageBuffer = this->uniformBuffers[uniformBufferIndex].bufferData;
	UniformSpec::UniformBufferSpec bufferSpec = this->shader->GetUniforms().UniformBufferAt(uniformBufferIndex);

	memcpy(storageBuffer, &data, sizeof(T_ShaderProg) < bufferSpec.size ? sizeof(T_ShaderProg) : bufferSpec.size);
}

template <ShaderLike T_ShaderProg>
GLuint ShaderVariableStorage<T_ShaderProg>::GetStorageBuffer(const std::string& storageBufferName) {
	UniformSpec spec = this->shader->GetUniforms();

	for (int i = 0; i < spec.StorageBuffersCount(); i++) {
		if (spec.StorageBufferAt(i).name == storageBufferName) {
			return GetStorageBuffer(i);
		}
	}

	return 0;
}

template <ShaderLike T_ShaderProg>
GLuint ShaderVariableStorage<T_ShaderProg>::GetStorageBuffer(int storageBufferIndex) {
	if (storageBufferIndex < 0 || storageBufferIndex >= this->shader->GetUniforms().StorageBufferCount()) {
		return 0;
	}

	return this->storageBuffers[storageBufferIndex].bufferHandle;
}

template <ShaderLike T_ShaderProg>
void ShaderVariableStorage<T_ShaderProg>::BindStorageBuffer(const std::string& storageBufferName, GLuint bufferHandle) {
	UniformSpec spec = this->shader->GetUniforms();

	for (int i = 0; i < spec.StorageBuffersCount(); i++) {
		if (spec.StorageBufferAt(i).name == storageBufferName) {
			BindStorageBuffer(i, bufferHandle);

			return;
		}
	}
}

template <ShaderLike T_ShaderProg>
void ShaderVariableStorage<T_ShaderProg>::BindStorageBuffer(int storageBufferIndex, GLuint bufferHandle) {
	if (storageBufferIndex < 0 || storageBufferIndex >= this->shader->GetUniforms().StorageBuffersCount()) {
		return;
	}

	this->storageBuffers[storageBufferIndex].bufferHandle = bufferHandle;
}

template<> inline bool IsUniformOfRightType<float>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Float1;
}
template<> inline bool IsUniformOfRightType<glm::vec2>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Float2;
}
template<> inline bool IsUniformOfRightType<glm::vec3>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Float3;
}
template<> inline bool IsUniformOfRightType<glm::vec4>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Float4;
}
template<> inline bool IsUniformOfRightType<unsigned int>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Uint1;
}
template<> inline bool IsUniformOfRightType<glm::uvec2>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Uint2;
}
template<> inline bool IsUniformOfRightType<glm::uvec3>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Uint3;
}
template<> inline bool IsUniformOfRightType<glm::uvec4>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Uint4;
}
template<> inline bool IsUniformOfRightType<glm::mat3>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Matrix3x3;
}
template<> inline bool IsUniformOfRightType<glm::mat4>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Matrix4x4;
}
template<> inline bool IsUniformOfRightType<Texture2D>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Sampler2D || type == UniformSpec::UniformType::Image2D;
}
template<> inline bool IsUniformOfRightType<Cubemap>(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Cubemap;
}