#include "DungeonGenerator.h"

#include "GltfImporter.h"
#include "imgui.h"

#include <glm/gtc/random.hpp>
#include <set>

// Move this to awake/enable(?)
DungeonGenerator::DungeonGenerator(DungeonGeneratorSettings settings) : settings(settings) {
    int attempts = 0;
    while (!this->GenerateDungeon()) {
        attempts++;
        if (attempts > this->MAX_ATTEMPTS) {
            spdlog::error("DungeonGenerator: Failed to generate a dungeon");
            return;
        }
    }

    this->CreateRoomNodes();
}

DungeonGenerator::~DungeonGenerator() {
    // ?
    auto attachedScenes = this->GetNode()->GetAttachedScenes();
    for (auto* scene : attachedScenes) {
        this->GetNode()->DetachScene(scene);
        this->GetScene()->QueueDelete(scene);
    }
}

void DungeonGenerator::Regenerate() {
    // ?
    for (auto* scene : this->GetNode()->GetAttachedScenes()) {
        this->GetNode()->DetachScene(scene);
        this->GetScene()->QueueDelete(scene);
    }

    int attempts = 0;
    while (!this->GenerateDungeon()) {
        attempts++;
        if (attempts > this->MAX_ATTEMPTS) {
            spdlog::error("DungeonGenerator: Failed to generate a dungeon");
            return;
        }
    }
    this->CreateRoomNodes();
}

bool DungeonGenerator::GenerateDungeon() {
    this->marginSize = this->settings.margin * this->ROOM_SIZE;

    // Main path
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    const int roomCount = this->settings.mapColumns * this->settings.mapColumns;

    this->map.clear();
    this->map.resize(roomCount, Room());
    Room room;

    // Start position
    int startPosition = glm::linearRand(0, roomCount - 1);
    this->map.at(startPosition).roomType = RoomType::Start;
    std::set<int> visited = { startPosition };

    this->Traverse(visited, startPosition, this->settings.steps);

    // Branches
    for (std::size_t i = 0; i < this->settings.numberOfBranches; ++i) {
        int attempts = 0;

        while (true) {
            if (attempts >= this->MAX_ATTEMPTS) {
                return false;
            }

            int randomIndex = glm::linearRand(0, static_cast<int>(visited.size() - 1));
            startPosition = *std::next(visited.begin(), randomIndex);

            if (this->map.at(startPosition).roomType != RoomType::Goal) {
                break;
            }
        };

        int branchLength = glm::linearRand(this->settings.minBranchLength, this->settings.maxBranchLength);
        Traverse(visited, startPosition, branchLength, false);
    } 
    
    // Rooms
    for (std::size_t i = 0; i < this->settings.numberOf2x2Rooms; ++i) {
        int attempts = 0;
        while (true) {
            if (attempts >= this->MAX_ATTEMPTS)
                return false;

            if (GenerateRooms(visited, glm::uvec2(2, 2), RoomType::Big2x2))
                break;

            attempts++;
        }
    }
    for (std::size_t i = 0; i < this->settings.numberOf3x3Rooms; ++i) {
        int attempts = 0;
        while (true) {
            if (attempts >= this->MAX_ATTEMPTS)
                return false;

            if (GenerateRooms(visited, glm::uvec2(3, 3), RoomType::Big3x3))
                break;

            attempts++;
        }
    }
    
    return true;
}

