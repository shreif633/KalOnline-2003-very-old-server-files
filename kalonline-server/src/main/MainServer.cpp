#include "MainServer.h"
#include "AssetLoader.h"
#include "PacketHandler.h"
#include <chrono>

MainServer::MainServer(Config& config) 
    : config_(config),
      running_(false),
      assetLoader_(std::make_unique<AssetLoader>(config_.GetString("game.config_path", "../Config"))) {
}

MainServer::~MainServer() {
    Shutdown();
}

bool MainServer::Initialize() {
    Logger::Info("Initializing Main Game Server...");
    
    // Load all game assets from original config files
    if (!assetLoader_->LoadAll()) {
        Logger::Error("Failed to load game assets");
        return false;
    }
    
    // Initialize database connection
    if (!Database::Initialize(config_)) {
        Logger::Error("Failed to initialize database");
        return false;
    }
    
    // Load maps from database
    if (!worldManager_.LoadMapsFromDatabase()) {
        Logger::Warn("Failed to load maps from database, using defaults");
    }
    
    // Spawn initial monsters and NPCs based on loaded data
    SpawnInitialEntities();
    
    // Initialize network server
    int port = config_.GetInt("game.port", 9003);
    networkServer_ = std::make_unique<NetworkServer>();
    
    if (!networkServer_->Start(port, [this](auto conn, auto data) {
        HandlePacket(conn, data);
    })) {
        Logger::Error("Failed to start network server on port {}", port);
        return false;
    }
    
    Logger::Info("Main Game Server initialized successfully on port {}", port);
    return true;
}

void MainServer::SpawnInitialEntities() {
    Logger::Info("Spawning initial entities...");
    
    const auto& monsters = assetLoader_->GetMonsters();
    const auto& npcs = assetLoader_->GetNPCs();
    const auto& spawns = assetLoader_->GetGenMonsterSpawns();
    
    // Spawn NPCs
    for (const auto& [index, npcTemplate] : npcs) {
        auto npc = std::make_shared<NPC>(
            GenerateEntityId(),
            npcTemplate->index,
            npcTemplate->mapId,
            static_cast<float>(npcTemplate->position.first),
            static_cast<float>(npcTemplate->position.second)
        );
        
        npc->SetKind(npcTemplate->kind);
        npc->SetShape(npcTemplate->shape);
        npc->SetHtmlId(npcTemplate->htmlId);
        
        worldManager_.RegisterEntity(npc);
    }
    
    // Spawn initial monsters at spawn points
    for (const auto& spawn : spawns) {
        auto monsterIt = monsters.find(spawn->index);
        if (monsterIt == monsters.end()) continue;
        
        const auto& monsterTemplate = monsterIt->second;
        
        // Spawn up to maxCount monsters at this spawn point
        for (int i = 0; i < spawn->maxCount; ++i) {
            auto [minX, maxX, minY, maxY] = spawn->rect;
            
            float x = static_cast<float>(minX + (rand() % (maxX - minX + 1)));
            float y = static_cast<float>(minY + (rand() % (maxY - minY + 1)));
            
            auto monster = std::make_shared<Monster>(
                GenerateEntityId(),
                monsterTemplate->index,
                spawn->mapId,
                x,
                y
            );
            
            // Set monster stats from template
            monster->SetLevel(monsterTemplate->level);
            monster->SetHP(monsterTemplate->hp);
            monster->SetMaxHP(monsterTemplate->hp);
            monster->SetMP(monsterTemplate->mp);
            monster->SetMaxMP(monsterTemplate->mp);
            monster->SetSTR(monsterTemplate->str);
            monster->SetDEX(monsterTemplate->dex);
            monster->SetINT(monsterTemplate->int_);
            monster->SetWIS(monsterTemplate->wis);
            monster->SetHTH(monsterTemplate->hth);
            
            monster->SetMinAttack(monsterTemplate->minAttack);
            monster->SetMaxAttack(monsterTemplate->maxAttack);
            monster->SetDefense(monsterTemplate->minDefense);
            monster->SetAbsorb(monsterTemplate->absorb);
            
            monster->SetExpReward(monsterTemplate->exp);
            monster->SetItemGroup(monsterTemplate->itemGroup);
            
            worldManager_.RegisterEntity(monster);
        }
    }
    
    Logger::Info("Entity spawning complete");
}

