#include "QueryRouter.hpp"
#include "Logger.hpp"
#include <cstring>
#include <format>

namespace kal::db {

QueryRouter& QueryRouter::instance() {
    static QueryRouter instance;
    return instance;
}

QueryResultWrapper QueryRouter::process_request(const QueryRequest& request) {
    LOG_DEBUG("Processing query type {} for user {}", 
              static_cast<int>(request.type), request.user_id);
    
    try {
        switch (request.type) {
            // Character operations
            case QueryType::LoadCharacter:
                return handle_load_character(request);
            case QueryType::SaveCharacter:
                return handle_save_character(request);
            case QueryType::CreateCharacter:
                return handle_create_character(request);
            case QueryType::DeleteCharacter:
                return handle_delete_character(request);
            case QueryType::ListCharacters:
                return handle_list_characters(request);
            
            // Inventory operations
            case QueryType::LoadInventory:
                return handle_load_inventory(request);
            case QueryType::SaveInventory:
                return handle_save_inventory(request);
            case QueryType::AddItem:
                return handle_add_item(request);
            case QueryType::RemoveItem:
                return handle_remove_item(request);
            case QueryType::MoveItem:
                return handle_move_item(request);
            case QueryType::EquipItem:
                return handle_equip_item(request);
            case QueryType::UnequipItem:
                return handle_unequip_item(request);
            
            // Skill operations
            case QueryType::LoadSkills:
                return handle_load_skills(request);
            case QueryType::SaveSkills:
                return handle_save_skills(request);
            case QueryType::LearnSkill:
                return handle_learn_skill(request);
            
            // Quest operations
            case QueryType::LoadQuests:
                return handle_load_quests(request);
            case QueryType::SaveQuests:
                return handle_save_quests(request);
            case QueryType::UpdateQuest:
                return handle_update_quest(request);
            
            // Mail operations
            case QueryType::SendMail:
                return handle_send_mail(request);
            case QueryType::ReadMail:
                return handle_read_mail(request);
            case QueryType::DeleteMail:
                return handle_delete_mail(request);
            
            // Guild operations
            case QueryType::CreateGuild:
                return handle_create_guild(request);
            case QueryType::JoinGuild:
                return handle_join_guild(request);
            case QueryType::LeaveGuild:
                return handle_leave_guild(request);
            
            // World data
            case QueryType::GetTeleporterList:
                return handle_get_teleporter_list(request);
            case QueryType::GetNPCList:
                return handle_get_npc_list(request);
            case QueryType::GetMonsterInfo:
                return handle_get_monster_info(request);
            case QueryType::GetItemInfo:
                return handle_get_item_info(request);
            
            default:
                LOG_WARN("Unknown query type: {}", static_cast<int>(request.type));
                return {
                    QueryResult::InvalidParameter,
                    std::monostate{},
                    std::format("Unknown query type: {}", static_cast<int>(request.type))
                };
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Query processing failed: {}", e.what());
        return {
            QueryResult::DatabaseError,
            std::monostate{},
            std::string(e.what())
        };
    }
}

QueryResultWrapper QueryRouter::handle_load_character(const QueryRequest& request) {
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        SELECT id, name, level, strength, health, intelligence, wisdom, dexterity,
               experience, gold, map_id, position_x, position_y, position_z,
               hair_style, face_style, costume_id
        FROM players
        WHERE id = $1 AND owner_user_id = $2
    )";
    
    auto result = db.execute_query(query, request.character_id, request.user_id);
    if (!result || result->empty()) {
        return {
            QueryResult::NotFound,
            std::monostate{},
            "Character not found"
        };
    }
    
    auto row = *result->begin();
    CharacterData char_data{
        .id = row.get<uint32_t>(0),
        .name = row.get<std::string>(1),
        .level = row.get<uint8_t>(2),
        .strength = row.get<uint16_t>(3),
        .health = row.get<uint16_t>(4),
        .intelligence = row.get<uint16_t>(5),
        .wisdom = row.get<uint16_t>(6),
        .dexterity = row.get<uint16_t>(7),
        .experience = row.get<uint32_t>(8),
        .gold = row.get<uint32_t>(9),
        .map_id = row.get<uint16_t>(10),
        .position_x = row.get<float>(11),
        .position_y = row.get<float>(12),
        .position_z = row.get<float>(13),
        .hair_style = row.get<uint8_t>(14),
        .face_style = row.get<uint8_t>(15),
        .costume_id = row.get<uint8_t>(16)
    };
    
    return {QueryResult::Success, char_data, ""};
}

QueryResultWrapper QueryRouter::handle_save_character(const QueryRequest& request) {
    if (request.payload.size() < sizeof(CharacterData)) {
        return {
            QueryResult::InvalidParameter,
            std::monostate{},
            "Invalid payload size"
        };
    }
    
    const CharacterData* char_data = reinterpret_cast<const CharacterData*>(request.payload.data());
    
    auto& db = kal::Database::instance();
    std::string query = R"(
        UPDATE players
        SET level = $1, strength = $2, health = $3, intelligence = $4,
            wisdom = $5, dexterity = $6, experience = $7, gold = $8,
            map_id = $9, position_x = $10, position_y = $11, position_z = $12,
            last_save = NOW()
        WHERE id = $13 AND owner_user_id = $14
    )";
    
