#pragma once
#include "Entities.h"
#include "WorldManager.h"
#include <vector>
#include <memory>
#include <chrono>

namespace kal {

struct MonsterSpawnData {
    int monsterId;      // Template ID from InitMonster.txt
    int mapId;
    float x, y;
    int respawnTime;    // Seconds
};

class SpawnManager {
public:
    explicit SpawnManager(EntityManager& entityManager, WorldManager& worldManager);
    
    void LoadSpawnsFromConfig();
    void Update(float deltaTime);
    
private:
    EntityManager& entityManager_;
    WorldManager& worldManager_;
    
    std::vector<MonsterSpawnData> spawnPoints_;
    std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> deathTimes_;
};

} // namespace kal