void DungeonGenerator::Traverse(std::set<int>& visited, const int startPosition, const int steps, const bool markGoal) {
    int currentPosition = startPosition;
    std::vector<int> previousPositions;
    int step = 0;

    while (step < steps && step > -1) {
        bool backtrack = false;

        std::vector<Directions> availableDirections = this->GetAvailableDirections(currentPosition);

        while (!availableDirections.empty()) {
            Directions direction = availableDirections.at(
                glm::linearRand(0, static_cast<int>(availableDirections.size() - 1))
            );

            int nextPosition = currentPosition;
            bool moved = false;
            switch (direction) {
                case Directions::Up:
                    nextPosition = currentPosition - this->settings.mapColumns;
                    if (!visited.contains(nextPosition) && nextPosition < map.size()) {
                        map.at(currentPosition).doors.top = true;
                        map.at(nextPosition).doors.bottom = true;
                        currentPosition = nextPosition;
                        previousPositions.push_back(currentPosition);
                        visited.insert(currentPosition);
                        // temporary
                        if (markGoal) {
                            map.at(currentPosition).roomType = RoomType::Normal;
                        } else {
                            map.at(currentPosition).roomType = RoomType::BigRoom;
                        }
                        availableDirections.clear();
                        moved = true;
                    } else {
                        std::erase(availableDirections, Directions::Up);
                    }
                    break;
                case Directions::Down:
                    nextPosition = currentPosition + this->settings.mapColumns;
                    if (!visited.contains(nextPosition) && nextPosition < map.size()) {
                        map.at(currentPosition).doors.bottom = true;
                        map.at(nextPosition).doors.top = true;
                        currentPosition = nextPosition;
                        previousPositions.push_back(currentPosition);
                        visited.insert(currentPosition);
                        // temporary
                        if (markGoal) {
                            map.at(currentPosition).roomType = RoomType::Normal;
                        } else {
                            map.at(currentPosition).roomType = RoomType::BigRoom;
                        }
                        availableDirections.clear();
                        moved = true;
                    } else {
                        std::erase(availableDirections, Directions::Down);
                    }
                    break;
                case Directions::Left:
                    nextPosition = currentPosition - 1;
                    if (!visited.contains(nextPosition) && nextPosition < map.size()) {
                        map.at(currentPosition).doors.left = true;
                        map.at(nextPosition).doors.right = true;
                        currentPosition = nextPosition;
                        previousPositions.push_back(currentPosition);
                        visited.insert(currentPosition);
                        // temporary
                        if (markGoal) {
                            map.at(currentPosition).roomType = RoomType::Normal;
                        } else {
                            map.at(currentPosition).roomType = RoomType::BigRoom;
                        }
                        availableDirections.clear();
                        moved = true;
                    } else {
                        std::erase(availableDirections, Directions::Left);
                    }
                    break;
                case Directions::Right:
                    nextPosition = currentPosition + 1;
                    if (!visited.contains(nextPosition) && nextPosition < map.size()) {
                        map.at(currentPosition).doors.right = true;
                        map.at(nextPosition).doors.left = true;
                        currentPosition = nextPosition;
                        previousPositions.push_back(currentPosition);
                        visited.insert(currentPosition);
                        // temporary
                        if (markGoal) {
                            map.at(currentPosition).roomType = RoomType::Normal;
                        } else {
                            map.at(currentPosition).roomType = RoomType::BigRoom;
                        }
                        availableDirections.clear();
                        moved = true;
                    } else {
                        std::erase(availableDirections, Directions::Right);
                    }
                    break;
            }

            if (availableDirections.empty() && !previousPositions.empty() && moved == false) {
                currentPosition = previousPositions.back();
                previousPositions.pop_back();

                backtrack = true;
            } else if (availableDirections.empty() && previousPositions.empty() && moved == false) {
                break;
            }
        }

        if (backtrack == true)
            continue;
        
        if (step == steps - 1 && markGoal == true) {
            map.at(currentPosition).roomType = RoomType::Goal;
        }
        
        step++;
    }
}

bool DungeonGenerator::GenerateRooms(std::set<int>& visited, glm::uvec2 roomSize, RoomType roomType) {
    int randomIndex = glm::linearRand(0, static_cast<int>(visited.size() - 1));
    int roomPosition = *std::next(visited.begin(), randomIndex);
   
    if (!this->CheckIfRoomPositionIsValid(roomPosition, roomSize))
        return false;

    map.at(roomPosition).roomType = roomType;
    for (int i = 1; i < roomSize.x * roomSize.y; ++i) {
        int index = roomPosition + (i % roomSize.x + this->settings.mapColumns * (i / roomSize.x));
        map.at(index).roomType = RoomType::BigRoom;
    }

    return true;
}

bool DungeonGenerator::CheckIfRoomPositionIsValid(int roomPosition, glm::uvec2 roomSize) {
    const std::set<RoomType> INVALID_ROOMS = {
        RoomType::Goal,
        RoomType::Start,
        RoomType::Big3x3,
        RoomType::Big2x2,
        RoomType::BigRoom,
    };

    if ((roomPosition % this->settings.mapColumns) + roomSize.x > this->settings.mapColumns ||
        (roomPosition / this->settings.mapColumns) + roomSize.y > this->settings.mapColumns) {
        return false;
    }

    for (int y = 0; y < roomSize.y; y++) {
        for (int x = 0; x < roomSize.x; x++) {
            int index = roomPosition + x + this->settings.mapColumns * y;
            if (INVALID_ROOMS.contains(map.at(index).roomType))
                return false;
        }
    }
    return true;
}

