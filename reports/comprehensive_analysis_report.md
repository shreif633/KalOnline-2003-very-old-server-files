# KalOnline 2003 Server Files - Comprehensive Analysis Report

## Executive Summary

This report provides a comprehensive analysis of the KalOnline 2003 MMORPG server files. KalOnline is a classic Korean fantasy MMORPG that was popular in the early 2000s. These server files represent a complete game server implementation including authentication, database, and main game servers.

## 1. Project Overview

### 1.1 Project Identity
- **Project Name**: KalOnline-2003-very-old-server-files
- **Game Type**: MMORPG (Massively Multiplayer Online Role-Playing Game)
- **Era**: Early 2000s (2003)
- **Origin**: South Korea (Inixsoft)
- **Language Support**: English (LangEN.txt)

### 1.2 Server Architecture

The server infrastructure consists of three main server components:

#### 1.2.1 Authentication Server (AuthSvrT.exe)
- **Port**: 50001 (main), 50002 (command)
- **Purpose**: Handles user authentication, login verification, and session management
- **Configuration**: AuthConfig.txt
- **Features**:
  - ODBC Database connection (KAL_AUTH)
  - Subnet security configuration
  - Netmarble authentication support
  - Billing system integration
  - Party system support
  - Quest system activation
  - Storage system support

#### 1.2.2 Database Server (DBSvrT.exe)
- **Port**: 40001
- **Purpose**: Manages all persistent game data and database operations
- **Configuration**: DBConfig.txt
- **Features**:
  - ODBC Database connection (KAL_DB)
  - Character data persistence
  - Item storage management
  - Quest progress tracking
  - Netmarble authsocket activation

#### 1.2.3 Main Game Server (MainSvrT.exe)
- **Port**: 30001
- **Purpose**: Core game logic, world management, and player interactions
- **Configuration**: MainConfig.txt
- **Features**:
  - Map management
  - NPC interactions
  - Monster spawning and AI
  - Combat system
  - Party mechanics
  - Quest system
  - Maximum 100 concurrent users (configurable)
  - 20 worker threads

## 2. Configuration Analysis

### 2.1 Server Configuration Files

#### MainConfig.txt
```
Title = "KAL Main Server"
NumberOfThreads = 20
Language = "LangEN.txt"
Country = 255
Port = 30001
AuthServer = "127.0.0.1"
DBServer = "127.0.0.1"
MaxUser = 100
ItemVer = 1
ServerId = 1
AgeLimit = 0
```

**System Flags**:
- 0: Closed beta test mode
- 1: No alive packet (test server)
- 2: Pay system
- 4: Party system
- 5: Quest system

#### AuthConfig.txt
```
Title = "KAL Authentication Server"
ODBC = "KAL_AUTH"
Port = 50001
CMDPort = 50002
BillingServer = "127.0.0.1"
BillingPort = 55903
```

**System Flags**:
- 1: Cosmo server (test)
- 4: Party system
- 5: Quest system
- 6: Storage system
- 7: Billing system

#### DBConfig.txt
```
Title = "KAL Database Server"
ODBC = "KAL_DB"
Port = 40001
```

### 2.2 Network Configuration

All servers are configured for localhost operation (127.0.0.1) by default but can be modified for WAN deployment:
- Auth Server: 50001/50002
- DB Server: 40001
- Main Server: 30001
- Billing: 55903

## 3. Game World Structure

### 3.1 Map System

The game world consists of a grid-based map system with 148+ map tiles:

**Map File Format**: `.ksm` files located in `/Map/` directory
**Naming Convention**: `n_XXX_YYY.ksm` where XXX and YYY represent grid coordinates

**Map Grid Layout**:
- Starting zone: 0,0 to 7,5 (beginner area)
- Main continent: 25-36 coordinates
- Special zones: Various dungeon and town maps

**Map Index Types**:
- Index 0: Standard outdoor maps
- Index 1: Town/safe zones
- Index 2: Special/dungeon maps
- Index 3: Regular hunting grounds

### 3.2 Teleportation System

Two types of fast travel:

