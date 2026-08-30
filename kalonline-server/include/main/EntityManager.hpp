#pragma once

#include "core/Entities.hpp"
#include "WorldManager.h"
#include "Database.h"
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>

class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    // Entity Creation & Management
    std::shared_ptr<Player> CreatePlayer(uint32_t id);
    std::shared_ptr<Monster> CreateMonster(int templateId, int mapId, float x, float y);
    std::shared_ptr<NPC> CreateNPC(int templateId, int mapId, float x, float y);
    
    // Database Operations
    bool LoadPlayer(uint32_t playerId, std::shared_ptr<Player>& player);
    bool SavePlayer(const std::shared_ptr<Player>& player);
    bool SaveAllPlayers();
    
    // Spawning
    void SpawnAllMonsters(WorldManager& world);
    void SpawnAllNPCs(WorldManager& world);
    
    // Updates
    void UpdateAll(uint32_t deltaMs);
    void UpdateMonsterAI(std::shared_ptr<Monster> monster, WorldManager& world, uint32_t deltaMs);
    
    // Accessors
    std::vector<std::shared_ptr<Player>> GetAllPlayers() const;
    std::vector<std::shared_ptr<Monster>> GetAllMonsters() const;
    std::vector<std::shared_ptr<NPC>> GetAllNPCs() const;

private:
    mutable std::mutex entityMutex_;
    std::vector<std::shared_ptr<Player>> players_;
    std::vector<std::shared_ptr<Monster>> monsters_;
    std::vector<std::shared_ptr<NPC>> npcs_;
    
    uint32_t nextEntityId_ = 1000; // Start after reserved IDs
};
