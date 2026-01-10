#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <Mesh.h>
#include <Material.h>

struct ShaderGlobalUniforms;
class MeshRenderer;
class Scene;
class ComputeShaderDispatch;
class Texture2D;
class Light;

// struct RenderBatch {
// 	Mesh* mesh;
// 	Material* material;
// 	int argsSize;
// };

class SceneGraphics {
	friend class Scene;
	friend class LightSystem;
private:
	struct RenderNode {
		const Mesh::SubMesh* mesh;
		const Material* material;
		const unsigned int instanceCount;
		const glm::mat4 transformation;
	};

	enum class PassType {
		DepthPrepass,
		Shadows,
		Color
	};
	
	Scene* scene;

	std::vector<RenderNode> currentRenders;
	GLuint globalUniformsBuffer;
	GLuint objectUniformsBuffer;
	
	glm::vec2 screenResolution;
	
	GLuint depthPrepassFramebuffer;
	GLuint depthPrepassDepthTexture;

	GLuint colorPassFramebuffer;
	GLuint colorPassOutputTexture;

	SceneGraphics(Scene* scene);

	void RenderObjects(const ShaderGlobalUniforms& globalUniforms, PassType pass);
	void RenderFullscreenFrameQuad();

	void BindGlobalUniformBuffer(const ShaderGlobalUniforms& globalUniforms);

	void Render();
public:
	glm::vec2 GetScreenResolution() const;
	void UpdateScreenResolution(glm::vec2 newResolution);
	
	void DrawMesh(MeshRenderer* renderer);
	void DrawMesh(const Mesh* mesh, int subMeshIndex, const Material* material, const glm::mat4& transformation);
	
	void DrawMeshInstanced(MeshRenderer* renderer, unsigned int instanceCount);
	void DrawMeshInstanced(const Mesh* mesh, int subMeshIndex, const Material* material, const glm::mat4& transformation, unsigned int instanceCount);
};

