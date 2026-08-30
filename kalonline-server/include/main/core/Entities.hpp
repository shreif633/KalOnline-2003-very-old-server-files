#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/vec3.hpp>

namespace kal::main {

// Entity types
enum class EntityType : uint8_t {
    None = 0,
    Player = 1,
    Monster = 2,
    NPC = 3,
    DropItem = 4,
    Projectile = 5
};

// Character states
enum class CharacterState : uint8_t {
    Idle = 0,
    Walking = 1,
    Running = 2,
    Attacking = 3,
    Casting = 4,
    Sitting = 5,
    Dead = 6,
    Stunned = 7
};

// Base entity class
class Entity {
public:
    Entity(uint32_t id, EntityType type);
    virtual ~Entity() = default;
    
    uint32_t id() const { return id_; }
    EntityType type() const { return type_; }
    
    // Position
    glm::vec3 position() const { return position_; }
    void set_position(const glm::vec3& pos) { position_ = pos; }
    void set_position(float x, float y, float z) { position_ = {x, y, z}; }
    
    uint16_t map_id() const { return map_id_; }
    void set_map_id(uint16_t map_id) { map_id_ = map_id; }
    
    // State
    CharacterState state() const { return state_; }
    void set_state(CharacterState state) { state_ = state; }
    
    bool is_alive() const { return current_hp_ > 0; }
    bool is_dead() const { return current_hp_ <= 0; }
    
    // Movement
    bool is_moving() const { return is_moving_; }
    void set_moving(bool moving) { is_moving_ = moving; }
    
    glm::vec3 movement_target() const { return movement_target_; }
    void set_movement_target(const glm::vec3& target) { movement_target_ = target; }
    
    float movement_speed() const { return movement_speed_; }
    void set_movement_speed(float speed) { movement_speed_ = speed; }
    
    // Update
    virtual void update(uint64_t delta_ms);
    
protected:
    uint32_t id_;
    EntityType type_;
    glm::vec3 position_{0.0f, 0.0f, 0.0f};
    uint16_t map_id_{0};
    CharacterState state_{CharacterState::Idle};
    uint32_t current_hp_{100};
    uint32_t max_hp_{100};
    bool is_moving_{false};
    glm::vec3 movement_target_{0.0f, 0.0f, 0.0f};
    float movement_speed_{5.0f}; // meters per second
};

// Character base class (Player, Monster, NPC)
class Character : public Entity {
public:
    Character(uint32_t id, EntityType type);
    ~Character() override = default;
    
    // Stats
    uint8_t level() const { return level_; }
    void set_level(uint8_t level) { level_ = level; }
    
    uint16_t strength() const { return strength_; }
    void set_strength(uint16_t str) { strength_ = str; }
    
    uint16_t health() const { return health_; }
    void set_health(uint16_t hth) { health_ = hth; }
    
    uint16_t intelligence() const { return intelligence_; }
    void set_intelligence(uint16_t intl) { intelligence_ = intl; }
    
    uint16_t wisdom() const { return wisdom_; }
    void set_wisdom(uint16_t wis) { wisdom_ = wis; }
    
    uint16_t dexterity() const { return dexterity_; }
    void set_dexterity(uint16_t dex) { dexterity_ = dex; }
    
    uint32_t experience() const { return experience_; }
    void set_experience(uint32_t exp) { experience_ = exp; }
    
    uint32_t gold() const { return gold_; }
    void set_gold(uint32_t gold) { gold_ = gold; }
    void add_gold(int32_t amount);
    
    // HP/MP
    uint32_t current_hp() const { return current_hp_; }
    uint32_t max_hp() const { return max_hp_; }
    void set_max_hp(uint32_t hp) { max_hp_ = hp; }
    void set_current_hp(uint32_t hp) { current_hp_ = std::min(hp, max_hp_); }
    
    uint32_t current_mp() const { return current_mp_; }
    uint32_t max_mp() const { return max_mp_; }
    void set_max_mp(uint32_t mp) { max_mp_ = mp; }
    void set_current_mp(uint32_t mp) { current_mp_ = std::min(mp, max_mp_); }
    
    // Combat stats
    uint16_t attack_power() const { return attack_power_; }
    uint16_t defense() const { return defense_; }
    uint16_t magic_attack() const { return magic_attack_; }
    uint16_t magic_defense() const { return magic_defense_; }
    uint8_t hit_rate() const { return hit_rate_; }
    uint8_t dodge_rate() const { return dodge_rate_; }
    uint8_t critical_rate() const { return critical_rate_; }
    
    void recalculate_stats();
    
    // Combat operations
    virtual uint32_t calculate_damage_to(const Character* target) const;
    virtual bool can_attack(const Character* target) const;
    
    bool take_damage(uint32_t damage, uint32_t attacker_id = 0);
    bool heal(uint32_t amount);
    bool consume_mp(uint32_t amount);
    
    void die(uint32_t killer_id = 0);
    void respawn();
    
    // Aggro
    void add_aggro(uint32_t source_id, int32_t amount);
    void clear_aggro();
    uint32_t get_top_aggro_target() const;
    
protected:
    uint8_t level_{1};
    uint16_t strength_{10};
    uint16_t health_{10};
    uint16_t intelligence_{10};
    uint16_t wisdom_{10};
    uint16_t dexterity_{10};
    uint32_t experience_{0};
    uint32_t gold_{0};
    
    uint32_t current_mp_{50};
    uint32_t max_mp_{50};
    
    uint16_t attack_power_{10};
    uint16_t defense_{5};
    uint16_t magic_attack_{10};
    uint16_t magic_defense_{5};
    uint8_t hit_rate_{50};
    uint8_t dodge_rate_{10};
    uint8_t critical_rate_{5};
    
