# KalOnline C++23 Server Modernization

Complete modernization of the KalOnline (2003 MMORPG) server suite using modern C++23, PostgreSQL, and cross-platform architecture.

## Project Structure

```
kalonline-server/
├── CMakeLists.txt              # Main build configuration
├── conanfile.txt               # Conan dependencies
├── config/
│   └── config.yaml.example     # Configuration template
├── include/
│   └── common/
│       ├── crypto/
│       │   └── Crypto.hpp      # XOR/Blowfish encryption
│       ├── logger/
│       │   └── Logger.hpp      # Async thread-safe logging
│       ├── config/
│       │   └── Config.hpp      # YAML configuration
│       ├── network/
│       │   └── Network.hpp     # ASIO TCP server
│       ├── database/
│       │   └── Database.hpp    # PostgreSQL libpqxx wrapper
│       └── utils/
│           └── Utils.hpp       # Utility functions
├── src/
│   ├── common/                 # Common library implementations
│   ├── auth/                   # Auth server
│   ├── db/                     # Database server
│   └── main/                   # Main game server
├── migrations/                 # PostgreSQL migration scripts
├── tests/                      # Unit tests
└── docs/                       # Documentation
```

## Features

### Core Infrastructure
- **C++23 Standard**: Modules, coroutines, ranges, std::expected
- **Cross-Platform**: Windows & Linux support
- **Async I/O**: ASIO-based with C++23 coroutines
- **Thread-Safe Logging**: Async queue-based logging system
- **Connection Pooling**: Efficient PostgreSQL connection management

### Security
- **Original Protocol**: 100% compatible with 2003 client
- **Encryption**: XOR + Blowfish packet encryption
- **Password Hashing**: BCrypt support
- **SQL Injection Prevention**: Prepared statements only

### Performance Targets
- <2ms tick latency
- 2000+ concurrent connections
- Lock-free queues where possible
- Zero-copy packet handling

## Building

### Prerequisites
- CMake 3.28+
- C++23 compatible compiler (GCC 13+, Clang 16+, MSVC 2022)
- Conan or vcpkg
- PostgreSQL 16+ development libraries

### Build Steps

```bash
# Create build directory
mkdir build && cd build

# Install dependencies (Conan)
conan install .. --build=missing

# Configure
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

# Build
cmake --build . --config Release

# Run tests
ctest --config Release
```

## Configuration

Copy `config/config.yaml.example` to `config/config.yaml` and modify:

```yaml
database:
  host: localhost
  port: 5432
  name: kalonline
  username: kaluser
  password: your_password

network:
  auth_port: 11000
  db_port: 11001
  main_port: 11002

game:
  exp_multiplier: 1.0      # Preserve original EXP curves
  drop_rate_multiplier: 1.0 # Original drop rates
```

## Database Setup

```bash
# Create PostgreSQL user and database
psql -U postgres
CREATE USER kaluser WITH PASSWORD 'your_password';
CREATE DATABASE kalonline OWNER kaluser;
\q

# Run migrations
psql -U kaluser -d kalonline -f migrations/kal_auth_pg.sql
psql -U kaluser -d kalonline -f migrations/kal_db_pg.sql
```

## Running Servers

```bash
# Auth Server
./kal-auth --config config/config.yaml

# Database Server
./kal-db --config config/config.yaml

# Main Game Server
./kal-main --config config/config.yaml
```

## Docker Deployment

```bash
# Build and run with docker-compose
docker-compose up -d

# View logs
docker-compose logs -f kal-main
```

## Development

### Code Style
- Follow C++ Core Guidelines
- Use `clang-format` with provided `.clang-format`
- Document all public APIs with Doxygen comments

### Testing
```bash
# Run all tests
ctest --output-on-failure

# Run specific test
./kal_tests --gtest_filter="CryptoTest.*"
```

## Architecture

### Server Communication Flow
```
Client → Auth Server → DB Server → Main Game Server
         (Login)        (Data)        (Gameplay)
```

### Packet Format (Original 2003 Protocol)
```
[Length: 2 bytes][Opcode: 2 bytes][Payload: N bytes]
```

### Key Components

1. **Auth Server**: Handle login, session management, BCrypt passwords
2. **DB Server**: Query router, connection pooling, async operations
3. **Main Server**: World manager, entities, combat, skills, items

## Migration from MSSQL

All database schemas have been converted from Microsoft SQL Server to PostgreSQL:
- `DATETIME` → `TIMESTAMP`
- `BIT` → `BOOLEAN`
- `IMAGE` → `BYTEA`
- `IDENTITY` → `SERIAL`/`GENERATED ALWAYS AS IDENTITY`

See `migrations/` for complete schema conversions.

## License

This is a reconstruction project based on reverse engineering for educational purposes.
Original KalOnline copyright belongs to its respective owners.

## Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

## Support

- Documentation: `docs/` directory
- Issues: GitHub Issues
- Discord: [link]
