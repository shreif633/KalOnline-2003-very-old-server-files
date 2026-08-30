#include "main/core/Entities.hpp"
#include "common/logger/Logger.h"
#include "common/database/Database.h"
#include <algorithm>

namespace kal::main {

struct PartyMember {
    uint32_t playerId;
    std::string playerName;
    uint8_t level;
    uint16_t jobClass;
    uint32_t currentHP;
    uint32_t maxHP;
    uint32_t currentMP;
    uint32_t maxMP;
    bool isLeader;
    bool isOnline;
    float x, y;
    int mapId;
};

struct Party {
    uint32_t partyId;
    std::vector<PartyMember> members;
    uint8_t lootType; // 0:free, 1:round-robin, 2:master-loot
    bool isOpen; // can anyone join?
};

struct GuildMember {
    uint32_t playerId;
    std::string playerName;
    uint8_t rank; // 0:member, 1:officer, 2:leader
    uint8_t level;
    uint16_t jobClass;
    uint64_t contribution;
    std::string title;
    bool isOnline;
};

struct Guild {
    uint32_t guildId;
    std::string name;
    std::string leaderName;
    uint32_t leaderId;
    uint8_t level;
    uint64_t experience;
    uint32_t castleId; // 0 if none
    std::vector<GuildMember> members;
    std::vector<uint32_t> alliedGuilds;
    std::vector<uint32_t> enemyGuilds;
};

class SocialSystemImpl {
private:
    std::unordered_map<uint32_t, Party> parties_;
    std::unordered_map<uint32_t, Guild> guilds_;
    std::unordered_map<uint32_t, std::vector<uint32_t>> playerFriends_;
    std::unordered_map<uint32_t, std::vector<uint32_t>> playerBlocks_;
    mutable std::shared_mutex mutex_;

public:
    SocialSystemImpl() {
        Logger::Info("SocialSystem initialized");
        LoadGuilds();
    }

    ~SocialSystemImpl() {
        Logger::Info("SocialSystem destroyed");
    }

    void LoadGuilds() {
        try {
            auto db = Database::GetInstance();
            auto result = db->ExecuteQuery(
                "SELECT id, name, leader_id, leader_name, level, exp, castle_id "
                "FROM guilds ORDER BY id"
            );
            
            while (result && result->Next()) {
                Guild guild;
                guild.guildId = result->GetInt(0);
                guild.name = result->GetString(1);
                guild.leaderId = result->GetInt(2);
                guild.leaderName = result->GetString(3);
                guild.level = result->GetInt(4);
                guild.experience = result->GetInt64(5);
                guild.castleId = result->GetInt(6);
                
                // Load members
                auto memberResult = db->ExecuteQuery(
                    "SELECT player_id, player_name, rank, level, job_class, contribution, title "
                    "FROM guild_members WHERE guild_id = $1 ORDER BY rank DESC, contribution DESC",
                    {std::to_string(guild.guildId)}
                );
                
                while (memberResult && memberResult->Next()) {
                    GuildMember member;
                    member.playerId = memberResult->GetInt(0);
                    member.playerName = memberResult->GetString(1);
                    member.rank = memberResult->GetInt(2);
                    member.level = memberResult->GetInt(3);
                    member.jobClass = memberResult->GetInt(4);
                    member.contribution = memberResult->GetInt64(5);
                    member.title = memberResult->GetString(6);
                    member.isOnline = false; // Will be updated on login
                    
                    guild.members.push_back(member);
                }
                
                guilds_[guild.guildId] = guild;
            }
            
            Logger::Info("Loaded {} guilds", guilds_.size());
        } catch (const std::exception& e) {
            Logger::Error("Failed to load guilds: {}", e.what());
        }
    }

    // Party System
    uint32_t CreateParty(uint32_t leaderId, const std::string& leaderName) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        static uint32_t nextPartyId = 1;
        Party party;
        party.partyId = nextPartyId++;
        party.lootType = 0;
        party.isOpen = false;
        
        PartyMember leader;
        leader.playerId = leaderId;
        leader.playerName = leaderName;
        leader.level = 1; // TODO: Get actual level
        leader.jobClass = 0;
        leader.currentHP = 100;
        leader.maxHP = 100;
        leader.currentMP = 50;
        leader.maxMP = 50;
        leader.isLeader = true;
        leader.isOnline = true;
        leader.x = 0;
        leader.y = 0;
        leader.mapId = 0;
        
        party.members.push_back(leader);
        parties_[party.partyId] = party;
        
        Logger::Info("Created party {} with leader {}", party.partyId, leaderName);
        return party.partyId;
    }

    bool JoinParty(uint32_t partyId, uint32_t playerId, const std::string& playerName) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto it = parties_.find(partyId);
        if (it == parties_.end()) {
            return false;
        }
        
        Party& party = it->second;
        
        // Check if already in party
        for (const auto& member : party.members) {
            if (member.playerId == playerId) {
                return false;
            }
        }
        
        PartyMember member;
        member.playerId = playerId;
        member.playerName = playerName;
        member.level = 1; // TODO: Get actual level
        member.jobClass = 0;
        member.currentHP = 100;
        member.maxHP = 100;
        member.currentMP = 50;
        member.maxMP = 50;
        member.isLeader = false;
        member.isOnline = true;
        member.x = 0;
        member.y = 0;
        member.mapId = 0;
        
        party.members.push_back(member);
        