    db.execute_query(query,
        char_data->level, char_data->strength, char_data->health,
        char_data->intelligence, char_data->wisdom, char_data->dexterity,
        char_data->experience, char_data->gold, char_data->map_id,
        char_data->position_x, char_data->position_y, char_data->position_z,
        char_data->id, request.user_id);
    
    LOG_DEBUG("Character {} saved", char_data->id);
    return {QueryResult::Success, true, ""};
}

QueryResultWrapper QueryRouter::handle_create_character(const QueryRequest& request) {
    if (request.payload.size() < 2) {
        return {
            QueryResult::InvalidParameter,
            std::monostate{},
            "Invalid payload"
        };
    }
    
    std::string name(request.payload.begin(), request.payload.end());
    
    // Validate name
    if (name.length() < 3 || name.length() > 20) {
        return {
            QueryResult::InvalidParameter,
            std::monostate{},
            "Invalid character name length"
        };
    }
    
    auto& db = kal::Database::instance();
    
    // Check character count for this user
    std::string count_query = "SELECT COUNT(*) FROM players WHERE owner_user_id = $1";
    auto count_result = db.execute_query(count_query, request.user_id);
    if (count_result && !count_result->empty()) {
        int count = (*count_result->begin()).get<int>(0);
        if (count >= 4) { // Max 4 characters per account
            return {
                QueryResult::AlreadyExists,
                std::monostate{},
                "Maximum character limit reached"
            };
        }
    }
    
    // Create new character
    std::string insert_query = R"(
        INSERT INTO players (owner_user_id, name, level, map_id, position_x, position_y, position_z)
        VALUES ($1, $2, 1, 1, 100.0, 100.0, 0.0)
        RETURNING id
    )";
    
    auto result = db.execute_query(insert_query, request.user_id, name);
    if (!result || result->empty()) {
        return {
            QueryResult::DatabaseError,
            std::monostate{},
            "Failed to create character"
        };
    }
    
    uint32_t char_id = result->begin()->get<uint32_t>(0);
    LOG_INFO("Character '{}' created (ID: {})", name, char_id);
    
    CharacterData new_char{
        .id = char_id,
        .name = name,
        .level = 1,
        .strength = 10,
        .health = 10,
        .intelligence = 10,
        .wisdom = 10,
        .dexterity = 10,
        .experience = 0,
        .gold = 100,
        .map_id = 1,
        .position_x = 100.0f,
        .position_y = 100.0f,
        .position_z = 0.0f,
        .hair_style = 0,
        .face_style = 0,
        .costume_id = 0
    };
    
    return {QueryResult::Success, new_char, ""};
}

QueryResultWrapper QueryRouter::handle_delete_character(const QueryRequest& request) {
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        DELETE FROM players
        WHERE id = $1 AND owner_user_id = $2
    )";
    
    auto result = db.execute_query(query, request.character_id, request.user_id);
    if (result && result->affected_rows() > 0) {
        LOG_INFO("Character {} deleted", request.character_id);
        return {QueryResult::Success, true, ""};
    }
    
    return {
        QueryResult::NotFound,
        std::monostate{},
        "Character not found"
    };
}

