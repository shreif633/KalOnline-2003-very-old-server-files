#include "PacketHandler.h"
#include "GameServer.h"
#include "EntityManager.h"
#include "Logger.h"
#include <cstring>

namespace kal {

PacketHandler::PacketHandler(GameServer& server) 
    : server_(server) {
}

bool PacketHandler::HandlePacket(Player* player, const uint8_t* data, size_t size) {
    if (!player || !data || size < sizeof(PacketHeader)) {
        return false;
    }

    PacketReader reader(data, size);
    PacketHeader header = reader.ReadHeader();

    // Validate packet size
    if (header.size != size) {
        Logger::Warn("Packet size mismatch: header={}, actual={}", header.size, size);
        return false;
    }

    // Route based on opcode
    auto opcode = static_cast<ClientOpcode>(header.opcode);
    
    switch (opcode) {
        case ClientOpcode::C_AUTH_LOGIN_REQUEST:
            HandleLoginRequest(player, reader);
            break;
            
        case ClientOpcode::C_CHARACTER_LIST_REQUEST:
            HandleCharacterList(player, reader);
            break;
            
        case ClientOpcode::C_CHARACTER_CREATE:
            HandleCharacterCreate(player, reader);
            break;
            
        case ClientOpcode::C_CHARACTER_DELETE:
            HandleCharacterDelete(player, reader);
            break;
            
        case ClientOpcode::C_WORLD_ENTER_REQUEST:
            HandleWorldEnter(player, reader);
            break;
            
        case ClientOpcode::C_MOVE_START:
            HandleMoveStart(player, reader);
            break;
            
        case ClientOpcode::C_MOVE_STOP:
            HandleMoveStop(player, reader);
            break;
            
        case ClientOpcode::C_CHAT_MESSAGE:
            HandleChatMessage(player, reader);
            break;
            
        case ClientOpcode::C_ATTACK_REQUEST:
            HandleAttackRequest(player, reader);
            break;
            
        case ClientOpcode::C_SKILL_CAST_REQUEST:
            HandleSkillCast(player, reader);
            break;
            
        case ClientOpcode::C_ITEM_USE:
            HandleItemUse(player, reader);
            break;
            
        case ClientOpcode::C_ITEM_EQUIP:
            HandleItemEquip(player, reader);
            break;
            
        case ClientOpcode::C_ITEM_DROP:
            HandleItemDrop(player, reader);
            break;
            
        case ClientOpcode::C_SHOP_BUY:
            HandleShopBuy(player, reader);
            break;
            
        case ClientOpcode::C_SHOP_SELL:
            HandleShopSell(player, reader);
            break;
            
        case ClientOpcode::C_NPC_TALK:
            HandleNpcTalk(player, reader);
            break;
            
        case ClientOpcode::C_QUEST_ACCEPT:
            HandleQuestAccept(player, reader);
            break;
            
        case ClientOpcode::C_PARTY_CREATE:
            HandlePartyCreate(player, reader);
            break;
            
        case ClientOpcode::C_GUILD_CREATE:
            HandleGuildCreate(player, reader);
            break;
            
        default:
            Logger::Debug("Unhandled client opcode: 0x{:04X}", header.opcode);
            return false;
    }

    return true;
}

void PacketHandler::HandleLoginRequest(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "LoginRequest")) return;

    std::string username = reader.ReadStringKR(32);
    std::string password = reader.ReadStringKR(32);
    uint32_t version = reader.ReadUInt32();

    Logger::Info("Login attempt: user={}, version={}", username, version);

    // TODO: Validate credentials with AuthServer/Database
    // For now, send success response
    // Server will send S_AUTH_LOGIN_RESULT via GameServer
}

void PacketHandler::HandleCharacterList(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "CharList")) return;

    int32_t accountId = reader.ReadInt32();
    
    Logger::Info("Character list request for account: {}", accountId);

    // TODO: Query database for characters and send S_CHARACTER_LIST
}

void PacketHandler::HandleCharacterCreate(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "CharCreate")) return;

    std::string name = reader.ReadStringKR(32);
    uint8_t face = reader.ReadUInt8();
    uint8_t hair = reader.ReadUInt8();
    uint8_t job = reader.ReadUInt8();
    uint8_t str = reader.ReadUInt8();
    uint8_t hth = reader.ReadUInt8();
    uint8_t int_ = reader.ReadUInt8();
    uint8_t wis = reader.ReadUInt8();
    uint8_t dex = reader.ReadUInt8();

    Logger::Info("Character create: {} (face={}, hair={}, job={})", 
                 name, face, hair, job);

    // TODO: Create character in database and send S_CHARACTER_CREATE_RESULT
}

void PacketHandler::HandleCharacterDelete(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "CharDelete")) return;

    int32_t charId = reader.ReadInt32();
    std::string password = reader.ReadStringKR(32);

    Logger::Info("Character delete request: id={}", charId);

    // TODO: Delete character from database and send S_CHARACTER_DELETE_RESULT
}

