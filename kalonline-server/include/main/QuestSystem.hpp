#pragma once

#include "core/Entities.hpp"
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>
#include <functional>

enum class QuestState : uint8_t {
    NOT_STARTED = 0,
    IN_PROGRESS = 1,
    COMPLETED = 2,
    FAILED = 3
};

struct QuestObjective {
    int id;
    std::string type; // KILL, COLLECT, TALK, EXPLORE
    int targetId; // Monster ID, Item ID, NPC ID
    int requiredCount;
    int currentCount;
    bool isComplete;
};

struct QuestTemplate {
    int id;
    std::string name;
    int level;
    int giverNpcId;
    int receiverNpcId;
    std::string description;
    std::vector<QuestObjective> objectives;
    std::vector<int> rewardItems;
    int rewardExp;
    int rewardGold;
    int rewardReputation;
    bool isRepeatable;
    bool isDaily;
};

struct PlayerQuest {
    int questId;
    QuestState state;
    std::vector<QuestObjective> objectives;
    uint64_t startTime;
    uint64_t expiryTime; // For daily quests
};

class QuestSystem {
public:
    QuestSystem();
    ~QuestSystem();

    // Initialization
    bool LoadQuestData();
    
    // Quest Management
    bool AcceptQuest(std::shared_ptr<Player> player, int questId);
    bool CompleteQuest(std::shared_ptr<Player> player, int questId);
    bool FailQuest(std::shared_ptr<Player> player, int questId);
    bool AbandonQuest(std::shared_ptr<Player> player, int questId);
    
    // Objective Tracking
    bool UpdateObjective(std::shared_ptr<Player> player, int questId, int objectiveType, int targetId);
    void OnMonsterKill(std::shared_ptr<Player> player, int monsterId);
    void OnItemPickup(std::shared_ptr<Player> player, int itemId);
    void OnNpcTalk(std::shared_ptr<Player> player, int npcId);
    
    // Validation
    bool CanAcceptQuest(const Player& player, int questId) const;
    bool HasCompletedQuest(const Player& player, int questId) const;
    bool IsQuestInProgress(const Player& player, int questId) const;
    
    // Accessors
    const QuestTemplate* GetQuestTemplate(int questId) const;
    const PlayerQuest* GetPlayerQuest(const Player& player, int questId) const;
    std::vector<const PlayerQuest*> GetActiveQuests(const Player& player) const;
    std::vector<const PlayerQuest*> GetCompletedQuests(const Player& player) const;
    
    // Daily Reset
    void ResetDailyQuests();

private:
    mutable std::mutex questMutex_;
    std::unordered_map<int, QuestTemplate> questDatabase_;
    std::unordered_map<uint32_t, std::vector<PlayerQuest>> playerQuests_; // playerId -> [quests]
    
    bool CheckQuestCompletion(PlayerQuest& quest);
    void GiveQuestRewards(std::shared_ptr<Player> player, const QuestTemplate& quest);
};
