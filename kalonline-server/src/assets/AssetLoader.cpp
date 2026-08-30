#include "AssetLoader.h"
#include <algorithm>
#include <cctype>
#include <iostream>

AssetLoader::AssetLoader(const std::string& configPath) 
    : configPath_(configPath) {
}

std::string AssetLoader::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n;");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> AssetLoader::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    
    // Handle parentheses and quoted strings
    std::string current;
    bool inQuotes = false;
    int parenLevel = 0;
    
    for (char c : line) {
        if (c == '"') {
            inQuotes = !inQuotes;
            current += c;
        } else if (c == '(' && !inQuotes) {
            if (!current.empty() && trim(current) != "") {
                tokens.push_back(trim(current));
                current.clear();
            }
            parenLevel++;
            tokens.push_back("(");
        } else if (c == ')' && !inQuotes) {
            if (!current.empty() && trim(current) != "") {
                tokens.push_back(trim(current));
                current.clear();
            }
            parenLevel--;
            tokens.push_back(")");
        } else if ((c == ' ' || c == '\t') && !inQuotes && parenLevel == 0) {
            if (!current.empty() && trim(current) != "") {
                tokens.push_back(trim(current));
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty() && trim(current) != "") {
        tokens.push_back(trim(current));
    }
    
    return tokens;
}

std::string AssetLoader::extractQuotedString(const std::string& line, const std::string& key) {
    std::string searchKey = "(" + key + " ";
    size_t pos = line.find(searchKey);
    if (pos == std::string::npos) return "";
    
    size_t start = line.find('"', pos);
    if (start == std::string::npos) return "";
    
    size_t end = line.find('"', start + 1);
    if (end == std::string::npos) return "";
    
    return line.substr(start + 1, end - start - 1);
}

std::string AssetLoader::extractValue(const std::string& line, const std::string& key) {
    std::string searchKey = "(" + key + " ";
    size_t pos = line.find(searchKey);
    if (pos == std::string::npos) {
        // Try without space for single values
        searchKey = "(" + key + ")";
        pos = line.find(searchKey);
        if (pos != std::string::npos) return "";
        return "";
    }
    
    size_t start = pos + searchKey.length();
    size_t end = line.find(')', start);
    if (end == std::string::npos) return "";
    
    std::string value = trim(line.substr(start, end - start));
    
    // Remove quotes if present
    if (value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.length() - 2);
    }
    
    return value;
}

std::vector<int> AssetLoader::extractIntArray(const std::string& line, const std::string& key) {
    std::vector<int> result;
    std::string searchKey = "(" + key + " ";
    size_t pos = line.find(searchKey);
    if (pos == std::string::npos) return result;
    
    size_t start = pos + searchKey.length();
    size_t end = line.find(')', start);
    if (end == std::string::npos) return result;
    
    std::string content = line.substr(start, end - start);
    std::istringstream iss(content);
    int value;
    while (iss >> value) {
        result.push_back(value);
    }
    
    return result;
}

std::pair<int, int> AssetLoader::extractIntPair(const std::string& line, const std::string& key) {
    auto arr = extractIntArray(line, key);
    if (arr.size() >= 2) {
        return {arr[0], arr[1]};
    }
    return {0, 0};
}

std::tuple<int, int, int> AssetLoader::extractIntTriple(const std::string& line, const std::string& key) {
    auto arr = extractIntArray(line, key);
    if (arr.size() >= 3) {
        return {arr[0], arr[1], arr[2]};
    }
    return {0, 0, 0};
}

std::tuple<int, int, int, int> AssetLoader::extractIntQuad(const std::string& line, const std::string& key) {
    auto arr = extractIntArray(line, key);
    if (arr.size() >= 4) {
        return {arr[0], arr[1], arr[2], arr[3]};
    }
    return {0, 0, 0, 0};
}

bool AssetLoader::LoadAll() {
    Logger::Info("Loading game assets from {}", configPath_);
    
    bool success = true;
    success &= LoadInitItem();
    success &= LoadInitMonster();
    success &= LoadInitNPC();
    success &= LoadInitSkill();
    success &= LoadQuest();
    success &= LoadGoods();
    success &= LoadItemGroup();
    success &= LoadGenMonster();
    success &= LoadEtc();
    success &= LoadPrefix();
    
    if (success) {
        Logger::Info("Asset loading complete:");
        Logger::Info("  - Items: {}", items_.size());
        Logger::Info("  - Monsters: {}", monsters_.size());
        Logger::Info("  - NPCs: {}", npcs_.size());
        Logger::Info("  - Skills: {}", skills_.size());
        Logger::Info("  - Quests: {}", quests_.size());
        Logger::Info("  - Goods lists: {}", goods_.size());
        Logger::Info("  - Item groups: {}", itemGroups_.size());
        Logger::Info("  - Spawn points: {}", genMonsterSpawns_.size());
        Logger::Info("  - Teleporters: {}", teleporters_.size());
        Logger::Info("  - Prefixes: {}", prefixes_.size());
    } else {
        Logger::Error("Some asset files failed to load");
    }
    
    return success;
}

