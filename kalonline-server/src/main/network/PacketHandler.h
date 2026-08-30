#pragma once
#include "Protocol.h"
#include "PacketBuilder.h"
#include <cstdint>
#include <memory>

namespace kal {

// Forward declarations
class GameServer;
class Player;

/**
 * @brief Handles all incoming network packets from clients
 * 
 * Routes packets to appropriate handlers based on opcode and processes
 * game logic accordingly. Uses PacketReader for binary deserialization.
 */
class PacketHandler {
public:
    explicit PacketHandler(GameServer& server);

    /**
     * @brief Process an incoming packet from a client
     * @param player The player who sent the packet
     * @param data Raw packet data including header
     * @return true if packet was processed successfully
     */
    bool HandlePacket(Player* player, const uint8_t* data, size_t size);

private:
    GameServer& server_;

    // --- Client Opcode Handlers ---
    void HandleLoginRequest(Player* player, PacketReader& reader);
    void HandleCharacterList(Player* player, PacketReader& reader);
    void HandleCharacterCreate(Player* player, PacketReader& reader);
    void HandleCharacterDelete(Player* player, PacketReader& reader);
    void HandleWorldEnter(Player* player, PacketReader& reader);
    
    void HandleMoveStart(Player* player, PacketReader& reader);
    void HandleMoveStop(Player* player, PacketReader& reader);
    
    void HandleChatMessage(Player* player, PacketReader& reader);
    
    void HandleAttackRequest(Player* player, PacketReader& reader);
    void HandleSkillCast(Player* player, PacketReader& reader);
    
    void HandleItemUse(Player* player, PacketReader& reader);
    void HandleItemEquip(Player* player, PacketReader& reader);
    void HandleItemDrop(Player* player, PacketReader& reader);
    void HandleShopBuy(Player* player, PacketReader& reader);
    void HandleShopSell(Player* player, PacketReader& reader);
    
    void HandleNpcTalk(Player* player, PacketReader& reader);
    void HandleQuestAccept(Player* player, PacketReader& reader);
    
    void HandlePartyCreate(Player* player, PacketReader& reader);
    void HandleGuildCreate(Player* player, PacketReader& reader);

    // --- Helper Methods ---
    bool ValidatePacket(Player* player, PacketReader& reader, const char* packetType);
};

} // namespace kal