        Logger::Info("Player {} joined party {}", playerName, partyId);
        return true;
    }

    bool LeaveParty(uint32_t playerId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        for (auto& [partyId, party] : parties_) {
            auto it = std::find_if(party.members.begin(), party.members.end(),
                [playerId](const PartyMember& m) { return m.playerId == playerId; });
            
            if (it != party.members.end()) {
                if (it->isLeader) {
                    // Transfer leadership or disband
                    if (party.members.size() > 1) {
                        party.members.erase(it);
                        party.members[0].isLeader = true;
                    } else {
                        parties_.erase(partyId);
                    }
                } else {
                    party.members.erase(it);
                    
                    if (party.members.empty()) {
                        parties_.erase(partyId);
                    }
                }
                
                Logger::Info("Player {} left party {}", playerId, partyId);
                return true;
            }
        }
        
        return false;
    }

    const Party* GetParty(uint32_t partyId) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = parties_.find(partyId);
        return it != parties_.end() ? &it->second : nullptr;
    }

    // Guild System
    const Guild* GetGuild(uint32_t guildId) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = guilds_.find(guildId);
        return it != guilds_.end() ? &it->second : nullptr;
    }

    bool CreateGuild(uint32_t leaderId, const std::string& guildName, const std::string& leaderName) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        // Check if guild with same name exists
        for (const auto& [id, guild] : guilds_) {
            if (guild.name == guildName) {
                return false;
            }
        }
        
        static uint32_t nextGuildId = 1;
        Guild guild;
        guild.guildId = nextGuildId++;
        guild.name = guildName;
        guild.leaderId = leaderId;
        guild.leaderName = leaderName;
        guild.level = 1;
        guild.experience = 0;
        guild.castleId = 0;
        
        GuildMember leader;
        leader.playerId = leaderId;
        leader.playerName = leaderName;
        leader.rank = 2; // Leader
        leader.level = 1; // TODO: Get actual level
        leader.jobClass = 0;
        leader.contribution = 0;
        leader.isOnline = true;
        
        guild.members.push_back(leader);
        guilds_[guild.guildId] = guild;
        
        // Save to database
        SaveGuild(guild);
        
        Logger::Info("Created guild {} led by {}", guildName, leaderName);
        return true;
    }

    void SaveGuild(const Guild& guild) const {
        try {
            auto db = Database::GetInstance();
            db->ExecuteQuery(
                "INSERT INTO guilds (id, name, leader_id, leader_name, level, exp) "
                "VALUES ($1, $2, $3, $4, $5, $6) "
                "ON CONFLICT (id) DO UPDATE SET name=$2, leader_id=$3, leader_name=$4, level=$5, exp=$6",
                {
                    std::to_string(guild.guildId),
                    guild.name,
                    std::to_string(guild.leaderId),
                    guild.leaderName,
                    std::to_string(guild.level),
                    std::to_string(guild.experience)
                }
            );
        } catch (const std::exception& e) {
            Logger::Error("Failed to save guild: {}", e.what());
        }
    }

    // Friend System
    bool AddFriend(uint32_t playerId, uint32_t friendId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto& friends = playerFriends_[playerId];
        if (std::find(friends.begin(), friends.end(), friendId) == friends.end()) {
            friends.push_back(friendId);
            
            // Save to database
            try {
                auto db = Database::GetInstance();
                db->ExecuteQuery(
                    "INSERT INTO friends (player_id, friend_id) VALUES ($1, $2)",
                    {std::to_string(playerId), std::to_string(friendId)}
                );
            } catch (...) {}
            
            return true;
        }
        
        return false;
    }

    bool RemoveFriend(uint32_t playerId, uint32_t friendId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto it = playerFriends_.find(playerId);
        if (it != playerFriends_.end()) {
            auto& friends = it->second;
            friends.erase(
                std::remove(friends.begin(), friends.end(), friendId),
                friends.end()
            );
            return true;
        }
        
        return false;
    }

    // Block System
    bool BlockPlayer(uint32_t playerId, uint32_t blockedId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto& blocks = playerBlocks_[playerId];
        if (std::find(blocks.begin(), blocks.end(), blockedId) == blocks.end()) {
            blocks.push_back(blockedId);
            return true;
        }
        
        return false;
    }

    bool IsBlocked(uint32_t playerId, uint32_t checkerId) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = playerBlocks_.find(checkerId);
        if (it != playerBlocks_.end()) {
            return std::find(it->second.begin(), it->second.end(), playerId) != it->second.end();
        }
        
        return false;
    }
};

// Global instance
static std::unique_ptr<SocialSystemImpl> g_socialSystem;

void InitializeSocialSystem() {
    g_socialSystem = std::make_unique<SocialSystemImpl>();
}

void ShutdownSocialSystem() {
    g_socialSystem.reset();
}

uint32_t CreateParty(uint32_t leaderId, const std::string& leaderName) {
    return g_socialSystem ? g_socialSystem->CreateParty(leaderId, leaderName) : 0;
}

bool JoinParty(uint32_t partyId, uint32_t playerId, const std::string& playerName) {
    return g_socialSystem ? g_socialSystem->JoinParty(partyId, playerId, playerName) : false;
}

bool LeaveParty(uint32_t playerId) {
    return g_socialSystem ? g_socialSystem->LeaveParty(playerId) : false;
}

bool CreateGuild(uint32_t leaderId, const std::string& guildName, const std::string& leaderName) {
    return g_socialSystem ? g_socialSystem->CreateGuild(leaderId, guildName, leaderName) : false;
}

bool AddFriend(uint32_t playerId, uint32_t friendId) {
    return g_socialSystem ? g_socialSystem->AddFriend(playerId, friendId) : false;
}

bool BlockPlayer(uint32_t playerId, uint32_t blockedId) {
    return g_socialSystem ? g_socialSystem->BlockPlayer(playerId, blockedId) : false;
}

} // namespace kal::main
