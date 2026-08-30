-- KalOnline PostgreSQL Migration Script - Game Database (kal_db)
-- Converts MSSQL kal_db schema to PostgreSQL 16+
-- Source: Original kal_db.sql from 2003 server files

-- Enable necessary extensions
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Drop existing tables if they exist (for clean migration)
DROP TABLE IF EXISTS BuffRemain CASCADE;
DROP TABLE IF EXISTS Event CASCADE;
DROP TABLE IF EXISTS Friend CASCADE;
DROP TABLE IF EXISTS GuildAlliance CASCADE;
DROP TABLE IF EXISTS GuildCastle CASCADE;
DROP TABLE IF EXISTS GuildMember CASCADE;
DROP TABLE IF EXISTS GuildWar CASCADE;
DROP TABLE IF EXISTS Guild CASCADE;
DROP TABLE IF EXISTS ItemRestored CASCADE;
DROP TABLE IF EXISTS Item CASCADE;
DROP TABLE IF EXISTS Log CASCADE;
DROP TABLE IF EXISTS Mail CASCADE;
DROP TABLE IF EXISTS MLMMsg CASCADE;
DROP TABLE IF EXISTS MLM CASCADE;
DROP TABLE IF EXISTS NameChanged CASCADE;
DROP TABLE IF EXISTS PKBulletin CASCADE;
DROP TABLE IF EXISTS PlayerDeleted CASCADE;
DROP TABLE IF EXISTS Player CASCADE;
DROP TABLE IF EXISTS Quest CASCADE;
DROP TABLE IF EXISTS ReservedName CASCADE;
DROP TABLE IF EXISTS Shortcut CASCADE;
DROP TABLE IF EXISTS Skill CASCADE;
DROP TABLE IF EXISTS Statistics CASCADE;
DROP TABLE IF EXISTS Teleport CASCADE;

-- ============================================================================
-- PLAYER TABLE - Core character data
-- ============================================================================
CREATE TABLE Player (
    uid INTEGER NOT NULL,                          -- Account UID reference
    pid SERIAL PRIMARY KEY,                        -- Character unique ID
    admin SMALLINT NOT NULL DEFAULT 0,             -- Admin level (0: Normal, 1-5: GM levels)
    name VARCHAR(14) NOT NULL UNIQUE,              -- Character name (max 14 chars)
    class SMALLINT NOT NULL DEFAULT 0,             -- 0: Knight, 1: Mage, 2: Archer
    specialty SMALLINT NOT NULL DEFAULT 0,         -- Specialization within class
    level SMALLINT NOT NULL DEFAULT 1,             -- Character level (1-60)
    contribute SMALLINT NOT NULL DEFAULT 0,        -- Contribution points
    exp BIGINT NOT NULL DEFAULT 0,                 -- Current experience
    gid INTEGER NOT NULL DEFAULT 0,                -- Guild ID (0 = no guild)
    grole SMALLINT NOT NULL DEFAULT 0,             -- Guild role (0: None, 1: Leader, etc.)
    strength SMALLINT NOT NULL DEFAULT 0,          -- STR stat
    health SMALLINT NOT NULL DEFAULT 0,            -- HTH stat
    intelligence SMALLINT NOT NULL DEFAULT 0,      -- INT stat
    wisdom SMALLINT NOT NULL DEFAULT 0,            -- WIS stat
    dexterity SMALLINT NOT NULL DEFAULT 0,         -- DEX stat
    cur_hp SMALLINT NOT NULL DEFAULT 0,            -- Current HP
    cur_mp SMALLINT NOT NULL DEFAULT 0,            -- Current MP
    pu_point SMALLINT NOT NULL DEFAULT 0,          -- Physical Upgrade points
    su_point SMALLINT NOT NULL DEFAULT 0,          -- Skill Upgrade points
    killed SMALLINT NOT NULL DEFAULT 0,            -- PK kills
    map_id SMALLINT NOT NULL DEFAULT 0,            -- Current map ID
    x_coord INTEGER NOT NULL DEFAULT 0,            -- X position
    y_coord INTEGER NOT NULL DEFAULT 0,            -- Y position
    z_coord INTEGER NOT NULL DEFAULT 0,            -- Z position
    face SMALLINT NOT NULL DEFAULT 0,              -- Face style
    hair SMALLINT NOT NULL DEFAULT 0,              -- Hair style
    revival_id SMALLINT NOT NULL DEFAULT 0,        -- Resurrection point ID
    rage INTEGER NOT NULL DEFAULT 0,               -- Rage meter (warrior mechanic)
    honor_point INTEGER NOT NULL DEFAULT 0,        -- Honor points (PvP currency)
    honor_kill INTEGER NOT NULL DEFAULT 0,         -- Honor kills count
    honor_death INTEGER NOT NULL DEFAULT 0,        -- Honor deaths count
    dkp_total INTEGER NOT NULL DEFAULT 0,          -- DKP total (raid points)
    dkp_win INTEGER NOT NULL DEFAULT 0,            -- DKP wins
    pl_total INTEGER NOT NULL DEFAULT 0,           -- PL total
    pl_win INTEGER NOT NULL DEFAULT 0,             -- PL wins
    sv_total INTEGER NOT NULL DEFAULT 0,           -- SV total
    sv_win INTEGER NOT NULL DEFAULT 0,             -- SV wins
    reward_point INTEGER NOT NULL DEFAULT 0,       -- Reward points
    emok_day INTEGER NOT NULL DEFAULT 0,           -- Daily Emok counter
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP WITH TIME ZONE,
    is_deleted BOOLEAN DEFAULT FALSE,
    deleted_at TIMESTAMP WITH TIME ZONE
);