#### Goto Points (NPC Teleporters)
- 80+ teleport locations
- Connect major towns and cities
- Examples:
  - ID 1002: Beginner town
  - ID 1004: Knight city
  - ID 1006: Mage city
  - ID 1012: Archer city

#### Portal System
- Inter-zone connections
- Country-restricted access (0, 1, 2)
- Coordinate-based entry/exit points
- Dungeon and special area access

## 4. Character System

### 4.1 Character Classes

Three playable classes with distinct characteristics:

#### Knight (Class 0)
- **Role**: Melee tank/DPS
- **Weapons**: Swords, Shields
- **Armor**: Heavy armor
- **Key Stats**: Strength, Health
- **Special Skills**: 
  - Weapon upgrades
  - Thunder attack
  - Shield bash
  - Berserk mode
  - Defense chants

#### Mage (Class 1)
- **Role**: Magic DPS/Support
- **Weapons**: Wands/Staves
- **Armor**: Cloth robes
- **Key Stats**: Intelligence, Wisdom
- **Special Skills**:
  - Fire magic
  - Ice magic
  - Lightning magic
  - Self-healing
  - Shock magic

#### Archer (Class 2)
- **Role**: Ranged DPS
- **Weapons**: Bows
- **Armor**: Light armor
- **Key Stats**: Dexterity
- **Special Skills**: (Similar progression system)

### 4.2 Skill System

**Skill Structure**:
```
(skill (class X) (index Y) (redistribute Z) (limit A B C D) 
       (maxlevel L) (mp M) (lasttime T) (delay D1 D2 D3) 
       (value1 V1) (value2 V2))
```

**Knight Skills** (35+ skills):
- Basic: Run, Behead
- Combat: Thunder, Mock, Transcendental Strike
- Defensive: Upblock Shield, Protect
- Advanced: Fatal Wound, Wild Accuracy
- Ultimate: Reincarnate, Berserk

**Mage Skills**:
- Elemental: Fire, Ice, Lightning magic
- Support: Self-heal, Any heal
- Offensive: Shock magic, Protect Range

### 4.3 Attribute System

**Primary Attributes**:
- STR (Strength): Physical damage, carrying capacity
- HTH (Health): HP pool, physical resilience
- INT (Intelligence): Magic damage, MP pool
- WIS (Wisdom): Magic defense, MP regeneration
- DEX (Dexterity): Accuracy, dodge rate, attack speed

## 5. Item System

### 5.1 Item Categories

Based on InitItem.txt analysis (100+ items documented):

#### Weapons
- **Swords** (Knight): 15+ variants, levels 1-40+
  - Attack range: 3-184 damage
  - Attack speed: 700ms
  - Hit rates: 15-55
  
- **Bows** (Archer): 10+ variants
  - Attack range: 160-176 (long range)
  - Attack speed: 800ms
  
- **Wands** (Mage): 10+ variants
  - Magic attack: 2-103
  - Physical attack: 1-59
  - Attack speed: 1000ms

#### Armor Sets
Each class has tiered armor sets at levels 8, 16, 24, 32+:

**Knight Armor**:
- Upper armor, Helmet, Gauntlet, Boots, Lower armor, Shield
- High defense, moderate dodge
- Absorb stats for damage reduction

**Archer Armor**:
- Similar structure to Knight
- Higher dodge stats
- Lower defense values

**Mage Armor**:
- Cloth/robe equipment
- Elemental resistances (Fire, Ice, Lightning)
- High dodge, low defense

### 5.2 Item Properties

**Standard Attributes**:
- Level requirement
- Class restriction
- Buy/Sell prices
- Endurance (durability)
- Defense/Attack values
- Dodge/Accuracy modifiers
- Elemental resistances
- Special effects (HP/MP bonuses)

### 5.3 Prefix/Enchantment System

90+ prefix types for item customization:

**Stat Prefixes**:
- Strength prefixes (levels 1-10)
- Health prefixes (levels 1-10)
- Intelligence prefixes (levels 1-10)
- Wisdom prefixes (levels 1-10)
- Dexterity prefixes (levels 1-10)

