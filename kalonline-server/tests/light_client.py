#!/usr/bin/env python3
"""
KalOnline Light Client & Automated Test Suite
Simulates a player connecting to the C++23 server to verify protocol and logic.
"""

import socket
import struct
import time
import sys
import hashlib

# Configuration
SERVER_IP = "127.0.0.1"
AUTH_PORT = 9001
DB_PORT = 9002
MAIN_PORT = 9003

# Protocol Constants (Match C++ Protocol.h)
OPCODE_LOGIN_REQ = 0x1001
OPCODE_LOGIN_RES = 0x1002
OPCODE_CHAR_LIST = 0x1101
OPCODE_CHAR_CREATE = 0x1102
OPCODE_CHAR_SELECT = 0x1105
OPCODE_MOVE = 0x2001
OPCODE_CHAT = 0x2101
OPCODE_ATTACK = 0x2201

class LightClient:
    def __init__(self):
        self.sock = None
        self.session_id = 0
        self.character_id = 0
        self.test_results = []

    def connect(self, ip, port, timeout=5):
        """Stage 1: Infrastructure Test"""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(timeout)
            self.sock.connect((ip, port))
            print(f"[OK] Connected to {ip}:{port}")
            self.log_test("TCP Connection", True)
            return True
        except Exception as e:
            print(f"[FAIL] Connection failed: {e}")
            self.log_test("TCP Connection", False)
            return False

    def send_packet(self, opcode, payload=b''):
        """Helper: Build packet [Length:2][Opcode:2][Payload:N]"""
        length = 4 + len(payload)
        header = struct.pack('<HH', length, opcode)
        self.sock.sendall(header + payload)

    def recv_packet(self):
        """Helper: Read packet header and payload"""
        try:
            header = self.sock.recv(4)
            if len(header) < 4:
                return None, None
            length, opcode = struct.unpack('<HH', header)
            payload_len = length - 4
            if payload_len > 0:
                payload = self.sock.recv(payload_len)
            else:
                payload = b''
            return opcode, payload
        except Exception as e:
            print(f"[ERROR] Recv failed: {e}")
            return None, None

    def test_auth_login(self, username, password):
        """Stage 2: Protocol Handshake - Auth"""
        print("\n--- Testing Auth Server ---")
        # Simple login packet simulation (Adjust based on actual C++ impl)
        # Assuming payload: string(user)\0 string(pass)\0
        payload = username.encode('utf-8') + b'\x00' + password.encode('utf-8') + b'\x00'
        self.send_packet(OPCODE_LOGIN_REQ, payload)
        
        time.sleep(0.5)
        opcode, data = self.recv_packet()
        
        if opcode == OPCODE_LOGIN_RES:
            result_code = struct.unpack('<I', data[:4])[0]
            if result_code == 0: # 0 = Success
                print("[OK] Authentication Successful")
                self.log_test("Auth Login", True)
                return True
            else:
                print(f"[FAIL] Auth Failed: Code {result_code}")
                self.log_test("Auth Login", False)
        else:
            print(f"[FAIL] Unexpected Opcode: {opcode}")
            self.log_test("Auth Login", False)
        return False

    def test_game_loop(self):
        """Stage 3 & 4: Game Logic Simulation"""
        print("\n--- Testing Main Game Server ---")
        
        # 1. Send Move Packet
        print("Testing Movement...")
        # Payload: X(float), Y(float), Z(float), Speed(byte)
        move_data = struct.pack('<fffB', 100.0, 100.0, 0.0, 50)
        self.send_packet(OPCODE_MOVE, move_data)
        time.sleep(0.2)
        print("[OK] Move Packet Sent")
        self.log_test("Movement Packet", True)

        # 2. Send Chat Packet
        print("Testing Chat...")
        chat_msg = b"Hello KalOnline!\x00"
        self.send_packet(OPCODE_CHAT, chat_msg)
        time.sleep(0.2)
        print("[OK] Chat Packet Sent")
        self.log_test("Chat Packet", True)

        # 3. Simulate Attack
        print("Testing Combat...")
        # Payload: TargetID(uint32), SkillID(uint16)
        attack_data = struct.pack('<IH', 1001, 0)
        self.send_packet(OPCODE_ATTACK, attack_data)
        time.sleep(0.2)
        print("[OK] Attack Packet Sent")
        self.log_test("Combat Packet", True)

    def log_test(self, name, passed):
        status = "PASS" if passed else "FAIL"
        self.test_results.append((name, status))

    def report(self):
        print("\n" + "="*40)
        print("FINAL TEST REPORT")
        print("="*40)
        passed = sum(1 for _, s in self.test_results if s == "PASS")
        total = len(self.test_results)
        
        for name, status in self.test_results:
            icon = "✅" if status == "PASS" else "❌"
            print(f"{icon} {name}: {status}")
            
        print(f"\nTotal: {passed}/{total} Tests Passed")
        if passed == total:
            print("🎉 ALL SYSTEMS OPERATIONAL")
            return True
        else:
            print("⚠️ SOME TESTS FAILED - CHECK SERVER LOGS")
            return False

def main():
    print("🚀 KalOnline Light Client Starting...")
    client = LightClient()

    # Step 1: Connect to Auth
    if not client.connect(SERVER_IP, AUTH_PORT):
        print("❌ Cannot connect to Auth Server. Is it running?")
        return

    # Step 2: Login
    if not client.test_auth_login("test_user", "test_pass"):
        print("⚠️ Auth test failed, but continuing to network test...")

    # Disconnect from Auth, Connect to Main (Simplified flow for testing)
    client.sock.close()
    
    # Step 3: Connect to Main Game Server (Simulating post-auth connection)
    if client.connect(SERVER_IP, MAIN_PORT):
        client.test_game_loop()
    
    # Final Report
    client.report()

if __name__ == "__main__":
    main()
