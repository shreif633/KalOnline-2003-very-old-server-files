#include "main/core/Entities.hpp"
#include "common/logger/Logger.h"
#include "common/database/Database.h"
#include <algorithm>

namespace kal::main {

enum class QuestState : uint8_t {
    NotStarted = 0,
    InProgress = 1,
    Completed = 2,
    Failed = 3,
    Abandoned = 4
};

struct QuestObjective {
    uint32_t objectiveId;
    uint8_t type; // 0:kill, 1:collect, 2:talk, 3:visit, 4:use_item
    uint32_t targetId; // monsterId, itemId, npcId, mapId
    uint16_t requiredCount;
    uint16_t currentCount;
    bool isCompleted;
};

struct QuestData {
    uint32_t questId;
    std::string title;
    std::string description;
    uint16_t minLevel;
    int requiredJobClass; // -1 for all
    uint32_t giverNpcId;
    uint32_t receiverNpcId;
    uint32_t expReward;
    uint32_t goldReward;
    std::vector<uint32_t> itemRewards;
    std::vector<QuestObjective> objectives;
    bool isRepeatable;
    uint32_t cooldownHours;
};

struct PlayerQuest {
    uint32_t questId;
    QuestState state;
    uint64_t startTime;
    uint64_t completeTime;
    std::vector<QuestObjective> objectives;
};

class QuestSystemImpl {
private:
    std::unordered_map<uint32_t, QuestData> questDatabase_;
    std::unordered_map<uint32_t, std::vector<PlayerQuest>> playerQuests_; // playerId -> quests
    mutable std::shared_mutex mutex_;

public:
    QuestSystemImpl() {
        Logger::Info("QuestSystem initialized");
        LoadQuestDatabase();
    }

    ~QuestSystemImpl() {
        Logger::Info("QuestSystem destroyed");
    }

    void LoadQuestDatabase() {
        try {
            auto db = Database::GetInstance();
            auto result = db->ExecuteQuery(
                "SELECT id, title, description, min_level, required_job, giver_npc, receiver_npc, "
                "exp_reward, gold_reward, is_repeatable, cooldown_hours FROM quests ORDER BY id"
            );
            
            while (result && result->Next()) {
                QuestData quest;
                quest.questId = result->GetInt(0);
                quest.title = result->GetString(1);
                quest.description = result->GetString(2);
                quest.minLevel = result->GetInt(3);
                quest.requiredJobClass = result->GetInt(4);
                quest.giverNpcId = result->GetInt(5);
                quest.receiverNpcId = result->GetInt(6);
                quest.expReward = result->GetInt(7);
                quest.goldReward = result->GetInt(8);
                quest.isRepeatable = result->GetInt(9);
                quest.cooldownHours = result->GetInt(10);
                
                // Load objectives
                auto objResult = db->ExecuteQuery(
                    "SELECT id, type, target_id, required_count FROM quest_objectives "
                    "WHERE quest_id = $1 ORDER BY order_index",
                    {std::to_string(quest.questId)}
                );
                
                while (objResult && objResult->Next()) {
                    QuestObjective obj;
                    obj.objectiveId = objResult->GetInt(0);
                    obj.type = objResult->GetInt(1);
                    obj.targetId = objResult->GetInt(2);
                    obj.requiredCount = objResult->GetInt(3);
                    obj.currentCount = 0;
                    obj.isCompleted = false;
                    
                    quest.objectives.push_back(obj);
                }
                
                // Load item rewards
                auto rewardResult = db->ExecuteQuery(
                    "SELECT item_id, count FROM quest_rewards WHERE quest_id = $1",
                    {std::to_string(quest.questId)}
                );
                
                while (rewardResult && rewardResult->Next()) {
                    quest.itemRewards.push_back(rewardResult->GetInt(0));
                }
                
                questDatabase_[quest.questId] = quest;
            }
            
            Logger::Info("Loaded {} quests", questDatabase_.size());
        } catch (const std::exception& e) {
            Logger::Error("Failed to load quest database: {}", e.what());
        }
    }

    const QuestData* GetQuestData(uint32_t questId) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = questDatabase_.find(questId);
        return it != questDatabase_.end() ? &it->second : nullptr;
    }

    bool CanAcceptQuest(uint32_t playerId, uint32_t questId) const {
        const QuestData* quest = GetQuestData(questId);
        if (!quest) return false;
        
        // TODO: Check player level
        // TODO: Check job class
        // TODO: Check if already has quest
        // TODO: Check if quest is on cooldown
        
        return true;
    }

    bool AcceptQuest(uint32_t playerId, uint32_t questId) {
        if (!CanAcceptQuest(playerId, questId)) {
            return false;
        }
        
        const QuestData* quest = GetQuestData(questId);
        if (!quest) return false;
        
        PlayerQuest pq;
        pq.questId = questId;
        pq.state = QuestState::InProgress;
        pq.startTime = getCurrentTimeMs();
        pq.completeTime = 0;
        
        // Copy objectives
        for (const auto& obj : quest->objectives) {
            pq.objectives.push_back(obj);
        }
        
        std::unique_lock<std::shared_mutex> lock(mutex_);
        playerQuests_[playerId].push_back(pq);
        
        // Save to database
        SaveQuestState(playerId, pq);
        
        Logger::Info("Player {} accepted quest {}", playerId, quest->title);
        return true;
    }

