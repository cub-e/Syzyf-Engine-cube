#pragma once

#include "Debug.h"
#include <GameObject.h>
#include <set>

struct DungeonGeneratorSettings {
    int mapColumns = 10;
    int steps = 10;

    int numberOfBranches = 0;
    int minBranchLength = 0;
    int maxBranchLength = 0;

    int numberOf2x2Rooms = 0;
    int numberOf3x3Rooms = 0;

    float margin = 0.1f;
};

class DungeonGenerator : public GameObject, public ImGuiDrawable {
public:
    DungeonGeneratorSettings settings;
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

    enum class Directions {
        Left,
        Right,
        Up,
        Down,
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

    // width and height of the normal room scene
    const float ROOM_SIZE = 1.0f;
    // max attempts to try to generate a dungeon (or find a place to start a branch)
    //     before it gives up
    const int MAX_ATTEMPTS = 100;

    float marginSize;
    std::vector<Room> map;
public:
    DungeonGenerator(DungeonGeneratorSettings settings = {});
    virtual ~DungeonGenerator();

    void Regenerate();

    void DrawImGui();
private:
    bool GenerateDungeon();
    void Traverse(std::set<int>& visited, const int startPosition, const int steps, const bool markGoal = true);
    std::vector<Directions> GetAvailableDirections(const int currentPosition);
    bool GenerateRooms(std::set<int>& visited, glm::uvec2 roomSize, RoomType roomType);
    bool CheckIfRoomPositionIsValid(int roomPosition, glm::uvec2 roomSize);

    void CreateRoomNodes();
};
