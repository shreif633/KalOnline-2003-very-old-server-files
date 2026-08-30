#pragma once

#include "common/network/Network.hpp"
#include "common/database/Database.hpp"
#include "common/logger/Logger.hpp"
#include <asio.hpp>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>
#include <expected>
#include <span>

namespace kal::main {

using namespace network;
using namespace database;

// Entity types
enum class EntityType : uint8_t {
    Player = 0,
    Monster = 1,
    NPC = 2,
    Drop = 3,
    Projectile = 4
};

// Character stats structure (matches original KalOnline)
struct CharacterStats {
    uint32_t level{1};
    uint64_t experience{0};
    uint16_t strength{10};      // STR - affects attack power
    uint16_t health{10};         // HTH - affects HP
    uint16_t intelligence{10};   // INT - affects MP and magic damage
    uint16_t wisdom{10};         // WIS - affects MP regeneration
    uint16_t dexterity{10};      // DEX - affects accuracy and evasion
    
    uint32_t max_hp{100};
    uint32_t current_hp{100};
    uint32_t max_mp{50};
    uint32_t current_mp{50};
    
    uint16_t attack_power{10};
    uint16_t defense{5};
    uint16_t magic_attack{10};
    uint16_t magic_defense{5};
    uint16_t accuracy{10};
    uint16_t evasion{10};
    uint16_t critical_rate{5};    // Percentage * 10
    uint16_t critical_damage{150}; // Percentage (150 = 1.5x)
    
    void recalculate();
};

// Item structure
struct Item {
    uint32_t item_id{0};
    uint16_t count{1};
    int8_t slot{-1};           // -1 for inventory, 0-11 for equipment
    uint8_t location{0};       // 0: inventory, 1: equipped, 2: warehouse
    uint32_t prefix_id{0};     // Prefix/suffix modifier
    uint8_t quality{0};        // 0-100 quality percentage
    uint32_t durability{0};
    uint64_t socket_flags{0};  // For gem sockets
    std::array<uint32_t, 4> gems{};
    
    bool is_equipped() const { return location == 1; }
    bool is_stackable() const;
};

// Skill structure
struct Skill {
    uint32_t skill_id{0};
    uint8_t level{1};
    uint32_t cooldown_end{0};  // Timestamp in ms
    uint32_t duration{0};      // Duration in ms for buffs/DoTs
    uint32_t effect_value{0};  // Calculated effect value
};

// Buff/Debuff effect
struct Effect {
    uint32_t effect_id{0};
    uint32_t source_id{0};     // Caster entity ID
    uint32_t end_time{0};      // Absolute timestamp
    int32_t param_value{0};    // Effect magnitude
    uint8_t tick_interval{0};  // Seconds between ticks
    uint32_t last_tick{0};     // Last tick timestamp
    bool is_debuff{false};
};

// Base GameObject
struct GameObject {
    uint32_t id{0};
    EntityType type{EntityType::Player};
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    uint16_t map_id{0};
    uint16_t direction{0};     // 0-359 degrees
    bool is_active{true};
    uint32_t last_update{0};
    
    virtual ~GameObject() = default;
    virtual void update(uint32_t delta_ms) {}
};

// Character (base for Player, Monster, NPC)
struct Character : public GameObject {
    std::string name;
    CharacterStats stats;
    uint32_t target_id{0};
    uint32_t attacker_id{0};
    std::vector<Effect> active_effects;
    std::unordered_map<uint32_t, Skill> skills;
    
    bool is_dead() const { return stats.current_hp == 0; }
    void take_damage(uint32_t damage, uint32_t attacker_id);
    void heal(uint32_t amount);
    void add_effect(const Effect& effect);
    void remove_effect(uint32_t effect_id);
};

// Player specific data
struct Player : public Character {
    uint32_t account_id{0};
    uint32_t character_id{0};
    uint8_t job_class{0};      // 0: none, 1-8: different jobs
    uint8_t faction{0};        // 0: neutral, 1: Kang, 2: Mai
    uint64_t gold{0};
    uint64_t bank_gold{0};
    std::array<Item, 300> inventory{};  // 12x25 grid
    std::array<Item, 12> equipment{};   // Equipment slots
    std::array<Item, 100> warehouse{};  // Warehouse
    uint32_t guild_id{0};
    uint8_t guild_rank{0};
    uint32_t party_id{0};
    uint32_t quest_progress{0};
    std::vector<uint32_t> learned_skills;
    std::vector<uint32_t> friend_list;
    std::vector<uint32_t> block_list;
    
    ConnectionPtr connection;
    bool is_trading{false};
    uint32_t trade_partner{0};
    std::array<Item, 12> trade_items{};
    uint64_t trade_gold{0};
};

// Monster specific data
struct Monster : public Character {
    uint32_t monster_id{0};
    uint32_t spawn_point_id{0};
    float spawn_x{0};
    float spawn_y{0};
    float spawn_z{0};
    uint32_t respawn_time{0};  // milliseconds
    uint32_t death_time{0};
    uint8_t aggro_radius{50};
    uint8_t follow_radius{100};
    uint32_t exp_reward{0};
    std::vector<std::pair<uint32_t, uint8_t>> drop_table;  // item_id, chance%
    std::unordered_map<uint32_t, int64_t> damage_dealt;    // entity_id -> damage
};

// NPC specific data
struct NPC : public Character {
    uint32_t npc_id{0};
    uint8_t npc_type{0};  // 0: general, 1: merchant, 2: skill trainer, etc.
    uint32_t shop_items[24]{};
    uint8_t shop_count{0};
};

// Drop item on ground
struct DropItem : public GameObject {
    uint32_t item_id{0};
    uint16_t count{1};
    uint32_t owner_id{0};    // 0 = FFA, else only that player can pick up
    uint32_t expire_time{0}; // Timestamp when it disappears
};

// Combat result
struct CombatResult {
    bool hit{false};
    bool critical{false};
    uint32_t damage{0};
    uint8_t result_code{0};  // 0: miss, 1: hit, 2: critical, 3: dodge
};

// Main Game Server
class MainServer {
public:
    explicit MainServer(asio::io_context& io_context, uint16_t port,
                       std::shared_ptr<DatabasePool> db_pool);
    ~MainServer();
    
