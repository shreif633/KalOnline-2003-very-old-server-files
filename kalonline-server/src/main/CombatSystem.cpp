#include "main/CombatSystem.hpp"
#include "common/logger/Logger.hpp"
#include <cmath>
#include <random>
#include <algorithm>

namespace kal::main {

using namespace logger;

// ============================================================================
// CharacterStats Implementation
// ============================================================================

void CharacterStats::recalculate() {
    // Original KalOnline stat formulas (preserved exactly)
    
    // HP = Base(10) + HTH * 10 + Level * 5
    max_hp = 10 + static_cast<uint32_t>(health) * 10 + level * 5;
    
    // MP = Base(10) + INT * 5 + WIS * 3
    max_mp = 10 + static_cast<uint32_t>(intelligence) * 5 + static_cast<uint32_t>(wisdom) * 3;
    
    // Attack Power = STR * 2 + Weapon bonus (weapon added separately)
    attack_power = strength * 2;
    
    // Defense = (STR + DEX) / 2 + Armor bonus
    defense = (strength + dexterity) / 2;
    
    // Magic Attack = INT * 3
    magic_attack = intelligence * 3;
    
    // Magic Defense = (INT + WIS) / 2
    magic_defense = (intelligence + wisdom) / 2;
    
    // Accuracy = DEX * 1.5
    accuracy = static_cast<uint16_t>(dexterity * 1.5f);
    
    // Evasion = DEX * 1.2
    evasion = static_cast<uint16_t>(dexterity * 1.2f);
    
    // Critical rate base (can be modified by equipment/skills)
    critical_rate = 50; // 5% base
    
    // Ensure current values don't exceed maximums
    if (current_hp > max_hp) current_hp = max_hp;
    if (current_mp > max_mp) current_mp = max_mp;
}

// ============================================================================
// Item Implementation
// ============================================================================

bool Item::is_stackable() const {
    // Items with ID < 100000 are generally stackable (consumables, materials)
    // This matches original KalOnline item categorization
    return item_id < 100000 && item_id > 0;
}

// ============================================================================
// Character Implementation
// ============================================================================

void Character::take_damage(uint32_t damage, uint32_t attacker_id) {
    if (damage >= stats.current_hp) {
        stats.current_hp = 0;
        LOG_DEBUG("Character {} died from attacker {}", name, attacker_id);
    } else {
        stats.current_hp -= damage;
    }
    attacker_id = attacker_id; // Track last attacker for aggro
}

void Character::heal(uint32_t amount) {
    stats.current_hp = std::min(stats.current_hp + amount, stats.max_hp);
    stats.current_mp = std::min(stats.current_mp + amount, stats.max_mp);
}

void Character::add_effect(const Effect& effect) {
    active_effects.push_back(effect);
    LOG_DEBUG("Effect {} added to character {}", effect.effect_id, name);
}

void Character::remove_effect(uint32_t effect_id) {
    std::erase_if(active_effects, [effect_id](const Effect& e) {
        return e.effect_id == effect_id;
    });
}

// ============================================================================
// CombatSystem Implementation - Core Combat Formulas
// ============================================================================

auto MainServer::calculate_base_damage(const Character& attacker) -> uint32_t {
    // Original KalOnline damage formula:
    // Base Damage = (Attack Power + Weapon Damage) * Skill Modifier
    
    uint32_t base_dmg = attacker.stats.attack_power;
    
    // Add weapon damage if equipped (slot 0 = main hand)
    if (const auto* player = dynamic_cast<const Player*>(&attacker)) {
        if (player->equipment[0].item_id != 0) {
            // Weapon damage would be loaded from item database
            // For now, use a placeholder based on item quality
            base_dmg += player->equipment[0].quality / 10;
        }
    }
    
    return base_dmg;
}

auto MainServer::calculate_defense_reduction(const Character& defender, 
                                              uint32_t raw_damage) -> uint32_t {
    // Original KalOnline defense formula:
    // Mitigation = Defense / (Defense + Attacker Level * 10)
    // Final Damage = Raw Damage * (1 - Mitigation)
    
    float defense = static_cast<float>(defender.stats.defense);
    float attacker_level = static_cast<float>(defender.stats.level);
    
    float mitigation = defense / (defense + attacker_level * 10.0f);
    
    // Cap mitigation at 75%
    mitigation = std::min(mitigation, 0.75f);
    
    uint32_t reduced_damage = static_cast<uint32_t>(raw_damage * (1.0f - mitigation));
    
    // Minimum 1 damage always
    return std::max(reduced_damage, 1u);
}

bool MainServer::calculate_hit_chance(const Character& attacker,
                                      const Character& defender) {
    // Original KalOnline hit formula:
    // Hit Chance = (Attacker Accuracy) / (Attacker Accuracy + Defender Evasion)
    // Base minimum hit chance: 5%, maximum: 95%
    
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_real_distribution<> dist(0.0f, 1.0f);
    
    float accuracy = static_cast<float>(attacker.stats.accuracy);
    float evasion = static_cast<float>(defender.stats.evasion);
    
    float hit_chance = accuracy / (accuracy + evasion);
    
    // Apply level difference modifier
    int level_diff = static_cast<int>(attacker.stats.level) - 
                     static_cast<int>(defender.stats.level);
    
    if (level_diff > 0) {
        hit_chance += 0.02f * level_diff; // +2% per level above
    } else if (level_diff < 0) {
        hit_chance -= 0.02f * std::abs(level_diff); // -2% per level below
    }
    
    // Clamp to 5%-95%
    hit_chance = std::clamp(hit_chance, 0.05f, 0.95f);
    
    return dist(gen) < hit_chance;
}

bool MainServer::calculate_critical(const Character& attacker) {
    // Critical chance = Critical Rate / 1000
    // Base crit rate is 50 (5%), can be increased by equipment/skills
    
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_int_distribution<> dist(0, 999);
    
    return dist(gen) < attacker.stats.critical_rate;
}

auto MainServer::calculate_damage(const Character& attacker, 
                                   const Character& defender,
                                   uint32_t skill_id) -> CombatResult {
    CombatResult result;
    
    // Step 1: Check if hit lands
    result.hit = calculate_hit_chance(attacker, defender);
    
    if (!result.hit) {
        result.result_code = 0; // Miss
        result.damage = 0;
        return result;
    }
    
    // Step 2: Calculate base damage
    uint32_t base_damage = calculate_base_damage(attacker);
    
    // Step 3: Apply skill multiplier if applicable
    float skill_multiplier = 1.0f;
    if (skill_id != 0) {
        // Skill damage multipliers would be loaded from skill database
        // Placeholder values based on typical MMORPG skill tiers
        if (skill_id < 100) skill_multiplier = 1.2f;      // Basic skills
        else if (skill_id < 200) skill_multiplier = 1.5f;  // Intermediate
        else if (skill_id < 300) skill_multiplier = 2.0f;  // Advanced
        else skill_multiplier = 2.5f;                       // Ultimate
    }
    
    base_damage = static_cast<uint32_t>(base_damage * skill_multiplier);
    
    // Step 4: Check for critical hit
    result.critical = calculate_critical(attacker);
    if (result.critical) {
        float crit_multiplier = attacker.stats.critical_damage / 100.0f;
        base_damage = static_cast<uint32_t>(base_damage * crit_multiplier);
    }
    
    // Step 5: Apply defense reduction
    result.damage = calculate_defense_reduction(defender, base_damage);
    
    // Step 6: Set result code
    if (result.critical) {
        result.result_code = 2; // Critical hit
    } else {
        result.result_code = 1; // Normal hit
    }
    
    LOG_DEBUG("Combat: {} vs {} - Hit={} Crit={} Dmg={}", 
              attacker.name, defender.name, 
              result.hit, result.critical, result.damage);
    
    return result;
}

void MainServer::process_attack(uint32_t attacker_id, uint32_t target_id, 
                                uint32_t skill_id) {
    auto attacker = get_character(attacker_id);
    auto target = get_character(target_id);
    
    if (!attacker || !target) {
        LOG_WARN("Invalid attack: attacker={} target={}", attacker_id, target_id);
        return;
    }
    
    if (attacker->is_dead() || target->is_dead()) {
        return; // Can't attack while dead
    }
    
    // Calculate distance - melee range check
    float dx = attacker->x - target->x;
    float dy = attacker->y - target->y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    // Melee range is typically 50 units in KalOnline
    if (distance > 50.0f && skill_id == 0) {
        LOG_DEBUG("Attack out of range: distance={}", distance);
        return;
    }
    
    // Calculate damage
    CombatResult result = calculate_damage(*attacker, *target, skill_id);
    
    // Apply damage
    if (result.hit) {
        target->take_damage(result.damage, attacker_id);
        
        // Update aggro for monsters
        if (auto* monster = dynamic_cast<Monster*>(target.get())) {
            monster->damage_dealt[attacker_id] += result.damage;
            monster->attacker_id = attacker_id;
        }
        
        // Send damage packet to nearby players
        Packet damage_packet(20);
        auto* ptr = reinterpret_cast<uint32_t*>(damage_packet.data());
        ptr[0] = 0x0501; // OpCode for damage notification
        ptr[1] = target_id;
        ptr[2] = attacker_id;
        ptr[3] = result.damage;
        ptr[4] = result.result_code; // 0=miss, 1=hit, 2=crit
        
        broadcast_to_nearby(target_id, damage_packet);
        
        // Check for death
        if (target->is_dead()) {
            handle_character_death(*target, attacker_id);
        }
    } else {
        // Send miss packet
        Packet miss_packet(12);
        auto* ptr = reinterpret_cast<uint32_t*>(miss_packet.data());
        ptr[0] = 0x0501;
        ptr[1] = target_id;
        ptr[2] = attacker_id;
        ptr[3] = 0; // damage
        ptr[4] = 0; // miss
        
        broadcast_to_nearby(target_id, miss_packet);
    }
}

void MainServer::process_skill(uint32_t caster_id, uint32_t target_id,
                               uint32_t skill_id, float target_x, float target_y) {
    auto caster = get_character(caster_id);
    
    if (!caster || caster->is_dead()) {
        return;
    }
    
    // Find skill in caster's skill list
    auto it = caster->skills.find(skill_id);
    if (it == caster->skills.end()) {
        LOG_WARN("Character {} doesn't have skill {}", caster_id, skill_id);
        return;
    }
    
    Skill& skill = it->second;
    uint32_t current_time = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count());
    
