# Product Requirements Document (PRD)
# KalOnline MMORPG Server Implementation

## Document Information

| Field | Value |
|-------|-------|
| **Project Name** | KalOnline Server Restoration |
| **Version** | 1.0 |
| **Status** | Analysis Complete |
| **Date** | 2024 |
| **Product Type** | MMORPG Game Server |
| **Era** | Early 2000s (2003) |

---

## 1. Executive Summary

### 1.1 Product Vision
KalOnline is a classic Korean fantasy MMORPG from 2003 featuring three competing nations, distinct character classes, and a rich skill-based progression system. This document outlines the requirements and specifications for deploying and operating the KalOnline game server.

### 1.2 Target Audience
- Game server administrators
- Private server enthusiasts
- Game development researchers
- MMORPG preservationists

### 1.3 Key Value Propositions
- Complete three-tier server architecture
- Proven MMORPG mechanics from successful 2000s era
- Extensive content library (items, monsters, quests, maps)
- Modular configuration system
- Class-based faction warfare

---

## 2. Product Overview

### 2.1 Product Description

KalOnline is a subscription-based (with cash shop) MMORPG featuring:
- **Three Character Classes**: Knight, Mage, Archer
- **Three Competing Nations**: Each tied to a class
- **Level Cap**: 60 (based on quest references)
- **Combat System**: Real-time action combat with skills
- **Progression**: Skill-based advancement with stat allocation
- **Social Features**: Party system, potential guild system
- **PvP**: Nation-based warfare, potential dueling

### 2.2 Core Features

#### 2.2.1 Character System
- **Class Selection**: Choose between Knight, Mage, or Archer at creation
- **Stat Allocation**: STR, HTH, INT, WIS, DEX
- **Skill Learning**: Class-specific skill trees
- **Equipment**: Tiered gear by level and class
- **Customization**: Prefix/enchantment system for items

#### 2.2.2 Combat System
- **Real-time Combat**: Action-based fighting
- **Skill Usage**: MP-based active and passive skills
- **Weapon Types**: Swords, Bows, Wands
- **Defense Mechanics**: Dodge, absorb, resistance stats
- **AI System**: Monster behavior patterns

#### 2.2.3 World Exploration
- **Map System**: 148+ interconnected map tiles
- **Teleportation**: NPC teleporters and portal network
- **Monster Spawning**: Area-based spawn system
- **NPC Interaction**: Shops, services, quests
- **Dungeons**: Special instanced areas (inferred)

#### 2.2.4 Social Systems
- **Party Formation**: Group up to 5 players (estimated)
- **Chat System**: Presumed global/local/party channels
- **Trading**: Player-to-player item exchange (inferred)
- **Friends List**: Standard MMO feature (presumed)

#### 2.2.5 Economy
- **Currency**: Coin-based economy
- **Shops**: NPC vendors with buy/sell mechanics
- **Cash Shop**: Time-limited premium items
- **Loot System**: Monster drop tables
- **Item Repair**: Durability and maintenance costs

---

## 3. User Personas

### 3.1 Primary Personas

#### Persona 1: The Grinder
- **Goal**: Reach max level, acquire best gear
- **Behavior**: Spends hours hunting monsters, optimizing builds
- **Needs**: Balanced EXP curves, rewarding loot drops
- **Frustrations**: Pay-to-win mechanics, unbalanced classes

#### Persona 2: The PvP Warrior
- **Goal**: Dominate in player versus player combat
- **Behavior**: Participates in nation wars, duels
- **Needs**: Fair PvP balance, meaningful rewards
- **Frustrations**: One-shot builds, lag, cheating

#### Persona 3: The Social Player
- **Goal**: Build relationships, join communities
- **Behavior**: Forms parties, helps newbies, chats frequently
- **Needs**: Robust social features, group content
- **Frustrations**: Lack of group activities, toxic community