    void start();
    void stop();
    
    // Packet handling
    void on_packet_received(ConnectionPtr conn, const Packet& packet);
    
    // World management
    std::shared_ptr<Player> get_player(uint32_t entity_id) const;
    std::shared_ptr<Character> get_character(uint32_t entity_id) const;
    std::vector<std::shared_ptr<Character>> get_nearby_characters(
        uint32_t entity_id, float radius) const;
    
    // Entity management
    std::expected<uint32_t, std::string> create_player(
        ConnectionPtr conn, const std::string& name);
    void remove_player(uint32_t entity_id);
    void spawn_monster(uint32_t monster_id, float x, float y, float z, 
                      uint16_t map_id);
    
    // Combat system
    CombatResult calculate_damage(const Character& attacker, 
                                  const Character& defender,
                                  uint32_t skill_id = 0);
    void process_attack(uint32_t attacker_id, uint32_t target_id, 
                       uint32_t skill_id = 0);
    void process_skill(uint32_t caster_id, uint32_t target_id,
                      uint32_t skill_id, float target_x, float target_y);
    
    // Movement
    void process_movement(uint32_t entity_id, float x, float y, float z,
                         uint16_t direction);
    bool validate_movement(uint32_t entity_id, float old_x, float old_y,
                          float new_x, float new_y, uint32_t delta_ms);
    
    // Database operations
    std::expected<std::shared_ptr<Player>, std::string> load_player(
        uint32_t character_id);
    bool save_player(const Player& player);
    
    // Statistics
    struct Stats {
        std::atomic<uint32_t> active_players{0};
        std::atomic<uint32_t> total_logins{0};
        std::atomic<uint64_t> monsters_killed{0};
        std::atomic<uint64_t> items_dropped{0};
        std::atomic<uint64_t> skills_used{0};
        std::chrono::system_clock::time_point start_time;
    };
    
    const Stats& get_stats() const { return m_stats; }
    
private:
    // Packet handlers
    void handle_login(ConnectionPtr conn, const Packet& packet);
    void handle_character_select(ConnectionPtr conn, const Packet& packet);
    void handle_character_create(ConnectionPtr conn, const Packet& packet);
    void handle_movement(ConnectionPtr conn, const Packet& packet);
    void handle_attack(ConnectionPtr conn, const Packet& packet);
    void handle_skill(ConnectionPtr conn, const Packet& packet);
    void handle_chat(ConnectionPtr conn, const Packet& packet);
    void handle_item_use(ConnectionPtr conn, const Packet& packet);
    void handle_equip_item(ConnectionPtr conn, const Packet& packet);
    void handle_drop_item(ConnectionPtr conn, const Packet& packet);
    void handle_pickup_item(ConnectionPtr conn, const Packet& packet);
    void handle_npc_talk(ConnectionPtr conn, const Packet& packet);
    void handle_shop_buy(ConnectionPtr conn, const Packet& packet);
    void handle_shop_sell(ConnectionPtr conn, const Packet& packet);
    void handle_quest_update(ConnectionPtr conn, const Packet& packet);
    void handle_party_invite(ConnectionPtr conn, const Packet& packet);
    void handle_guild_action(ConnectionPtr conn, const Packet& packet);
    void handle_trade_action(ConnectionPtr conn, const Packet& packet);
    void handle_mail_send(ConnectionPtr conn, const Packet& packet);
    void handle_logout(ConnectionPtr conn, const Packet& packet);
    
    // Internal helpers
    void broadcast_to_nearby(uint32_t entity_id, const Packet& packet);
    void send_to_player(uint32_t entity_id, const Packet& packet);
    void update_character_stats(Character& character);
    void process_effects(Character& character, uint32_t current_time);
    void check_respawn_monsters();
    
    // Combat formulas (preserving original KalOnline logic)
    uint32_t calculate_base_damage(const Character& attacker);
    uint32_t calculate_defense_reduction(const Character& defender, 
                                         uint32_t raw_damage);
    bool calculate_hit_chance(const Character& attacker,
                             const Character& defender);
    bool calculate_critical(const Character& attacker);
    
    asio::io_context& m_io_context;
    TcpServer m_server;
    std::shared_ptr<DatabasePool> m_db_pool;
    
    mutable std::mutex m_entities_mutex;
    std::unordered_map<uint32_t, std::shared_ptr<GameObject>> m_entities;
    std::unordered_map<uint32_t, std::shared_ptr<Player>> m_players;
    std::unordered_map<ConnectionPtr, uint32_t> m_connection_to_player;
    
    std::mutex m_drops_mutex;
    std::unordered_map<uint32_t, DropItem> m_ground_items;
    
    std::atomic<bool> m_running{false};
    asio::steady_timer m_world_timer;
    asio::steady_timer m_cleanup_timer;
    
    Stats m_stats;
    
    static constexpr uint32_t WORLD_TICK_MS = 100;  // 10ms world tick
    static constexpr uint32_t CLEANUP_INTERVAL_S = 60;
};

} // namespace kal::main