std::vector<DungeonGenerator::Directions> DungeonGenerator::GetAvailableDirections(const int currentPosition) {
    std::vector<Directions> availableDirections;

    if (currentPosition >= this->settings.mapColumns)
        availableDirections.push_back(Directions::Up);

    const int roomCount = this->settings.mapColumns * this->settings.mapColumns;
    if (currentPosition < roomCount - this->settings.mapColumns)
        availableDirections.push_back(Directions::Down);

    if (currentPosition % this->settings.mapColumns != 0)
        availableDirections.push_back(Directions::Left);

    if ((currentPosition + 1) % settings.mapColumns != 0)
        availableDirections.push_back(Directions::Right);

    return availableDirections;
}

// Replace with something smarter later
void DungeonGenerator::CreateRoomNodes() {
    const float offset = (this->settings.mapColumns * (this->ROOM_SIZE + marginSize) * 0.5f) + this->ROOM_SIZE * 0.5f;
    glm::vec3 startPosition = {
        -offset,
        0.0f,
        -offset,
    };

    for (std::size_t y = 0; y < this->settings.mapColumns; y++) {
        for (std::size_t x = 0; x < this->settings.mapColumns; x++) {
            const std::size_t index = y * this->settings.mapColumns + x;
            Room& room = this->map.at(index);

            Scene* scene = nullptr;
            glm::vec3 modelOffset = glm::vec3(0.0f);
            switch (room.roomType) {
                case RoomType::Start:
                    // temporary
                    scene = GltfImporter::LoadScene(
                        "./res/models/RoomStart.glb",
                        std::format("Room {}x{}: Start", x, y)
                    );
                    break;
                case RoomType::Normal:
                    scene = GltfImporter::LoadScene(
                        "./res/models/Room.glb",
                        std::format("Room {}x{}: Normal", x, y)
                    );
                    break;
                case RoomType::Goal:
                    scene = GltfImporter::LoadScene(
                        "./res/models/RoomGoal.glb",
                        std::format("Room {}x{}: Goal", x, y)
                    );
                    break;
                case RoomType::Empty:
                    scene = GltfImporter::LoadScene(
                        "./res/models/RoomEmpty.glb",
                        std::format("Room {}x{}: Goal", x, y)
                    );
                    break;
                case RoomType::Big2x2:
                    scene = GltfImporter::LoadScene(
                        "./res/models/Room2x2.glb",
                        std::format("Room {}x{}: Goal", x, y)
                    );
                    modelOffset = glm::vec3(
                        (this->ROOM_SIZE + this->marginSize) * 0.5f,
                        0.0f,
                        (this->ROOM_SIZE + this->marginSize) * 0.5f
                    );
                    break;
                case RoomType::Big3x3:
                    scene = GltfImporter::LoadScene(
                        "./res/models/Room3x3.glb",
                        std::format("Room {}x{}: Goal", x, y)
                    );
                    modelOffset = glm::vec3(
                        this->ROOM_SIZE + this->marginSize,
                        0.0f,
                        this->ROOM_SIZE + this->marginSize
                    );
                    break;
                case RoomType::BigRoom:
                    continue;
                default:
                    continue;
            }

            this->GetNode()->AttachScene(scene);
            SceneNode* node = scene->GetRootNode();

            const float roomSize = this->ROOM_SIZE + marginSize;
            const glm::vec3 positionOffset = {
                x * roomSize, 
                0.0f,
                y * roomSize, 
            };
            node->LocalTransform().Position() = startPosition + positionOffset + modelOffset;
        }
    }
}

void DungeonGenerator::DrawImGui() {
    ImGui::SliderInt("Steps", &this->settings.steps, 1, 100); 
    ImGui::SliderInt("Columns", &this->settings.mapColumns, 1, 20);
    ImGui::SliderInt("Number of branches", &this->settings.numberOfBranches, 0, 10);
    ImGui::SliderInt("Min branch length", &this->settings.minBranchLength, 0, 10);
    ImGui::SliderInt("Max branch length", &this->settings.maxBranchLength, 0, 10);

    ImGui::SliderInt("Number of 2x2 rooms", &this->settings.numberOf2x2Rooms, 0, 10);
    ImGui::SliderInt("Number of 3x3 rooms", &this->settings.numberOf3x3Rooms, 0, 10);

    // Remove later
    ImGui::SliderFloat("Margin size", &this->settings.margin, 0.0f, 1.0f);


    if (ImGui::Button("Regenerate Dungeon"))
        this->Regenerate();
}
