#pragma once

#include "Network.h"
#include "Database.h"
#include "WorldManager.h"
#include "EntityManager.h"
#include "CombatSystem.h"
#include "SkillSystem.h"
#include "ItemSystem.h"
#include "SocialSystem.h"
#include "QuestSystem.h"
#include "Logger.h"
#include <atomic>
#include <thread>
#include <memory>

class MainServer {
public:
    MainServer();
    ~MainServer();

    bool Start(uint16_t port);
    void Stop();
    void Update();
    
    // Game Loop
    void GameTick();
    void ProcessPackets();
    void UpdateEntities(uint32_t deltaMs);
    void UpdateCombat(uint32_t deltaMs);
    void UpdateSkills(uint32_t deltaMs);
    void UpdateBuffs(uint32_t deltaMs);
    void UpdateMonsterAI(uint32_t deltaMs);
    
    // Session Management
    void OnPlayerConnect(std::shared_ptr<TcpConnection> conn);
    void OnPlayerDisconnect(uint32_t playerId);
    std::shared_ptr<PlayerSession> GetPlayerSession(uint32_t playerId) const;
    
    // World Access
    WorldManager& GetWorldManager() { return worldManager_; }
    EntityManager& GetEntityManager() { return entityManager_; }
    CombatSystem& GetCombatSystem() { return combatSystem_; }
    SkillSystem& GetSkillSystem() { return skillSystem_; }
    ItemSystem& GetItemSystem() { return itemSystem_; }
    SocialSystem& GetSocialSystem() { return socialSystem_; }
    QuestSystem& GetQuestSystem() { return questSystem_; }

private:
    std::unique_ptr<TcpServer> server_;
    WorldManager worldManager_;
    EntityManager entityManager_;
    CombatSystem combatSystem_;
    SkillSystem skillSystem_;
    ItemSystem itemSystem_;
    SocialSystem socialSystem_;
    QuestSystem questSystem_;
    
    std::unordered_map<uint32_t, std::shared_ptr<PlayerSession>> playerSessions_;
    mutable std::mutex sessionMutex_;
    
    std::atomic<bool> running_{false};
    std::thread gameLoopThread_;
    
    uint64_t lastTickTime_ = 0;
    uint32_t tickCount_ = 0;
    
    // Configuration
    uint32_t tickRateMs_ = 33; // 30 Hz default
    bool pvpEnabled_ = true;
    bool expRate_ = 1.0f;
    bool dropRate_ = 1.0f;
};
