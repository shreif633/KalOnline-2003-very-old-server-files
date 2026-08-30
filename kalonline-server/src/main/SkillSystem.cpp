#include "main/core/Entities.hpp"
#include "common/logger/Logger.h"
#include "common/database/Database.h"
#include <algorithm>
#include <chrono>

namespace kal::main {

struct SkillData {
    uint32_t skillId;
    std::string name;
    uint8_t maxLevel;
    uint16_t requiredLevel;
    int requiredJobClass; // -1 for all
    uint16_t mpCost;
    uint16_t spCost;
    uint32_t cooldownMs;
    uint32_t durationMs;
    uint8_t targetType; // 0=self, 1=single, 2=area, 3=ground
    float range;
    float areaRadius;
    std::string effectScript;
};

class SkillSystemImpl {
private:
    std::unordered_map<uint32_t, SkillData> skillDatabase_;
    mutable std::shared_mutex mutex_;

public:
    SkillSystemImpl() {
        Logger::Info("SkillSystem initialized");
        LoadSkillDatabase();
    }

    ~SkillSystemImpl() {
        Logger::Info("SkillSystem destroyed");
    }

    void LoadSkillDatabase() {
        try {
            auto db = Database::GetInstance();
            auto result = db->ExecuteQuery("SELECT id, name, max_level, required_level, required_job, "
                                           "mp_cost, sp_cost, cooldown, duration, target_type, "
                                           "range, area_radius, effect_script FROM skills ORDER BY id");
            
            while (result && result->Next()) {
                SkillData skill;
                skill.skillId = result->GetInt(0);
                skill.name = result->GetString(1);
                skill.maxLevel = result->GetInt(2);
                skill.requiredLevel = result->GetInt(3);
                skill.requiredJobClass = result->GetInt(4);
                skill.mpCost = result->GetInt(5);
                skill.spCost = result->GetInt(6);
                skill.cooldownMs = result->GetInt(7);
                skill.durationMs = result->GetInt(8);
                skill.targetType = result->GetInt(9);
                skill.range = result->GetFloat(10);
                skill.areaRadius = result->GetFloat(11);
                skill.effectScript = result->GetString(12);
                
                skillDatabase_[skill.skillId] = skill;
            }
            
            Logger::Info("Loaded {} skills from database", skillDatabase_.size());
        } catch (const std::exception& e) {
            Logger::Error("Failed to load skill database: {}", e.what());
        }
    }

    const SkillData* GetSkillData(uint32_t skillId) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = skillDatabase_.find(skillId);
        return it != skillDatabase_.end() ? &it->second : nullptr;
    }

    bool CanLearnSkill(const Character& character, uint32_t skillId) const {
        const SkillData* skill = GetSkillData(skillId);
        if (!skill) return false;
        
        if (character.level() < skill->requiredLevel) {
            Logger::Debug("Skill {} requires level {}", skill->name, skill->requiredLevel);
            return false;
        }
        
        if (skill->requiredJobClass >= 0) {
            // TODO: Check job class compatibility
        }
        
        return true;
    }

    bool LearnSkill(Character& character, uint32_t skillId) {
        if (!CanLearnSkill(character, skillId)) {
            return false;
        }
        
        const SkillData* skill = GetSkillData(skillId);
        if (!skill) return false;
        
        // Deduct SP
        // TODO: Implement character SP deduction
        
        // Add skill to character
        // TODO: Implement character skill list
        
        Logger::Info("Character {} learned skill {}", character.id(), skill->name);
        return true;
    }

    bool ExecuteSkill(Character& caster, uint32_t skillId, 
                      const std::vector<std::shared_ptr<Character>>& targets) {
        const SkillData* skill = GetSkillData(skillId);
        if (!skill) {
            Logger::Warn("Attempted to execute unknown skill {}", skillId);
            return false;
        }

        // Check MP cost
        // TODO: Check character MP
        
        // Check cooldown
        // TODO: Check skill cooldown
        
        // Validate targets
        if (!ValidateTargets(caster, *skill, targets)) {
            return false;
        }

        // Execute skill effect
        ApplySkillEffect(caster, targets, *skill);
        
        // Start cooldown
        // TODO: Set skill cooldown
        
        Logger::Debug("Skill {} executed by character {}", skill->name, caster.id());
        return true;
    }

private:
    bool ValidateTargets(const Character& caster, const SkillData& skill,
                        const std::vector<std::shared_ptr<Character>>& targets) const {
        if (targets.empty() && skill.targetType != 0) {
            Logger::Debug("Skill {} requires targets", skill.name);
            return false;
        }

        for (const auto& target : targets) {
            if (!target) continue;
            
            float dx = target->position().x - caster.position().x;
            float dy = target->position().y - caster.position().y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance > skill.range) {
                Logger::Debug("Target out of range for skill {}", skill.name);
                return false;
            }
        }
        
        return true;
    }

    void ApplySkillEffect(Character& caster, 
                         const std::vector<std::shared_ptr<Character>>& targets,
                         const SkillData& skill) {
        // Parse and execute effect script
        // TODO: Implement effect system
        
        for (const auto& target : targets) {
            if (!target) continue;
            
            // Apply buff/debuff
            // Apply damage/healing
            // Trigger visual effects
        }
    }
};

// Global instance
static std::unique_ptr<SkillSystemImpl> g_skillSystem;

void InitializeSkillSystem() {
    g_skillSystem = std::make_unique<SkillSystemImpl>();
}

void ShutdownSkillSystem() {
    g_skillSystem.reset();
}

bool ExecuteSkill(uint32_t casterId, uint32_t skillId, 
                  const std::vector<uint32_t>& targetIds) {
    if (!g_skillSystem) return false;
    
    // TODO: Get caster and targets from EntityManager
    return false;
}

} // namespace kal::main
