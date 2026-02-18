#include "GltfImporter.h"

#include "Material.h"
#include "Scene.h"
#include "Texture.h"
#include "fastgltf/types.hpp"

#include <fastgltf/core.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

SceneNode* GltfImporter::LoadScene(Scene* scene, const fs::path path, ShaderProgram* shaderProgram) {
  if (!std::filesystem::exists(path)) {
    spdlog::warn("GltfImporter: File not found: {}", path.string());
    return nullptr;
  }

  // Check options/extensions again later, want to enable clearcoat at least
  fastgltf::Parser parser(fastgltf::Extensions::None);

  constexpr auto gltfOptions =
    fastgltf::Options::DontRequireValidAssetMember |
    fastgltf::Options::AllowDouble |
    fastgltf::Options::LoadExternalImages |
    fastgltf::Options::GenerateMeshIndices;

  auto gltfFile = fastgltf::MappedGltfFile::FromPath(path);
  if (!bool(gltfFile)) {
    spdlog::warn("Failed to open glTF file: {}\n\tPath:{}", fastgltf::getErrorMessage(gltfFile.error()), path.string());
    return nullptr;
  }

  auto asset = parser.loadGltf(gltfFile.get(), path.parent_path(), gltfOptions);
  if (asset.error() != fastgltf::Error::None) {
    spdlog::warn("Failed to load glTF: {}\n\tPath: {}", fastgltf::getErrorMessage(asset.error()), path.string());
    return nullptr;
  }
  
  std::vector<Material*> materials = LoadMaterials(scene, asset.get(), shaderProgram);

  return scene->CreateNode("GltfImporterNode");
}

std::vector<Material*> GltfImporter::LoadMaterials(Scene* scene, fastgltf::Asset& asset, ShaderProgram* shaderProgram) {
  std::vector<Material*> materials;
  materials.reserve(asset.materials.size());
  ResourceDatabase* resources = scene->Resources();

  for (auto& gltfMaterial : asset.materials) {
    Material* material = new Material(shaderProgram);
    material->name = gltfMaterial.name;
    spdlog::info("Loading material: {}", gltfMaterial.name);

    // Diffuse
    glm::vec4 baseColorFactor = glm::make_vec4(gltfMaterial.pbrData.baseColorFactor.data());
    material->SetValue("baseColorFactor", baseColorFactor);

    if (gltfMaterial.pbrData.baseColorTexture.has_value()) {
      std::size_t textureIndex = gltfMaterial.pbrData.baseColorTexture->textureIndex;
      if (asset.textures[textureIndex].imageIndex.has_value()) {
        std::size_t imageIndex = asset.textures[textureIndex].imageIndex.value();
        Texture2D* texture = LoadImage(scene, asset, asset.images[imageIndex], Texture::ColorTextureRGBA);
        material->SetValue("albedoMap", texture);
      }
    }
   
    // MetallicRoughness
    float roughnessFactor = gltfMaterial.pbrData.roughnessFactor;
    material->SetValue("roughnessFactor", roughnessFactor);
    float metallicFactor = gltfMaterial.pbrData.metallicFactor;
    material->SetValue("metallicFactor", metallicFactor);

    if (gltfMaterial.pbrData.metallicRoughnessTexture.has_value()) {
      std::size_t textureIndex = gltfMaterial.pbrData.metallicRoughnessTexture->textureIndex;
      if (asset.textures[textureIndex].imageIndex.has_value()) {
        std::size_t imageIndex = asset.textures[textureIndex].imageIndex.value();
        Texture2D* texture = LoadImage(scene, asset, asset.images[imageIndex], Texture::TechnicalMapXYZ);
        material->SetValue("metallicRoughnessMap", texture);
      }
    }
   
    // Normal
    if (gltfMaterial.normalTexture.has_value()) {
      std::size_t textureIndex = gltfMaterial.normalTexture->textureIndex;
      if (asset.textures[textureIndex].imageIndex.has_value()) {
        std::size_t imageIndex = asset.textures[textureIndex].imageIndex.value();
        Texture2D* texture = LoadImage(scene, asset, asset.images[imageIndex], Texture::TechnicalMapXYZ);
        material->SetValue("normalMap", texture);
      }
    }

    // Add other textures/properties eg. emissive, clearcoat, occlusion, alpha threshold
  }
  
  return materials;
}

Texture2D* GltfImporter::LoadImage(Scene* scene, fastgltf::Asset& asset, fastgltf::Image& image, const TextureParams loadParams) {
  Texture2D* result = nullptr;

  std::visit(fastgltf::visitor {
    [](auto& arg) {},
    [&](fastgltf::sources::URI& filePath) {
      const std::string path(filePath.uri.path().begin(), filePath.uri.path().end());
      result = scene->Resources()->Get<Texture2D>(path, loadParams);
    },
    [&](fastgltf::sources::Array& vector) {
      const unsigned char* data = reinterpret_cast<const unsigned char*>(vector.bytes.data());
      int length = static_cast<int>(vector.bytes.size());
      // SKIPS RESOURCE MANAGER -> no caching
      result = Texture2D::Load(data, length, loadParams);
    },
    [&](fastgltf::sources::BufferView& view) {
      auto& bufferView = asset.bufferViews[view.bufferViewIndex];
      auto& buffer = asset.buffers[bufferView.bufferIndex];

      std::visit(fastgltf::visitor {
        [](auto& arg) {},
        [&](fastgltf::sources::Array& vector) {
          const unsigned char* data = reinterpret_cast<const unsigned char*>(vector.bytes.data() + bufferView.byteOffset);
          int length = static_cast<int>(bufferView.byteLength);
          // SKIPS RESOURCE MANAGER -> no caching
          result = Texture2D::Load(data, length, loadParams);
        }
      }, buffer.data);
    },
  }, image.data);

  return result;
}