**Combat Prefixes**:
- Attack value additions (0-14 damage)
- Magic power enhancements (0-14 magic)
- Defense bonuses (1-5)
- HP/MP increases (50-360 HP, 25-150 MP)
- Hit/Dodge improvements

**Combined Prefixes**:
- Multi-stat combinations
- Level-gated powerful prefixes
- Special hybrid builds (STR+HTH+DEX, etc.)

### 5.4 Consumables

- Refresh potions (HP recovery: 100-300)
- Time-based consumables (1, 7, 15, 30 day items)
- Event items
- Cash shop items ( Goods.txt)

## 6. Monster System

### 6.1 Monster Design

**Monster Attributes**:
```
(monster (name X) (index Y) (country 0 1 2) (race R) (level L) 
         (ai A) (range R) (sight S1 S2) (exp E) (itemgroup I G)
         (str S) (hth H) (int I) (wis W) (dex D) 
         (hp HP) (mp MP) (aspeed AS) (hit HT) (dodge DG)
         (attack MIN MAX) (magic M) (defense DEF) 
         (absorb AB) (mspeed MS1 MS2) (resist R1 R2 R3 R4 R5))
```

### 6.2 Monster Progression

**Low Level Monsters** (1-10):
- Slimes, basic creatures
- HP: 1-45
- Attack: 7-23
- EXP: 2-22
- Drop groups: 1-8

**Scaling Characteristics**:
- Stats increase with level
- Better item drops at higher levels
- Varied AI behaviors (melee, ranged, magical)
- Elemental resistances develop

### 6.3 Monster Spawning

**GenMonster.txt Configuration**:
- Area-based spawning zones
- Maximum spawn counts per area
- Spawn cycle timing
- Rectangular spawn boundaries
- Map-specific configurations

Example:
```
(genmonster (index 2) (map 0) (area 6) (max 10) (cycle 1) 
            (rect 8019 8170 8079 8211))
```

## 7. NPC System

### 7.1 NPC Types

**NPC Kind Classification**:
- Kind 0: General NPCs (teleporters, officials)
- Kind 1: Merchants (weapon, armor, item shops)
- Kind 2: Service NPCs (repair, storage, etc.)
- Kind 3: Quest givers
- Kind 4: Special event NPCs

### 7.2 NPC Distribution

**Starting Zone** (Map 0):
- 38+ NPCs in beginner area
- Multiple shopkeepers per country
- Quest initiators
- Teleportation services

**Major Cities**:
- Country-specific NPCs (0, 1, 2 = Knight, Archer, Mage nations)
- Specialized merchants
- Class trainers
- Quest hubs

### 7.3 NPC Behavior

```
(npc (kind K) (move M))
(gennpc (index I) (country C) (kind K) (shape S) 
        (html H) (map M) (xy X Y Z) (dir DX DY))
```

- Static vs. moving NPCs
- Direction/orientation settings
- HTML dialog references
- Precise coordinate placement

## 8. Quest System

### 8.1 Quest Structure

```
(quest (index QID STEP)
  (case (if CONDITIONS)
    (then REWARDS/ACTIONS)))
```

### 8.2 Quest Components

**Conditions**:
- Class requirements
- Level thresholds
- Item possession/trade
- Party status
- Previous quest completion
- Specialty/skill levels
- Contribution points

**Actions/Rewards**:
- HTML dialog display
- Link to next quest step
- Item grants/removals
- Experience points
- Contribution points
- SU points (skill upgrade?)
- Quest state saves
- Quest completion flags

### 8.3 Quest Examples

**Beginner Quest Chain** (Quest 0):
- Multiple branching paths
- Class-specific objectives
- Progressive difficulty
- Tutorial-style introduction

**Level Milestone Quests** (Quest 7000):
- Level 20, 40, 60 checkpoints
- Item collection requirements
- Story progression markers

**Event Quests**:
- Special conditions
- Limited-time rewards
- Guild/group activities

## 9. Economy System

### 9.1 Currency