CREATE INDEX idx_player_uid ON Player(uid);
CREATE INDEX idx_player_name ON Player(name);
CREATE INDEX idx_player_level ON Player(level);
CREATE INDEX idx_player_guild ON Player(gid);
CREATE INDEX idx_player_map ON Player(map_id);

-- ============================================================================
-- ITEM TABLE - Character inventory and equipment
-- ============================================================================
CREATE TABLE Item (
    pid INTEGER NOT NULL REFERENCES Player(pid) ON DELETE CASCADE,
    iid INTEGER NOT NULL,                          -- Item instance ID
    item_index SMALLINT NOT NULL,                  -- Item type index (from InitItem.txt)
    prefix_type SMALLINT NOT NULL DEFAULT 0,       -- Prefix/enchantment type
    info_field INTEGER NOT NULL DEFAULT 0,         -- Additional item flags/info
    quantity INTEGER NOT NULL DEFAULT 1,           -- Stack size
    max_durability SMALLINT,                       -- Maximum durability
    cur_durability SMALLINT,                       -- Current durability
    socket_gem SMALLINT,                           -- Socketed gem type
    extra_attack SMALLINT,                         -- Bonus attack
    extra_magic SMALLINT,                          -- Bonus magic
    extra_defense SMALLINT,                        -- Bonus defense
    extra_hit SMALLINT,                            -- Bonus hit rate
    extra_dodge SMALLINT,                          -- Bonus dodge rate
    protect_level SMALLINT,                        -- Protection/upgrade level
    location SMALLINT NOT NULL DEFAULT 0,          -- Equipment slot: 0-19=Equip, 20+=Inventory
    is_equipped BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP WITH TIME ZONE,           -- For time-limited cash items
    PRIMARY KEY (pid, iid)
);

CREATE INDEX idx_item_pid ON Item(pid);
CREATE INDEX idx_item_location ON Item(location);
CREATE INDEX idx_item_equipped ON Item(is_equipped);

