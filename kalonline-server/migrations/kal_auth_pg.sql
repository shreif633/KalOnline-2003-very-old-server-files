-- KalOnline PostgreSQL Migration Script
-- Converts MSSQL kal_auth schema to PostgreSQL 16+
-- Source: Original kal_auth.sql from 2003 server files

-- Enable necessary extensions
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Drop existing tables if they exist (for clean migration)
DROP TABLE IF EXISTS AuthStatistics CASCADE;
DROP TABLE IF EXISTS CLogin CASCADE;
DROP TABLE IF EXISTS CNum CASCADE;
DROP TABLE IF EXISTS ExpTable CASCADE;
DROP TABLE IF EXISTS GuildRank CASCADE;
DROP TABLE IF EXISTS IP CASCADE;
DROP TABLE IF EXISTS ItemBuy CASCADE;
DROP TABLE IF EXISTS ItemDetail CASCADE;
DROP TABLE IF EXISTS ItemInfo CASCADE;
DROP TABLE IF EXISTS Log CASCADE;
DROP TABLE IF EXISTS Login CASCADE;
DROP TABLE IF EXISTS LoginDeleted CASCADE;
DROP TABLE IF EXISTS PrizeWinner CASCADE;
DROP TABLE IF EXISTS Rank CASCADE;
DROP TABLE IF EXISTS TradeLog CASCADE;
DROP TABLE IF EXISTS Users CASCADE;

-- ============================================================================
-- LOGIN TABLE - Core authentication table
-- ============================================================================
CREATE TABLE Login (
    uid INTEGER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    id VARCHAR(16) NOT NULL UNIQUE,
    pwd BYTEA NOT NULL,                    -- Changed from varbinary(16) to BYTEA
    birth TIMESTAMP WITHOUT TIME ZONE,
    type SMALLINT DEFAULT 0,
    exp_time INTEGER DEFAULT 0,
    info INTEGER DEFAULT 0,
    pwd2nd INTEGER DEFAULT 0,
    email VARCHAR(50),
    first_name VARCHAR(50),
    second_name VARCHAR(50),
    sn VARCHAR(25),
    ip_address VARCHAR(30),
    create_date TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP WITH TIME ZONE,
    is_banned BOOLEAN DEFAULT FALSE,
    ban_reason TEXT,
    ban_expire TIMESTAMP WITH TIME ZONE
);

CREATE INDEX idx_login_id ON Login(id);
CREATE INDEX idx_login_email ON Login(email);
CREATE INDEX idx_login_banned ON Login(is_banned);

-- ============================================================================
-- USERS TABLE - Extended user information
-- ============================================================================
CREATE TABLE Users (
    uid INTEGER PRIMARY KEY REFERENCES Login(uid) ON DELETE CASCADE,
    password_hash VARCHAR(255) NOT NULL,   -- BCrypt hash for modern security
    salt VARCHAR(64),
    account_type SMALLINT DEFAULT 0,       -- 0: Normal, 1: Premium, 2: GM
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    last_ip VARCHAR(45),                   -- IPv6 compatible
    login_count INTEGER DEFAULT 0,
    failed_login_attempts INTEGER DEFAULT 0,
    locked_until TIMESTAMP WITH TIME ZONE
);

CREATE INDEX idx_users_account_type ON Users(account_type);

-- ============================================================================
-- CLOGIN TABLE - Cash/Time-based login tracking
-- ============================================================================
CREATE TABLE CLogin (
    cid INTEGER PRIMARY KEY,
    type SMALLINT NOT NULL DEFAULT 0,
    exp_time INTEGER NOT NULL DEFAULT 0
);

-- ============================================================================
-- CNUM TABLE - Character number mapping
-- ============================================================================
CREATE TABLE CNum (
    uid INTEGER NOT NULL,
    num INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (uid)
);

-- ============================================================================
-- EXP TABLE - Experience requirements per level
-- ============================================================================
CREATE TABLE ExpTable (
    level SMALLINT PRIMARY KEY,
    exp_required BIGINT NOT NULL
);