- Base unit: Coins (Item ID 361/362)
- Shop buy/sell mechanics
- Player trading potential
- Gold sinks (repair, services)

### 9.2 Shop System

**Goods.txt Configuration**:
- Pre-defined shop packages
- Time-limited items (1, 7, 15, 30 days)
- Cash shop integration
- Bundle deals

**Pricing Strategy**:
- Tier-based pricing by level
- Class-specific equipment costs
- Stat prefix value scaling
- Example prices:
  - Level 1 sword: 4 coins
  - Level 40 sword: 52,000 coins (sell only)
  - HP potion: 20 coins

### 9.3 Item Groups (Loot Tables)

**Group System** (140+ groups):
- Monster-specific drop tables
- Weighted random selection
- Money + item combinations
- Level-appropriate rewards

Example:
```
(group (index 101) (money 920 2) 
       (item (960 443 0) (1000 47 0)))
```

## 10. Technical Specifications

### 10.1 Executables

| File | Purpose | Size Estimate |
|------|---------|---------------|
| AuthSvrT.exe | Authentication Server | Windows PE executable |
| DBSvrT.exe | Database Server | Windows PE executable |
| MainSvrT.exe | Main Game Server | Windows PE executable |
| dbghelp.dll | Debug helper library | Windows system DLL |

### 10.2 Symbol Files

- AuthSvrT.pdb
- DBSvrT.pdb  
- MainSvrT.pdb

These PDB (Program Database) files contain debugging symbols useful for development and troubleshooting.

### 10.3 Log Files

**Directory Structure**:
- `/AuL/` - Authentication logs
- `/DBL/` - Database logs
- `/MaL/` - Main server logs

**Log Format**:
```
[PID YYYY/MM/DD HH:MM:SS]: Message
```

Example errors captured:
- Invalid packet type errors
- Socket read failures
- Connection issues

### 10.4 Database Files

- `/DB Files/kal_auth.bak` - Authentication database backup
- `/DB Files/kal_db.bak` - Game database backup

Requires MSSQL Server or compatible ODBC driver.

## 11. Localization

### 11.1 Language Support

**Current**: English (LangEN.txt)
- Monster names
- NPC dialogs
- Item descriptions
- Skill names
- System messages

**Encoding Issues**:
Some text shows encoding corruption (original Korean characters not properly rendered in English file).

### 11.2 Country System

Three competing nations:
- Country 0: Knight nation
- Country 1: Archer nation
- Country 2: Mage nation

Features:
- Country-specific NPCs
- War relations
- PvP considerations
- Exclusive content per nation

## 12. System Requirements (Inferred)

### Server Side
- **OS**: Windows (based on .exe/.pdb files)
- **Database**: MSSQL Server or compatible ODBC
- **Memory**: Minimum 512MB RAM (2003 era)
- **Network**: TCP/IP sockets on ports 30001, 40001, 50001-2

### Client Side (Estimated)
- **OS**: Windows 98/2000/XP
- **CPU**: Pentium II/III 500MHz+
- **RAM**: 128-256MB
- **Graphics**: DirectX-compatible 3D accelerator
- **Storage**: ~500MB-1GB
- **Network**: 56k modem minimum, broadband recommended

## 13. Security Considerations

### 13.1 Current State

**Default Configuration Risks**:
- All servers bound to 127.0.0.1
- Default ports well-known
- No encryption mentioned
- Plaintext config files
- Debug symbols present

### 13.2 Recommendations

For production deployment:
1. Change all default passwords
2. Configure proper firewall rules
3. Enable network encryption
4. Remove debug symbols
5. Implement rate limiting
6. Add anti-cheat measures
7. Secure ODBC connections
8. Regular backup procedures

## 14. Development Status

### 14.1 Completeness Assessment

**Fully Implemented**:
- ✅ Three-server architecture
- ✅ Character class system
- ✅ Skill system (70+ skills)
- ✅ Item system (200+ items)
- ✅ Monster system (100+ monsters)
- ✅ NPC system (40+ NPCs)
- ✅ Quest framework
- ✅ Map system (148+ tiles)
- ✅ Teleportation network
- ✅ Shop/economy system
- ✅ Loot tables

