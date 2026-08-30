# 🚀 KalOnline Server Launch & Test Plan

## Phase 1: Database Setup (PostgreSQL)

### 1.1 Install PostgreSQL (if not installed)
```bash
sudo apt-get update
sudo apt-get install -y postgresql postgresql-contrib
```

### 1.2 Create Database & User
```bash
sudo -u postgres psql << EOF
CREATE DATABASE kal_online;
CREATE USER kal_user WITH PASSWORD 'kal_password';
GRANT ALL PRIVILEGES ON DATABASE kal_online TO kal_user;
\c kal_online
GRANT ALL ON SCHEMA public TO kal_user;
EOF
```

### 1.3 Run Migration Scripts
```bash
cd /workspace/kalonline-server
sudo -u postgres psql -d kal_online -f migrations/kal_auth_pg.sql
sudo -u postgres psql -d kal_online -f migrations/kal_db_pg.sql
```

### 1.4 Insert Test Data (Optional but Recommended)
```bash
sudo -u postgres psql -d kal_online << EOF
-- Create a test account (password is 'test123' hashed with BCrypt in C++ code)
INSERT INTO users (username, password_hash, email, created_at) 
VALUES ('test_user', '\$2b\$10\$XQxqz5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z5Z', 'test@kalonline.com', NOW());

-- Insert sample map data
INSERT INTO maps (id, name, min_x, max_x, min_y, max_y) 
VALUES (1, 'StartZone', -500, 500, -500, 500);
EOF
```

---

## Phase 2: Configuration

### 2.1 Update config.yaml
Edit `/workspace/kalonline-server/config.yaml`:
```yaml
database:
  host: "localhost"
  port: 5432
  name: "kal_online"
  user: "kal_user"
  password: "kal_password"

auth_server:
  port: 9001
  max_connections: 1000

db_server:
  port: 9002
  pool_size: 10

main_server:
  port: 9003
  tick_rate: 30
  view_distance: 100.0
```

### 2.2 Verify Asset Paths
Ensure these folders exist and contain original files:
- `/workspace/Config/` (InitItem.txt, InitMonster.txt, etc.)
- `/workspace/Map/` (*.ksm files)

---

## Phase 3: Launch Servers

### 3.1 Open Terminal Tabs (Run 3 separate terminals)

**Terminal 1: Auth Server**
```bash
cd /workspace/kalonline-server/build
./kal-auth
```
*Expected Log: "Auth Server listening on port 9001"*

**Terminal 2: DB Server**
```bash
cd /workspace/kalonline-server/build
./kal-db
```
*Expected Log: "DB Server listening on port 9002"*

**Terminal 3: Main Game Server**
```bash
cd /workspace/kalonline-server/build
./kal-main
```
*Expected Logs:*
- *"Loading maps from .ksm files..."*
- *"Loaded X items from InitItem.txt"*
- *"Loaded Y monsters from InitMonster.txt"*
- *"Main Server listening on port 9003"*
- *"Game loop started at 30 TPS"*

---

## Phase 4: Automated Testing (Light Client)

### 4.1 Run Python Test Suite
```bash
cd /workspace/kalonline-server/tests
python3 light_client.py
```

**Expected Output:**
```
🚀 KalOnline Light Client Starting...
[OK] Connected to 127.0.0.1:9001
[OK] Authentication Successful
[OK] Connected to 127.0.0.1:9003
Testing Movement... [OK] Move Packet Sent
Testing Chat... [OK] Chat Packet Sent
Testing Combat... [OK] Attack Packet Sent

========================================
FINAL TEST REPORT
========================================
✅ TCP Connection: PASS
✅ Auth Login: PASS
✅ Movement Packet: PASS
✅ Chat Packet: PASS
✅ Combat Packet: PASS

Total: 5/5 Tests Passed
🎉 ALL SYSTEMS OPERATIONAL
```

---

## Phase 5: Manual Testing (Original Client)

### 5.1 Configure Client
- Modify client's server IP to `127.0.0.1` (or your server IP)
- Ensure client version matches 2003 protocol

### 5.2 Test Checklist
| Feature | Test Method | Expected Result |
|---------|-------------|-----------------|
| **Login** | Enter credentials | Success message, character select screen |
| **Character Create** | Create new char | Character appears in list |
| **Spawn** | Enter world | Player appears on map, terrain visible |
| **Movement** | Walk around | Smooth movement, no clipping through walls |
| **Chat** | Type message | Message appears in chat window |
| **NPC Interaction** | Talk to NPC | Dialog window opens |
| **Combat** | Attack monster | Damage numbers, HP reduction |
| **Skills** | Cast skill | Animation, cooldown, mana cost |
| **Items** | Pick up item | Item added to inventory |
| **Party** | Invite player | Party UI shows member |
| **Guild** | Create guild | Guild name appears in UI |

---

## Phase 6: Monitoring & Debugging

### 6.1 Check Server Logs
- Look for `[ERROR]` or `[WARN]` tags in terminal output
- Check `logs/` directory for detailed files

### 6.2 Database Verification
```bash
sudo -u postgres psql -d kal_online -c "SELECT * FROM users;"
sudo -u postgres psql -d kal_online -c "SELECT COUNT(*) FROM players;"
```

### 6.3 Network Diagnostics
```bash
# Check ports are listening
netstat -tulpn | grep :900

# Test connectivity
telnet 127.0.0.1 9001
```

---

## Troubleshooting Common Issues

| Issue | Solution |
|-------|----------|
| **Connection Refused** | Ensure servers are running, check firewall (`ufw allow 9001,9002,9003`) |
| **Auth Failed** | Verify BCrypt hash in DB matches C++ implementation |
| **Map Not Loading** | Check `.ksm` file paths in config, verify file permissions |
| **Crash on Startup** | Check database connection string in `config.yaml` |
| **Protocol Mismatch** | Verify client version is 2003, check packet opcodes in `Protocol.h` |

---

## Success Criteria ✅

The project is considered **fully successful** when:
1. All 3 servers start without errors
2. Light Client passes 5/5 tests
3. Original client can login, create character, and enter world
4. Player can move, chat, and interact with NPCs
5. Combat system deals damage correctly
6. No memory leaks after 1 hour of uptime

**Next Steps After Success:**
- Deploy to Linux production server (systemd/Docker)
- Implement Qt6 GUI management console
- Add anti-cheat validation
- Load testing with 1000+ bots
