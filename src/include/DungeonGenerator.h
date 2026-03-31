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

  const unsigned int mapColumns = 10;
  std::vector<Room> map;
public:
  DungeonGenerator();
  virtual ~DungeonGenerator();

  void Awake();
};
