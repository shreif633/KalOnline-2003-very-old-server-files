#include "EntityManager.hpp"
#include "Logger.h"
#include "Database.h"
#include <algorithm>

EntityManager::EntityManager() {
    Logger::Info("EntityManager initialized");
}

EntityManager::~EntityManager() {
    ClearAllEntities();
    Logger::Info("EntityManager destroyed");
}

bool EntityManager::CreatePlayer(uint32_t playerId, const std::string& name, int jobClass, 
                                  int level, float x, float y, int mapId) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    if (entities_.find(playerId) != entities_.end()) {
        Logger::Warn("Entity {} already exists", playerId);
        return false;
    }
    
    auto player = std::make_shared<Player>(playerId, name, jobClass, level);
    player->SetPosition(mapId, x, y);
    
    entities_[playerId] = player;
    players_.push_back(player);
    
    Logger::Info("Created player {} (ID: {}, Class: {}, Level: {})", 
                 name, playerId, jobClass, level);
    return true;
}

bool EntityManager::CreateMonster(uint32_t monsterId, int monsterType, int level,
                                   float x, float y, int mapId) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    if (entities_.find(monsterId) != entities_.end()) {
        return false;
    }
    
    auto monster = std::make_shared<Monster>(monsterId, monsterType, level);
    monster->SetPosition(mapId, x, y);
    
    entities_[monsterId] = monster;
    monsters_.push_back(monster);
    
    return true;
}

bool EntityManager::CreateNPC(uint32_t npcId, int npcType, float x, float y, int mapId) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    if (entities_.find(npcId) != entities_.end()) {
        return false;
    }
    
    auto npc = std::make_shared<NPC>(npcId, npcType);
    npc->SetPosition(mapId, x, y);
    
    entities_[npcId] = npc;
    npcs_.push_back(npc);
    
    return true;
}

std::shared_ptr<Entity> EntityManager::GetEntity(uint32_t entityId) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = entities_.find(entityId);
    if (it != entities_.end()) {
        return it->second;
    }
    
    return nullptr;
}

std::shared_ptr<Player> EntityManager::GetPlayer(uint32_t playerId) const {
    auto entity = GetEntity(playerId);
    if (entity) {
        return std::dynamic_pointer_cast<Player>(entity);
    }
    return nullptr;
}

std::shared_ptr<Monster> EntityManager::GetMonster(uint32_t monsterId) const {
    auto entity = GetEntity(monsterId);
    if (entity) {
        return std::dynamic_pointer_cast<Monster>(entity);
    }
    return nullptr;
}

std::shared_ptr<NPC> EntityManager::GetNPC(uint32_t npcId) const {
    auto entity = GetEntity(npcId);
    if (entity) {
        return std::dynamic_pointer_cast<NPC>(entity);
    }
    return nullptr;
}

bool EntityManager::RemoveEntity(uint32_t entityId) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = entities_.find(entityId);
    if (it == entities_.end()) {
        return false;
    }
    
    auto entity = it->second;
    
    // Remove from specific container
    if (auto player = std::dynamic_pointer_cast<Player>(entity)) {
        players_.erase(
            std::remove_if(players_.begin(), players_.end(),
                [entityId](const std::shared_ptr<Player>& p) {
                    return p && p->GetId() == entityId;
                }),
            players_.end()
        );
    } else if (auto monster = std::dynamic_pointer_cast<Monster>(entity)) {
        monsters_.erase(
            std::remove_if(monsters_.begin(), monsters_.end(),
                [entityId](const std::shared_ptr<Monster>& m) {
                    return m && m->GetId() == entityId;
                }),
            monsters_.end()
        );
    } else if (auto npc = std::dynamic_pointer_cast<NPC>(entity)) {
        npcs_.erase(
            std::remove_if(npcs_.begin(), npcs_.end(),
                [entityId](const std::shared_ptr<NPC>& n) {
                    return n && n->GetId() == entityId;
                }),
            npcs_.end()
        );
    }
    
    entities_.erase(it);
    return true;
}

std::vector<std::shared_ptr<Player>> EntityManager::GetAllPlayers() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return players_;
}

std::vector<std::shared_ptr<Monster>> EntityManager::GetAllMonsters() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return monsters_;
}

std::vector<std::shared_ptr<NPC>> EntityManager::GetAllNPCs() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return npcs_;
}

