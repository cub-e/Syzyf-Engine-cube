#pragma once

#include <GameObject.h>

class DungeonGenerator : public GameObject {
private:
  enum class RoomType {
    Empty,
    Normal,
    Start,
    Goal,
    Big2x2,
    Big3x3,
    BigRoom,
  };

  struct Room {
  private:
    struct Doors {
      bool top = false;
      bool bottom = false;
      bool left = false;
      bool right = false;
    };
  public:
    RoomType roomType = RoomType::Empty;
    Doors doors;
  };

  const glm::vec2 roomSize = { 1.0f, 1.0f };
  const float marginSize = roomSize.x * 0.1;
  const unsigned int mapColumns = 10;
  std::vector<Room> map;
public:
  DungeonGenerator();
  virtual ~DungeonGenerator();

  // Add imgui
};