void PacketHandler::HandleWorldEnter(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "WorldEnter")) return;

    int32_t charId = reader.ReadInt32();

    Logger::Info("World enter request: charId={}", charId);

    // TODO: Load character data, spawn player, send S_WORLD_ENTER_RESULT + spawns
}

void PacketHandler::HandleMoveStart(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "MoveStart")) return;

    uint32_t entityId = reader.ReadUInt32();
    float x = reader.ReadFloat();
    float y = reader.ReadFloat();
    float z = reader.ReadFloat();
    uint16_t speed = reader.ReadUInt16();
    uint8_t direction = reader.ReadUInt8();

    // Validate movement (anti-cheat)
    // TODO: Check if movement is valid (speed hack detection, collision)

    // Broadcast movement to nearby players
    // TODO: Use WorldManager to get nearby entities and send S_MOVE_START
}

void PacketHandler::HandleMoveStop(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "MoveStop")) return;

    uint32_t entityId = reader.ReadUInt32();
    float x = reader.ReadFloat();
    float y = reader.ReadFloat();
    float z = reader.ReadFloat();

    // Update entity position
    // TODO: Broadcast S_MOVE_STOP to nearby players
}

void PacketHandler::HandleChatMessage(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "Chat")) return;

    uint8_t type = reader.ReadUInt8();
    std::string message = reader.ReadStringKR();

    // TODO: Process chat based on type (normal, shout, party, guild)
    // TODO: Broadcast S_CHAT_MESSAGE to appropriate recipients
}

void PacketHandler::HandleAttackRequest(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "Attack")) return;

    uint32_t attackerId = reader.ReadUInt32();
    uint32_t targetId = reader.ReadUInt32();
    uint8_t skillSlot = reader.ReadUInt8();

    Logger::Debug("Attack: attacker={}, target={}, skill={}", 
                  attackerId, targetId, skillSlot);

    // TODO: Use CombatSystem to process attack and send damage results
}

void PacketHandler::HandleSkillCast(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "SkillCast")) return;

    uint32_t casterId = reader.ReadUInt32();
    uint32_t targetId = reader.ReadUInt32();
    uint16_t skillId = reader.ReadUInt16();

    Logger::Debug("Skill cast: caster={}, target={}, skill={}", 
                  casterId, targetId, skillId);

    // TODO: Use SkillSystem to validate and execute skill
}

void PacketHandler::HandleItemUse(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "ItemUse")) return;

    uint32_t itemId = reader.ReadUInt32();
    uint8_t slot = reader.ReadUInt8();

    // TODO: Use ItemSystem to process item usage
}

void PacketHandler::HandleItemEquip(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "ItemEquip")) return;

    uint32_t itemId = reader.ReadUInt32();
    uint8_t slot = reader.ReadUInt8();

    // TODO: Use ItemSystem to equip item and broadcast S_EQUIPMENT_UPDATE
}

void PacketHandler::HandleItemDrop(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "ItemDrop")) return;

    uint32_t itemId = reader.ReadUInt32();
    uint16_t amount = reader.ReadUInt16();
    float x = reader.ReadFloat();
    float y = reader.ReadFloat();

    // TODO: Use ItemSystem to drop item on ground
}

void PacketHandler::HandleShopBuy(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "ShopBuy")) return;

    uint32_t npcId = reader.ReadUInt32();
    uint32_t itemId = reader.ReadUInt32();
    uint16_t amount = reader.ReadUInt16();

    // TODO: Use ItemSystem to process purchase
}

void PacketHandler::HandleShopSell(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "ShopSell")) return;

    uint32_t npcId = reader.ReadUInt32();
    uint32_t itemId = reader.ReadUInt32();
    uint16_t amount = reader.ReadUInt16();

    // TODO: Use ItemSystem to process sale
}

void PacketHandler::HandleNpcTalk(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "NpcTalk")) return;

    uint32_t npcId = reader.ReadUInt32();

    Logger::Debug("NPC talk: player={}, npc={}", player ? player->GetId() : 0, npcId);

    // TODO: Use NPC system to open dialog and send S_NPC_DIALOG
}

void PacketHandler::HandleQuestAccept(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "QuestAccept")) return;

    uint32_t npcId = reader.ReadUInt32();
    uint16_t questId = reader.ReadUInt16();

    // TODO: Use QuestSystem to accept quest
}

void PacketHandler::HandlePartyCreate(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "PartyCreate")) return;

    Logger::Info("Party create request");

    // TODO: Use SocialSystem to create party
}

void PacketHandler::HandleGuildCreate(Player* player, PacketReader& reader) {
    if (!ValidatePacket(player, reader, "GuildCreate")) return;

    std::string guildName = reader.ReadStringKR(32);

    Logger::Info("Guild create request: {}", guildName);

    // TODO: Use SocialSystem to create guild
}

bool PacketHandler::ValidatePacket(Player* player, PacketReader& reader, const char* packetType) {
    if (!player) {
        Logger::Error("{}: No player associated", packetType);
        return false;
    }

    if (!reader.CanRead(1)) {
        Logger::Warn("{}: Insufficient data", packetType);
        return false;
    }

    return true;
}

} // namespace kal
