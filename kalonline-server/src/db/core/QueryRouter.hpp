#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <expected>
#include "Database.hpp"

namespace kal::db {

// Query types supported by DB Server
enum class QueryType : uint16_t {
    // Character operations
    LoadCharacter = 1,
    SaveCharacter = 2,
    DeleteCharacter = 3,
    CreateCharacter = 4,
    ListCharacters = 5,
    
    // Inventory operations
    LoadInventory = 10,
    SaveInventory = 11,
    AddItem = 12,
    RemoveItem = 13,
    MoveItem = 14,
    EquipItem = 15,
    UnequipItem = 16,
    
    // Skill operations
    LoadSkills = 20,
    SaveSkills = 21,
    LearnSkill = 22,
    UnlearnSkill = 23,
    
    // Quest operations
    LoadQuests = 30,
    SaveQuests = 31,
    UpdateQuest = 32,
    CompleteQuest = 33,
    
    // Guild operations
    CreateGuild = 40,
    DisbandGuild = 41,
    JoinGuild = 42,
    LeaveGuild = 43,
    KickFromGuild = 44,
    PromoteMember = 45,
    DemoteMember = 46,
    SetGuildNotice = 47,
    
    // Social operations
    AddFriend = 50,
    RemoveFriend = 51,
    AddBlock = 52,
    RemoveBlock = 53,
    SendMail = 54,
    ReadMail = 55,
    DeleteMail = 56,
    
    // World operations
    GetTeleporterList = 60,
    GetNPCList = 61,
    GetMonsterInfo = 62,
    GetItemInfo = 63,
    GetSkillInfo = 64,
    
    // Economy
    GetShopItems = 70,
    BuyItem = 71,
    SellItem = 72,
    GetWarehouseItems = 73,
    StoreWarehouseItem = 74,
    RetrieveWarehouseItem = 75,
    
    // Admin
    BanPlayer = 100,
    UnbanPlayer = 101,
    MutePlayer = 102,
    UnmutePlayer = 103,
    GMCommand = 104
};

// Request structure
struct QueryRequest {
    QueryType type;
    uint32_t request_id;
    uint32_t user_id;
    uint32_t character_id;
    std::vector<uint8_t> payload;
};

// Response variants
struct CharacterData {
    uint32_t id;
    std::string name;
    uint8_t level;
    uint16_t strength;
    uint16_t health;
    uint16_t intelligence;
    uint16_t wisdom;
    uint16_t dexterity;
    uint32_t experience;
    uint32_t gold;
    uint16_t map_id;
    float position_x;
    float position_y;
    float position_z;
    uint8_t hair_style;
    uint8_t face_style;
    uint8_t costume_id;
};

struct ItemData {
    uint32_t id;
    uint32_t item_type_id;
    uint8_t slot;
    uint8_t location; // 0=inventory, 1=equipped, 2=warehouse
    int16_t quantity;
    int16_t durability;
    uint32_t flags;
    std::vector<uint8_t> socket_data;
    std::string prefix_name;
};

struct SkillData {
    uint32_t skill_id;
    uint8_t level;
    uint64_t cooldown_end;
};

struct QuestData {
    uint32_t quest_id;
    uint8_t state; // 0=not started, 1=in progress, 2=completed
    std::vector<uint8_t> objectives;
    uint64_t start_time;
    uint64_t complete_time;
};

struct MailData {
    uint32_t id;
    std::string sender;
    std::string recipient;
    std::string subject;
    std::string body;
    uint64_t sent_time;
    bool is_read;
    bool has_item;
    uint32_t item_id;
    int16_t item_quantity;
};

struct GuildData {
    uint32_t id;
    std::string name;
    uint32_t leader_id;
    std::string notice;
    uint8_t level;
    uint32_t experience;
    uint32_t member_count;
};

using QueryResponse = std::variant<
    std::monostate,              // Empty/None
    bool,                        // Simple success/failure
    CharacterData,               // Character data
    std::vector<CharacterData>,  // Character list
    std::vector<ItemData>,       // Inventory
    std::vector<SkillData>,      // Skills
    std::vector<QuestData>,      // Quests
    std::vector<MailData>,       // Mail
    GuildData,                   // Guild info
    std::vector<uint8_t>         // Raw binary data
>;

// Result codes
enum class QueryResult : uint8_t {
    Success = 0,
    NotFound = 1,
    DatabaseError = 2,
    InvalidParameter = 3,
    AlreadyExists = 4,
    PermissionDenied = 5,
    InventoryFull = 6,
    InsufficientFunds = 7,
    LevelTooLow = 8,
    CooldownActive = 9,
    UnknownError = 255
};

struct QueryResultWrapper {
    QueryResult result;
    QueryResponse data;
    std::string error_message;
};

class QueryRouter {
public:
    static QueryRouter& instance();
    
