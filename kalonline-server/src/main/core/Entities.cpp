#include "Entities.hpp"
#include <algorithm>
#include <cmath>
#include "Logger.hpp"

namespace kal::main {

// Entity implementation
Entity::Entity(uint32_t id, EntityType type)
    : id_(id), type_(type) {}

void Entity::update(uint64_t delta_ms) {
    if (!is_moving_) return;
    
    // Calculate movement
    float distance = glm::distance(position_, movement_target_);
    float move_distance = movement_speed_ * (delta_ms / 1000.0f);
    
    if (move_distance >= distance) {
        // Reached target
        position_ = movement_target_;
        is_moving_ = false;
        set_state(CharacterState::Idle);
    } else {
        // Move towards target
        glm::vec3 direction = glm::normalize(movement_target_ - position_);
        position_ += direction * move_distance;
    }
}

// Character implementation
Character::Character(uint32_t id, EntityType type)
    : Entity(id, type) {
    recalculate_stats();
}

void Character::recalculate_stats() {
    // Base HP from health stat
    max_hp_ = 50 + (health_ * 10);
    current_hp_ = std::min(current_hp_, max_hp_);
    
    // Base MP from wisdom stat
    max_mp_ = 30 + (wisdom_ * 8);
    current_mp_ = std::min(current_mp_, max_mp_);
    
    // Attack power from strength
    attack_power_ = strength_ + (level_ * 2);
    
    // Defense from health and level
    defense_ = health_ / 2 + (level_ * 3);
    
    // Magic attack from intelligence
    magic_attack_ = intelligence_ + (level_ * 2);
    
    // Magic defense from wisdom and level
    magic_defense_ = wisdom_ / 2 + (level_ * 2);
    
    // Hit rate from dexterity
    hit_rate_ = 50 + (dexterity_ / 2);
    
    // Dodge rate from dexterity and level
    dodge_rate_ = 10 + (dexterity_ / 3);
    
    // Critical rate from dexterity
    critical_rate_ = 5 + (dexterity_ / 5);
}

void Character::add_gold(int32_t amount) {
    if (amount > 0) {
        gold_ += amount;
    } else {
        gold_ = std::max<int32_t>(0, static_cast<int32_t>(gold_) + amount);
    }
}

uint32_t Character::calculate_damage_to(const Character* target) const {
    if (!target) return 0;
    
    // Base damage
    float base_damage = static_cast<float>(attack_power_);
    
    // Apply defense reduction
    float defense_factor = 100.0f / (100.0f + target->defense_);
    float reduced_damage = base_damage * defense_factor;
    
    // Random variance (90% - 110%)
    float variance = 0.9f + (static_cast<float>(rand()) / RAND_MAX * 0.2f);
    reduced_damage *= variance;
    
    // Critical hit check
    bool is_critical = (rand() % 100) < critical_rate_;
    if (is_critical) {
        reduced_damage *= 1.5f;
        LOG_DEBUG("Critical hit! Damage: {}", static_cast<int>(reduced_damage));
    }
    
    // Hit/miss check
    int hit_chance = hit_rate_ - target->dodge_rate_;
    hit_chance = std::clamp(hit_chance, 20, 80); // Min 20%, max 80%
    
    if ((rand() % 100) >= hit_chance) {
        return 0; // Miss
    }
    
    return static_cast<uint32_t>(std::max(1.0f, reduced_damage));
}

bool Character::can_attack(const Character* target) const {
    if (!target || !target->is_alive()) return false;
    if (!is_alive()) return false;
    
    // Can't attack same entity
    if (id_ == target->id_) return false;
    
    return true;
}

bool Character::take_damage(uint32_t damage, uint32_t attacker_id) {
    if (!is_alive()) return false;
    
    current_hp_ = (damage >= current_hp_) ? 0 : (current_hp_ - damage);
    
    if (attacker_id != 0) {
        // Add aggro to attacker
        add_aggro(attacker_id, static_cast<int32_t>(damage));
    }
    
    if (current_hp_ == 0) {
        die(attacker_id);
        return true; // Died
    }
    
    return false; // Still alive
}

bool Character::heal(uint32_t amount) {
    if (!is_alive()) return false;
    
    uint32_t old_hp = current_hp_;
    current_hp_ = std::min(current_hp_ + amount, max_hp_);
    
    return current_hp_ > old_hp;
}

bool Character::consume_mp(uint32_t amount) {
    if (current_mp_ < amount) return false;
    
    current_mp_ -= amount;
    return true;
}

void Character::die(uint32_t killer_id) {
    killer_id_ = killer_id;
    set_state(CharacterState::Dead);
    clear_aggro();
    
    LOG_INFO("Entity {} died (killer: {})", id_, killer_id);
}

void Character::respawn() {
    current_hp_ = max_hp_;
    current_mp_ = max_mp_;
    set_state(CharacterState::Idle);
    killer_id_ = 0;
}

void Character::add_aggro(uint32_t source_id, int32_t amount) {
    aggro_table_[source_id] += amount;
}

void Character::clear_aggro() {
    aggro_table_.clear();
}

uint32_t Character::get_top_aggro_target() const {
    uint32_t top_target = 0;
    int32_t top_aggro = 0;
    
    for (const auto& [source_id, aggro] : aggro_table_) {
        if (aggro > top_aggro) {
            top_aggro = aggro;
            top_target = source_id;
        }
    }
    
    return top_target;
}

// Player implementation
Player::Player(uint32_t id)
    : Character(id, EntityType::Player) {}

void Player::equip_item(uint8_t slot, uint32_t item_id) {
    switch (slot) {
        case 0: equipment_.helmet = item_id; break;
        case 1: equipment_.armor = item_id; break;
        case 2: equipment_.pants = item_id; break;
        case 3: equipment_.gloves = item_id; break;
        case 4: equipment_.boots = item_id; break;
        case 5: equipment_.weapon = item_id; break;
        case 6: equipment_.shield = item_id; break;
        case 7: equipment_.accessory1 = item_id; break;
        case 8: equipment_.accessory2 = item_id; break;
        case 9: equipment_.cloak = item_id; break;
    }
    
    recalculate_stats();
}

void Player::unequip_item(uint8_t slot) {
    switch (slot) {
        case 0: equipment_.helmet = 0; break;
        case 1: equipment_.armor = 0; break;
        case 2: equipment_.pants = 0; break;
        case 3: equipment_.gloves = 0; break;
        case 4: equipment_.boots = 0; break;
        case 5: equipment_.weapon = 0; break;
        case 6: equipment_.shield = 0; break;
        case 7: equipment_.accessory1 = 0; break;
        case 8: equipment_.accessory2 = 0; break;
        case 9: equipment_.cloak = 0; break;
    }
    
    recalculate_stats();
}

void Player::add_friend(uint32_t player_id) {
    if (!is_friend(player_id)) {
        friends_.push_back(player_id);
    }
}

void Player::remove_friend(uint32_t player_id) {
    friends_.erase(
        std::remove(friends_.begin(), friends_.end(), player_id),
        friends_.end());
}

bool Player::is_friend(uint32_t player_id) const {
    return std::find(friends_.begin(), friends_.end(), player_id) != friends_.end();
}

void Player::add_block(uint32_t player_id) {
    if (!is_blocked(player_id)) {
        blocks_.push_back(player_id);
    }
}

void Player::remove_block(uint32_t player_id) {
    blocks_.erase(
        std::remove(blocks_.begin(), blocks_.end(), player_id),
        blocks_.end());
}

bool Player::is_blocked(uint32_t player_id) const {
    return std::find(blocks_.begin(), blocks_.end(), player_id) != blocks_.end();
}

// Monster implementation
Monster::Monster(uint32_t id, uint32_t monster_type_id)
    : Character(id, EntityType::Monster)
    , monster_type_id_(monster_type_id) {
    set_spawn_position(position_);
}

void Monster::update(uint64_t delta_ms) {
    Character::update(delta_ms);
    
    if (ai_state_ == AIState::Dead) return;
    
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    switch (ai_state_) {
        case AIState::Idle:
            // Check for players in aggro range
            if (!players_in_range_.empty()) {
                ai_state_ = AIState::Chase;
            }
            break;
            
        case AIState::Patrol:
            // Patrol logic would go here
            if (now - last_patrol_update_ > 5000) {
                last_patrol_update_ = now;
                // Pick new patrol point within radius
            }
            break;
            
        case AIState::Chase: {
            // Chase top aggro target
            uint32_t target_id = get_top_aggro_target();
            if (target_id == 0 || players_in_range_.empty()) {
                ai_state_ = AIState::Return;
                break;
            }
            
            float dist = glm::distance(position_, movement_target_);
            if (dist <= 2.0f) {
                // In melee range, start attacking
                ai_state_ = AIState::Attack;
            }
            break;
        }
        
        case AIState::Attack: {
            uint32_t target_id = get_top_aggro_target();
            if (target_id == 0) {
                ai_state_ = AIState::Return;
                break;
            }
            
            // Attack cooldown (e.g., 2 seconds)
            if (now - last_attack_time_ >= 2000) {
                last_attack_time_ = now;
                // Perform attack on target
                LOG_DEBUG("Monster {} attacks target {}", id_, target_id);
            }
            break;
        }
        
        case AIState::Return:
            // Return to spawn position
            if (glm::distance(position_, spawn_position_) < 1.0f) {
                ai_state_ = AIState::Idle;
                clear_aggro();
            }
            break;
            
        case AIState::Dead:
            // Respawn timer would go here
            break;
    }
}

void Monster::on_player_entered_range(uint32_t player_id) {
    if (std::find(players_in_range_.begin(), players_in_range_.end(), player_id) 
        == players_in_range_.end()) {
        players_in_range_.push_back(player_id);
        
        if (ai_state_ == AIState::Idle || ai_state_ == AIState::Patrol) {
            ai_state_ = AIState::Chase;
        }
    }
}

void Monster::on_player_left_range(uint32_t player_id) {
    players_in_range_.erase(
        std::remove(players_in_range_.begin(), players_in_range_.end(), player_id),
        players_in_range_.end());
    
    if (players_in_range_.empty() && ai_state_ == AIState::Chase) {
        ai_state_ = AIState::Return;
    }
}

// NPC implementation
NPC::NPC(uint32_t id, uint32_t npc_type_id)
    : Character(id, EntityType::NPC)
    , npc_type_id_(npc_type_id) {}

void NPC::add_shop_item(uint32_t item_id, uint16_t price, uint16_t stock) {
    shop_items_.push_back({item_id, price, stock});
}

void NPC::add_teleport_destination(uint16_t map_id, float x, float y, float z,
                                   uint16_t cost, const std::string& name) {
    teleport_destinations_.push_back({map_id, x, y, z, cost, name});
}

} // namespace kal::main