-- ============================================================================
-- SKILL TABLE - Learned skills and levels
-- ============================================================================
CREATE TABLE Skill (
    pid INTEGER NOT NULL REFERENCES Player(pid) ON DELETE CASCADE,
    skill_index SMALLINT NOT NULL,                 -- Skill ID from InitSkill.txt
    skill_level SMALLINT NOT NULL DEFAULT 1,       -- Skill level (1-max)
    cooldown_until TIMESTAMP WITH TIME ZONE,       -- Cooldown expiration
    PRIMARY KEY (pid, skill_index)
);

CREATE INDEX idx_skill_pid ON Skill(pid);

-- ============================================================================
-- QUEST TABLE - Quest progress tracking
-- ============================================================================
CREATE TABLE Quest (
    pid INTEGER NOT NULL REFERENCES Player(pid) ON DELETE CASCADE,
    quest_id SMALLINT NOT NULL,                    -- Quest ID from Quest.txt
    quest_flag SMALLINT NOT NULL DEFAULT 0,        -- Quest state flags
    is_cleared SMALLINT NOT NULL DEFAULT 0,        -- Completion status (0/1)
    quest_time INTEGER NOT NULL DEFAULT 0,         -- Timer for timed quests
    quest_repeat INTEGER NOT NULL DEFAULT 0,       -- Repeat count
    monster_count INTEGER NOT NULL DEFAULT 0,      -- Kill progress
    completed_at TIMESTAMP WITH TIME ZONE,
    PRIMARY KEY (pid, quest_id)
);

CREATE INDEX idx_quest_pid ON Quest(pid);
CREATE INDEX idx_quest_cleared ON Quest(is_cleared);

-- ============================================================================
-- FRIEND TABLE - Friends list
-- ============================================================================
CREATE TABLE Friend (
    pid INTEGER NOT NULL REFERENCES Player(pid) ON DELETE CASCADE,
    friend_pid INTEGER NOT NULL,                   -- Friend's character PID
    friend_name VARCHAR(14) NOT NULL,              -- Friend's name (cached)
    is_blocked BOOLEAN DEFAULT FALSE,              -- If true, this is a block list entry
    added_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (pid, friend_pid)
);

CREATE INDEX idx_friend_pid ON Friend(pid);

