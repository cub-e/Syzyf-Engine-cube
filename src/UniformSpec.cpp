#include <UniformSpec.h>

#include <glad/glad.h>

#include <Shader.h>

unsigned int UniformTypeSizes[] {
	1 * sizeof(GLfloat),
	2 * sizeof(GLfloat),
	3 * sizeof(GLfloat),
	4 * sizeof(GLfloat),
	1 * sizeof(GLuint),
	2 * sizeof(GLuint),
	3 * sizeof(GLuint),
	4 * sizeof(GLuint),
	9 * sizeof(GLfloat),
	16 * sizeof(GLfloat),
	sizeof(Texture2D*),
	sizeof(Cubemap*),
	0
};

bool IsUniformTypeSupported(GLenum type) {
	return (
		type == GL_UNSIGNED_INT
		||
		type == GL_FLOAT
		||
		(type >= GL_FLOAT_VEC2 && type <= GL_FLOAT_VEC4)
		||
		(type >= GL_UNSIGNED_INT_VEC2 && type <= GL_UNSIGNED_INT_VEC4)
		||
		type == GL_FLOAT_MAT3
		||
		type == GL_FLOAT_MAT4
		||
		type == GL_SAMPLER_2D
		||
		type == GL_SAMPLER_CUBE
	);
}

UniformType GLEnumToUniformType(GLenum type) {
	const static std::map<GLenum, UniformType> dict {
		{ GL_FLOAT, UniformType::Float1 },
		{ GL_FLOAT_VEC2, UniformType::Float2 },
		{ GL_FLOAT_VEC3, UniformType::Float3 },
		{ GL_FLOAT_VEC4, UniformType::Float4 },
		{ GL_UNSIGNED_INT, UniformType::Uint1 },
		{ GL_UNSIGNED_INT_VEC2, UniformType::Uint2 },
		{ GL_UNSIGNED_INT_VEC3, UniformType::Uint3 },
		{ GL_UNSIGNED_INT_VEC4, UniformType::Uint4 },
		{ GL_FLOAT_MAT3, UniformType::Matrix3x3 },
		{ GL_FLOAT_MAT4, UniformType::Matrix4x4 },
		{ GL_SAMPLER_2D, UniformType::Sampler2D },
		{ GL_SAMPLER_CUBE, UniformType::Cubemap },
	};

	return dict.at(type);
}

UniformVariable::UniformVariable(UniformType type, std::string name):
type(type),
name(name),
offset(0) { }

void UniformSpec::CreateFrom(GLuint programHandle) {
	int uniformCount = 0;
	glGetProgramiv(programHandle, GL_ACTIVE_UNIFORMS, &uniformCount);

	this->variables.reserve(uniformCount);

	int bufferSize = 0;
	glGetProgramiv(programHandle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &bufferSize);
	char uniformName[bufferSize + 1];

	for (int i = 0; i < uniformCount; i++) {
		int uniformSize = 0;
		GLenum uniformType;
		glGetActiveUniform(programHandle, i, bufferSize, nullptr, &uniformSize, &uniformType, uniformName);

		if (IsUniformTypeSupported(uniformType)) {
			this->variables.push_back({ GLEnumToUniformType(uniformType), std::string(uniformName) });
		}
		else {
			spdlog::warn("Unsupported type: {}", uniformType);
			this->variables.push_back({ UniformType::Unsupported, uniformName });
		}
	}
}

UniformSpec::UniformSpec() { }

UniformSpec::UniformSpec(std::vector<UniformVariable> variables):
variables(variables){
	int offset = 0;
	for (auto& var : variables) {
		var.offset = offset;

		offset += UniformTypeSizes[(int) var.type];
	}
}


UniformSpec::UniformSpec(const ShaderProgram* program) {
	GLuint handle = program->GetHandle();

	CreateFrom(handle);
}

UniformSpec::UniformSpec(const ComputeShaderProgram* program) {
	GLuint handle = program->GetHandle();

	CreateFrom(handle);
}

unsigned int UniformSpec::GetBufferSize() const {
	unsigned int size = 0;

	for (const auto& var : this->variables) {
		size += UniformTypeSizes[(int) var.type];
	}

	return size;
}

unsigned int UniformSpec::VariableCount() const {
	return this->variables.size();
}

const UniformVariable& UniformSpec::VariableAt(int index) const {
	return this->variables.at(index);
}

const UniformVariable& UniformSpec::operator[](int index) const {
	return VariableAt(index);
}