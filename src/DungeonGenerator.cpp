#include "DungeonGenerator.h"
#include "GltfImporter.h"

// Move this to awake(?)
DungeonGenerator::DungeonGenerator() {
  const glm::vec3 startPosition = {
    -(this->mapColumns * (this->roomSize.x + marginSize) * 0.5f) + roomSize.x * 0.5f,
    0.0f,
    -(this->mapColumns * (this->roomSize.y + marginSize) * 0.5f) + roomSize.y * 0.5f,
  };

  const std::size_t roomCount = this->mapColumns * this->mapColumns;
  for (std::size_t i = 0; i < roomCount; ++i) {
    SceneNode* node = this->GetScene()->CreateNode();
    Scene* roomScene = GltfImporter::LoadScene("./res/models/Room.glb");
    node->AttachScene(roomScene);

    const glm::vec3 positionOffset = {
      i % this->mapColumns * (this->roomSize.x + marginSize),
      0.0f,
      static_cast<int>(i / this->mapColumns) * (this->roomSize.y + marginSize),
    };
    node->LocalTransform().Position() = startPosition + positionOffset; 
  } 
}

DungeonGenerator::~DungeonGenerator() {}