bool AssetLoader::LoadInitItem() {
    std::string filePath = configPath_ + "/InitItem.txt";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        Logger::Error("Failed to open {}", filePath);
        return false;
    }
    
    std::string line;
    int itemCount = 0;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line.find("(item") != 0) {
            continue;
        }
        
        auto item = std::make_shared<ItemTemplate>();
        item->index = std::stoi(extractValue(line, "Index"));
        item->name = extractValue(line, "name");
        item->image = extractQuotedString(line, "Image");
        
        auto action = extractIntArray(line, "Action");
        if (action.size() >= 2) {
            item->actionType = action[0];
            item->actionValue = action[1];
        }
        
        // Parse class (e.g., "weapon sword")
        size_t classPos = line.find("(class ");
        if (classPos != std::string::npos) {
            size_t start = classPos + 7;
            size_t end = line.find(')', start);
            if (end != std::string::npos) {
                item->itemClass = trim(line.substr(start, end - start));
            }
        }
        
        item->code = extractIntArray(line, "code");
        item->country = extractIntArray(line, "country");
        item->level = std::stoi(extractValue(line, "level"));
        item->wearLocation = std::stoi(extractValue(line, "wear"));
        
        // Parse limit class
        size_t limitPos = line.find("(limit ");
        if (limitPos != std::string::npos) {
            size_t start = limitPos + 7;
            size_t end = line.find(')', start);
            if (end != std::string::npos) {
                std::string limitStr = trim(line.substr(start, end - start));
                std::istringstream iss(limitStr);
                iss >> item->limitClass;
                item->isKnightOnly = (item->limitClass == "Knight");
                item->isArcherOnly = (item->limitClass == "archer");
            }
        }
        
        item->range = std::stoi(extractValue(line, "range"));
        item->buyPrice = std::stoi(extractValue(line, "buy"));
        item->sellPrice = std::stoi(extractValue(line, "sell"));
        item->endurance = std::stoi(extractValue(line, "endurance"));
        
        // Parse specialty section for stats
        size_t specPos = line.find("(specialty");
        if (specPos != std::string::npos) {
            std::string specialty = line.substr(specPos);
            
            auto aspeed = extractIntArray(specialty, "aspeed");
            if (!aspeed.empty()) item->attackSpeed = aspeed[0];
            
            auto attack = extractIntArray(specialty, "Attack");
            if (attack.size() >= 2) {
                item->minAttack = attack[0];
                item->maxAttack = attack[1];
            }
            
            auto hit = extractIntArray(specialty, "hit");
            if (!hit.empty()) item->hitRate = hit[0];
            
            auto defense = extractIntArray(specialty, "defense");
            if (!defense.empty()) item->defense = defense[0];
            
            auto dodge = extractIntArray(specialty, "dodge");
            if (!dodge.empty()) item->dodge = dodge[0];
            
            auto absorb = extractIntArray(specialty, "absorb");
            if (!absorb.empty()) item->absorb = absorb[0];
            
            auto refresh = extractIntArray(specialty, "refresh hp");
            if (!refresh.empty()) item->refreshHP = refresh[0];
        }
        
        // Parse use/cooltime for consumables
        auto useVal = extractValue(line, "use");
        if (!useVal.empty()) {
            item->useType = std::stoi(useVal);
            item->coolTime = std::stoi(extractValue(line, "cooltime"));
            item->effectId = std::stoi(extractValue(line, "effect"));
        }
        
        items_[item->index] = item;
        itemCount++;
    }
    
    Logger::Info("Loaded {} items from InitItem.txt", itemCount);
    return itemCount > 0;
}