-- Insert base experience table (levels 1-60 based on quest references)
INSERT INTO ExpTable (level, exp_required) VALUES
(1, 0),
(2, 100),
(3, 300),
(4, 700),
(5, 1400),
(6, 2500),
(7, 4000),
(8, 6000),
(9, 8500),
(10, 12000),
(11, 16000),
(12, 21000),
(13, 27000),
(14, 34000),
(15, 42000),
(16, 51000),
(17, 61000),
(18, 72000),
(19, 84000),
(20, 97000),
(21, 111000),
(22, 126000),
(23, 142000),
(24, 159000),
(25, 177000),
(26, 196000),
(27, 216000),
(28, 237000),
(29, 259000),
(30, 282000),
(31, 306000),
(32, 331000),
(33, 357000),
(34, 384000),
(35, 412000),
(36, 441000),
(37, 471000),
(38, 502000),
(39, 534000),
(40, 567000),
(41, 601000),
(42, 636000),
(43, 672000),
(44, 709000),
(45, 747000),
(46, 786000),
(47, 826000),
(48, 867000),
(49, 909000),
(50, 952000),
(51, 1050000),
(52, 1155000),
(53, 1270000),
(54, 1397000),
(55, 1536000),
(56, 1689000),
(57, 1857000),
(58, 2042000),
(59, 2246000),
(60, 2470000);