    // Check cooldown
    if (current_time < skill.cooldown_end) {
        LOG_DEBUG("Skill {} on cooldown", skill_id);
        return;
    }
    
    // Check MP cost (placeholder - would come from skill database)
    uint32_t mp_cost = skill.level * 5;
    if (caster->stats.current_mp < mp_cost) {
        LOG_DEBUG("Not enough MP for skill {}", skill_id);
        return;
    }
    
    caster->stats.current_mp -= mp_cost;
    
    // Set cooldown
    skill.cooldown_end = current_time + 1000; // 1 second base cooldown
    
    // Determine targets based on skill type
    std::vector<std::shared_ptr<Character>> targets;
    
    if (target_id != 0) {
        auto target = get_character(target_id);
        if (target) {
            targets.push_back(target);
        }
    } else {
        // Area of effect skills - get all characters in range
        float radius = 100.0f; // Default AoE radius
        targets = get_nearby_characters(caster_id, radius);
    }
    
    // Apply skill effect
    for (auto& target : targets) {
        bool is_hostile = (target->type == EntityType::Monster) ||
                         (target->type == EntityType::Player && 
                          target_id == target->id);
        
        if (is_hostile) {
            // Damage skill
            CombatResult result = calculate_damage(*caster, *target, skill_id);
            
            if (result.hit) {
                target->take_damage(result.damage, caster_id);
                
                LOG_INFO("Skill {} hit {} for {} damage", 
                        skill_id, target->name, result.damage);
                
                if (target->is_dead()) {
                    handle_character_death(*target, caster_id);
                }
            }
        } else {
            // Buff/heal skill
            uint32_t heal_amount = skill.effect_value;
            target->heal(heal_amount);
            
            LOG_DEBUG("Skill {} healed {} for {}", 
                     skill_id, target->name, heal_amount);
        }
    }
    