    // Process a query request
    QueryResultWrapper process_request(const QueryRequest& request);
    
    // Individual query handlers
    QueryResultWrapper handle_load_character(const QueryRequest& request);
    QueryResultWrapper handle_save_character(const QueryRequest& request);
    QueryResultWrapper handle_create_character(const QueryRequest& request);
    QueryResultWrapper handle_delete_character(const QueryRequest& request);
    QueryResultWrapper handle_list_characters(const QueryRequest& request);
    
    QueryResultWrapper handle_load_inventory(const QueryRequest& request);
    QueryResultWrapper handle_save_inventory(const QueryRequest& request);
    QueryResultWrapper handle_add_item(const QueryRequest& request);
    QueryResultWrapper handle_remove_item(const QueryRequest& request);
    QueryResultWrapper handle_move_item(const QueryRequest& request);
    QueryResultWrapper handle_equip_item(const QueryRequest& request);
    QueryResultWrapper handle_unequip_item(const QueryRequest& request);
    
    QueryResultWrapper handle_load_skills(const QueryRequest& request);
    QueryResultWrapper handle_save_skills(const QueryRequest& request);
    QueryResultWrapper handle_learn_skill(const QueryRequest& request);
    
    QueryResultWrapper handle_load_quests(const QueryRequest& request);
    QueryResultWrapper handle_save_quests(const QueryRequest& request);
    QueryResultWrapper handle_update_quest(const QueryRequest& request);
    
    QueryResultWrapper handle_send_mail(const QueryRequest& request);
    QueryResultWrapper handle_read_mail(const QueryRequest& request);
    QueryResultWrapper handle_delete_mail(const QueryRequest& request);
    
    QueryResultWrapper handle_create_guild(const QueryRequest& request);
    QueryResultWrapper handle_join_guild(const QueryRequest& request);
    QueryResultWrapper handle_leave_guild(const QueryRequest& request);
    
    QueryResultWrapper handle_get_teleporter_list(const QueryRequest& request);
    QueryResultWrapper handle_get_npc_list(const QueryRequest& request);
    QueryResultWrapper handle_get_monster_info(const QueryRequest& request);
    QueryResultWrapper handle_get_item_info(const QueryRequest& request);
    
private:
    QueryRouter() = default;
    ~QueryRouter() = default;
    QueryRouter(const QueryRouter&) = delete;
    QueryRouter& operator=(const QueryRouter&) = delete;
    
    // Helper methods
    std::vector<uint8_t> serialize_inventory(const std::vector<ItemData>& items);
    std::vector<uint8_t> serialize_skills(const std::vector<SkillData>& skills);
    std::vector<uint8_t> serialize_quests(const std::vector<QuestData>& quests);
    
    std::expected<std::vector<ItemData>, std::string> deserialize_inventory(
        const std::vector<uint8_t>& blob);
    std::expected<std::vector<SkillData>, std::string> deserialize_skills(
        const std::vector<uint8_t>& blob);
    std::expected<std::vector<QuestData>, std::string> deserialize_quests(
        const std::vector<uint8_t>& blob);
};

} // namespace kal::db