    uint32_t killer_id_{0};
    std::unordered_map<uint32_t, int32_t> aggro_table_;
};

// Player entity
class Player : public Character {
public:
    Player(uint32_t id);
    ~Player() override = default;
    
    const std::string& name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }
    
    uint32_t user_id() const { return user_id_; }
    void set_user_id(uint32_t user_id) { user_id_ = user_id_; }
    
    uint8_t hair_style() const { return hair_style_; }
    void set_hair_style(uint8_t style) { hair_style_ = style; }
    
    uint8_t face_style() const { return face_style_; }
    void set_face_style(uint8_t style) { face_style_ = style; }
    
    uint8_t costume_id() const { return costume_id_; }
    void set_costume_id(uint8_t costume) { costume_id_ = costume; }
    
    // Equipment
    struct Equipment {
        uint32_t helmet{0};
        uint32_t armor{0};
        uint32_t pants{0};
        uint32_t gloves{0};
        uint32_t boots{0};
        uint32_t weapon{0};
        uint32_t shield{0};
        uint32_t accessory1{0};
        uint32_t accessory2{0};
        uint32_t cloak{0};
    };
    
    const Equipment& equipment() const { return equipment_; }
    void equip_item(uint8_t slot, uint32_t item_id);
    void unequip_item(uint8_t slot);
    
    // Party
    uint32_t party_id() const { return party_id_; }
    void set_party_id(uint32_t party_id) { party_id_ = party_id; }
    
    // Guild
    uint32_t guild_id() const { return guild_id_; }
    void set_guild_id(uint32_t guild_id) { guild_id_ = guild_id; }
    
    // Friend list
    const std::vector<uint32_t>& friends() const { return friends_; }
    void add_friend(uint32_t player_id);
    void remove_friend(uint32_t player_id);
    bool is_friend(uint32_t player_id) const;
    
    // Block list
    const std::vector<uint32_t>& blocks() const { return blocks_; }
    void add_block(uint32_t player_id);
    void remove_block(uint32_t player_id);
    bool is_blocked(uint32_t player_id) const;
    
private:
    std::string name_;
    uint32_t user_id_{0};
    uint8_t hair_style_{0};
    uint8_t face_style_{0};
    uint8_t costume_id_{0};
    
    Equipment equipment_;
    uint32_t party_id_{0};
    uint32_t guild_id_{0};
    
    std::vector<uint32_t> friends_;
    std::vector<uint32_t> blocks_;
};

// Monster entity
class Monster : public Character {
public:
    Monster(uint32_t id, uint32_t monster_type_id);
    ~Monster() override = default;
    
    uint32_t monster_type_id() const { return monster_type_id_; }
    
    uint32_t exp_reward() const { return exp_reward_; }
    uint32_t gold_reward() const { return gold_reward_; }
    
    // AI state
    enum class AIState {
        Idle,
        Patrol,
        Chase,
        Attack,
        Return,
        Dead
    };
    
    AIState ai_state() const { return ai_state_; }
    void set_ai_state(AIState state) { ai_state_ = state; }
    
    float aggro_range() const { return aggro_range_; }
    void set_aggro_range(float range) { aggro_range_ = range; }
    
    float chase_range() const { return chase_range_; }
    void set_chase_range(float range) { chase_range_ = range; }
    
    glm::vec3 spawn_position() const { return spawn_position_; }
    void set_spawn_position(const glm::vec3& pos) { spawn_position_ = pos; }
    
    float patrol_radius() const { return patrol_radius_; }
    void set_patrol_radius(float radius) { patrol_radius_ = radius; }
    
    void update(uint64_t delta_ms) override;
    
    void on_player_entered_range(uint32_t player_id);
    void on_player_left_range(uint32_t player_id);
    
private:
    uint32_t monster_type_id_;
    uint32_t exp_reward_{10};
    uint32_t gold_reward_{5};
    
    AIState ai_state_{AIState::Idle};
    float aggro_range_{10.0f};
    float chase_range_{50.0f};
    glm::vec3 spawn_position_{0.0f, 0.0f, 0.0f};
    float patrol_radius_{5.0f};
    
    uint64_t last_attack_time_{0};
    uint64_t last_patrol_update_{0};
    std::vector<uint32_t> players_in_range_;
};

// NPC entity
class NPC : public Character {
public:
    NPC(uint32_t id, uint32_t npc_type_id);
    ~NPC() override = default;
    
    uint32_t npc_type_id() const { return npc_type_id_; }
    
    enum class NPCType {
        Standard,
        Shopkeeper,
        QuestGiver,
        Teleporter,
        WarehouseKeeper,
        Blacksmith,
        Healer,
        GuildMaster
    };
    
    NPCType npc_type() const { return npc_type_; }
    void set_npc_type(NPCType type) { npc_type_ = type; }
    
    // Shop items (for shopkeepers)
    struct ShopItem {
        uint32_t item_id;
        uint16_t price;
        uint16_t stock; // 0 = unlimited
    };
    
    const std::vector<ShopItem>& shop_items() const { return shop_items_; }
    void add_shop_item(uint32_t item_id, uint16_t price, uint16_t stock = 0);
    
    // Teleporter destinations
    struct TeleportDestination {
        uint16_t map_id;
        float x, y, z;
        uint16_t cost;
        std::string name;
    };
    
    const std::vector<TeleportDestination>& teleport_destinations() const {
        return teleport_destinations_;
    }
    void add_teleport_destination(uint16_t map_id, float x, float y, float z, 
                                  uint16_t cost, const std::string& name);
    
private:
    uint32_t npc_type_id_;
    NPCType npc_type_{NPCType::Standard};
    std::vector<ShopItem> shop_items_;
    std::vector<TeleportDestination> teleport_destinations_;
};

} // namespace kal::main
