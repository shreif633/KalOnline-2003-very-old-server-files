#pragma once

#include "core/Entities.hpp"
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>

struct Party {
    uint32_t id;
    std::vector<uint32_t> memberIds;
    uint32_t leaderId;
    int lootMode; // 0=Free, 1=Round Robin, 2=Master Looter
    bool isRaid;
};

struct Guild {
    uint32_t id;
    std::string name;
    uint32_t leaderId;
    std::vector<uint32_t> memberIds;
    std::unordered_map<uint32_t, int> memberRanks; // memberId -> rank (0=Member, 1=Officer, 2=Leader)
    std::string notice;
    int level;
    int reputation;
    std::vector<uint32_t> alliedGuilds;
    std::vector<uint32_t> enemyGuilds;
    uint32_t castleId = 0; // If guild owns a castle
};

class SocialSystem {
public:
    SocialSystem();
    ~SocialSystem();

    // Party System
    std::shared_ptr<Party> CreateParty(uint32_t leaderId);
    bool JoinParty(uint32_t partyId, uint32_t playerId);
    bool LeaveParty(uint32_t playerId);
    bool KickFromParty(uint32_t partyId, uint32_t playerId, uint32_t kickerId);
    bool DisbandParty(uint32_t partyId);
    std::shared_ptr<Party> GetParty(uint32_t partyId) const;
    std::shared_ptr<Party> GetPlayerParty(uint32_t playerId) const;
    
    // Guild System
    std::shared_ptr<Guild> CreateGuild(const std::string& name, uint32_t leaderId);
    bool JoinGuild(uint32_t guildId, uint32_t playerId);
    bool LeaveGuild(uint32_t playerId);
    bool KickFromGuild(uint32_t guildId, uint32_t playerId, uint32_t kickerId);
    bool SetGuildRank(uint32_t guildId, uint32_t playerId, int rank);
    bool SetGuildNotice(uint32_t guildId, const std::string& notice, uint32_t playerId);
    bool DisbandGuild(uint32_t guildId);
    bool AddGuildAlly(uint32_t guildId1, uint32_t guildId2);
    bool AddGuildEnemy(uint32_t guildId1, uint32_t guildId2);
    std::shared_ptr<Guild> GetGuild(uint32_t guildId) const;
    std::shared_ptr<Guild> GetPlayerGuild(uint32_t playerId) const;
    std::vector<std::shared_ptr<Guild>> GetAllGuilds() const;
    
    // Friend/Block System
    bool AddFriend(uint32_t playerId, uint32_t friendId);
    bool RemoveFriend(uint32_t playerId, uint32_t friendId);
    bool BlockPlayer(uint32_t playerId, uint32_t blockedId);
    bool UnblockPlayer(uint32_t playerId, uint32_t blockedId);
    std::vector<uint32_t> GetFriendList(uint32_t playerId) const;
    std::vector<uint32_t> GetBlockList(uint32_t playerId) const;
    bool IsFriend(uint32_t playerId, uint32_t otherId) const;
    bool IsBlocked(uint32_t playerId, uint32_t otherId) const;
    
    // Chat Filtering
    bool CanCommunicate(uint32_t playerId, uint32_t targetId) const; // Checks block list
    
    // Database Operations
    bool LoadGuilds();
    bool SaveGuild(const Guild& guild);
    bool LoadFriends(uint32_t playerId);
    bool SaveFriend(uint32_t playerId, uint32_t friendId);

private:
    mutable std::mutex socialMutex_;
    std::unordered_map<uint32_t, std::shared_ptr<Party>> parties_;
    std::unordered_map<uint32_t, std::shared_ptr<Guild>> guilds_;
    std::unordered_map<uint32_t, std::vector<uint32_t>> friendLists_; // playerId -> [friendIds]
    std::unordered_map<uint32_t, std::vector<uint32_t>> blockLists_; // playerId -> [blockedIds]
    
    uint32_t nextPartyId_ = 1;
    uint32_t nextGuildId_ = 1;
};