    // Increment skill usage counter
    m_stats.skills_used.fetch_add(1, std::memory_order_relaxed);
    
    // Broadcast skill animation
    Packet skill_packet(24);
    auto* ptr = reinterpret_cast<uint32_t*>(skill_packet.data());
    ptr[0] = 0x0502; // OpCode for skill cast
    ptr[1] = caster_id;
    ptr[2] = target_id;
    ptr[3] = skill_id;
    ptr[4] = static_cast<uint32_t>(target_x * 1000);
    ptr[5] = static_cast<uint32_t>(target_y * 1000);
    
    broadcast_to_nearby(caster_id, skill_packet);
}

void MainServer::handle_character_death(Character& deceased, uint32_t killer_id) {
    LOG_INFO("Character {} killed by {}", deceased.name, killer_id);
    
    if (auto* monster = dynamic_cast<Monster*>(&deceased)) {
        // Monster death - distribute rewards
        m_stats.monsters_killed.fetch_add(1, std::memory_order_relaxed);
        
        // Find top damage dealer
        uint32_t top_damager = 0;
        int64_t max_damage = 0;
        
        for (const auto& [damager_id, damage] : monster->damage_dealt) {
            if (damage > max_damage) {
                max_damage = damage;
                top_damager = damager_id;
            }
        }
        
        if (top_damager != 0) {
            auto killer = get_character(top_damager);
            if (killer && killer->type == EntityType::Player) {
                auto* player = static_cast<Player*>(killer.get());
                
                // Give experience
                player->stats.experience += monster->exp_reward;
                LOG_DEBUG("Player {} gained {} exp", player->name, monster->exp_reward);
                
                // Check for level up
                check_level_up(*player);
            }
        }
        
        // Spawn drop items
        spawn_drops(*monster);
        
        // Schedule respawn
        monster->death_time = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
    } else if (auto* player = dynamic_cast<Player*>(&deceased)) {
        // Player death - apply penalty
        LOG_INFO("Player {} died", player->name);
        
        // Drop random item (if not safe zone)
        // Lose some experience (optional - depends on server settings)
        
        // Respawn at nearest town
        // This would trigger teleport to spawn point
    }
}