#### Persona 4: The Collector
- **Goal**: Obtain rare items, complete sets
- **Behavior**: Farms specific monsters, trades extensively
- **Needs**: Diverse item pool, trading mechanisms
- **Frustrations**: Drop rates too low, no auction house

### 3.2 Secondary Personas

- **Casual Explorer**: Logs in occasionally, enjoys exploration
- **Role-Player**: Immerses in lore, creates character backstory
- **Competitor**: Ranks on leaderboards, seeks recognition
- **Altmaker**: Creates multiple characters to experience all classes

---

## 4. Functional Requirements

### 4.1 Authentication & Account Management

#### FR-1.1: User Registration
- **Priority**: Critical
- **Description**: Players must be able to create accounts
- **Acceptance Criteria**:
  - Unique username requirement
  - Password encryption
  - Email validation (optional)
  - Terms of service acceptance

#### FR-1.2: Login System
- **Priority**: Critical
- **Description**: Secure authentication for game access
- **Acceptance Criteria**:
  - Username/password authentication
  - Session token generation
  - Failed login tracking
  - Remember me option

#### FR-1.3: Character Creation
- **Priority**: Critical
- **Description**: Create new game characters
- **Acceptance Criteria**:
  - Select class (Knight/Mage/Archer)
  - Choose starting nation
  - Customize appearance (if supported)
  - Validate character name uniqueness
  - Limit characters per account

#### FR-1.4: Character Selection
- **Priority**: Critical
- **Description**: Choose character to play
- **Acceptance Criteria**:
  - Display all characters on account
  - Show character level, class, equipment
  - Delete character option (with delay)
  - Enter game world selection

### 4.2 Core Gameplay

#### FR-2.1: Character Movement
- **Priority**: Critical
- **Description**: Navigate the game world
- **Acceptance Criteria**:
  - WASD/keybind movement
  - Collision detection
  - Map boundary enforcement
  - Smooth interpolation

#### FR-2.2: Combat Mechanics
- **Priority**: Critical
- **Description**: Engage in real-time combat
- **Acceptance Criteria**:
  - Auto-attack on target
  - Skill activation with hotkeys
  - Damage calculation (attack vs defense)
  - Critical hit system
  - Miss/dodge mechanics

#### FR-2.3: Skill System
- **Priority**: Critical
- **Description**: Learn and use class abilities
- **Acceptance Criteria**:
  - Skill point allocation on level up
  - Prerequisite checking
  - MP cost deduction
  - Cooldown management
  - Buff/debuff application

#### FR-2.4: Experience & Leveling
- **Priority**: Critical
- **Description**: Gain EXP and advance levels
- **Acceptance Criteria**:
  - EXP from monster kills
  - Level-up stat increases
  - EXP curve balancing
  - Death penalty (EXP loss?)
  - Rested EXP bonus (optional)

#### FR-2.5: Item Management
- **Priority**: Critical
- **Description**: Inventory and equipment handling
- **Acceptance Criteria**:
  - Inventory slots (size TBD)
  - Equipment slots (head, body, hands, feet, weapon, etc.)
  - Drag-drop interface
  - Item stacking for consumables
  - Weight limit (if applicable)

#### FR-2.6: Equipment System
- **Priority**: Critical
- **Description**: Equip and upgrade gear
- **Acceptance Criteria**:
  - Class restrictions enforcement
  - Level requirements
  - Stat bonuses application
  - Durability tracking
  - Prefix/enchantment support

### 4.3 World & Environment

#### FR-3.1: Map Loading
- **Priority**: Critical
- **Description**: Seamless zone transitions
- **Acceptance Criteria**:
  - Load .ksm map files
  - Handle map boundaries
  - Portal/gate transitions
  - Minimap display

#### FR-3.2: NPC Interaction
- **Priority**: High
- **Description**: Talk to and use NPCs
- **Acceptance Criteria**:
  - Dialog windows
  - Shop interfaces
  - Service menus (repair, storage)
  - Quest giving/completion

