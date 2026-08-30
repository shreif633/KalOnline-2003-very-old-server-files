#include "MainServer.hpp"
#include "PacketHandler.h"
#include "Config.h"
#include <chrono>

MainServer::MainServer() 
    : worldManager_()
    , entityManager_()
    , combatSystem_()
    , skillSystem_()
    , itemSystem_()
    , socialSystem_()
    , questSystem_() {
    
    Logger::Info("MainServer constructed");
}

MainServer::~MainServer() {
    Stop();
    Logger::Info("MainServer destroyed");
}

bool MainServer::Start(uint16_t port) {
    try {
        // Load configuration
        tickRateMs_ = static_cast<uint32_t>(Config::GetInt("main.tick_rate_ms", 33));
        pvpEnabled_ = Config::GetBool("main.pvp_enabled", true);
        expRate_ = Config::GetFloat("main.exp_rate", 1.0f);
        dropRate_ = Config::GetFloat("main.drop_rate", 1.0f);
        
        Logger::Info("Main Server Configuration:");
        Logger::Info("  Tick Rate: {}ms ({} Hz)", tickRateMs_, 1000 / tickRateMs_);
        Logger::Info("  PvP Enabled: {}", pvpEnabled_ ? "Yes" : "No");
        Logger::Info("  EXP Rate: x{}", expRate_);
        Logger::Info("  Drop Rate: x{}", dropRate_);
        
        // Initialize world (load maps from database)
        if (!worldManager_.LoadMapsFromDatabase()) {
            Logger::Error("Failed to load world maps");
            return false;
        }
        
        // Initialize item templates
        if (!itemSystem_.LoadItemTemplates()) {
            Logger::Error("Failed to load item templates");
            return false;
        }
        
        // Initialize skill data
        if (!skillSystem_.LoadSkillData()) {
            Logger::Error("Failed to load skill data");
            return false;
        }
        
        // Initialize quest data
        if (!questSystem_.LoadQuestData()) {
            Logger::Error("Failed to load quest data");
            return false;
        }
        
        // Spawn monsters and NPCs
        Logger::Info("Spawning monsters and NPCs...");
        entityManager_.SpawnAllMonsters(worldManager_);
        entityManager_.SpawnAllNPCs(worldManager_);
        
        // Create TCP server
        server_ = std::make_unique<TcpServer>();
        
        // Set connection callback
        server_->SetOnConnect([this](std::shared_ptr<TcpConnection> conn) {
            OnPlayerConnect(conn);
        });
        
        // Set disconnection callback
        server_->SetOnDisconnect([this](std::shared_ptr<TcpConnection> conn) {
            // Find player ID by connection
            std::lock_guard<std::mutex> lock(sessionMutex_);
            for (auto& [playerId, session] : playerSessions_) {
                if (session && session->connection == conn) {
                    OnPlayerDisconnect(playerId);
                    break;
                }
            }
        });
        
        // Set packet received callback
        server_->SetOnReceive([this](std::shared_ptr<TcpConnection> conn, const uint8_t* data, size_t length) {
            PacketHandler::HandlePacket(*this, conn, data, length);
        });
        
        // Start listening
        if (!server_->Listen(port)) {
            Logger::Error("Failed to start TCP server on port {}", port);
            return false;
        }
        
        running_ = true;
        
        // Start game loop thread
        gameLoopThread_ = std::thread(&MainServer::GameTick, this);
        
        Logger::Info("Main Server started successfully on port {}", port);
        return true;
        
    } catch (const std::exception& e) {
        Logger::Critical("Failed to start Main Server: {}", e.what());
        return false;
    }
}

void MainServer::Stop() {
    if (!running_) return;
    
    Logger::Info("Stopping Main Server...");
    running_ = false;
    
    // Wait for game loop thread to finish
    if (gameLoopThread_.joinable()) {
        gameLoopThread_.join();
    }
    
    // Disconnect all players
    {
        std::lock_guard<std::mutex> lock(sessionMutex_);
        for (auto& [playerId, session] : playerSessions_) {
            if (session && session->connection) {
                session->connection->Disconnect();
            }
        }
        playerSessions_.clear();
    }
    
    // Save all player data
    entityManager_.SaveAllPlayers();
    
    // Stop TCP server
    if (server_) {
        server_->Stop();
        server_.reset();
    }
    
    Logger::Info("Main Server stopped");
}