QueryResultWrapper QueryRouter::handle_list_characters(const QueryRequest& request) {
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        SELECT id, name, level, strength, health, intelligence, wisdom, dexterity,
               experience, map_id, hair_style, face_style, costume_id
        FROM players
        WHERE owner_user_id = $1
        ORDER BY id
    )";
    
    auto result = db.execute_query(query, request.user_id);
    if (!result) {
        return {
            QueryResult::DatabaseError,
            std::monostate{},
            "Database error"
        };
    }
    
    std::vector<CharacterData> characters;
    for (const auto& row : *result) {
        characters.push_back({
            .id = row.get<uint32_t>(0),
            .name = row.get<std::string>(1),
            .level = row.get<uint8_t>(2),
            .strength = row.get<uint16_t>(3),
            .health = row.get<uint16_t>(4),
            .intelligence = row.get<uint16_t>(5),
            .wisdom = row.get<uint16_t>(6),
            .dexterity = row.get<uint16_t>(7),
            .experience = row.get<uint32_t>(8),
            .gold = 0,
            .map_id = row.get<uint16_t>(9),
            .position_x = 0,
            .position_y = 0,
            .position_z = 0,
            .hair_style = row.get<uint8_t>(10),
            .face_style = row.get<uint8_t>(11),
            .costume_id = row.get<uint8_t>(12)
        });
    }
    
    return {QueryResult::Success, characters, ""};
}

QueryResultWrapper QueryRouter::handle_load_inventory(const QueryRequest& request) {
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        SELECT id, item_type_id, slot, location, quantity, durability, flags, prefix_name
        FROM items
        WHERE owner_id = $1
        ORDER BY location, slot
    )";
    
    auto result = db.execute_query(query, request.character_id);
    if (!result) {
        return {
            QueryResult::DatabaseError,
            std::monostate{},
            "Database error"
        };
    }
    
    std::vector<ItemData> inventory;
    for (const auto& row : *result) {
        ItemData item{
            .id = row.get<uint32_t>(0),
            .item_type_id = row.get<uint32_t>(1),
            .slot = row.get<uint8_t>(2),
            .location = row.get<uint8_t>(3),
            .quantity = row.get<int16_t>(4),
            .durability = row.get<int16_t>(5),
            .flags = row.get<uint32_t>(6),
            .prefix_name = row.get<std::string>(7)
        };
        inventory.push_back(item);
    }
    
    return {QueryResult::Success, inventory, ""};
}

QueryResultWrapper QueryRouter::handle_save_inventory(const QueryRequest& request) {
    auto items = deserialize_inventory(request.payload);
    if (!items) {
        return {
            QueryResult::InvalidParameter,
            std::monostate{},
            items.error()
        };
    }
    
    auto& db = kal::Database::instance();
    
    // Save each item
    for (const auto& item : *items) {
        std::string query = R"(
            INSERT INTO items (owner_id, item_type_id, slot, location, quantity, durability, flags, prefix_name)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
            ON CONFLICT (id) DO UPDATE SET
                slot = EXCLUDED.slot,
                location = EXCLUDED.location,
                quantity = EXCLUDED.quantity,
                durability = EXCLUDED.durability,
                flags = EXCLUDED.flags,
                prefix_name = EXCLUDED.prefix_name
        )";
        
        db.execute_query(query, request.character_id, item.item_type_id, item.slot,
                        item.location, item.quantity, item.durability, item.flags, item.prefix_name);
    }
    
    LOG_DEBUG("Inventory saved for character {}", request.character_id);
    return {QueryResult::Success, true, ""};
}

QueryResultWrapper QueryRouter::handle_load_skills(const QueryRequest& request) {
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        SELECT skill_id, skill_level, cooldown_end
        FROM player_skills
        WHERE player_id = $1
    )";
    
    auto result = db.execute_query(query, request.character_id);
    if (!result) {
        return {
            QueryResult::DatabaseError,
            std::monostate{},
            "Database error"
        };
    }
    
    std::vector<SkillData> skills;
    for (const auto& row : *result) {
        skills.push_back({
            .skill_id = row.get<uint32_t>(0),
            .level = row.get<uint8_t>(1),
            .cooldown_end = row.get<uint64_t>(2)
        });
    }
    
    return {QueryResult::Success, skills, ""};
}

QueryResultWrapper QueryRouter::handle_learn_skill(const QueryRequest& request) {
    if (request.payload.size() < sizeof(uint32_t)) {
        return {
            QueryResult::InvalidParameter,
            std::monostate{},
            "Invalid payload"
        };
    }
    
    uint32_t skill_id = *reinterpret_cast<const uint32_t*>(request.payload.data());
    
    auto& db = kal::Database::instance();
    
    // Check if already learned
    std::string check_query = "SELECT 1 FROM player_skills WHERE player_id = $1 AND skill_id = $2";
    auto check_result = db.execute_query(check_query, request.character_id, skill_id);
    if (check_result && !check_result->empty()) {
        return {
            QueryResult::AlreadyExists,
            std::monostate{},
            "Skill already learned"
        };
    }
    
    // Learn the skill
    std::string insert_query = R"(
        INSERT INTO player_skills (player_id, skill_id, skill_level)
        VALUES ($1, $2, 1)
    )";
    
    db.execute_query(insert_query, request.character_id, skill_id);
    LOG_INFO("Character {} learned skill {}", request.character_id, skill_id);
    
    return {QueryResult::Success, true, ""};
}