bool AssetLoader::LoadInitMonster() {
    std::string filePath = configPath_ + "/InitMonster.txt";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        Logger::Error("Failed to open {}", filePath);
        return false;
    }
    
    std::string line;
    int monsterCount = 0;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line.find("(monster") != 0) {
            continue;
        }
        
        auto monster = std::make_shared<MonsterTemplate>();
        monster->index = std::stoi(extractValue(line, "index"));
        monster->name = extractValue(line, "name");
        monster->country = extractIntArray(line, "country");
        monster->race = std::stoi(extractValue(line, "race"));
        monster->level = std::stoi(extractValue(line, "level"));
        monster->aiType = std::stoi(extractValue(line, "ai"));
        monster->range = std::stoi(extractValue(line, "range"));
        monster->sight = extractIntPair(line, "sight");
        monster->exp = std::stoi(extractValue(line, "exp"));
        
        auto itemGroup = extractIntArray(line, "itemgroup");
        if (itemGroup.size() >= 2) {
            monster->itemGroup = itemGroup[0];
            monster->itemGroupChance = itemGroup[1];
        }
        
        monster->str = std::stoi(extractValue(line, "str"));
        monster->hth = std::stoi(extractValue(line, "hth"));
        monster->int_ = std::stoi(extractValue(line, "int"));
        monster->wis = std::stoi(extractValue(line, "wis"));
        monster->dex = std::stoi(extractValue(line, "dex"));
        monster->hp = std::stoi(extractValue(line, "hp"));
        monster->mp = std::stoi(extractValue(line, "mp"));
        monster->attackSpeed = std::stoi(extractValue(line, "aspeed"));
        monster->hit = std::stoi(extractValue(line, "hit"));
        monster->dodge = std::stoi(extractValue(line, "dodge"));
        
        auto attack = extractIntArray(line, "attack");
        if (attack.size() >= 3) {
            monster->minAttack = attack[1];
            monster->maxAttack = attack[2];
        }
        
        auto defense = extractIntArray(line, "defense");
        if (defense.size() >= 2) {
            monster->minDefense = defense[0];
            monster->maxDefense = defense[1];
        }
        
        monster->absorb = std::stoi(extractValue(line, "absorb"));
        
        auto mspeed = extractIntArray(line, "mspeed");
        if (mspeed.size() >= 2) {
            monster->moveSpeed = {mspeed[0], mspeed[1]};
        }
        
        monster->resist = extractIntArray(line, "resist");
        
        // Parse quest drops
        size_t questPos = line.find("(quest ");
        if (questPos != std::string::npos) {
            std::string questSection = line.substr(questPos);
            // Simple parsing - can be enhanced for complex cases
            auto questArr = extractIntArray(questSection, "quest");
            if (questArr.size() >= 4) {
                monster->questDrops.push_back({questArr[0], questArr[1], questArr[2], questArr[3]});
            }
        }
        
        monsters_[monster->index] = monster;
        monsterCount++;
    }
    
    Logger::Info("Loaded {} monsters from InitMonster.txt", monsterCount);
    return monsterCount > 0;
}

bool AssetLoader::LoadInitNPC() {
    std::string filePath = configPath_ + "/InitNPC.txt";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        Logger::Error("Failed to open {}", filePath);
        return false;
    }
    
    std::string line;
    int npcCount = 0;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line.find("(gennpc") != 0) {
            continue;
        }
        
        auto npc = std::make_shared<NPCTemplate>();
        npc->index = std::stoi(extractValue(line, "index"));
        npc->country = extractIntArray(line, "country");
        npc->kind = std::stoi(extractValue(line, "kind"));
        npc->shape = std::stoi(extractValue(line, "shape"));
        npc->htmlId = std::stoi(extractValue(line, "html"));
        npc->mapId = std::stoi(extractValue(line, "map"));
        
        auto xy = extractIntArray(line, "xy");
        if (xy.size() >= 3) {
            npc->position = {xy[0], xy[1]};
            npc->z = xy[2];
        }
        
        auto dir = extractIntArray(line, "dir");
        if (dir.size() >= 2) {
            npc->direction = {dir[0], dir[1]};
        }
        
        auto questVal = extractValue(line, "quest");
        if (!questVal.empty()) {
            auto questArr = extractIntArray(line, "quest");
            if (questArr.size() >= 2) {
                npc->questId = questArr[0];
            }
        }
        
        auto cidVal = extractValue(line, "cid");
        if (!cidVal.empty()) {
            npc->cid = std::stoi(cidVal);
        }
        
        auto warVal = extractValue(line, "warrelation");
        if (!warVal.empty()) {
            npc->warRelation = std::stoi(warVal);
        }
        
        npcs_[npc->index] = npc;
        npcCount++;
    }
    
    Logger::Info("Loaded {} NPCs from InitNPC.txt", npcCount);
    return npcCount > 0;
}