#### FR-3.3: Monster AI
- **Priority**: Critical
- **Description**: Enemy behavior and spawning
- **Acceptance Criteria**:
  - Aggro radius detection
  - Leash distance
  - Attack patterns
  - Loot dropping
  - Respawn timers

#### FR-3.4: Teleportation
- **Priority**: High
- **Description**: Fast travel system
- **Acceptance Criteria**:
  - NPC teleporter dialogs
  - Portal zone triggers
  - Map/file verification
  - Fee system (optional)

### 4.4 Social Features

#### FR-4.1: Party System
- **Priority**: High
- **Description**: Form groups with other players
- **Acceptance Criteria**:
  - Invite/accept/decline mechanics
  - Party member list UI
  - EXP sharing rules
  - Loot distribution options
  - Party解散 (disband)

#### FR-4.2: Chat System
- **Priority**: High
- **Description**: Player communication
- **Acceptance Criteria**:
  - Local chat (proximity-based)
  - Global chat (shout)
  - Party chat
  - Whisper/tell
  - Chat filters/moderation

#### FR-4.3: Trading
- **Priority**: Medium
- **Description**: Player-to-player item exchange
- **Acceptance Criteria**:
  - Trade request/acceptance
  - Trade window UI
  - Item/currency placement
  - Confirmation step
  - Trade lock during confirmation

### 4.5 Quest System

#### FR-5.1: Quest Tracking
- **Priority**: High
- **Description**: Manage active quests
- **Acceptance Criteria**:
  - Quest log UI
  - Objective tracking
  - Completion notifications
  - Quest abandonment option

#### FR-5.2: Quest Conditions
- **Priority**: High
- **Description**: Validate quest requirements
- **Acceptance Criteria**:
  - Level checks
  - Item collection
  - Monster kill counts
  - NPC interaction
  - Previous quest completion

#### FR-5.3: Quest Rewards
- **Priority**: High
- **Description**: Distribute quest completions
- **Acceptance Criteria**:
  - Experience points
  - Item rewards
  - Currency rewards
  - Reputation/contribution
  - Skill point rewards

### 4.6 Economy

#### FR-6.1: Shop System
- **Priority**: High
- **Description**: NPC vendor interactions
- **Acceptance Criteria**:
  - Buy items with currency
  - Sell items for currency
  - Price display
  - Stock limits (optional)
  - Class-specific shops

#### FR-6.2: Cash Shop
- **Priority**: Medium
- **Description**: Premium item purchases
- **Acceptance Criteria**:
  - Time-limited items (1/7/15/30 days)
  - Cosmetic items
  - Convenience items
  - Secure transaction logging

#### FR-6.3: Currency System
- **Priority**: Critical
- **Description**: Money handling
- **Acceptance Criteria**:
  - Coin pickup/drop
  - Secure storage
  - Transaction logging
  - Anti-duplication measures

### 4.7 Administration

#### FR-7.1: GM Commands
- **Priority**: Medium
- **Description**: Game master tools
- **Acceptance Criteria**:
  - Teleport to player/map
  - Spawn items
  - Kick/ban players
  - Broadcast messages
  - Monitor chat/logs

#### FR-7.2: Server Monitoring
- **Priority**: High
- **Description**: Track server health
- **Acceptance Criteria**:
  - Player count display
  - Performance metrics
  - Error logging
  - Crash recovery

---

## 5. Non-Functional Requirements

### 5.1 Performance

#### NFR-1.1: Concurrent Users
- **Requirement**: Support 100+ concurrent players (configurable)
- **Measurement**: Peak simultaneous connections
- **Target**: < 5% connection failures during peak

#### NFR-1.2: Response Time
- **Requirement**: Sub-second action response
- **Measurement**: Client action to server acknowledgment
- **Target**: < 200ms average latency