QueryResultWrapper QueryRouter::handle_load_quests(const QueryRequest& request) {
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        SELECT quest_id, state, objectives, start_time, complete_time
        FROM player_quests
        WHERE player_id = $1
    )";
    
    auto result = db.execute_query(query, request.character_id);
    if (!result) {
        return {
            QueryResult::DatabaseError,
            std::monostate{},
            "Database error"
        };
    }
    
    std::vector<QuestData> quests;
    for (const auto& row : *result) {
        QuestData quest{
            .quest_id = row.get<uint32_t>(0),
            .state = row.get<uint8_t>(1),
            .objectives = row.get<std::vector<uint8_t>>(2),
            .start_time = row.get<uint64_t>(3),
            .complete_time = row.get<uint64_t>(4)
        };
        quests.push_back(quest);
    }
    
    return {QueryResult::Success, quests, ""};
}

QueryResultWrapper QueryRouter::handle_update_quest(const QueryRequest& request) {
    // Parse quest update from payload
    if (request.payload.size() < sizeof(uint32_t) + 1) {
        return {
            QueryResult::InvalidParameter,
            std::monostate{},
            "Invalid payload"
        };
    }
    
    uint32_t quest_id = *reinterpret_cast<const uint32_t*>(request.payload.data());
    uint8_t state = request.payload[sizeof(uint32_t)];
    
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        UPDATE player_quests
        SET state = $1, objectives = $2
        WHERE player_id = $3 AND quest_id = $4
    )";
    
    // Simplified: just update state, objectives would be parsed from payload
    db.execute_query(query, state, std::vector<uint8_t>{}, request.character_id, quest_id);
    
    return {QueryResult::Success, true, ""};
}

QueryResultWrapper QueryRouter::handle_send_mail(const QueryRequest& request) {
    // Parse mail data from payload
    if (request.payload.size() < 4) {
        return {
            QueryResult::InvalidParameter,
            std::monostate{},
            "Invalid payload"
        };
    }
    
    // Simplified implementation - full version would parse all fields
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        INSERT INTO mail (sender_id, recipient_name, subject, body, sent_time)
        VALUES ($1, $2, $3, $4, NOW())
    )";
    
    // Would need proper parsing here
    db.execute_query(query, request.user_id, "recipient", "subject", "body");
    
    return {QueryResult::Success, true, ""};
}

QueryResultWrapper QueryRouter::handle_read_mail(const QueryRequest& request) {
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        UPDATE mail SET is_read = TRUE WHERE id = $1 AND recipient_id = $2
    )";
    
    uint32_t mail_id = *reinterpret_cast<const uint32_t*>(request.payload.data());
    db.execute_query(query, mail_id, request.user_id);
    
    return {QueryResult::Success, true, ""};
}

QueryResultWrapper QueryRouter::handle_delete_mail(const QueryRequest& request) {
    auto& db = kal::Database::instance();
    
    std::string query = "DELETE FROM mail WHERE id = $1 AND recipient_id = $2";
    uint32_t mail_id = *reinterpret_cast<const uint32_t*>(request.payload.data());
    
    auto result = db.execute_query(query, mail_id, request.user_id);
    if (result && result->affected_rows() > 0) {
        return {QueryResult::Success, true, ""};
    }
    
    return {QueryResult::NotFound, std::monostate{}, "Mail not found"};
}

