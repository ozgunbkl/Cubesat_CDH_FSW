# CubeSat Command & Data Handling (C&DH) Flight Software

A production-grade, safety-critical Flight Software (FSW) system implementing the central "Brain" and Command & Data Handling subsystem for CubeSat missions. Built on FreeRTOS and validated on ESP32 hardware, this core manages the orchestration of data across the entire satellite.

## 🎯 Mission Overview

This C&DH subsystem serves as the command authority and data orchestrator for CubeSat operations, managing:

- **Centralized Command Routing** - NEW: CCSDS Router extracts Application Process Identifiers (APIDs) to dispatch commands to ADCS, EPS, and other subsystems
- **Telecommand Processing** - Secure uplink command reception, decapsulation, and validation
- **Telemetry Generation** - Periodic health data collection with CCSDS-compliant headers
- **Data Archiving** - On-board storage with scheduled downlink capability
- **Mode Management** - Safe/Nominal/Critical operational state transitions
- **Fault Monitoring** - Watchdog-based health monitoring with automated FDIR (Fault Detection, Isolation, and Recovery)

## 🏗️ System Architecture

### The Routing Backbone (Post Office Pattern)

**NEW**: Following the implementation of the CCSDS Router (`cdhs_router.c`), the CDH now acts as a central dispatcher. It receives validated packets from the Communications layer and routes them based on the 11-bit APID extracted from CCSDS packet headers.

### Concurrency Model