void MainServer::Run() {
    if (!running_) {
        running_ = true;
        
        Logger::Info("Starting main game loop...");
        
        auto lastTick = std::chrono::steady_clock::now();
        int tickCount = 0;
        
        while (running_) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick).count();
            
            // Target 30 ticks per second (33ms per tick)
            if (elapsed >= 33) {
                GameTick();
                lastTick = now;
                tickCount++;
                
                // Log stats every 10 seconds
                if (tickCount % 300 == 0) {
                    Logger::Debug("Game tick stats: {} entities", worldManager_.GetEntityCount(0));
                }
            }
            
            // Small sleep to prevent CPU spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void MainServer::GameTick() {
    // Update all entities
    auto allMaps = worldManager_.GetAllMaps();
    
    for (const auto* map : allMaps) {
        // Get all characters in this map
        auto characters = worldManager_.GetCharactersInRange(map->mapId, 
            (map->minX + map->maxX) / 2.0f,
            (map->minY + map->maxY) / 2.0f,
            std::max(map->maxX - map->minX, map->maxY - map->minY)
        );
        
        for (auto& character : characters) {
            // Update buffs/debuffs
            character->UpdateBuffs(33); // 33ms tick
            
            // Update cooldowns
            character->UpdateCooldowns(33);
            
            // If player, send updates
            if (auto player = std::dynamic_pointer_cast<Player>(character)) {
                // Send status updates to client
                // This would be implemented in PacketHandler
            }
        }
        
        // Update monsters
        auto monsters = worldManager_.GetMonstersInRange(map->mapId,
            (map->minX + map->maxX) / 2.0f,
            (map->minY + map->maxY) / 2.0f,
            std::max(map->maxX - map->minX, map->maxY - map->minY)
        );
        
        for (auto& monster : monsters) {
            monster->UpdateAI(33);
            
            // Check if monster should respawn HP/MP
            if (monster->GetHP() < monster->GetMaxHP()) {
                monster->SetHP(std::min(monster->GetMaxHP(), 
                    monster->GetHP() + monster->GetMaxHP() / 100)); // 1% per tick
            }
        }
    }
    
    // Process combat actions
    combatSystem_.ProcessPendingActions();
}

void MainServer::HandlePacket(const std::shared_ptr<NetworkConnection>& conn, const std::vector<uint8_t>& data) {
    if (data.size() < 4) return; // Minimum packet size
    
    // Parse packet header [Length:2][Opcode:2]
    uint16_t length = data[0] | (data[1] << 8);
    uint16_t opcode = data[2] | (data[3] << 8);
    
    if (length != data.size()) {
        Logger::Warn("Packet length mismatch: declared {}, actual {}", length, data.size());
        return;
    }
    
    // Find or create session for this connection
    auto sessionId = GetSessionId(conn);
    
    // Route packet to handler
    PacketHandler handler(*this, conn, sessionId);
    handler.HandlePacket(opcode, data);
}

uint32_t MainServer::GetSessionId(const std::shared_ptr<NetworkConnection>& conn) {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    
    auto it = connectionToSession_.find(conn);
    if (it != connectionToSession_.end()) {
        return it->second;
    }
    
    // Create new session
    uint32_t newSessionId = nextSessionId_++;
    connectionToSession_[conn] = newSessionId;
    sessionToConnection_[newSessionId] = conn;
    
    return newSessionId;
}

void MainServer::RemoveSession(const std::shared_ptr<NetworkConnection>& conn) {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    
    auto it = connectionToSession_.find(conn);
    if (it != connectionToSession_.end()) {
        uint32_t sessionId = it->second;
        
        // Remove player from world if exists
        auto player = std::dynamic_pointer_cast<Player>(worldManager_.GetEntityBySession(sessionId));
        if (player) {
            worldManager_.UnregisterEntity(player->GetId());
        }
        
        connectionToSession_.erase(it);
        sessionToConnection_.erase(sessionId);
    }
}

void MainServer::Shutdown() {
    if (!running_) return;
    
    Logger::Info("Shutting down Main Game Server...");
    running_ = false;
    
    // Stop network server
    if (networkServer_) {
        networkServer_->Stop();
    }
    
    // Save all player data
    SaveAllPlayers();
    
    // Clear world
    worldManager_.ClearWorld();
    
    Logger::Info("Main Game Server shutdown complete");
}

void MainServer::SaveAllPlayers() {
    Logger::Info("Saving all player data...");
    
    // Iterate through all players and save to database
    // This would use the DBServer to queue save requests
}

uint32_t MainServer::GenerateEntityId() {
    static std::atomic<uint32_t> idCounter{1000};
    return idCounter.fetch_add(1);
}

// Singleton access
static std::unique_ptr<MainServer> g_mainServer;

MainServer& MainServer::GetInstance() {
    if (!g_mainServer) {
        throw std::runtime_error("MainServer not initialized");
    }
    return *g_mainServer;
}

void MainServer::CreateInstance(Config& config) {
    if (!g_mainServer) {
        g_mainServer = std::make_unique<MainServer>(config);
    }
}

void MainServer::DestroyInstance() {
    g_mainServer.reset();
}