QueryResultWrapper QueryRouter::handle_create_guild(const QueryRequest& request) {
    if (request.payload.size() < 2) {
        return {
            QueryResult::InvalidParameter,
            std::monostate{},
            "Invalid payload"
        };
    }
    
    std::string guild_name(request.payload.begin(), request.payload.end());
    
    auto& db = kal::Database::instance();
    
    // Check if guild exists
    std::string check_query = "SELECT 1 FROM guilds WHERE name = $1";
    auto check_result = db.execute_query(check_query, guild_name);
    if (check_result && !check_result->empty()) {
        return {
            QueryResult::AlreadyExists,
            std::monostate{},
            "Guild name already taken"
        };
    }
    
    // Create guild
    std::string insert_query = R"(
        INSERT INTO guilds (name, leader_id, notice, level)
        VALUES ($1, $2, $3, 1)
        RETURNING id
    )";
    
    auto result = db.execute_query(insert_query, guild_name, request.character_id, "");
    if (!result || result->empty()) {
        return {
            QueryResult::DatabaseError,
            std::monostate{},
            "Failed to create guild"
        };
    }
    
    uint32_t guild_id = result->begin()->get<uint32_t>(0);
    LOG_INFO("Guild '{}' created (ID: {}) by character {}", guild_name, guild_id, request.character_id);
    
    return {QueryResult::Success, true, ""};
}

QueryResultWrapper QueryRouter::handle_join_guild(const QueryRequest& request) {
    uint32_t guild_id = *reinterpret_cast<const uint32_t*>(request.payload.data());
    
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        INSERT INTO guild_members (guild_id, player_id, rank, join_date)
        VALUES ($1, $2, 0, NOW())
    )";
    
    db.execute_query(query, guild_id, request.character_id);
    
    return {QueryResult::Success, true, ""};
}

QueryResultWrapper QueryRouter::handle_leave_guild(const QueryRequest& request) {
    auto& db = kal::Database::instance();
    
    std::string query = "DELETE FROM guild_members WHERE player_id = $1";
    db.execute_query(query, request.character_id);
    
    return {QueryResult::Success, true, ""};
}

QueryResultWrapper QueryRouter::handle_get_teleporter_list(const QueryRequest& request) {
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        SELECT id, name, map_id, position_x, position_y, position_z, cost
        FROM teleporters
        ORDER BY id
    )";
    
    auto result = db.execute_query(query);
    if (!result) {
        return {
            QueryResult::DatabaseError,
            std::monostate{},
            "Database error"
        };
    }
    
    // Return as raw binary data
    std::vector<uint8_t> data;
    for (const auto& row : *result) {
        // Serialize teleporter data
        // Full implementation would properly serialize
    }
    
    return {QueryResult::Success, data, ""};
}

QueryResultWrapper QueryRouter::handle_get_npc_list(const QueryRequest& request) {
    uint16_t map_id = *reinterpret_cast<const uint16_t*>(request.payload.data());
    
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        SELECT id, name, type, map_id, position_x, position_y, position_z
        FROM npcs
        WHERE map_id = $1
    )";
    
    auto result = db.execute_query(query, map_id);
    if (!result) {
        return {
            QueryResult::DatabaseError,
            std::monostate{},
            "Database error"
        };
    }
    
    std::vector<uint8_t> npc_data;
    // Serialize NPC list
    
    return {QueryResult::Success, npc_data, ""};
}

QueryResultWrapper QueryRouter::handle_get_monster_info(const QueryRequest& request) {
    uint32_t monster_id = *reinterpret_cast<const uint32_t*>(request.payload.data());
    
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        SELECT id, name, level, hp, attack, defense, exp_reward, gold_reward
        FROM monsters
        WHERE id = $1
    )";
    
    auto result = db.execute_query(query, monster_id);
    if (!result || result->empty()) {
        return {
            QueryResult::NotFound,
            std::monostate{},
            "Monster not found"
        };
    }
    
    std::vector<uint8_t> monster_data;
    // Serialize monster info
    
    return {QueryResult::Success, monster_data, ""};
}

QueryResultWrapper QueryRouter::handle_get_item_info(const QueryRequest& request) {
    uint32_t item_id = *reinterpret_cast<const uint32_t*>(request.payload.data());
    
    auto& db = kal::Database::instance();
    
    std::string query = R"(
        SELECT id, name, type, min_level, attack, defense, price, max_stack
        FROM items_metadata
        WHERE id = $1
    )";
    
    auto result = db.execute_query(query, item_id);
    if (!result || result->empty()) {
        return {
            QueryResult::NotFound,
            std::monostate{},
            "Item not found"
        };
    }
    
    std::vector<uint8_t> item_data;
    // Serialize item info
    
    return {QueryResult::Success, item_data, ""};
}

