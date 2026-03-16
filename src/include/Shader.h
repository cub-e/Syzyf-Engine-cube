#pragma once

#include <filesystem>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <VertexSpec.h>
#include <UniformSpec.h>
#include <Resources.h>

namespace fs = std::filesystem;

class SceneGraphics;
class ComputeShader;

template<class T>
concept ShaderLike = requires(T a) {
	{ a.GetHandle() } -> std::same_as<GLuint>;
	{ a.GetUniforms() } -> std::same_as<const UniformSpec&>;
};

const fs::path BaseShaderPath{"./res/shaders"};

enum class ShaderProgramFlags {
	None = 0,
	IgnoreDepthPrepass = 1,
	DontCastShadows = 2,
	UsePatches = 4
};

class ComputeShaderProgram {
private:
	ResourceRef<ComputeShader> computeShader;
	UniformSpec uniforms;

	GLuint handle;
public:	
	ComputeShaderProgram(ResourceRef<ComputeShader> computeShader);
	~ComputeShaderProgram();

	GLuint GetHandle() const;
	const UniformSpec& GetUniforms() const;
};

class ComputeDispatchData;

class ShaderProgram;

struct ShaderVariantInfo {
	struct ShaderVariantPoint {
		std::string name;
		std::string value;
	};

	std::vector<ShaderVariantPoint> variantPoints;

	ShaderVariantInfo(std::initializer_list<ShaderVariantPoint> variantPoints);
};

class ShaderBase : public Resource {
protected:
	const fs::path filePath;
	const ShaderVariantInfo variantInfo;
	const GLuint handle;

	ShaderBase(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle);
public:
	virtual ~ShaderBase();
	
	const fs::path& GetFilePath() const;
	std::string GetName() const;
	const ShaderVariantInfo& GetVariantInfo() const;
	GLuint GetHandle() const;
	virtual GLenum GetType() const = 0;
};

class VertexShader : public ShaderBase {
	friend class ShaderBase;
private:
	const VertexSpec vertexSpec;
	
	VertexShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle, VertexSpec spec);
public:
	static VertexShader Load(fs::path filePath);

	const VertexSpec& GetVertexSpec() const;

	virtual GLenum GetType() const;
};

class GeometryShader : public ShaderBase {
	friend class ShaderBase;
private:
	GeometryShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle);
public:
	static GeometryShader Load(fs::path filePath);

	virtual GLenum GetType() const;
};

class TesselationControlShader : public ShaderBase {
	friend class ShaderBase;
private:
	TesselationControlShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle);
public:
	static TesselationControlShader Load(fs::path filePath);

	virtual GLenum GetType() const;
};

class TesselationEvaluationShader : public ShaderBase {
	friend class ShaderBase;
private:
	TesselationEvaluationShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle);
public:
	static TesselationEvaluationShader Load(fs::path filePath);

	virtual GLenum GetType() const;
};

class PixelShader : public ShaderBase {
	friend class ShaderBase;
private:
	PixelShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle);
public:
	static PixelShader Load(fs::path filePath);

	virtual GLenum GetType() const;
};

class ComputeShader : public ShaderBase {
	friend class ShaderBase;
private:
	ComputeShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle);
public:
	static ComputeShader Load(fs::path filePath);

	virtual GLenum GetType() const;
};

class ShaderBuilder {
public:
	ResourceRef<VertexShader> vertexShader;
	ResourceRef<GeometryShader> geometryShader;
	ResourceRef<TesselationEvaluationShader> tessEvalShader;
	ResourceRef<TesselationControlShader> tessCtrlShader;
	ResourceRef<PixelShader> pixelShader;
	
	ShaderBuilder& WithVertexShader(ResourceRef<VertexShader> vertexShader);
	ShaderBuilder& WithGeometryShader(ResourceRef<GeometryShader> geometryShader);
	ShaderBuilder& WithTessEvaluationShader(ResourceRef<TesselationEvaluationShader> tessEvalShader);
	ShaderBuilder& WithTessControlShader(ResourceRef<TesselationControlShader> tessCtrlShader);
	ShaderBuilder& WithPixelShader(ResourceRef<PixelShader> pixelShader);

	ShaderProgram* Link();
};

class ShaderProgram {
	friend class SceneGraphics;
	friend class ShaderBuilder;
private:
	ResourceRef<VertexShader> vertexShader;
	ResourceRef<GeometryShader> geometryShader;
	ResourceRef<PixelShader> pixelShader;
	UniformSpec uniforms;
	ShaderProgramFlags flags;

	GLuint handle;

	ShaderProgram(ResourceRef<VertexShader> vertexShader, ResourceRef<GeometryShader> geometryShader, ResourceRef<PixelShader> pixelShader, GLuint handle);
public:
	~ShaderProgram();
	static ShaderBuilder Build();
	
	GLuint GetHandle() const;
	const UniformSpec& GetUniforms() const;
	const VertexSpec& GetVertexSpec() const;

	bool IgnoresDepthPrepass() const;
	bool CastsShadows() const;
	bool UsesPatches() const;

	void SetIgnoresDepthPrepass(bool ignores);
	void SetCastsShadows(bool casts);
};

class ComputeShaderDispatch {
private:
	ComputeDispatchData* dispatchData;
	ComputeShaderProgram* program;
public:
	ComputeShaderDispatch(ResourceRef<ComputeShader> compShader);
	ComputeShaderDispatch(ComputeShaderProgram* program);

	void Dispatch(int groupsX, int groupsY, int groupsZ) const;

	ComputeDispatchData* GetData();
	ComputeShaderProgram* GetProgram();
};