The FSW implements a multi-threaded, message-passing architecture using FreeRTOS primitives:
```
┌─────────────────────────────────────────────────────────────┐
│                    C&DH Flight Software                     │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────┐      ┌──────────────┐                     │
│  │   Comms      │─────▶│ CDHS Router  │──── NEW MODULE     │
│  │ (Link Layer) │      │ (APID Dispatch)   │                │
│  └──────────────┘      └──────────────┘     │               │ 
│                                    ┌────────▼─────────┐     │
│  ┌──────────────┐                  │  Subsystem Hub   │     │
│  │  Watchdog    │                  │ (ADCS/EPS/PLD)   │     │
│  │   Monitor    │                  └────────┬─────────┘     │
│  └──────────────┘                           │               │
│                                             │               │
│  ┌──────────────┐      ┌──────────────┐     │               │
│  │  EPS Health  │      │ System Mode  │◀────┘              │
│  │   Monitor    │─────▶│   (Mutex)    │                    │
│  └──────────────┘      └──────────────┘                     │
│                                 │                           │
│  ┌──────────────┐               │                           │
│  │   Telemetry  │               │                           │
│  │  Generator   │───────────────┘                           │
│  └──────┬───────┘                                           │
│         │                                                   │
│         │                  ┌──────────────┐                 │
│         └─────────────────▶│   TM Queue   │                │
│                            └──────┬───────┘                 │
│                                   │                         │
│  ┌──────────────┐                 │                         │
│  │ Data Logger  │◀────────────────┘                        │
│  │  & Downlink  │                                           │
│  └──────────────┘                                           │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### FreeRTOS Task Structure

| Task           | Priority | Stack Size | Period       | Purpose                           |
|----------------|----------|------------|--------------|-----------------------------------|
| CDHS Router    | 3        | 4096       | Event-driven | CCSDS packet parsing & routing    |
| TC Processor   | 3        | 2048       | Event-driven | Command validation & execution    |
| TM Generator   | 2        | 2048       | 500ms        | Telemetry packet creation         |
| Data Logger    | 2        | 2048       | Event-driven | TM archiving & downlink           |
| EPS Monitor    | 2        | 2048       | 1000ms       | Power subsystem health check      |
| Watchdog Monitor | 4 (Highest) | 2048  | 1000ms       | Task liveness detection           |

## 🔒 Safety-Critical Features

### 1. CCSDS Packet Routing (NEW)

The CDH interprets standard CCSDS headers to ensure commands reach the correct hardware. It handles:

- **Primary Header Extraction**: Reading the 11-bit APID from incoming packets
- **Secondary Header Parsing**: Validating MET (Mission Elapsed Time) for time-tagged commands
- **Endianness Management**: Using `ntohs()` to handle big-endian network data on the little-endian ESP32
- **Integration with Comms**: Receives CRC-validated packets from the Communications subsystem

### 2. Software Watchdog Timer

Monitors all critical tasks for liveness. If any task fails to call `watchdog_pet()` within 15 seconds, the system automatically transitions to MODE_CRITICAL.

### 3. CRC-16 Command Validation

All frames arriving from the Comms Subsystem are verified using CRC-16 CCITT-FALSE. Corrupted packets are dropped before reaching the application logic. Ensures zero-tolerance for data corruption in vacuum environments.

## 📡 Command & Telemetry Interface

### Telecommand Routing (APIDs)

| APID  | Destination   | Function                              |
|-------|---------------|---------------------------------------|
| 0x010 | ADCS          | Attitude control & orbit maintenance  |
| 0x020 | EPS           | Power rail management & battery heater|
| 0x040 | CDHS          | Internal FSW updates & mode changes   |
| 0x050 | Housekeeping  | Health status requests                |

### CCSDS Telemetry Packet Structure

The CDH generates telemetry using the following standardized layout:

- **Primary Header (6B)**: Version, Type, Sec. Header Flag, APID, Seq. Count, Length
- **Secondary Header (8B)**: 64-bit Mission Elapsed Time (MET) from the Time Service
- **Payload**: Actual sensor or status data

## 🔗 Project Integration (The FSW Stack)

This repository is a core component of a modular, multi-repo architecture. It integrates with:

- **[CubeSat_FSW_Comms](https://github.com/ozgunbkl/CubeSat_FSW_Comms)**: Handles radio synchronization and Link-Layer framing with CRC validation
- **[CubeSat_Time_Service](https://github.com/ozgunbkl/CubeSat_Time_Service)**: Provides the 8-byte MET for telemetry synchronization and time-tagged command validation

## 🧪 Hardware Validation

**Test Platform:**
- **MCU**: ESP32-DevKitC (Dual-core Xtensa LX6, 240 MHz)
- **Framework**: ESP-IDF v4.4+ with FreeRTOS kernel

### ✅ Grand Integration Success

The system has been validated to process raw byte streams from the Comms Parser and correctly route them to the CDH internal logic:
```
[TEST] Feeding 21 bytes into Parser...
DEBUG SAT: Calc: 0xC78E, Recv: 0xC78E
CDHS Router: Routing packet to ADCS...
[ADCS] Command received: Orbit Maintenance Boost
test_FullChain_RadioToRouter: PASS
```

## 📁 Project Structure
```
Cubesat_CDH_FSW/
├── src/
│   ├── cdhs_router.c           # NEW: CCSDS APID-based dispatcher
│   ├── tc_processor.c          # Telecommand handler
│   ├── tm_generator.c          # Telemetry creation logic
│   ├── eps_monitor.c           # Power subsystem health
│   └── watchdog.c              # Task supervision
├── include/
│   ├── cdhs_router.h           # NEW: Routing prototypes
│   ├── ccsds_packet.h          # CCSDS header structures
│   └── cdh_config.h            # System configuration
├── test/
│   └── test_integration.c      # Cross-repo system test
└── platformio.ini              # Build configuration
```

## 🧰 Recent Updates

- **[x] CCSDS Router Implementation** - Added `cdhs_router.c` and `cdhs_router.h` for APID-based packet routing
- **[x] Communications Integration** - Integrated with CubeSat_FSW_Comms for CRC-validated packet reception
- **[x] Time Service Integration** - Integrated with CubeSat_Time_Service for MET-based timestamping
- **[x] End-to-End Validation** - Successful integration testing from Comms Parser through Router to subsystem dispatch

## 📚 Related Repositories

This C&DH subsystem is the heart of the FSW stack. Check out the complete architecture:

- [Communications Subsystem](https://github.com/ozgunbkl/CubeSat_FSW_Comms)
- [Time Service](https://github.com/ozgunbkl/CubeSat_Time_Service)
- [All Repositories](https://github.com/ozgunbkl?tab=repositories)

---

**Project Status**: Active development with recent CCSDS routing integration complete