#### NFR-1.3: Server Uptime
- **Requirement**: 99% uptime target
- **Measurement**: Scheduled maintenance excluded
- **Target**: < 8.76 hours downtime/month

#### NFR-1.4: Database Performance
- **Requirement**: Efficient data operations
- **Measurement**: Query response times
- **Target**: < 50ms for common queries

### 5.2 Security

#### NFR-2.1: Authentication Security
- **Requirement**: Secure credential handling
- **Implementation**: Password hashing, salted storage
- **Compliance**: No plaintext passwords

#### NFR-2.2: Data Protection
- **Requirement**: Protect user data
- **Implementation**: Encrypted communications (SSL/TLS recommended)
- **Compliance**: GDPR/regional privacy laws

#### NFR-2.3: Cheat Prevention
- **Requirement**: Detect and prevent exploitation
- **Implementation**: Server-side validation, rate limiting
- **Monitoring**: Unusual pattern detection

#### NFR-2.4: DDoS Mitigation
- **Requirement**: Resist denial of service attacks
- **Implementation**: Rate limiting, firewall rules
- **Response**: Automatic blocking of suspicious IPs

### 5.3 Scalability

#### NFR-3.1: Horizontal Scaling
- **Requirement**: Support multiple game servers
- **Implementation**: Shared database, load balancing
- **Goal**: Add capacity without downtime

#### NFR-3.2: Database Scaling
- **Requirement**: Handle growing data volume
- **Implementation**: Indexing, partitioning strategies
- **Goal**: Maintain performance as data grows

### 5.4 Reliability

#### NFR-4.1: Data Persistence
- **Requirement**: No data loss on crash
- **Implementation**: Frequent auto-saves, transaction logging
- **Recovery**: Point-in-time restoration capability

#### NFR-4.2: Fault Tolerance
- **Requirement**: Graceful failure handling
- **Implementation**: Connection retry logic, state recovery
- **Goal**: Minimize player impact from failures

#### NFR-4.3: Backup Strategy
- **Requirement**: Regular data backups
- **Implementation**: Daily full backups, hourly incremental
- **Retention**: 30-day backup history minimum

### 5.5 Usability

#### NFR-5.1: Configuration Simplicity
- **Requirement**: Easy server setup
- **Implementation**: Well-documented config files
- **Goal**: Deploy in under 2 hours for experienced admins

#### NFR-5.2: Error Messaging
- **Requirement**: Clear error communication
- **Implementation**: User-friendly messages, detailed logs
- **Goal**: Reduce support tickets through clarity

---

## 6. Technical Specifications

### 6.1 Server Architecture

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   Auth Server   │────▶│    DB Server    │◀────│   Main Server   │
│   Port: 50001   │     │   Port: 40001   │     │   Port: 30001   │
│                 │     │                 │     │                 │
│ - Login         │     │ - Character     │     │ - Game Logic    │
│ - Session       │     │ - Items         │     │ - Combat        │
│ - Billing       │     │ - Quests        │     │ - World State   │
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

### 6.2 Technology Stack

#### Backend
- **Language**: C++ (inferred from executables)
- **Platform**: Windows Server 2000/2003+
- **Database**: Microsoft SQL Server
- **Connectivity**: ODBC drivers
- **Networking**: TCP/IP sockets

#### Data Storage
- **Character Data**: MSSQL database
- **Item Data**: MSSQL database
- **Configuration**: Text files (.txt)
- **Maps**: Binary files (.ksm)

### 6.3 Network Ports

| Service | Port | Protocol | Purpose |
|---------|------|----------|---------|
| Main Server | 30001 | TCP | Game client connections |
| DB Server | 40001 | TCP | Inter-server DB queries |
| Auth Server | 50001 | TCP | Authentication requests |
| Auth Command | 50002 | TCP | Admin commands |
| Billing | 55903 | TCP | Payment processing |

### 6.4 Database Schema (Inferred)