void MainServer::check_level_up(Player& player) {
    // Experience table would be loaded from database
    // Placeholder formula: Exp needed = Level^3 * 100
    
    uint64_t exp_needed = static_cast<uint64_t>(player.stats.level) * 
                          player.stats.level * 
                          player.stats.level * 100;
    
    while (player.stats.experience >= exp_needed) {
        player.stats.experience -= exp_needed;
        player.stats.level++;
        
        LOG_INFO("Player {} leveled up to {}!", player.name, player.stats.level);
        
        // Stat points to allocate (original KalOnline gives points per level)
        // Players would manually allocate these through UI
        
        // Recalculate stats
        player.stats.recalculate();
        
        // Full heal on level up
        player.stats.current_hp = player.stats.max_hp;
        player.stats.current_mp = player.stats.max_mp;
        
        // Send level up packet
        Packet levelup_packet(16);
        auto* ptr = reinterpret_cast<uint32_t*>(levelup_packet.data());
        ptr[0] = 0x0401; // OpCode for level up
        ptr[1] = player.id;
        ptr[2] = player.stats.level;
        ptr[3] = static_cast<uint32_t>(player.stats.experience);
        
        send_to_player(player.id, levelup_packet);
        
        // Broadcast to nearby players
        Packet broadcast_packet(12);
        auto* bptr = reinterpret_cast<uint32_t*>(broadcast_packet.data());
        bptr[0] = 0x0402;
        bptr[1] = player.id;
        bptr[2] = player.stats.level;
        
        broadcast_to_nearby(player.id, broadcast_packet);
        
        // Calculate next level requirement
        exp_needed = static_cast<uint64_t>(player.stats.level) * 
                     player.stats.level * 
                     player.stats.level * 100;
    }
}

void MainServer::spawn_drops(const Monster& monster) {
    if (monster.drop_table.empty()) {
        return;
    }
    
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_int_distribution<> chance_dist(1, 100);
    
    uint32_t current_time = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count()
    );
    
    for (const auto& [item_id, chance_percent] : monster.drop_table) {
        if (chance_dist(gen) <= chance_percent) {
            DropItem drop;
            drop.id = generate_entity_id();
            drop.item_id = item_id;
            drop.count = 1;
            drop.x = monster.x + (static_cast<float>(chance_dist(gen) % 100) - 50.0f);
            drop.y = monster.y + (static_cast<float>(chance_dist(gen) % 100) - 50.0f);
            drop.z = monster.z;
            drop.map_id = monster.map_id;
            drop.owner_id = monster.id; // Monster ID as temporary owner
            drop.expire_time = current_time + 300000; // 5 minutes
            
            std::lock_guard lock(m_drops_mutex);
            m_ground_items[drop.id] = drop;
            
            m_stats.items_dropped.fetch_add(1, std::memory_order_relaxed);
            
            // Broadcast drop spawn
            Packet drop_packet(24);
            auto* ptr = reinterpret_cast<uint32_t*>(drop_packet.data());
            ptr[0] = 0x0601; // OpCode for item drop
            ptr[1] = drop.id;
            ptr[2] = drop.item_id;
            ptr[3] = drop.count;
            ptr[4] = static_cast<uint32_t>(drop.x * 1000);
            ptr[5] = static_cast<uint32_t>(drop.y * 1000);
            ptr[6] = static_cast<uint32_t>(drop.z * 1000);
            
            broadcast_to_nearby(drop.id, drop_packet);
        }
    }
}

uint32_t MainServer::generate_entity_id() {
    static std::atomic<uint32_t> next_id{1000};
    return next_id.fetch_add(1, std::memory_order_relaxed);
}

} // namespace kal::main
