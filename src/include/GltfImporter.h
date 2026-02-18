#pragma once

#include <filesystem>

#include <fastgltf/core.hpp>

class Scene;
class SceneNode;
class Material;
class Texture2D;
class TextureParams;
class ShaderProgram;

class GltfImporter {
public:
  static SceneNode* LoadScene(Scene* scene, const std::filesystem::path path, ShaderProgram* shaderProgram);
private:
  static std::vector<Material*> LoadMaterials(Scene* scene, fastgltf::Asset& asset, ShaderProgram* shaderProgram);
  static Texture2D* LoadImage(Scene* scene, fastgltf::Asset& asset, fastgltf::Image& image, const TextureParams loadParams);
};