#### KAL_AUTH Database
- Users table (account credentials)
- Sessions table (active logins)
- Billing table (payment records)
- Ban list table

#### KAL_DB Database
- Characters table (player data)
- Items table (inventory/storage)
- Quests table (progress tracking)
- Friends table (social connections)
- Guilds table (if implemented)
- Mail table (in-game messaging)

### 6.5 File Structure

```
/workspace/
├── AuthSvrT.exe          # Authentication server
├── AuthSvrT.pdb          # Debug symbols
├── DBSvrT.exe            # Database server
├── DBSvrT.pdb            # Debug symbols
├── MainSvrT.exe          # Main game server
├── MainSvrT.pdb          # Debug symbols
├── dbghelp.dll           # Debug helper
├── MainConfig.txt        # Main server config
├── AuthConfig.txt        # Auth server config
├── DBConfig.txt          # DB server config
├── LangEN.txt            # Localization
├── Config/
│   ├── InitMap.txt       # Map definitions
│   ├── InitItem.txt      # Item stats
│   ├── InitSkill.txt     # Skill definitions
│   ├── InitNPC.txt       # NPC placements
│   ├── InitMonster.txt   # Monster stats
│   ├── GenMonster.txt    # Spawn points
│   ├── Quest.txt         # Quest data
│   ├── Prefix.txt        # Item prefixes
│   ├── Goods.txt         # Shop items
│   ├── ItemGroup.txt     # Loot tables
│   └── Etc.txt           # Teleport points
├── Map/                  # 148 .ksm map files
├── DB Files/             # Database backups
├── AuL/                  # Auth logs
├── DBL/                  # DB logs
└── MaL/                  # Main logs
```

---

## 7. User Stories

### 7.1 Character Progression

**US-1**: As a new player, I want to create a character so that I can start playing the game.
- Acceptance: Can select class, name character, enter tutorial zone

**US-2**: As a player, I want to gain experience from killing monsters so that I can level up.
- Acceptance: EXP bar fills, level increases, stats improve

**US-3**: As a player, I want to learn new skills when I level up so that I have more abilities.
- Acceptance: Skill points awarded, can allocate to unlocked skills

**US-4**: As a player, I want to equip better gear as I progress so that I can tackle harder content.
- Acceptance: Can wear higher-level items, stats update correctly

### 7.2 Social Interaction

**US-5**: As a player, I want to form a party with friends so that we can hunt together.
- Acceptance: Can invite, accept, share EXP and loot

**US-6**: As a player, I want to communicate with others so that I can coordinate and socialize.
- Acceptance: Multiple chat channels work, messages visible

**US-7**: As a player, I want to trade items with other players so that I can get what I need.
- Acceptance: Secure trade window, both parties confirm

### 7.3 Content Engagement

**US-8**: As a player, I want to complete quests so that I can earn rewards and story progression.
- Acceptance: Quest objectives track, rewards distribute on completion

**US-9**: As a player, I want to explore different maps so that I can discover new areas.
- Acceptance: Can travel between zones, minimap updates

**US-10**: As a player, I want to fight varied monsters so that combat stays interesting.
- Acceptance: Different monster types, behaviors, and loot

### 7.4 Competition

**US-11**: As a PvP-focused player, I want to fight players from enemy nations so that I can prove my skill.
- Acceptance: Can attack enemy nation players, rewards for kills

**US-12**: As a competitive player, I want to obtain rare items so that I can stand out.
- Acceptance: Low drop rate items exist, visible to others

---

## 8. Success Metrics

### 8.1 Key Performance Indicators (KPIs)

| Metric | Target | Measurement Method |
|--------|--------|-------------------|
| Daily Active Users (DAU) | 50+ (single server) | Login tracking |
| Average Session Length | 2+ hours | Session timestamps |
| Player Retention (Day 7) | 40%+ | Cohort analysis |
| Player Retention (Day 30) | 20%+ | Cohort analysis |
| Conversion Rate (to paid) | 5-10% | Cash shop purchases |
| Average Revenue Per User (ARPU) | $5-15/month | Revenue / DAU |
| Server Uptime | 99%+ | Monitoring tools |
| Support Tickets/Week | < 20 | Ticket system |