bool AssetLoader::LoadInitSkill() {
    std::string filePath = configPath_ + "/InitSkill.txt";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        Logger::Error("Failed to open {}", filePath);
        return false;
    }
    
    std::string line;
    int skillCount = 0;
    int currentClass = 0;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';') {
            continue;
        }
        
        // Check for class change comments
        if (line.find(";knight") != std::string::npos) currentClass = 0;
        else if (line.find(";archer") != std::string::npos) currentClass = 1;
        else if (line.find(";magician") != std::string::npos) currentClass = 2;
        else if (line.find(";hunter") != std::string::npos) currentClass = 3;
        
        if (line.find("(skill") != 0) {
            continue;
        }
        
        auto skill = std::make_shared<SkillTemplate>();
        skill->classId = currentClass;
        skill->index = std::stoi(extractValue(line, "index"));
        skill->redistribute = std::stoi(extractValue(line, "redistribute"));
        skill->limit = extractIntArray(line, "limit");
        skill->maxLevel = std::stoi(extractValue(line, "maxlevel"));
        skill->mpCost = std::stoi(extractValue(line, "mp"));
        skill->lastTime = std::stoi(extractValue(line, "lasttime"));
        skill->delay = extractIntArray(line, "delay");
        skill->value1 = std::stoi(extractValue(line, "value1"));
        skill->value2 = std::stoi(extractValue(line, "value2"));
        
        skills_[skill->classId * 1000 + skill->index] = skill;
        skillCount++;
    }
    
    Logger::Info("Loaded {} skills from InitSkill.txt", skillCount);
    return skillCount > 0;
}

bool AssetLoader::LoadQuest() {
    std::string filePath = configPath_ + "/Quest.txt";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        Logger::Error("Failed to open {}", filePath);
        return false;
    }
    
    std::string line;
    int questCount = 0;
    
    // Simplified quest loading - full parser would be more complex
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line.find("(quest") != 0) {
            continue;
        }
        
        auto quest = std::make_shared<QuestData>();
        auto indexArr = extractIntArray(line, "index");
        if (indexArr.size() >= 2) {
            quest->index = indexArr[0];
            quest->step = indexArr[1];
        }
        
        // Parse linked flag
        auto linkedVal = extractValue(line, "linked");
        if (!linkedVal.empty()) {
            quest->reward.linked = std::stoi(linkedVal);
        }
        
        // Parse html/guide
        auto htmlVal = extractValue(line, "html");
        if (!htmlVal.empty()) {
            quest->reward.htmlId = std::stoi(htmlVal);
        }
        
        auto guideVal = extractValue(line, "guide");
        if (!guideVal.empty()) {
            quest->reward.guide = std::stoi(guideVal);
        }
        
        quests_[quest->index * 100 + quest->step] = quest;
        questCount++;
    }
    
    Logger::Info("Loaded {} quest entries from Quest.txt", questCount);
    return questCount > 0;
}

bool AssetLoader::LoadGoods() {
    std::string filePath = configPath_ + "/Goods.txt";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        Logger::Error("Failed to open {}", filePath);
        return false;
    }
    
    std::string line;
    int goodsCount = 0;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line.find("(goods") != 0) {
            continue;
        }
        
        auto goods = std::make_shared<GoodsList>();
        goods->index = std::stoi(extractValue(line, "index"));
        
        // Parse items - simplified
        size_t itemPos = line.find("(item ");
        if (itemPos != std::string::npos) {
            auto itemArr = extractIntArray(line.substr(itemPos), "item");
            // Handle multiple items in one goods list
            for (size_t i = 0; i + 2 < itemArr.size(); i += 3) {
                goods->items.push_back({itemArr[i], itemArr[i+1], itemArr[i+2]});
            }
        }
        
        goods_[goods->index] = goods;
        goodsCount++;
    }
    
    Logger::Info("Loaded {} goods lists from Goods.txt", goodsCount);
    return goodsCount > 0;
}

