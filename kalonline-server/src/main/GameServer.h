#pragma once
#include "NetworkServer.h"
#include <memory>
#include <atomic>
#include <chrono>
#include <functional>

namespace kal {

// Forward declarations
class WorldManager;
class EntityManager;
class CombatSystem;
class SkillSystem;
class ItemSystem;
class SocialSystem;
class QuestSystem;
class MonsterAI;
class SpawnManager;
class PacketHandler;
class NetworkSession;

/**
 * @brief Main game server class that orchestrates all game systems
 * 
 * Implements a fixed-timestep game loop (30 TPS) that updates all
 * game systems synchronously while handling network events asynchronously.
 */
class GameServer {
public:
    static GameServer& GetInstance();

    GameServer();
    ~GameServer();

    // Non-copyable
    GameServer(const GameServer&) = delete;
    GameServer& operator=(const GameServer&) = delete;

    /**
     * @brief Initialize all game systems and load assets
     * @return true if initialization successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Start the main game loop
     */
    void Run();

    /**
     * @brief Shutdown the server gracefully
     */
    void Shutdown();

    /**
     * @brief Check if server is running
     */
    bool IsRunning() const { return running_; }

    // System Accessors
    WorldManager& GetWorldManager() { return *worldManager_; }
    EntityManager& GetEntityManager() { return *entityManager_; }
    CombatSystem& GetCombatSystem() { return *combatSystem_; }
    SkillSystem& GetSkillSystem() { return *skillSystem_; }
    ItemSystem& GetItemSystem() { return *itemSystem_; }
    SocialSystem& GetSocialSystem() { return *socialSystem_; }
    QuestSystem& GetQuestSystem() { return *questSystem_; }

    // Network Methods
    void SendToSession(const std::shared_ptr<NetworkSession>& session, 
                       const std::vector<uint8_t>& packet);
    
    void BroadcastToNearby(int mapId, float x, float y,
                          const std::function<void(const std::shared_ptr<class Player>&)>& callback);

private:
    /**
     * @brief Main game loop with fixed timestep
     */
    void MainLoop();

    /**
     * @brief Update all game systems
     * @param deltaTime Time since last update in milliseconds
     */
    void Update(float deltaTime);

    /**
     * @brief Handle incoming packet from client
     */
    void HandlePacket(const std::shared_ptr<NetworkSession>& session,
                      const std::vector<uint8_t>& packet);

    /**
     * @brief Called when a client connects
     */
    void OnClientConnect(const std::shared_ptr<NetworkSession>& session);

    /**
     * @brief Called when a client disconnects
     */
    void OnClientDisconnect(const std::shared_ptr<NetworkSession>& session);

    /**
     * @brief Log server statistics
     */
    void LogStatistics();

    // State
    std::atomic<bool> running_;
    
    // Core Systems
    std::unique_ptr<WorldManager> worldManager_;
    std::unique_ptr<EntityManager> entityManager_;
    std::unique_ptr<CombatSystem> combatSystem_;
    std::unique_ptr<SkillSystem> skillSystem_;
    std::unique_ptr<ItemSystem> itemSystem_;
    std::unique_ptr<SocialSystem> socialSystem_;
    std::unique_ptr<QuestSystem> questSystem_;
    std::unique_ptr<MonsterAI> monsterAI_;
    std::unique_ptr<SpawnManager> spawnManager_;
    std::unique_ptr<PacketHandler> packetHandler_;
    
    // Network
    NetworkServer networkServer_;
    
    // Timing
    std::chrono::steady_clock::time_point lastTickTime_;
    std::atomic<uint64_t> tickCount_;
};

} // namespace kal