size_t EntityManager::GetPlayerCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return players_.size();
}

size_t EntityManager::GetMonsterCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return monsters_.size();
}

size_t EntityManager::GetNPCCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return npcs_.size();
}

void EntityManager::ClearAllEntities() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    entities_.clear();
    players_.clear();
    monsters_.clear();
    npcs_.clear();
    
    Logger::Info("All entities cleared");
}

bool EntityManager::LoadPlayerFromDatabase(uint32_t playerId) {
    try {
        auto db = Database::GetInstance();
        
        // Load player data
        auto result = db->ExecuteQuery(
            "SELECT id, name, job_class, level, str, hth, int_, wis, dex, "
            "exp, hp, mp, sp, stat_point, skill_point, x, y, map_id "
            "FROM players WHERE id = $1",
            {std::to_string(playerId)}
        );
        
        if (!result || !result->Next()) {
            Logger::Warn("Player {} not found in database", playerId);
            return false;
        }
        
        uint32_t id = result->GetInt(0);
        std::string name = result->GetString(1);
        int jobClass = result->GetInt(2);
        int level = result->GetInt(3);
        int str = result->GetInt(4);
        int hth = result->GetInt(5);
        int intStat = result->GetInt(6);
        int wis = result->GetInt(7);
        int dex = result->GetInt(8);
        int64_t exp = result->GetInt64(9);
        int hp = result->GetInt(10);
        int mp = result->GetInt(11);
        int sp = result->GetInt(12);
        int statPoint = result->GetInt(13);
        int skillPoint = result->GetInt(14);
        float x = result->GetFloat(15);
        float y = result->GetFloat(16);
        int mapId = result->GetInt(17);
        
        auto player = std::make_shared<Player>(id, name, jobClass, level);
        player->SetStats(str, hth, intStat, wis, dex);
        player->SetExperience(exp);
        player->SetHP(hp);
        player->SetMP(mp);
        player->SetSP(sp);
        player->SetStatPoints(statPoint);
        player->SetSkillPoints(skillPoint);
        player->SetPosition(mapId, x, y);
        
        std::unique_lock<std::shared_mutex> lock(mutex_);
        entities_[id] = player;
        players_.push_back(player);
        
        Logger::Info("Loaded player {} from database", name);
        return true;
        
    } catch (const std::exception& e) {
        Logger::Error("Failed to load player {}: {}", playerId, e.what());
        return false;
    }
}

bool EntityManager::SavePlayerToDatabase(const std::shared_ptr<Player>& player) {
    if (!player) return false;
    
    try {
        auto db = Database::GetInstance();
        
        db->ExecuteQuery(
            "UPDATE players SET "
            "level = $1, str = $2, hth = $3, int_ = $4, wis = $5, dex = $6, "
            "exp = $7, hp = $8, mp = $9, sp = $10, stat_point = $11, "
            "skill_point = $12, x = $13, y = $14, map_id = $15 "
            "WHERE id = $16",
            {
                std::to_string(player->GetLevel()),
                std::to_string(player->GetSTR()),
                std::to_string(player->GetHTH()),
                std::to_string(player->GetINT()),
                std::to_string(player->GetWIS()),
                std::to_string(player->GetDEX()),
                std::to_string(player->GetExperience()),
                std::to_string(player->GetHP()),
                std::to_string(player->GetMP()),
                std::to_string(player->GetSP()),
                std::to_string(player->GetStatPoints()),
                std::to_string(player->GetSkillPoints()),
                std::to_string(player->GetX()),
                std::to_string(player->GetY()),
                std::to_string(player->GetMapId()),
                std::to_string(player->GetId())
            }
        );
        
        Logger::Debug("Saved player {} to database", player->GetName());
        return true;
        
    } catch (const std::exception& e) {
        Logger::Error("Failed to save player {}: {}", player->GetId(), e.what());
        return false;
    }
}

void EntityManager::UpdateAllPlayers() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& player : players_) {
        if (player && player->IsActive()) {
            player->Update();
        }
    }
}

void EntityManager::UpdateAllMonsters() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& monster : monsters_) {
        if (monster && monster->IsActive()) {
            monster->Update();
        }
    }
}

void EntityManager::UpdateAllNPCs() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& npc : npcs_) {
        if (npc && npc->IsActive()) {
            npc->Update();
        }
    }
}