bool AssetLoader::LoadItemGroup() {
    std::string filePath = configPath_ + "/ItemGroup.txt";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        Logger::Error("Failed to open {}", filePath);
        return false;
    }
    
    std::string line;
    int groupCount = 0;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line.find("(group") != 0) {
            continue;
        }
        
        auto group = std::make_shared<ItemGroup>();
        group->index = std::stoi(extractValue(line, "index"));
        
        auto moneyArr = extractIntArray(line, "money");
        if (moneyArr.size() >= 2) {
            group->moneyDrop = {moneyArr[0], moneyArr[1]};
        }
        
        auto itemArr = extractIntArray(line, "item");
        for (size_t i = 0; i + 2 < itemArr.size(); i += 3) {
            group->items.push_back({itemArr[i], itemArr[i+1], itemArr[i+2]});
        }
        
        itemGroups_[group->index] = group;
        groupCount++;
    }
    
    Logger::Info("Loaded {} item groups from ItemGroup.txt", groupCount);
    return groupCount > 0;
}

bool AssetLoader::LoadGenMonster() {
    std::string filePath = configPath_ + "/GenMonster.txt";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        Logger::Error("Failed to open {}", filePath);
        return false;
    }
    
    std::string line;
    int spawnCount = 0;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line.find("(genmonster") != 0) {
            continue;
        }
        
        auto spawn = std::make_shared<GenMonsterSpawn>();
        spawn->index = std::stoi(extractValue(line, "index"));
        spawn->mapId = std::stoi(extractValue(line, "map"));
        spawn->area = std::stoi(extractValue(line, "area"));
        spawn->maxCount = std::stoi(extractValue(line, "max"));
        spawn->cycle = std::stoi(extractValue(line, "cycle"));
        
        auto rect = extractIntQuad(line, "rect");
        spawn->rect = rect;
        
        genMonsterSpawns_.push_back(spawn);
        spawnCount++;
    }
    
    Logger::Info("Loaded {} monster spawn points from GenMonster.txt", spawnCount);
    return spawnCount > 0;
}

bool AssetLoader::LoadEtc() {
    std::string filePath = configPath_ + "/Etc.txt";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        Logger::Error("Failed to open {}", filePath);
        return false;
    }
    
    std::string line;
    int teleporterCount = 0;
    int idCounter = 1;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line.find("(goto") != 0) {
            continue;
        }
        
        auto teleporter = std::make_shared<TeleporterData>();
        teleporter->id = idCounter++;
        
        auto targetMapVal = extractValue(line, "goto");
        if (!targetMapVal.empty()) {
            teleporter->targetMapId = std::stoi(targetMapVal);
        }
        
        auto xyArr = extractIntArray(line, "goto");
        if (xyArr.size() >= 4) {
            teleporter->sourceMapId = xyArr[0];
            teleporter->sourcePos = {xyArr[1], xyArr[2], xyArr[3]};
        }
        
        teleporters_.push_back(teleporter);
        teleporterCount++;
    }
    
    Logger::Info("Loaded {} teleporters from Etc.txt", teleporterCount);
    return teleporterCount > 0;
}

bool AssetLoader::LoadPrefix() {
    std::string filePath = configPath_ + "/Prefix.txt";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        Logger::Error("Failed to open {}", filePath);
        return false;
    }
    
    std::string line;
    int prefixCount = 0;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line.find("(prefix") != 0) {
            continue;
        }
        
        auto prefix = std::make_shared<PrefixData>();
        prefix->index = std::stoi(extractValue(line, "index"));
        prefix->grade = std::stoi(extractValue(line, "grade"));
        
        // Parse option type and value
        auto optionArr = extractIntArray(line, "option");
        if (optionArr.size() >= 2) {
            prefix->optionType = optionArr[0];
            prefix->optionValue = optionArr[1];
        }
        
        prefix->applyTo = extractIntArray(line, "applyto");
        
        prefixes_[prefix->index] = prefix;
        prefixCount++;
    }
    
    Logger::Info("Loaded {} prefixes from Prefix.txt", prefixCount);
    return prefixCount > 0;
}

const ItemTemplate* AssetLoader::GetItem(int index) const {
    auto it = items_.find(index);
    return it != items_.end() ? it->second.get() : nullptr;
}

const MonsterTemplate* AssetLoader::GetMonster(int index) const {
    auto it = monsters_.find(index);
    return it != monsters_.end() ? it->second.get() : nullptr;
}

const NPCTemplate* AssetLoader::GetNPC(int index) const {
    auto it = npcs_.find(index);
    return it != npcs_.end() ? it->second.get() : nullptr;
}

const SkillTemplate* AssetLoader::GetSkill(int classId, int index) const {
    auto it = skills_.find(classId * 1000 + index);
    return it != skills_.end() ? it->second.get() : nullptr;
}
