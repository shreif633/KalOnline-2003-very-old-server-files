#pragma once

#include "core/Entities.hpp"
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>

struct SkillData {
    int id;
    std::string name;
    int requiredLevel;
    int requiredSkillId = 0; // Prerequisite skill
    int manaCost;
    int staminaCost;
    float cooldownMs;
    int castTimeMs;
    int range;
    std::string targetType; // SELF, SINGLE, AREA, GROUND
    float effectDurationMs;
    std::vector<int> effects; // List of effect IDs
};

class SkillSystem {
public:
    SkillSystem();
    ~SkillSystem();

    // Initialization
    bool LoadSkillData();
    
    // Skill Learning
    bool LearnSkill(std::shared_ptr<Character> character, int skillId);
    bool CanLearnSkill(const Character& character, int skillId) const;
    
    // Skill Execution
    bool ExecuteSkill(std::shared_ptr<Character> caster, int skillId, 
                      std::shared_ptr<Character> target = nullptr,
                      float targetX = 0.0f, float targetY = 0.0f);
    bool ValidateSkillTarget(const Character& caster, int skillId, 
                            const Character& target) const;
    
    // Cooldown & Buff Management
    void UpdateCooldowns(uint32_t deltaMs);
    void UpdateCharacterBuffs(std::shared_ptr<Character> character, uint32_t deltaMs);
    bool IsSkillOnCooldown(uint32_t characterId, int skillId) const;
    
    // Accessors
    const SkillData* GetSkillData(int skillId) const;
    std::vector<int> GetLearnedSkills(uint32_t characterId) const;

private:
    mutable std::mutex skillMutex_;
    std::unordered_map<int, SkillData> skillDatabase_;
    std::unordered_map<uint32_t, std::unordered_map<int, uint64_t>> skillCooldowns_; // charId -> (skillId -> readyTime)
    std::unordered_map<uint32_t, std::vector<std::pair<int, uint64_t>>> activeBuffs_; // charId -> [(skillId, expiryTime)]
    
    bool HasPrerequisite(const Character& character, int skillId) const;
    void ApplySkillEffects(std::shared_ptr<Character> target, const std::vector<int>& effects, float durationMs);
};