// Serialization helpers
std::vector<uint8_t> QueryRouter::serialize_inventory(const std::vector<ItemData>& items) {
    std::vector<uint8_t> data;
    
    for (const auto& item : items) {
        // Simple binary serialization
        data.insert(data.end(), reinterpret_cast<const uint8_t*>(&item.id), 
                   reinterpret_cast<const uint8_t*>(&item.id) + sizeof(item.id));
        data.insert(data.end(), reinterpret_cast<const uint8_t*>(&item.item_type_id),
                   reinterpret_cast<const uint8_t*>(&item.item_type_id) + sizeof(item.item_type_id));
        data.push_back(item.slot);
        data.push_back(item.location);
        data.insert(data.end(), reinterpret_cast<const uint8_t*>(&item.quantity),
                   reinterpret_cast<const uint8_t*>(&item.quantity) + sizeof(item.quantity));
        data.insert(data.end(), reinterpret_cast<const uint8_t*>(&item.durability),
                   reinterpret_cast<const uint8_t*>(&item.durability) + sizeof(item.durability));
        data.insert(data.end(), reinterpret_cast<const uint8_t*>(&item.flags),
                   reinterpret_cast<const uint8_t*>(&item.flags) + sizeof(item.flags));
        
        // Prefix name
        uint8_t name_len = std::min<uint8_t>(item.prefix_name.length(), 255);
        data.push_back(name_len);
        data.insert(data.end(), item.prefix_name.begin(), item.prefix_name.begin() + name_len);
    }
    
    return data;
}

std::expected<std::vector<ItemData>, std::string> QueryRouter::deserialize_inventory(
    const std::vector<uint8_t>& blob) {
    
    std::vector<ItemData> items;
    size_t offset = 0;
    
    while (offset < blob.size()) {
        if (offset + sizeof(uint32_t) * 2 + 4 + sizeof(int16_t) * 2 + sizeof(uint32_t) > blob.size()) {
            break;
        }
        
        ItemData item;
        std::memcpy(&item.id, blob.data() + offset, sizeof(item.id));
        offset += sizeof(item.id);
        
        std::memcpy(&item.item_type_id, blob.data() + offset, sizeof(item.item_type_id));
        offset += sizeof(item.item_type_id);
        
        item.slot = blob[offset++];
        item.location = blob[offset++];
        
        std::memcpy(&item.quantity, blob.data() + offset, sizeof(item.quantity));
        offset += sizeof(item.quantity);
        
        std::memcpy(&item.durability, blob.data() + offset, sizeof(item.durability));
        offset += sizeof(item.durability);
        
        std::memcpy(&item.flags, blob.data() + offset, sizeof(item.flags));
        offset += sizeof(item.flags);
        
        if (offset >= blob.size()) break;
        
        uint8_t name_len = blob[offset++];
        if (offset + name_len <= blob.size()) {
            item.prefix_name.assign(blob.begin() + offset, blob.begin() + offset + name_len);
            offset += name_len;
        }
        
        items.push_back(item);
    }
    
    return items;
}

std::expected<std::vector<SkillData>, std::string> QueryRouter::deserialize_skills(
    const std::vector<uint8_t>& blob) {
    
    std::vector<SkillData> skills;
    size_t offset = 0;
    
    while (offset + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint64_t) <= blob.size()) {
        SkillData skill;
        std::memcpy(&skill.skill_id, blob.data() + offset, sizeof(skill.skill_id));
        offset += sizeof(skill.skill_id);
        
        skill.level = blob[offset++];
        
        std::memcpy(&skill.cooldown_end, blob.data() + offset, sizeof(skill.cooldown_end));
        offset += sizeof(skill.cooldown_end);
        
        skills.push_back(skill);
    }
    
    return skills;
}

std::expected<std::vector<QuestData>, std::string> QueryRouter::deserialize_quests(
    const std::vector<uint8_t>& blob) {
    
    std::vector<QuestData> quests;
    size_t offset = 0;
    
    while (offset + sizeof(uint32_t) + 1 + sizeof(uint64_t) * 2 <= blob.size()) {
        QuestData quest;
        std::memcpy(&quest.quest_id, blob.data() + offset, sizeof(quest.quest_id));
        offset += sizeof(quest.quest_id);
        
        quest.state = blob[offset++];
        
        // Objectives would need length prefix
        // Simplified for now
        
        std::memcpy(&quest.start_time, blob.data() + offset, sizeof(quest.start_time));
        offset += sizeof(quest.start_time);
        
        std::memcpy(&quest.complete_time, blob.data() + offset, sizeof(quest.complete_time));
        offset += sizeof(quest.complete_time);
        
        quests.push_back(quest);
    }
    
    return quests;
}

} // namespace kal::db
