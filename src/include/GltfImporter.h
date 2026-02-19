#pragma once

#include <filesystem>

#include <fastgltf/core.hpp>

class Scene;
class SceneNode;
class Material;
class Texture2D;
class TextureParams;
class Mesh;
class ShaderProgram;

class GltfImporter {
public:
  static SceneNode* LoadScene(Scene* scene, const std::filesystem::path path, ShaderProgram* shaderProgram, std::string name = "");
private:
  static SceneNode* CreateNode(fastgltf::Node& gltfNode, std::vector<Material*>& materials, fastgltf::Asset& asset, Scene* scene, SceneNode* parent = nullptr);
  static std::vector<Material*> LoadMaterials(Scene* scene, fastgltf::Asset& asset, ShaderProgram* shaderProgram);

  static Mesh* LoadMesh(fastgltf::Mesh&, fastgltf::Asset&, std::vector<Material*>& materials);

  static Texture2D* LoadImage(Scene* scene, fastgltf::Asset& asset, fastgltf::Image& image, const TextureParams loadParams);
};