    bool UpdateObjective(uint32_t playerId, uint32_t questId, uint32_t objectiveId, uint16_t count) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto playerIt = playerQuests_.find(playerId);
        if (playerIt == playerQuests_.end()) return false;
        
        for (auto& pq : playerIt->second) {
            if (pq.questId != questId || pq.state != QuestState::InProgress) continue;
            
            for (auto& obj : pq.objectives) {
                if (obj.objectiveId == objectiveId && !obj.isCompleted) {
                    obj.currentCount += count;
                    
                    if (obj.currentCount >= obj.requiredCount) {
                        obj.isCompleted = true;
                    }
                    
                    // Check if all objectives are complete
                    bool allComplete = std::all_of(pq.objectives.begin(), pq.objectives.end(),
                        [](const QuestObjective& o) { return o.isCompleted; });
                    
                    if (allComplete) {
                        pq.state = QuestState::Completed;
                        pq.completeTime = getCurrentTimeMs();
                    }
                    
                    SaveQuestState(playerId, pq);
                    return true;
                }
            }
        }
        
        return false;
    }

    bool CompleteQuest(uint32_t playerId, uint32_t questId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto playerIt = playerQuests_.find(playerId);
        if (playerIt == playerQuests_.end()) return false;
        
        for (auto& pq : playerIt->second) {
            if (pq.questId == questId && pq.state == QuestState::Completed) {
                const QuestData* quest = GetQuestData(questId);
                if (!quest) return false;
                
                // Give rewards
                // TODO: Give EXP
                // TODO: Give Gold
                // TODO: Give Items
                
                if (quest->isRepeatable) {
                    pq.state = QuestState::Completed; // Keep for cooldown tracking
                } else {
                    // Remove quest
                    playerIt->second.erase(
                        std::remove_if(playerIt->second.begin(), playerIt->second.end(),
                            [questId](const PlayerQuest& q) { return q.questId == questId; }),
                        playerIt->second.end()
                    );
                }
                
                Logger::Info("Player {} completed quest {}", playerId, quest->title);
                return true;
            }
        }
        
        return false;
    }

    bool AbandonQuest(uint32_t playerId, uint32_t questId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto playerIt = playerQuests_.find(playerId);
        if (playerIt == playerQuests_.end()) return false;
        
        playerIt->second.erase(
            std::remove_if(playerIt->second.begin(), playerIt->second.end(),
                [questId](const PlayerQuest& q) { return q.questId == questId; }),
            playerIt->second.end()
        );
        
        // Delete from database
        try {
            auto db = Database::GetInstance();
            db->ExecuteQuery(
                "DELETE FROM player_quests WHERE player_id = $1 AND quest_id = $2",
                {std::to_string(playerId), std::to_string(questId)}
            );
        } catch (...) {}
        
        return true;
    }

    std::vector<PlayerQuest> GetPlayerQuests(uint32_t playerId) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = playerQuests_.find(playerId);
        if (it != playerQuests_.end()) {
            return it->second;
        }
        
        return {};
    }

    void SaveQuestState(uint32_t playerId, const PlayerQuest& pq) const {
        try {
            auto db = Database::GetInstance();
            
            // Upsert quest state
            db->ExecuteQuery(
                "INSERT INTO player_quests (player_id, quest_id, state, start_time, complete_time) "
                "VALUES ($1, $2, $3, $4, $5) "
                "ON CONFLICT (player_id, quest_id) DO UPDATE SET state=$3, complete_time=$5",
                {
                    std::to_string(playerId),
                    std::to_string(pq.questId),
                    std::to_string(static_cast<int>(pq.state)),
                    std::to_string(pq.startTime),
                    std::to_string(pq.completeTime)
                }
            );
            
            // Update objectives
            for (const auto& obj : pq.objectives) {
                db->ExecuteQuery(
                    "INSERT INTO player_quest_objectives (player_id, quest_id, objective_id, current_count, is_completed) "
                    "VALUES ($1, $2, $3, $4, $5) "
                    "ON CONFLICT (player_id, quest_id, objective_id) DO UPDATE SET current_count=$4, is_completed=$5",
                    {
                        std::to_string(playerId),
                        std::to_string(pq.questId),
                        std::to_string(obj.objectiveId),
                        std::to_string(obj.currentCount),
                        obj.isCompleted ? "1" : "0"
                    }
                );
            }
        } catch (const std::exception& e) {
            Logger::Error("Failed to save quest state: {}", e.what());
        }
    }

    uint64_t getCurrentTimeMs() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
};

// Global instance
static std::unique_ptr<QuestSystemImpl> g_questSystem;

void InitializeQuestSystem() {
    g_questSystem = std::make_unique<QuestSystemImpl>();
}

void ShutdownQuestSystem() {
    g_questSystem.reset();
}

bool AcceptQuest(uint32_t playerId, uint32_t questId) {
    return g_questSystem ? g_questSystem->AcceptQuest(playerId, questId) : false;
}

bool UpdateQuestObjective(uint32_t playerId, uint32_t questId, uint32_t objectiveId, uint16_t count) {
    return g_questSystem ? g_questSystem->UpdateObjective(playerId, questId, objectiveId, count) : false;
}

bool CompleteQuest(uint32_t playerId, uint32_t questId) {
    return g_questSystem ? g_questSystem->CompleteQuest(playerId, questId) : false;
}

bool AbandonQuest(uint32_t playerId, uint32_t questId) {
    return g_questSystem ? g_questSystem->AbandonQuest(playerId, questId) : false;
}

} // namespace kal::main
