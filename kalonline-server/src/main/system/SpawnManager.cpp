#include "SpawnManager.h"
#include "AssetLoader.h"
#include "Logger.h"

namespace kal {

SpawnManager::SpawnManager(EntityManager& entityManager, WorldManager& worldManager)
    : entityManager_(entityManager)
    , worldManager_(worldManager) {
}

void SpawnManager::LoadSpawnsFromConfig() {
    // Load spawn points from GenMonster.txt via AssetLoader
    auto spawns = AssetLoader::GetInstance().GetMonsterSpawns();
    
    for (const auto& spawn : spawns) {
        MonsterSpawnData data;
        data.monsterId = spawn.templateId;
        data.mapId = spawn.mapId;
        data.x = spawn.x;
        data.y = spawn.y;
        data.respawnTime = spawn.respawnTime;
        
        spawnPoints_.push_back(data);
        
        // Initial spawn
        SpawnMonster(data);
    }
    
    Logger::Info("Loaded {} monster spawn points", spawnPoints_.size());
}

void SpawnManager::Update(float deltaTime) {
    // Check for respawns
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = deathTimes_.begin(); it != deathTimes_.end();) {
        uint32_t monsterId = it->first;
        auto deathTime = it->second;
        
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - deathTime).count();
        
        // Find corresponding spawn point
        for (const auto& spawn : spawnPoints_) {
            if (spawn.monsterId == static_cast<int>(monsterId)) {
                if (elapsed >= spawn.respawnTime) {
                    SpawnMonster(spawn);
                    it = deathTimes_.erase(it);
                    break;
                }
            }
        }
        
        if (it != deathTimes_.end()) {
            ++it;
        }
    }
}

void SpawnMonster(const MonsterSpawnData& spawn) {
    // Create monster entity at spawn point
    // TODO: Get monster template from AssetLoader and create entity
    Logger::Debug("Spawning monster {} at ({}, {}) on map {}", 
                  spawn.monsterId, spawn.x, spawn.y, spawn.mapId);
}

} // namespace kal