void MainServer::Update() {
    if (server_) {
        server_->Poll();
    }
}

void MainServer::GameTick() {
    auto lastTime = std::chrono::steady_clock::now();
    
    while (running_) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
        
        if (elapsedMs >= tickRateMs_) {
            uint32_t deltaMs = static_cast<uint32_t>(elapsedMs);
            
            // Process network packets
            ProcessPackets();
            
            // Update all systems
            UpdateEntities(deltaMs);
            UpdateCombat(deltaMs);
            UpdateSkills(deltaMs);
            UpdateBuffs(deltaMs);
            UpdateMonsterAI(deltaMs);
            
            tickCount_++;
            lastTime = currentTime;
            
            // Optional: Log tick rate every 60 seconds
            if (tickCount_ % 1800 == 0) { // ~60 sec at 30Hz
                Logger::Debug("Server tick count: {}, Uptime: ~{} minutes", tickCount_, tickCount_ / 1800);
            }
        }
        
        // Small sleep to prevent CPU spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void MainServer::ProcessPackets() {
    if (server_) {
        server_->Poll();
    }
}

void MainServer::UpdateEntities(uint32_t deltaMs) {
    entityManager_.UpdateAll(deltaMs);
}

void MainServer::UpdateCombat(uint32_t deltaMs) {
    // Process damage over time effects
    combatSystem_.UpdateDoTs(deltaMs);
    
    // Check aggro and monster targets
    auto& world = GetWorldManager();
    auto maps = world.GetAllMaps();
    
    for (const auto* map : maps) {
        auto monsters = world.GetMonstersInRange(map->mapId, 0, 0, std::numeric_limits<float>::max());
        for (auto& monster : monsters) {
            combatSystem_.UpdateAggro(monster, deltaMs);
        }
    }
}

void MainServer::UpdateSkills(uint32_t deltaMs) {
    skillSystem_.UpdateCooldowns(deltaMs);
}

void MainServer::UpdateBuffs(uint32_t deltaMs) {
    // Update all active buffs/debuffs on characters
    auto& world = GetWorldManager();
    auto maps = world.GetAllMaps();
    
    for (const auto* map : maps) {
        auto characters = world.GetCharactersInRange(map->mapId, 0, 0, std::numeric_limits<float>::max());
        for (auto& character : characters) {
            skillSystem_.UpdateCharacterBuffs(character, deltaMs);
        }
    }
}

void MainServer::UpdateMonsterAI(uint32_t deltaMs) {
    auto& world = GetWorldManager();
    auto maps = world.GetAllMaps();
    
    for (const auto* map : maps) {
        auto monsters = world.GetMonstersInRange(map->mapId, 0, 0, std::numeric_limits<float>::max());
        for (auto& monster : monsters) {
            entityManager_.UpdateMonsterAI(monster, world, deltaMs);
        }
    }
}

void MainServer::OnPlayerConnect(std::shared_ptr<TcpConnection> conn) {
    Logger::Info("New connection from {}", conn->GetRemoteEndpoint());
    
    // Create a temporary session (will be fully initialized on character select)
    auto session = std::make_shared<PlayerSession>();
    session->connection = conn;
    session->state = PlayerState::CONNECTED;
    session->connectTime = std::chrono::system_clock::now();
    
    // Send welcome packet or protocol version check
    // This is where you'd send initial protocol handshake
}

void MainServer::OnPlayerDisconnect(uint32_t playerId) {
    Logger::Info("Player {} disconnected", playerId);
    
    std::lock_guard<std::mutex> lock(sessionMutex_);
    
    auto it = playerSessions_.find(playerId);
    if (it != playerSessions_.end()) {
        auto session = it->second;
        
        // Save player data before removing
        if (session->player) {
            entityManager_.SavePlayer(session->player);
            
            // Remove entity from world
            worldManager_.UnregisterEntity(session->player->GetId());
        }
        
        // Remove from party if in one
        if (session->partyId > 0) {
            socialSystem_.LeaveParty(playerId);
        }
        
        playerSessions_.erase(it);
    }
}

std::shared_ptr<PlayerSession> MainServer::GetPlayerSession(uint32_t playerId) const {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    
    auto it = playerSessions_.find(playerId);
    if (it != playerSessions_.end()) {
        return it->second;
    }
    
    return nullptr;
}
