#include <UniformSpec.h>

#include <glad/glad.h>

#include <Shader.h>

struct UniformTypeInfo {
	UniformSpec::UniformType type;
	int size;
};

UniformTypeInfo GetUniformInfo(GLenum type) {
	const static std::map<GLenum, UniformTypeInfo> dict {
		{ GL_FLOAT, { UniformSpec::UniformType::Float1, 1 * sizeof(GLfloat)} },
		{ GL_FLOAT_VEC2, { UniformSpec::UniformType::Float2, 2 * sizeof(GLfloat)} },
		{ GL_FLOAT_VEC3, { UniformSpec::UniformType::Float3, 3 * sizeof(GLfloat)} },
		{ GL_FLOAT_VEC4, { UniformSpec::UniformType::Float4, 4 * sizeof(GLfloat)} },
		{ GL_UNSIGNED_INT, { UniformSpec::UniformType::Uint1, 1 * sizeof(GLuint)} },
		{ GL_UNSIGNED_INT_VEC2, { UniformSpec::UniformType::Uint2, 2 * sizeof(GLuint)} },
		{ GL_UNSIGNED_INT_VEC3, { UniformSpec::UniformType::Uint3, 3 * sizeof(GLuint)} },
		{ GL_UNSIGNED_INT_VEC4, { UniformSpec::UniformType::Uint4, 4 * sizeof(GLuint)} },
		{ GL_FLOAT_MAT3, { UniformSpec::UniformType::Matrix3x3, 9 * sizeof(GLfloat)} },
		{ GL_FLOAT_MAT4, { UniformSpec::UniformType::Matrix4x4, 16 * sizeof(GLfloat)} },
		{ GL_SAMPLER_2D, { UniformSpec::UniformType::Sampler2D, sizeof(Texture2D*)} },
		{ GL_SAMPLER_CUBE, { UniformSpec::UniformType::Cubemap, sizeof(Cubemap*)} },
		{ GL_IMAGE_2D, { UniformSpec::UniformType::Image2D, sizeof(Texture2D*)} },
	};

	if (dict.contains(type)) {
		return dict.at(type);
	}

	return { UniformSpec::UniformType::Unsupported, 0 };
}

void UniformSpec::CreateFrom(GLuint programHandle) {
	int uniformCount = 0;
	glGetProgramiv(programHandle, GL_ACTIVE_UNIFORMS, &uniformCount);

	this->variables.reserve(uniformCount);

	int bufferSize = 0;
	glGetProgramiv(programHandle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &bufferSize);
	char uniformName[bufferSize ];

	this->variablesBufferLength = 0;
	for (int i = 0; i < uniformCount; i++) {
		int uniformSize = 0;
		GLenum uniformType;
		glGetActiveUniform(programHandle, i, bufferSize, nullptr, &uniformSize, &uniformType, uniformName);

		UniformTypeInfo info = GetUniformInfo(uniformType);

		this->variables.push_back({ info.type, this->variablesBufferLength, uniformName });

		this->variablesBufferLength += info.size;
	}

	int uniformBufferCount = 0;
	glGetProgramInterfaceiv(programHandle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &uniformBufferCount);

	for (int i = 0; i < uniformBufferCount; i++) {
		struct {
			int nameLength;
			int bufferSize;
			int computeBuffer;
		} propValues;

		GLenum bufferProps[] {
			GL_NAME_LENGTH,
			GL_BUFFER_DATA_SIZE,
			GL_REFERENCED_BY_COMPUTE_SHADER
		};

		glGetProgramResourceiv(programHandle, GL_UNIFORM_BLOCK, i, 3, bufferProps, 3, nullptr, (int*) &propValues);

		char nameBuf[propValues.nameLength];

		glGetProgramResourceName(programHandle, GL_UNIFORM_BLOCK, i, propValues.nameLength, nullptr, nameBuf);

		spdlog::info("Uniform buffer {}: name {}, size {}, compute {}", i, std::string(nameBuf), propValues.bufferSize, (bool) propValues.computeBuffer);

		if (propValues.computeBuffer || i >= 2) {
			this->uniformBuffers.push_back({ std::string(nameBuf), i, propValues.bufferSize });
		}
	}

	int storageBufferCount = 0;
	glGetProgramInterfaceiv(programHandle, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &storageBufferCount);

	for (int i = 0; i < storageBufferCount; i++) {
		GLenum nameProp = GL_NAME_LENGTH;
		int nameLength = 0;

		glGetProgramResourceiv(programHandle, GL_SHADER_STORAGE_BLOCK, i, 1, &nameProp, 1, nullptr, &nameLength);

		char nameBuf[nameLength];
		glGetProgramResourceName(programHandle, GL_SHADER_STORAGE_BLOCK, i, nameLength, nullptr, nameBuf);

		spdlog::info("Buffer {}: {}", i, std::string(nameBuf));

		this->storageBuffers.push_back({ std::string(nameBuf) });
	}
}

UniformSpec::UniformSpec() { }

UniformSpec::UniformSpec(const ShaderProgram* program) {
	GLuint handle = program->GetHandle();

	CreateFrom(handle);
}

UniformSpec::UniformSpec(const ComputeShaderProgram* program) {
	GLuint handle = program->GetHandle();

	CreateFrom(handle);
}

unsigned int UniformSpec::GetBufferSize() const {
	return this->variablesBufferLength;
}

unsigned int UniformSpec::VariableCount() const {
	return this->variables.size();
}

unsigned int UniformSpec::UniformBuffersCount() const {
	return this->uniformBuffers.size();
}

unsigned int UniformSpec::StorageBuffersCount() const {
	return this->storageBuffers.size();
}

const UniformSpec::UniformVariableSpec& UniformSpec::VariableAt(int index) const {
	return this->variables.at(index);
}

const UniformSpec::UniformBufferSpec& UniformSpec::UniformBufferAt(int index) const {
	return this->uniformBuffers.at(index);
}

const UniformSpec::ShaderStorageBufferSpec& UniformSpec::StorageBufferAt(int index) const {
	return this->storageBuffers.at(index);
}

const UniformSpec::UniformVariableSpec& UniformSpec::operator[](int index) const {
	return VariableAt(index);
}