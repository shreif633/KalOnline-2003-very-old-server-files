#pragma once
#include <cstdint>

namespace kal {

// --- Opcodes (Based on 2003 Decompilation) ---
enum class ServerOpcode : uint16_t {
    // Auth & Login
    S_AUTH_LOGIN_RESULT       = 0x1002,
    S_SERVER_LIST             = 0x1004,
    S_CHARACTER_LIST          = 0x1102,
    S_CHARACTER_CREATE_RESULT = 0x1104,
    S_CHARACTER_DELETE_RESULT = 0x1106,
    
    // World Entry
    S_WORLD_ENTER_RESULT      = 0x1202,
    S_SPAWN_SELF              = 0x1204,
    S_SPAWN_OTHER_PLAYER      = 0x1206,
    S_SPAWN_NPC               = 0x1208,
    S_SPAWN_MONSTER           = 0x120A,
    S_REMOVE_ENTITY           = 0x120C,
    
    // Movement
    S_MOVE_START              = 0x2002,
    S_MOVE_STOP               = 0x2004,
    S_MOVE_POSITION           = 0x2006,
    
    // Chat & Social
    S_CHAT_MESSAGE            = 0x2102,
    S_SYSTEM_MESSAGE          = 0x2104,
    S_NOTICE_MESSAGE          = 0x2106,
    
    // Combat & Skills
    S_ATTACK_RESULT           = 0x2202,
    S_SKILL_CAST_RESULT       = 0x2204,
    S_SKILL_EFFECT            = 0x2206,
    S_DAMAGE_RECEIVED         = 0x2208,
    S_ENTITY_DIE              = 0x220A,
    S_ENTITY_REVIVE           = 0x220C,
    
    // Items & Inventory
    S_INVENTORY_INIT          = 0x2302,
    S_INVENTORY_ADD_ITEM      = 0x2304,
    S_INVENTORY_REMOVE_ITEM   = 0x2306,
    S_INVENTORY_UPDATE_ITEM   = 0x2308,
    S_EQUIPMENT_UPDATE        = 0x230A,
    S_SHOPItemList            = 0x2402,
    S_TRADE_RESULT            = 0x230C,
    
    // NPC & Quest
    S_NPC_DIALOG              = 0x2404,
    S_QUEST_UPDATE            = 0x2702,
    
    // Stats & Status
    S_STATS_UPDATE            = 0x1302,
    S_HP_MP_UPDATE            = 0x1304,
    S_EXP_UPDATE              = 0x1306,
    S_LEVEL_UP                = 0x1308,
    S_BUFF_ADDED              = 0x130A,
    S_BUFF_REMOVED            = 0x130C,
    
    // Time & Environment
    S_TIME_CHANGE             = 0x1402,
    S_WEATHER_CHANGE          = 0x1404
};

enum class ClientOpcode : uint16_t {
    // Auth & Login
    C_AUTH_LOGIN_REQUEST      = 0x1001,
    C_SERVER_LIST_REQUEST     = 0x1003,
    C_CHARACTER_LIST_REQUEST  = 0x1101,
    C_CHARACTER_CREATE        = 0x1103,
    C_CHARACTER_DELETE        = 0x1105,
    
    // World Entry
    C_WORLD_ENTER_REQUEST     = 0x1201,
    
    // Movement
    C_MOVE_START              = 0x2001,
    C_MOVE_STOP               = 0x2003,
    C_MOVE_POSITION           = 0x2005,
    
    // Chat & Social
    C_CHAT_MESSAGE            = 0x2101,
    
    // Combat & Skills
    C_ATTACK_REQUEST          = 0x2201,
    C_SKILL_CAST_REQUEST      = 0x2203,
    
    // Items & Inventory
    C_ITEM_USE                = 0x2301,
    C_ITEM_DROP               = 0x2303,
    C_ITEM_PICKUP             = 0x2305,
    C_ITEM_EQUIP              = 0x2307,
    C_ITEM_UNEQUIP            = 0x2309,
    C_SHOP_BUY                = 0x2401,
    C_SHOP_SELL               = 0x2403,
    
    // NPC & Quest
    C_NPC_TALK                = 0x2405,
    C_QUEST_ACCEPT            = 0x2701,
    C_QUEST_COMPLETE          = 0x2703,
    
    // Party & Guild
    C_PARTY_CREATE            = 0x2501,
    C_PARTY_INVITE            = 0x2503,
    C_GUILD_CREATE            = 0x2601
};

// --- Packet Structures (Binary Layouts) ---
#pragma pack(push, 1)

struct PacketHeader {
    uint16_t size;
    uint16_t opcode;
};

struct LoginRequestPacket {
    PacketHeader header;
    char username[32];   // EUC-KR encoded, null-terminated
    char password[32];   // EUC-KR encoded, null-terminated
    uint32_t version;
};

struct CharacterCreatePacket {
    PacketHeader header;
    char name[32];       // EUC-KR
    uint8_t face;
    uint8_t hair;
    uint8_t job;
    uint8_t strength;
    uint8_t health;
    uint8_t intelligence;
    uint8_t wisdom;
    uint8_t dexterity;
};

struct MoveStartPacket {
    PacketHeader header;
    uint32_t entity_id;
    float x;
    float y;
    float z;
    uint16_t speed;
    uint8_t direction; // 0-255 mapping to 360 degrees
};

struct AttackRequestPacket {
    PacketHeader header;
    uint32_t attacker_id;
    uint32_t target_id;
    uint8_t skill_slot; // 0 for basic attack
};

struct ChatMessagePacket {
    PacketHeader header;
    uint8_t type; // 0: Normal, 1: Shout, 2: Party, 3: Guild
    char message[256]; // EUC-KR
};

#pragma pack(pop)

// --- Constants ---
constexpr float WORLD_SCALE = 1.0f; // Coordinate scale factor
constexpr int MAX_USERNAME_LEN = 32;
constexpr int MAX_PASSWORD_LEN = 32;
constexpr int MAX_CHAR_NAME_LEN = 32;
constexpr int MAX_CHAT_LEN = 256;

constexpr uint32_t CLIENT_VERSION = 2003; // Expected client version

} // namespace kal