### 8.2 Quality Metrics

- **Crash Frequency**: < 1 crash per week per server
- **Bug Reports**: < 10 critical bugs open at any time
- **Response Time**: < 200ms average action latency
- **Data Loss Incidents**: 0 tolerated
- **Cheating Reports**: Investigated within 24 hours

---

## 9. Risks & Mitigations

### 9.1 Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Database corruption | Low | Critical | Regular backups, RAID storage |
| Server crashes | Medium | High | Auto-restart scripts, monitoring |
| DDoS attacks | Medium | High | Firewall, CDN, rate limiting |
| Exploits/cheats | High | High | Server validation, anti-cheat |
| Data breach | Low | Critical | Encryption, access controls |

### 9.2 Business Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Low player adoption | Medium | High | Marketing, community building |
| Legal issues (IP) | High | Critical | Consult legal, consider cease & desist risk |
| Competition | High | Medium | Unique features, community focus |
| Monetization failure | Medium | High | Balanced cash shop, listen to feedback |
| Server costs exceed revenue | Medium | High | Scale infrastructure with demand |

### 9.3 Operational Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Admin abuse | Medium | High | Audit logs, multiple admins |
| Burnout (volunteer staff) | Medium | Medium | Rotate duties, recruit actively |
| Knowledge loss | Medium | Medium | Documentation, cross-training |
| Update breaks compatibility | Low | High | Test server, rollback procedures |

---

## 10. Roadmap & Phases

### Phase 1: Foundation (Weeks 1-4)
- [ ] Set up database and ODBC connections
- [ ] Configure and test all three servers
- [ ] Verify basic login and character creation
- [ ] Test movement and basic combat
- [ ] Deploy to test environment

### Phase 2: Core Gameplay (Weeks 5-8)
- [ ] Implement full skill system
- [ ] Balance monster EXP and loot
- [ ] Test quest system end-to-end
- [ ] Verify shop and economy functions
- [ ] Closed alpha with small group

### Phase 3: Social Features (Weeks 9-12)
- [ ] Party system testing
- [ ] Chat system implementation
- [ ] Trading mechanics
- [ ] GM tools deployment
- [ ] Beta launch invitation-only

### Phase 4: Polish & Launch (Weeks 13-16)
- [ ] Bug fixing and balancing
- [ ] Security hardening
- [ ] Documentation completion
- [ ] Marketing preparation
- [ ] Public launch

### Phase 5: Post-Launch (Ongoing)
- [ ] Monitor server performance
- [ ] Community management
- [ ] Content updates (if developing)
- [ ] Event hosting
- [ ] Continuous improvement

---

## 11. Appendix

### 11.1 Glossary

| Term | Definition |
|------|------------|
| **MMORPG** | Massively Multiplayer Online Role-Playing Game |
| **NPC** | Non-Player Character |
| **EXP** | Experience Points |
| **HP/MP** | Health Points / Mana Points |
| **DPS** | Damage Per Second |
| **AoE** | Area of Effect |
| **Buff/Debuff** | Temporary positive/negative status effect |
| **Mob** | Mobile creature/monster |
| **Instance** | Private copy of a dungeon/area for a group |
| **Grinding** | Repetitive gameplay for EXP/items |

### 11.2 References

- Original KalOnline documentation (if available)
- Server configuration files analyzed
- Community knowledge from KalOnline private servers
- MMORPG design best practices

### 11.3 Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2024 | Analysis Team | Initial PRD based on file analysis |

---

**END OF PRODUCT REQUIREMENTS DOCUMENT**

*This PRD is based on analysis of the KalOnline 2003 server files. Some features may require additional development or verification.*