-- ============================================================================
-- GUILD TABLE - Guild information
-- ============================================================================
CREATE TABLE Guild (
    gid INTEGER PRIMARY KEY,
    name VARCHAR(14) NOT NULL UNIQUE,              -- Guild name
    standard INTEGER NOT NULL DEFAULT 0,           -- Guild standard/ranking
    exp TEXT NOT NULL DEFAULT '0',                 -- Guild experience (stored as text for large values)
    today_message VARCHAR(180),                    -- Daily guild message
    leader_name VARCHAR(14),                       -- Guild leader name
    subleader_name VARCHAR(14),                    -- Sub-leader name
    centurion_name VARCHAR(14),                    -- Centurion name
    ten_name VARCHAR(14),                          -- Ten name
    regular_name VARCHAR(14),                      -- Regular member representative
    temp_name VARCHAR(14),                         -- Temporary member representative
    subleader_able INTEGER NOT NULL DEFAULT 0,     -- Sub-leader capacity
    centurion_able INTEGER NOT NULL DEFAULT 0,     -- Centurion capacity
    ten_able INTEGER NOT NULL DEFAULT 0,           -- Ten capacity
    regular_able INTEGER NOT NULL DEFAULT 0,       -- Regular member capacity
    temp_able INTEGER NOT NULL DEFAULT 0,          -- Temporary member capacity
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    last_updated TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_guild_name ON Guild(name);

-- ============================================================================
-- GUILD MEMBER TABLE - Guild membership
-- ============================================================================
CREATE TABLE GuildMember (
    gid INTEGER NOT NULL REFERENCES Guild(gid) ON DELETE CASCADE,
    pid INTEGER NOT NULL REFERENCES Player(pid) ON DELETE CASCADE,
    role SMALLINT NOT NULL DEFAULT 0,              -- 0: Temp, 1: Regular, 2: Ten, 3: Centurion, 4: SubLeader, 5: Leader
    joined_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    contribution INTEGER NOT NULL DEFAULT 0,       -- Guild contribution points
    note TEXT,                                     -- Officer notes
    PRIMARY KEY (gid, pid)
);

CREATE INDEX idx_guildmember_gid ON GuildMember(gid);
CREATE INDEX idx_guildmember_pid ON GuildMember(pid);

-- ============================================================================
-- GUILD ALLIANCE TABLE - Guild alliances
-- ============================================================================
CREATE TABLE GuildAlliance (
    gid1 INTEGER NOT NULL REFERENCES Guild(gid) ON DELETE CASCADE,
    gid2 INTEGER NOT NULL REFERENCES Guild(gid) ON DELETE CASCADE,
    allied_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (gid1, gid2)
);

-- ============================================================================
-- GUILD CASTLE TABLE - Castle ownership (siege system)
-- ============================================================================
CREATE TABLE GuildCastle (
    castle_id INTEGER PRIMARY KEY,
    owner_gid INTEGER REFERENCES Guild(gid),
    tax_rate SMALLINT DEFAULT 0,                   -- Tax percentage
    last_siege_date TIMESTAMP WITH TIME ZONE,
    next_siege_date TIMESTAMP WITH TIME ZONE
);

-- ============================================================================
-- GUILD WAR TABLE - Guild war declarations
-- ============================================================================
CREATE TABLE GuildWar (
    war_id SERIAL PRIMARY KEY,
    challenger_gid INTEGER NOT NULL REFERENCES Guild(gid) ON DELETE CASCADE,
    target_gid INTEGER NOT NULL REFERENCES Guild(gid) ON DELETE CASCADE,
    declared_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP WITH TIME ZONE,
    is_active BOOLEAN DEFAULT TRUE,
    winner_gid INTEGER
);

CREATE INDEX idx_guildwar_challenger ON GuildWar(challenger_gid);
CREATE INDEX idx_guildwar_target ON GuildWar(target_gid);
CREATE INDEX idx_guildwar_active ON GuildWar(is_active);

-- ============================================================================
-- MAIL TABLE - In-game mail system
-- ============================================================================
CREATE TABLE Mail (
    mid SERIAL PRIMARY KEY,
    priority SMALLINT NOT NULL DEFAULT 0,          -- 0: Normal, 1: Important, 2: Urgent
    mail_type SMALLINT NOT NULL DEFAULT 0,         -- 0: Player, 1: System, 2: GM
    status SMALLINT NOT NULL DEFAULT 0,            -- 0: Unread, 1: Read, 2: Replied
    sender_pid INTEGER NOT NULL,
    sender_name VARCHAR(14) NOT NULL,
    recipient_pid INTEGER NOT NULL,
    recipient_name VARCHAR(14) NOT NULL,
    timestamp_start INTEGER NOT NULL DEFAULT 0,    -- Send timestamp
    timestamp_end INTEGER NOT NULL DEFAULT 0,      -- Expiration timestamp
    subject VARCHAR(100),
    body TEXT,
    item_iid INTEGER NOT NULL DEFAULT 0,           -- Attached item instance ID
    item_ver SMALLINT NOT NULL DEFAULT 0,          -- Item version
    item_index SMALLINT NOT NULL DEFAULT 0,        -- Item type index
    item_prefix SMALLINT NOT NULL DEFAULT 0,       -- Item prefix
    item_quantity INTEGER NOT NULL DEFAULT 0,      -- Item quantity
    coin_amount INTEGER NOT NULL DEFAULT 0,        -- Attached coins
    read_at TIMESTAMP WITH TIME ZONE,
    claimed_at TIMESTAMP WITH TIME ZONE
);

CREATE INDEX idx_mail_recipient ON Mail(recipient_pid);
CREATE INDEX idx_mail_sender ON Mail(sender_pid);
CREATE INDEX idx_mail_status ON Mail(status);
CREATE INDEX idx_mail_expiry ON Mail(timestamp_end);

-- ============================================================================
-- BUFF REMAIN TABLE - Active buffs/debuffs
-- ============================================================================
CREATE TABLE BuffRemain (
    pid INTEGER NOT NULL REFERENCES Player(pid) ON DELETE CASCADE,
    buff_index SMALLINT NOT NULL,                  -- Buff/skill ID
    remaining_time INTEGER NOT NULL DEFAULT 0,     -- Remaining duration in seconds
    applied_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (pid, buff_index)
);

CREATE INDEX idx_buff_pid ON BuffRemain(pid);

-- ============================================================================
-- SHORTCUT TABLE - UI shortcut bar configuration
-- ============================================================================
CREATE TABLE Shortcut (
    pid INTEGER NOT NULL REFERENCES Player(pid) ON DELETE CASCADE,
    slot_index SMALLINT NOT NULL,                  -- 0-7 or 0-11 depending on UI
    shortcut_type SMALLINT NOT NULL DEFAULT 0,     -- 0: Skill, 1: Item, 2: Emote, etc.
    shortcut_index INTEGER NOT NULL DEFAULT 0,     -- Reference ID (skill_id, item_id, etc.)
    PRIMARY KEY (pid, slot_index)
);

CREATE INDEX idx_shortcut_pid ON Shortcut(pid);

-- ============================================================================
-- EVENT TABLE - Scheduled game events
-- ============================================================================
CREATE TABLE Event (
    event_id SERIAL PRIMARY KEY,
    event_name VARCHAR(64) NOT NULL,
    event_type SMALLINT NOT NULL,                  -- 1: Boss spawn, 2: XP boost, 3: Drop rate, etc.
    start_time TIMESTAMP WITH TIME ZONE NOT NULL,
    end_time TIMESTAMP WITH TIME ZONE NOT NULL,
    is_recurring BOOLEAN DEFAULT FALSE,
    recurrence_pattern VARCHAR(32),                -- Cron-like pattern
    is_active BOOLEAN DEFAULT TRUE,
    description TEXT,
    parameters JSONB                               -- Event-specific parameters
);

CREATE INDEX idx_event_active ON Event(is_active);
CREATE INDEX idx_event_time ON Event(start_time, end_time);

-- ============================================================================
-- LOG TABLE - Game action logging
-- ============================================================================
CREATE TABLE Log (
    log_id BIGSERIAL PRIMARY KEY,
    log_type SMALLINT NOT NULL,                    -- 1: Login, 2: Logout, 3: Trade, 4: Item, 5: Chat, etc.
    pid INTEGER,
    character_name VARCHAR(14),
    ip_address VARCHAR(45),
    action_description TEXT,
    details JSONB,
    logged_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_log_type ON Log(log_type);
CREATE INDEX idx_log_pid ON Log(pid);
CREATE INDEX idx_log_date ON Log(logged_at);

-- ============================================================================
-- PLAYER DELETED TABLE - Soft delete tracking
-- ============================================================================
CREATE TABLE PlayerDeleted (
    pid INTEGER PRIMARY KEY,
    uid INTEGER NOT NULL,
    name VARCHAR(14) NOT NULL,
    class SMALLINT NOT NULL,
    level SMALLINT NOT NULL,
    deleted_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    deletion_reason TEXT,
    can_restore BOOLEAN DEFAULT TRUE,
    restore_deadline TIMESTAMP WITH TIME ZONE
);

-- ============================================================================
-- NAME CHANGED TABLE - Track name changes
-- ============================================================================
CREATE TABLE NameChanged (
    pid INTEGER NOT NULL,
    old_name VARCHAR(14) NOT NULL,
    new_name VARCHAR(14) NOT NULL,
    changed_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    reason SMALLINT DEFAULT 0                      -- 0: Player request, 1: GM forced, 2: Violation
);

CREATE INDEX idx_namechanged_pid ON NameChanged(pid);

-- ============================================================================
-- RESERVED NAME TABLE - Blocked/reserved character names
-- ============================================================================
CREATE TABLE ReservedName (
    name VARCHAR(14) PRIMARY KEY,
    reserved_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    reason TEXT,
    expires_at TIMESTAMP WITH TIME ZONE
);

-- ============================================================================
-- PK BULLETIN TABLE - Player killer wanted list
-- ============================================================================
CREATE TABLE PKBulletin (
    bulletin_id SERIAL PRIMARY KEY,
    pid INTEGER NOT NULL,
    character_name VARCHAR(14) NOT NULL,
    pk_count SMALLINT NOT NULL,
    bounty INTEGER NOT NULL DEFAULT 0,             -- Bounty amount
    posted_by INTEGER NOT NULL,                    -- GM who posted
    posted_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP WITH TIME ZONE,
    is_active BOOLEAN DEFAULT TRUE
);

CREATE INDEX idx_pkbulletin_active ON PKBulletin(is_active);

-- ============================================================================
-- MLM TABLE - Multi-level marketing/referral tracking
-- ============================================================================
CREATE TABLE MLM (
    pid INTEGER NOT NULL REFERENCES Player(pid) ON DELETE CASCADE,
    referrer_pid INTEGER,                          -- Who referred this player
    referee_pid INTEGER,                           -- Who this player referred
    referral_count INTEGER NOT NULL DEFAULT 0,
    total_bonus INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (pid)
);

-- ============================================================================
-- MLM MSG TABLE - MLM notification messages
-- ============================================================================
CREATE TABLE MLMMsg (
    msg_id SERIAL PRIMARY KEY,
    pid INTEGER NOT NULL,
    message TEXT NOT NULL,
    is_read BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_mlmsg_pid ON MLMMsg(pid);

-- ============================================================================
-- STATISTICS TABLE - Server-wide statistics
-- ============================================================================
CREATE TABLE Statistics (
    stat_date DATE PRIMARY KEY,
    total_players INTEGER NOT NULL DEFAULT 0,
    max_concurrent INTEGER NOT NULL DEFAULT 0,
    total_playtime_hours BIGINT NOT NULL DEFAULT 0,
    mobs_killed BIGINT NOT NULL DEFAULT 0,
    items_created BIGINT NOT NULL DEFAULT 0,
    trades_completed BIGINT NOT NULL DEFAULT 0,
    guilds_created SMALLINT NOT NULL DEFAULT 0,
    pvp_kills BIGINT NOT NULL DEFAULT 0,
    bosses_killed INTEGER NOT NULL DEFAULT 0,
    recorded_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- TELEPORT TABLE - Teleporter locations and costs
-- ============================================================================
CREATE TABLE Teleport (
    teleport_id SERIAL PRIMARY KEY,
    npc_pid INTEGER,                               -- NPC that offers teleport
    source_map SMALLINT NOT NULL,
    source_x INTEGER NOT NULL,
    source_y INTEGER NOT NULL,
    dest_map SMALLINT NOT NULL,
    dest_x INTEGER NOT NULL,
    dest_y INTEGER NOT NULL,
    cost INTEGER NOT NULL DEFAULT 0,               -- Teleport fee
    required_level SMALLINT DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    description VARCHAR(100)
);

CREATE INDEX idx_teleport_source ON Teleport(source_map);
CREATE INDEX idx_teleport_dest ON Teleport(dest_map);

-- ============================================================================
-- VIEWS FOR COMMON QUERIES
-- ============================================================================

-- Active players view
CREATE VIEW v_active_players AS
SELECT 
    p.pid,
    p.name,
    p.class,
    p.level,
    p.map_id,
    p.x_coord,
    p.y_coord,
    p.gid,
    g.name as guild_name,
    p.last_login
FROM Player p
LEFT JOIN Guild g ON p.gid = g.gid
WHERE p.is_deleted = FALSE;

-- Top players by level
CREATE VIEW v_top_players_level AS
SELECT 
    pid,
    name,
    class,
    level,
    exp,
    ROW_NUMBER() OVER (ORDER BY level DESC, exp DESC) as rank
FROM Player
WHERE is_deleted = FALSE
LIMIT 100;

-- Guild rankings view
CREATE VIEW v_guild_rankings AS
SELECT 
    g.gid,
    g.name,
    COUNT(gm.pid) as member_count,
    g.exp as guild_exp,
    g.today_message,
    g.leader_name
FROM Guild g
LEFT JOIN GuildMember gm ON g.gid = gm.gid
GROUP BY g.gid, g.name, g.exp, g.today_message, g.leader_name
ORDER BY guild_exp DESC;

-- Player inventory summary
CREATE VIEW v_player_inventory AS
SELECT 
    p.pid,
    p.name,
    COUNT(i.iid) as total_items,
    SUM(i.quantity) FILTER (WHERE i.location >= 20) as inventory_count,
    COUNT(i.iid) FILTER (WHERE i.is_equipped = TRUE) as equipped_count
FROM Player p
LEFT JOIN Item i ON p.pid = i.pid
WHERE p.is_deleted = FALSE
GROUP BY p.pid, p.name;

-- ============================================================================
-- TRIGGERS
-- ============================================================================

-- Update guild last_updated timestamp
CREATE TRIGGER update_guild_updated_at
    BEFORE UPDATE ON Guild
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- Clean up expired mail trigger function
CREATE OR REPLACE FUNCTION cleanup_expired_mail()
RETURNS TRIGGER AS $$
BEGIN
    DELETE FROM Mail WHERE timestamp_end > 0 AND timestamp_end < EXTRACT(EPOCH FROM NOW());
    RETURN NULL;
END;
$$ language 'plpgsql';

-- ============================================================================
-- COMMENTS FOR DOCUMENTATION
-- ============================================================================

COMMENT ON TABLE Player IS 'Core character data including stats, position, and progression';
COMMENT ON COLUMN Player.class IS '0: Knight, 1: Mage, 2: Archer';
COMMENT ON COLUMN Player.strength IS 'STR - Increases physical attack and HP';
COMMENT ON COLUMN Player.health IS 'HTH - Increases HP and defense';
COMMENT ON COLUMN Player.intelligence IS 'INT - Increases magic attack and MP';
COMMENT ON COLUMN Player.wisdom IS 'WIS - Increases MP and magic defense';
COMMENT ON COLUMN Player.dexterity IS 'DEX - Increases hit rate, dodge, and crit';
COMMENT ON TABLE Item IS 'Character inventory and equipment with location field (0-19=equip, 20+=inventory)';
COMMENT ON TABLE Skill IS 'Learned skills with levels per character';
COMMENT ON TABLE Quest IS 'Quest progress tracking including kill counts and timers';
COMMENT ON TABLE Guild IS 'Guild information with hierarchy and capacity limits';
COMMENT ON TABLE Mail IS 'In-game mail system with item and coin attachments';
COMMENT ON TABLE BuffRemain IS 'Active buffs/debuffs with remaining duration';

-- ============================================================================
-- INITIAL DATA
-- ============================================================================

-- Insert default teleport locations (example - should be populated from config)
INSERT INTO Teleport (source_map, source_x, source_y, dest_map, dest_x, dest_y, cost, required_level, description) VALUES
(0, 100, 100, 1, 200, 200, 100, 1, 'Starting Village to Town'),
(1, 200, 200, 0, 100, 100, 100, 1, 'Town to Starting Village');