-- ============================================================================
-- GUILD RANK TABLE - Guild ranking system
-- ============================================================================
CREATE TABLE GuildRank (
    guild_id INTEGER PRIMARY KEY,
    rank_position SMALLINT NOT NULL,
    points INTEGER DEFAULT 0,
    last_updated TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- IP BLOCKING TABLE - Security and access control
-- ============================================================================
CREATE TABLE IP (
    id SERIAL PRIMARY KEY,
    ip_address VARCHAR(45) NOT NULL,
    reason TEXT,
    banned_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP WITH TIME ZONE,
    is_active BOOLEAN DEFAULT TRUE
);

CREATE INDEX idx_ip_address ON IP(ip_address);
CREATE INDEX idx_ip_active ON IP(is_active);

-- ============================================================================
-- ITEM BUY LOG - Transaction history
-- ============================================================================
CREATE TABLE ItemBuy (
    transaction_id SERIAL PRIMARY KEY,
    uid INTEGER NOT NULL,
    item_index INTEGER NOT NULL,
    item_count INTEGER NOT NULL,
    price INTEGER NOT NULL,
    purchased_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    shop_type SMALLINT DEFAULT 0
);

CREATE INDEX idx_itembuy_uid ON ItemBuy(uid);
CREATE INDEX idx_itembuy_date ON ItemBuy(purchased_at);

-- ============================================================================
-- ITEM DETAIL TABLE - Detailed item information
-- ============================================================================
CREATE TABLE ItemDetail (
    item_id SERIAL PRIMARY KEY,
    owner_uid INTEGER,
    item_index SMALLINT NOT NULL,
    prefix_type SMALLINT DEFAULT 0,
    info_field INTEGER DEFAULT 0,
    quantity INTEGER DEFAULT 1,
    max_durability SMALLINT,
    cur_durability SMALLINT,
    socket_gem SMALLINT,
    extra_attack SMALLINT,
    extra_magic SMALLINT,
    extra_defense SMALLINT,
    extra_hit SMALLINT,
    extra_dodge SMALLINT,
    protect_level SMALLINT,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP WITH TIME ZONE  -- For cash shop items
);

CREATE INDEX idx_itemdetail_owner ON ItemDetail(owner_uid);

-- ============================================================================
-- ITEM INFO TABLE - Item metadata cache
-- ============================================================================
CREATE TABLE ItemInfo (
    item_index SMALLINT PRIMARY KEY,
    item_name VARCHAR(64) NOT NULL,
    item_type SMALLINT NOT NULL,
    required_level SMALLINT DEFAULT 0,
    required_class SMALLINT,  -- 0: Any, 1: Knight, 2: Mage, 3: Archer
    required_str SMALLINT DEFAULT 0,
    required_hth SMALLINT DEFAULT 0,
    required_int SMALLINT DEFAULT 0,
    required_wis SMALLINT DEFAULT 0,
    required_dex SMALLINT DEFAULT 0,
    attack_power SMALLINT DEFAULT 0,
    magic_power SMALLINT DEFAULT 0,
    defense SMALLINT DEFAULT 0,
    hit_rate SMALLINT DEFAULT 0,
    dodge_rate SMALLINT DEFAULT 0,
    absorb SMALLINT DEFAULT 0,
    durability SMALLINT DEFAULT 0,
    price INTEGER DEFAULT 0,
    cash_price INTEGER DEFAULT 0,
    stack_size SMALLINT DEFAULT 1,
    item_group SMALLINT,
    description TEXT
);

-- ============================================================================
-- LOG TABLE - Authentication and system logs
-- ============================================================================
CREATE TABLE Log (
    log_id BIGSERIAL PRIMARY KEY,
    log_type SMALLINT NOT NULL,  -- 1: Login, 2: Logout, 3: Error, 4: Security
    uid INTEGER,
    id VARCHAR(16),
    ip_address VARCHAR(45),
    message TEXT,
    logged_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_log_type ON Log(log_type);
CREATE INDEX idx_log_uid ON Log(uid);
CREATE INDEX idx_log_date ON Log(logged_at);

-- ============================================================================
-- LOGIN DELETED TABLE - Soft delete tracking
-- ============================================================================
CREATE TABLE LoginDeleted (
    uid INTEGER PRIMARY KEY,
    id VARCHAR(16) NOT NULL,
    deleted_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    deletion_reason TEXT,
    can_restore BOOLEAN DEFAULT TRUE,
    restore_deadline TIMESTAMP WITH TIME ZONE
);

-- ============================================================================
-- PRIZE WINNER TABLE - Event/prize tracking
-- ============================================================================
CREATE TABLE PrizeWinner (
    winner_id SERIAL PRIMARY KEY,
    uid INTEGER NOT NULL,
    prize_type SMALLINT NOT NULL,
    prize_description TEXT,
    awarded_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    claimed BOOLEAN DEFAULT FALSE,
    claimed_at TIMESTAMP WITH TIME ZONE
);

-- ============================================================================
-- RANK TABLE - Player rankings
-- ============================================================================
CREATE TABLE Rank (
    rank_id SERIAL PRIMARY KEY,
    character_uid INTEGER NOT NULL,
    rank_type SMALLINT NOT NULL,  -- 1: Level, 2: Contribution, 3: PK, etc.
    rank_value BIGINT NOT NULL,
    rank_position INTEGER,
    last_updated TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_rank_type ON Rank(rank_type);
CREATE INDEX idx_rank_value ON Rank(rank_value DESC);

-- ============================================================================
-- TRADE LOG TABLE - Transaction auditing
-- ============================================================================
CREATE TABLE TradeLog (
    trade_id BIGSERIAL PRIMARY KEY,
    trader_uid INTEGER NOT NULL,
    target_uid INTEGER NOT NULL,
    trade_type SMALLINT NOT NULL,  -- 1: Player trade, 2: Shop buy, 3: Shop sell
    item_index INTEGER,
    item_count INTEGER,
    amount INTEGER,
    traded_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_tradelog_trader ON TradeLog(trader_uid);
CREATE INDEX idx_tradelog_target ON TradeLog(target_uid);
CREATE INDEX idx_tradelog_date ON TradeLog(traded_at);

-- ============================================================================
-- AUTH STATISTICS TABLE - Server statistics snapshot
-- ============================================================================
CREATE TABLE AuthStatistics (
    reg_date TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    server_id SMALLINT,
    knight_count SMALLINT DEFAULT 0,
    mage_count SMALLINT DEFAULT 0,
    archer_count SMALLINT DEFAULT 0,
    total_users SMALLINT DEFAULT 0,
    fish_trap_count SMALLINT DEFAULT 0,
    private_shop_count SMALLINT DEFAULT 0,
    currency_total INTEGER DEFAULT 0
);

CREATE INDEX idx_authstats_date ON AuthStatistics(reg_date);

-- ============================================================================
-- SESSIONS TABLE - Active session management
-- ============================================================================
CREATE TABLE Sessions (
    session_id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    uid INTEGER NOT NULL REFERENCES Login(uid) ON DELETE CASCADE,
    token_hash VARCHAR(255) NOT NULL,
    ip_address VARCHAR(45) NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP WITH TIME ZONE NOT NULL,
    last_activity TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    is_valid BOOLEAN DEFAULT TRUE
);

CREATE INDEX idx_sessions_uid ON Sessions(uid);
CREATE INDEX idx_sessions_token ON Sessions(token_hash);
CREATE INDEX idx_sessions_expiry ON Sessions(expires_at);

-- ============================================================================
-- VIEWS FOR COMMON QUERIES
-- ============================================================================

-- Active users view
CREATE VIEW v_active_users AS
SELECT 
    l.uid,
    l.id as username,
    l.email,
    u.account_type,
    l.last_login,
    s.session_id,
    s.ip_address as current_ip
FROM Login l
JOIN Users u ON l.uid = u.uid
LEFT JOIN Sessions s ON l.uid = s.uid AND s.is_valid = TRUE AND s.expires_at > NOW()
WHERE l.is_banned = FALSE;

-- Login statistics view
CREATE VIEW v_login_stats AS
SELECT 
    DATE_TRUNC('day', logged_at) as stat_date,
    COUNT(*) FILTER (WHERE log_type = 1) as successful_logins,
    COUNT(*) FILTER (WHERE log_type = 3) as errors,
    COUNT(*) FILTER (WHERE log_type = 4) as security_events,
    COUNT(DISTINCT uid) as unique_users
FROM Log
GROUP BY DATE_TRUNC('day', logged_at)
ORDER BY stat_date DESC;

-- ============================================================================
-- TRIGGERS
-- ============================================================================

-- Update timestamp trigger function
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Apply to Users table
CREATE TRIGGER update_users_updated_at
    BEFORE UPDATE ON Users
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- Clean expired sessions trigger (runs on session check)
CREATE OR REPLACE FUNCTION cleanup_expired_sessions()
RETURNS TRIGGER AS $$
BEGIN
    DELETE FROM Sessions WHERE expires_at < NOW();
    RETURN NULL;
END;
$$ language 'plpgsql';

-- ============================================================================
-- INITIAL DATA
-- ============================================================================

-- Insert default admin account (password: 'admin' - should be changed!)
-- Note: In production, use proper BCrypt hashing
INSERT INTO Login (id, pwd, email, first_name, second_name, ip_address, create_date)
VALUES ('admin', E'\\x00000000000000000000000000000000', 'admin@kalonline.com', 'System', 'Administrator', '127.0.0.1', NOW());

-- Get the UID of the admin account we just created
DO $$
DECLARE
    admin_uid INTEGER;
BEGIN
    SELECT uid INTO admin_uid FROM Login WHERE id = 'admin';
    
    INSERT INTO Users (uid, password_hash, account_type, created_at, updated_at)
    VALUES (admin_uid, '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyYIxF.0WXa.', 2, NOW(), NOW());
END $$;

-- ============================================================================
-- COMMENTS FOR DOCUMENTATION
-- ============================================================================

COMMENT ON TABLE Login IS 'Core authentication table storing user credentials and account status';
COMMENT ON COLUMN Login.pwd IS 'Legacy password storage (varbinary). New accounts use Users.password_hash with BCrypt';
COMMENT ON COLUMN Login.type IS 'Account type flags from original system';
COMMENT ON COLUMN Login.info IS 'Additional account info flags';
COMMENT ON TABLE Users IS 'Extended user authentication with modern BCrypt password hashing';
COMMENT ON TABLE Sessions IS 'Active user sessions with expiration tracking';
COMMENT ON TABLE IP IS 'IP blocking and access control list';
COMMENT ON TABLE ExpTable IS 'Experience points required for each character level (1-60)';
