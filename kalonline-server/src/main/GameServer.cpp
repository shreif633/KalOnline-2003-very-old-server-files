#include "GameServer.h"
#include "WorldManager.h"
#include "EntityManager.h"
#include "CombatSystem.h"
#include "SkillSystem.h"
#include "ItemSystem.h"
#include "SocialSystem.h"
#include "QuestSystem.h"
#include "MonsterAI.h"
#include "SpawnManager.h"
#include "AssetLoader.h"
#include "KsmParser.h"
#include "PacketHandler.h"
#include "Logger.h"
#include "Config.h"
#include "Database.h"
#include <chrono>
#include <thread>
#include <atomic>

namespace kal {

GameServer::GameServer() 
    : running_(false)
    , worldManager_(std::make_unique<WorldManager>())
    , entityManager_(std::make_unique<EntityManager>(*worldManager_))
    , combatSystem_(std::make_unique<CombatSystem>(*entityManager_))
    , skillSystem_(std::make_unique<SkillSystem>(*entityManager_))
    , itemSystem_(std::make_unique<ItemSystem>(*entityManager_))
    , socialSystem_(std::make_unique<SocialSystem>(*entityManager_))
    , questSystem_(std::make_unique<QuestSystem>(*entityManager_))
    , monsterAI_(std::make_unique<MonsterAI>(*entityManager_, *worldManager_))
    , spawnManager_(std::make_unique<SpawnManager>(*entityManager_, *worldManager_))
    , packetHandler_(std::make_unique<PacketHandler>(*this))
    , lastTickTime_(std::chrono::steady_clock::now())
    , tickCount_(0)
{
    Logger::Info("GameServer constructed");
}

GameServer::~GameServer() {
    Shutdown();
    Logger::Info("GameServer destroyed");
}

bool GameServer::Initialize() {
    Logger::Info("=== Initializing Game Server ===");

    // 1. Load Configuration
    Logger::Info("Loading configuration...");
    auto& config = Config::GetInstance();
    
    // 2. Initialize Database Connection
    Logger::Info("Connecting to database...");
    if (!Database::Initialize(
        config.GetString("database.host", "localhost"),
        config.GetInt("database.port", 5432),
        config.GetString("database.user", "kalonline"),
        config.GetString("database.password", ""),
        config.GetString("database.game_db", "kal_db")
    )) {
        Logger::Error("Failed to connect to database");
        return false;
    }

    // 3. Load Static Assets from Original Files
    Logger::Info("Loading game assets from original files...");
    AssetLoader loader;
    
    // Load all text configuration files
    if (!loader.LoadItems("Config/InitItem.txt")) {
        Logger::Warn("Failed to load InitItem.txt, using database fallback");
    }
    
    if (!loader.LoadMonsters("Config/InitMonster.txt")) {
        Logger::Warn("Failed to load InitMonster.txt, using database fallback");
    }
    
    if (!loader.LoadNPCs("Config/InitNPC.txt")) {
        Logger::Warn("Failed to load InitNPC.txt, using database fallback");
    }
    
    if (!loader.LoadSkills("Config/InitSkill.txt")) {
        Logger::Warn("Failed to load InitSkill.txt, using database fallback");
    }
    
    if (!loader.LoadQuests("Config/Quest.txt")) {
        Logger::Warn("Failed to load Quest.txt, using database fallback");
    }
    
    if (!loader.LoadGoods("Config/Goods.txt")) {
        Logger::Warn("Failed to load Goods.txt, using database fallback");
    }
    
    if (!loader.LoadItemGroups("Config/ItemGroup.txt")) {
        Logger::Warn("Failed to load ItemGroup.txt, using database fallback");
    }
    
    if (!loader.LoadEtc("Config/Etc.txt")) {
        Logger::Warn("Failed to load Etc.txt, using database fallback");
    }
    
    if (!loader.LoadGenMonster("Config/GenMonster.txt")) {
        Logger::Warn("Failed to load GenMonster.txt, using database fallback");
    }

    // 4. Load Map Geometry (.ksm files)
    Logger::Info("Loading map geometry from .ksm files...");
    KsmParser ksmParser;
    auto mapFiles = ksmParser.ScanMapDirectory("Map");
    
    for (const auto& mapFile : mapFiles) {
        auto mapData = ksmParser.Parse(mapFile);
        if (mapData) {
            worldManager_->AddMap(std::move(mapData));
            Logger::Info("Loaded map: {} (ID: {})", mapFile, mapData->mapId);
        } else {
            Logger::Error("Failed to parse map file: {}", mapFile);
        }
    }

    // 5. Initialize World Manager with loaded maps
    if (!worldManager_->LoadMapsFromDatabase()) {
        Logger::Warn("Failed to load maps from database, using only .ksm data");
    }

    // 6. Initialize Systems
    Logger::Info("Initializing game systems...");
    
    if (!itemSystem_->Initialize()) {
        Logger::Error("Failed to initialize Item System");
        return false;
    }
    
    if (!skillSystem_->Initialize()) {
        Logger::Error("Failed to initialize Skill System");
        return false;
    }
    
    if (!questSystem_->Initialize()) {
        Logger::Error("Failed to initialize Quest System");
        return false;
    }

    // 7. Spawn Initial Monsters and NPCs
    Logger::Info("Spawning initial entities...");
    spawnManager_->SpawnAllFromConfig();

    // 8. Initialize Network Server
    int port = config.GetInt("game_server.port", 9003);
    if (!networkServer_.Initialize(port)) {
        Logger::Error("Failed to initialize network server on port {}", port);
        return false;
    }

    networkServer_.SetPacketHandler([this](const std::shared_ptr<NetworkSession>& session, 
                                           const std::vector<uint8_t>& packet) {
        HandlePacket(session, packet);
    });

    networkServer_.SetOnConnect([this](const std::shared_ptr<NetworkSession>& session) {
        OnClientConnect(session);
    });

    networkServer_.SetOnDisconnect([this](const std::shared_ptr<NetworkSession>& session) {
        OnClientDisconnect(session);
    });

    Logger::Info("=== Game Server Initialization Complete ===");
    Logger::Info("Listening on port {}", port);
    Logger::Info("Active maps: {}", worldManager_->GetAllMaps().size());
    Logger::Info("Loaded items: {}", itemSystem_->GetItemCount());
    Logger::Info("Loaded monsters: {}", spawnManager_->GetMonsterCount());
    Logger::Info("Loaded NPCs: {}", spawnManager_->GetNPCCount());
    
    return true;
}

void GameServer::Run() {
    if (!running_) {
        running_ = true;
        Logger::Info("Starting game loop at 30 TPS...");
        
        MainLoop();
    }
}

void GameServer::Shutdown() {
    if (running_) {
        Logger::Info("Shutting down game server...");
        running_ = false;
        
        // Stop network server
        networkServer_.Shutdown();
        
        // Save all player data
        entityManager_->SaveAllPlayers();
        
        // Clear world
        worldManager_->ClearWorld();
        
        // Close database
        Database::Shutdown();
        
        Logger::Info("Game server shut down gracefully");
    }
}

void GameServer::MainLoop() {
    constexpr auto TARGET_TICK_DURATION = std::chrono::milliseconds(33); // 30 TPS
    
    while (running_) {
        auto tickStart = std::chrono::steady_clock::now();
        
        // 1. Process Network Events (non-blocking)
        networkServer_.Poll();
        
        // 2. Update Game Systems
        float deltaTime = 33.0f; // Fixed timestep in ms
        Update(deltaTime);
        
        // 3. Maintain target tick rate
        auto tickEnd = std::chrono::steady_clock::now();
        auto tickDuration = std::chrono::duration_cast<std::chrono::milliseconds>(tickEnd - tickStart);
        
        if (tickDuration < TARGET_TICK_DURATION) {
            auto sleepTime = TARGET_TICK_DURATION - tickDuration;
            std::this_thread::sleep_for(sleepTime);
        } else {
            Logger::Warn("Tick took {}ms (target: 33ms)", tickDuration.count());
        }
        
        tickCount_++;
        
        // Log statistics every 30 seconds (~900 ticks)
        if (tickCount_ % 900 == 0) {
            LogStatistics();
        }
    }
}

void GameServer::Update(float deltaTime) {
    // Update all game systems with fixed timestep
    
    // 1. Entity Updates (movement interpolation, status effects)
    entityManager_->Update(deltaTime);
    
    // 2. Combat System (DoT ticks, aggro checks, death processing)
    combatSystem_->Update(deltaTime);
    
    // 3. Skill System (cooldowns, buff/debuff expiration)
    skillSystem_->Update(deltaTime);
    
    // 4. Monster AI (pathfinding, state transitions, attacks)
    monsterAI_->Update(deltaTime);
    
    // 5. Social System (party sync, guild updates)
    socialSystem_->Update(deltaTime);
    
    // 6. Quest System (objective checking, progress updates)
    questSystem_->Update(deltaTime);
    
    // 7. Spawn Manager (respawn dead monsters/NPCs)
    spawnManager_->Update(deltaTime);
}

void GameServer::HandlePacket(const std::shared_ptr<NetworkSession>& session, 
                              const std::vector<uint8_t>& packet) {
    if (packet.size() < sizeof(PacketHeader)) {
        Logger::Warn("Received invalid packet (too small): {} bytes", packet.size());
        return;
    }
    
    PacketReader reader(packet);
    PacketHeader header = reader.ReadHeader();
    
    // Validate packet size
    if (header.size != packet.size()) {
        Logger::Warn("Packet size mismatch: header says {}, actual {}", 
                     header.size, packet.size());
        return;
    }
    
    // Route to packet handler
    packetHandler_->HandlePacket(session, static_cast<ClientOpcode>(header.opcode), reader);
}

void GameServer::OnClientConnect(const std::shared_ptr<NetworkSession>& session) {
    Logger::Info("Client connected: {}", session->GetId());
    
    // Create a placeholder player entity for this session
    // Actual character will be created after login/selection
    auto player = entityManager_->CreatePlayer(session);
    if (player) {
        session->SetUserData(player);
        Logger::Debug("Created temporary player entity for session {}", session->GetId());
    }
}

void GameServer::OnClientDisconnect(const std::shared_ptr<NetworkSession>& session) {
    Logger::Info("Client disconnected: {}", session->GetId());
    
    // Get player entity from session
    auto player = std::static_pointer_cast<Player>(session->GetUserData());
    if (player) {
        // Save player data before removing
        entityManager_->SavePlayer(player);
        
        // Remove from world
        worldManager_->UnregisterEntity(player->GetId());
        entityManager_->RemoveEntity(player->GetId());
        
        // Notify nearby players
        BroadcastToNearby(player->GetMapId(), player->GetX(), player->GetY(), 
                         [player](const std::shared_ptr<Player>& other) {
            // Send remove entity packet
        });
        
        Logger::Debug("Removed player {} from world", player->GetName());
    }
    
    session->SetUserData(nullptr);
}

void GameServer::BroadcastToNearby(int mapId, float x, float y, 
                                    const std::function<void(const std::shared_ptr<Player>&)>& callback) {
    auto nearbyPlayers = worldManager_->GetCharactersInRange(mapId, x, y, 100.0f); // 100 unit radius
    
    for (const auto& character : nearbyPlayers) {
        auto player = std::dynamic_pointer_cast<Player>(character);
        if (player && player->IsOnline()) {
            callback(player);
        }
    }
}

void GameServer::SendToSession(const std::shared_ptr<NetworkSession>& session, 
                               const std::vector<uint8_t>& packet) {
    networkServer_.Send(session, packet);
}

void GameServer::LogStatistics() {
    auto activePlayers = entityManager_->GetOnlinePlayerCount();
    auto totalEntities = worldManager_->GetEntityCount(0); // Sum across all maps
    
    Logger::Info("[Stats] Tick: {}, Active Players: {}, Total Entities: {}, Uptime: {}s",
                 tickCount_, 
                 activePlayers,
                 totalEntities,
                 tickCount_ * 33 / 1000);
}

// Singleton Access
GameServer& GameServer::GetInstance() {
    static GameServer instance;
    return instance;
}

} // namespace kal