**Partially Documented**:
- ⚠️ Quest content (structure exists, limited quests defined)
- ⚠️ High-level content (level 60+ cap visible but incomplete)
- ⚠️ Endgame raids/dungeons
- ⚠️ PvP systems (war relations mentioned)

**Missing/Unclear**:
- ❌ Client files
- ❌ Complete quest chain content
- ❌ Guild system details
- ❌ Marriage/social systems
- ❌ Auction house/trading post
- ❌ Complete event system

### 14.2 Version Identification

Based on configuration and content:
- Likely closed beta or early live version
- Level cap appears to be 60 (based on quest references)
- Item levels go up to 40+
- Some systems marked as "reserved" or commented out

## 15. File Inventory

### Configuration Files (Root)
- README.md
- MainConfig.txt
- AuthConfig.txt
- DBConfig.txt
- LangEN.txt
- DB_Exec.txt

### Configuration Files (/Config/)
- InitMap.txt - Map initialization
- InitItem.txt - Item definitions
- InitSkill.txt - Skill definitions
- InitNPC.txt - NPC placements
- InitMonster.txt - Monster stats
- GenMonster.txt - Monster spawns
- Quest.txt - Quest definitions
- Prefix.txt - Item enchantments
- Goods.txt - Shop items
- ItemGroup.txt - Loot tables
- Etc.txt - Teleport points

### Map Files (/Map/)
- 148 .ksm files (binary map data)

### Executables
- AuthSvrT.exe + .pdb
- DBSvrT.exe + .pdb
- MainSvrT.exe + .pdb
- dbghelp.dll

### Database
- /DB Files/kal_auth.bak
- /DB Files/kal_db.bak

### Logs
- /AuL/*.log - Auth logs
- /DBL/*.log - DB logs
- /MaL/*.log - Main logs

## 16. Restoration/Deployment Guide

### 16.1 Prerequisites
1. Windows Server environment (or Wine on Linux)
2. MSSQL Server or compatible database
3. ODBC drivers configured

### 16.2 Setup Steps

1. **Database Setup**:
   ```sql
   -- Restore kal_auth.bak to KAL_AUTH database
   -- Restore kal_db.bak to KAL_DB database
   ```

2. **ODBC Configuration**:
   - Create System DSN "KAL_AUTH"
   - Create System DSN "KAL_DB"

3. **Configuration Updates**:
   - Edit MainConfig.txt - set AuthServer and DBServer IPs
   - Edit AuthConfig.txt - update SubNet for your network
   - Edit DBConfig.txt - configure SubNet

4. **Port Configuration**:
   - Open firewall ports: 30001, 40001, 50001, 50002

5. **Server Startup Order**:
   1. Start DBSvrT.exe
   2. Start AuthSvrT.exe
   3. Start MainSvrT.exe

6. **Verification**:
   - Check log files in /MaL/, /AuL/, /DBL/
   - Monitor for successful initialization messages

## 17. Conclusion

The KalOnline 2003 server files represent a remarkably complete MMORPG server implementation from the early 2000s era. The architecture demonstrates solid separation of concerns with dedicated authentication, database, and game servers. The game systems are well-developed with extensive content including multiple character classes, hundreds of items, diverse monsters, and a functional quest framework.

**Key Strengths**:
- Clean three-tier server architecture
- Comprehensive item and skill systems
- Well-structured configuration files
- Extensible design patterns
- Good separation of game data from logic

**Notable Limitations**:
- Requires Windows environment
- Dependent on proprietary MSSQL database
- Missing client-side files
- Some content appears incomplete
- No modern security features

**Historical Value**:
These files provide valuable insight into early 2000s MMORPG design and architecture, showcasing design patterns and game mechanics that influenced the genre's evolution.

---

*Report generated from analysis of KalOnline-2003 server files*
*Total files analyzed: 203*
*Configuration files: 17*
*Map files: 148*
*Executable files: 4*
*Log files: Multiple*
