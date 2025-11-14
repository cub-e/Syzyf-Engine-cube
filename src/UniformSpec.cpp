#include <UniformSpec.h>

#include <glad/glad.h>
#include <cmath>

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
		{ GL_UNSIGNED_INT_IMAGE_2D, { UniformSpec::UniformType::UImage2D, sizeof(Texture2D*)} },
	};

	if (dict.contains(type)) {
		return dict.at(type);
	}

	return { UniformSpec::UniformType::Unsupported, 0 };
}

bool IsTextureType(UniformSpec::UniformType type) {
	return type == UniformSpec::UniformType::Sampler2D || type == UniformSpec::UniformType::Cubemap || type == UniformSpec::UniformType::Image2D || type == UniformSpec::UniformType::UImage2D;
}

void UniformSpec::CreateFrom(GLuint programHandle) {
	int uniformCount = 0;
	glGetProgramiv(programHandle, GL_ACTIVE_UNIFORMS, &uniformCount);

	this->variables.reserve(uniformCount);

	int bufferSize = 0;
	glGetProgramiv(programHandle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &bufferSize);
	char uniformName[bufferSize];

	this->variablesBufferLength = 0;
	int textureCount = 0;
	for (int i = 0; i < uniformCount; i++) {
		int uniformSize = 0;
		GLenum uniformType;
		glGetActiveUniform(programHandle, i, bufferSize, nullptr, &uniformSize, &uniformType, uniformName);

		UniformTypeInfo info = GetUniformInfo(uniformType);

		GLenum locationProp = GL_LOCATION;
		int uniformLocation = -1;

		if (IsTextureType(info.type)) {
			glGetUniformiv(programHandle, i, &uniformLocation);
			textureCount++;
		}
		
		// glGetProgramResourceiv(programHandle, GL_UNIFORM, i, 1, &locationProp, 1, nullptr, &uniformLocation);

		spdlog::info("Uniform variable {}: name {}, binding {}", i, std::string(uniformName), uniformLocation);

		// if (IsTextureType(info.type) || uniformLocation >= 0) {
			this->variables.push_back({ info.type, this->variablesBufferLength, uniformLocation, uniformName });
	
			this->variablesBufferLength += info.size;
		// }
	}

	// bool textureBindings[textureCount];

	// for (int i = 0; i < this->variables.size(); i++) {
		// textureBindings[i] = true;

		// if (this->variables[i].binding < this->variables.size()) {
			// textureBindings[this->variables[i].binding] = false;
		// }
	// }

	// int textureBindingIndex = 0;

	// for (int i = 0; i < this->variables.size(); i++) {
		// spdlog::info("Variable {} type: {} binding: {}", i, IsTextureType(this->variables[i].type), this->variables[i].binding);
		// if (IsTextureType(this->variables[i].type) && this->variables[i].binding < 0) {
			// spdlog::info("Variable {} needs rebinding!", i);
		// }
	// }

	int uniformBufferCount = 0;
	glGetProgramInterfaceiv(programHandle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &uniformBufferCount);

	for (int i = 0; i < uniformBufferCount; i++) {
		struct {
			int nameLength;
			int bufferSize;
			int computeBuffer;
			int binding;
		} propValues;

		GLenum bufferProps[] {
			GL_NAME_LENGTH,
			GL_BUFFER_DATA_SIZE,
			GL_REFERENCED_BY_COMPUTE_SHADER,
			GL_BUFFER_BINDING
		};

		glGetProgramResourceiv(programHandle, GL_UNIFORM_BLOCK, i, 4, bufferProps, 4, nullptr, (int*) &propValues);

		char nameBuf[propValues.nameLength];

		glGetProgramResourceName(programHandle, GL_UNIFORM_BLOCK, i, propValues.nameLength, nullptr, nameBuf);

		spdlog::info("Uniform buffer {}: name {}, binding {}, size {}, compute {}", i, std::string(nameBuf), propValues.binding, propValues.bufferSize, (bool) propValues.computeBuffer);

		if (propValues.computeBuffer || i >= 2) {
			this->uniformBuffers.push_back({ std::string(nameBuf), propValues.binding, propValues.bufferSize });
		}
	}

	int storageBufferCount = 0;
	glGetProgramInterfaceiv(programHandle, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &storageBufferCount);

	for (int i = 0; i < storageBufferCount; i++) {
		struct {
			int nameLength;
			int bufferVariableCount;
			int binding;
		} propValues;

		GLenum bufferProps[] {
			GL_NAME_LENGTH,
			GL_NUM_ACTIVE_VARIABLES,
			GL_BUFFER_BINDING,
		};

		glGetProgramResourceiv(programHandle, GL_SHADER_STORAGE_BLOCK, i, 3, bufferProps, 3, nullptr, (int*) &propValues);

		char nameBuf[propValues.nameLength];
		glGetProgramResourceName(programHandle, GL_SHADER_STORAGE_BLOCK, i, propValues.nameLength, nullptr, nameBuf);

		spdlog::info("Buffer {}: name {}, binding {}, variable count {}", i, std::string(nameBuf), propValues.binding, propValues.bufferVariableCount);

		this->storageBuffers.push_back({ std::string(nameBuf), 0 });

		GLint bufferVars[propValues.bufferVariableCount];

		bufferProps[0] = GL_ACTIVE_VARIABLES;
		glGetProgramResourceiv(programHandle, GL_SHADER_STORAGE_BLOCK, i, 1, bufferProps, propValues.bufferVariableCount, nullptr, bufferVars);

		struct {
			int nameLength;
			int offset;
			int arraySize;
			int arrayStride;
			int topLevelArrayStride;
		} varPropValues;
		GLenum variableProps[] {
			GL_NAME_LENGTH,
			GL_OFFSET,
			GL_ARRAY_SIZE,
			GL_ARRAY_STRIDE,
			GL_TOP_LEVEL_ARRAY_STRIDE
		};

		for (int varIndex = 0; varIndex < propValues.bufferVariableCount; varIndex++) {
			glGetProgramResourceiv(programHandle, GL_BUFFER_VARIABLE, bufferVars[varIndex], 5, variableProps, 5, nullptr, (int*) &varPropValues);

			char varNameBuf[varPropValues.nameLength];
			
			glGetProgramResourceName(programHandle, GL_BUFFER_VARIABLE, bufferVars[varIndex], varPropValues.nameLength, nullptr, varNameBuf);

			spdlog::info(" variable {}: name {}, index {}, offset {}, arraySize {}, arrayStride {}", 
				varIndex, std::string(varNameBuf), bufferVars[varIndex], varPropValues.offset, varPropValues.arraySize, varPropValues.arraySize == 0 ? varPropValues.arrayStride : varPropValues.topLevelArrayStride
			);
		}
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