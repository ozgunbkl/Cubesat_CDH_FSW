# CubeSat Command & Data Handling (C&DH) Flight Software

A **production-grade, safety-critical Flight Software (FSW)** system implementing concurrent Command & Data Handling subsystem for CubeSat missions. Built on **FreeRTOS** and validated on **ESP32** hardware.


---

## 🎯 Mission Overview

This C&DH subsystem serves as the **command authority and data orchestrator** for CubeSat operations, managing:

- **Telecommand Processing** - Secure uplink command reception and validation
- **Telemetry Generation** - Periodic system health data collection
- **Data Archiving** - On-board storage with scheduled downlink capability
- **Mode Management** - Safe/Nominal/Critical operational state transitions
- **Fault Monitoring** - Watchdog-based health monitoring with automated recovery

---

## 🏗️ System Architecture

### Concurrency Model

The FSW implements a **multi-threaded, message-passing architecture** using FreeRTOS primitives:

```
┌─────────────────────────────────────────────────────────────┐
│                    C&DH Flight Software                     │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────┐      ┌──────────────┐                     │
│  │   TC Uplink  │─────▶│ Command Queue│─────               │
│  │   Injector   │      └──────────────┘     │               │
│  └──────────────┘                           ▼               │ 
│                                    ┌──────────────────┐     │
│  ┌──────────────┐                  │  TC Processor    │     │
│  │  Watchdog    │                  │  (CRC-16 Check)  │     │
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
│         │                 ┌──────────────┐                  │
│         └────────────────▶│ TM Queue     │                 │
│                           └──────┬───────┘                  │
│                                  │                          │
│  ┌──────────────┐                │                          │
│  │ Data Logger  │◀───────────────┘                         │
│  │ & Downlink   │                                           │
│  └──────────────┘                                           │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### FreeRTOS Task Structure

| Task | Priority | Stack Size | Period | Purpose |
|------|----------|------------|--------|---------|
| **TC Processor** | 3 | 2048 | Event-driven | Command validation & execution |
| **TM Generator** | 2 | 2048 | 500ms | Telemetry packet creation |
| **Data Logger** | 2 | 2048 | Event-driven | TM archiving & downlink |
| **EPS Monitor** | 2 | 2048 | 1000ms | Power subsystem health check |
| **Watchdog Monitor** | 4 (Highest) | 2048 | 1000ms | Task liveness detection |
| **TC Injector** | 1 | 2048 | Variable | Ground command simulation |

### Inter-Task Communication

**Message Queues:**
- `xCommandQueue` - TC packets from uplink to processor (depth: 10)
- `xTelemetryQueue` - TM packets from generator to logger (depth: 20)

**Synchronization:**
- `xModeMutex` - Protects global `system_mode` variable from race conditions

---

## 🔒 Safety-Critical Features

### 1. Software Watchdog Timer

Monitors all critical tasks for liveness. If any task fails to call `watchdog_pet()` within **15 seconds**, the system automatically transitions to `MODE_CRITICAL`.

```c
// Each task must periodically report health
watchdog_pet(TASK_TC_PROCESSOR);
```

### 2. CRC-16 Command Validation

All telecommands include **CRC-16 CCITT** checksum for data integrity:

```c
typedef struct __attribute__((packed)) {
    uint8_t command_id;
    uint8_t payload;
    uint16_t crc;
} telecommand_t;
```

- Corrupted commands are **immediately rejected**
- `__attribute__((packed))` ensures byte-alignment compatibility
- Zero tolerance for invalid data in safety-critical operations

### 3. Fault Detection & Recovery

**Autonomous FDIR Logic:**
- EPS voltage monitoring (critical threshold: < 3.3V)
- Automatic mode degradation on fault detection
- Coordinated subsystem shutdown in critical scenarios

---

## 🛰️ Operational Modes

The C&DH implements a **finite state machine** for mission phase management:

```
    SAFE ──────┐
     │         │
     │         │ (Recovery Complete)
     ▼         │
  NOMINAL  ────┤
     │         │
     │         │ (Fault Detected)
     ▼         │
  CRITICAL  ───┘
```

| Mode | Description | Active Tasks | Power Profile |
|------|-------------|--------------|---------------|
| **SAFE** | Boot & initialization | Watchdog, TC Processor | Minimal |
| **NOMINAL** | Normal mission operations | All tasks active | Full |
| **CRITICAL** | Fault recovery mode | Watchdog, essential services only | Reduced |

---

## 📡 Command & Telemetry Interface

### Telecommand Packets

| Command ID | Name | Payload | Function |
|------------|------|---------|----------|
| `0x00` | TC_NO_OP | None | Communication link test |
| `0x01` | TC_SET_MODE | Mode enum | Force system mode transition |
| `0x02` | TC_REQUEST_TM | None | Trigger immediate telemetry burst |

### Telemetry Packets

```c
typedef struct __attribute__((packed)) {
    uint32_t timestamp_ms;
    float voltage;
    uint8_t mode;
    uint8_t checksum;
} telemetry_packet_t;
```

**Telemetry Generation Cycle:**
- Generated every **500ms** in NOMINAL mode
- Archived in FIFO buffer (depth: 100 packets)
- Downlinked during scheduled ground station passes

---

## 🧪 Hardware Validation

### Test Platform

- **MCU:** ESP32-DevKitC (Dual-core Xtensa LX6, 240 MHz)
- **Framework:** ESP-IDF v4.4+ with FreeRTOS kernel
- **Build System:** PlatformIO

### Validated Scenarios

#### ✅ **Nominal Mission Cycle**
```
[TC_INJECTOR] Sending TC_SET_MODE → NOMINAL
[TC_PROC] CRC OK. Mode transition: SAFE → NOMINAL
[TM_GEN] Running NOMINAL mission cycle
[DATA_LOGGER] Archived packet T: 1002ms | V: 3.30V
```

#### ✅ **Autonomous Fault Recovery**
```
[EPS_MON] WARNING: Voltage below threshold (3.2V)
[EPS_MON] CRITICAL FAULT! Forcing MODE_CRITICAL
[TC_PROC] Mode degradation: NOMINAL → CRITICAL
[WATCHDOG] System in safe configuration
```

#### ✅ **Ground Station Downlink**
```
[TC_INJECTOR] Ground station pass detected
[DATA_LOGGER] --- DOWNLINK BURST ACTIVE ---
[DATA_LOGGER] Transmitting 5 archived packets...
[DATA_LOGGER] --- DOWNLINK COMPLETE ---
```


---

## 🚀 Getting Started

### Prerequisites

- **PlatformIO IDE** (VSCode extension recommended)
- **ESP32 development board**
- **USB-to-Serial driver** (CP210x or CH340)

### Build & Flash

1. **Clone the repository:**
   ```bash
   git clone https://github.com/ozgunbkl/Cubesat_CDH_FSW.git
   cd Cubesat_CDH_FSW
   ```

2. **Configure serial port:**
   Edit `platformio.ini`:
   ```ini
   [env:esp32dev]
   platform = espressif32
   board = esp32dev
   framework = espidf
   monitor_speed = 115200
   upload_port = /dev/ttyUSB0  ; Update with your port
   ```

3. **Build and upload:**
   ```bash
   platformio run --target upload
   ```

4. **Monitor serial output:**
   ```bash
   platformio device monitor
   ```

### Expected Boot Sequence

```
C&DH FSW Initialization Started...
WATCHDOG: Software Watchdog initialized.
TC Processor Task initialized and waiting for commands.
EPS Monitoring Task initialized and running.
TM Generator Task initialized and running.
DATA LOGGER: Task initialized, monitoring telemetry queue.
All tasks and communication channels launched. System running.
```

---

## 📁 Project Structure

```
Cubesat_CDH_FSW/
├── src/
│   ├── main.c                  # Entry point & task initialization
│   ├── tc_processor.c          # Telecommand handler
│   ├── tm_generator.c          # Telemetry creation logic
│   ├── data_logger.c           # Archiving & downlink
│   ├── eps_monitor.c           # Power subsystem health
│   ├── watchdog.c              # Task supervision
│   └── tc_injector.c           # Ground command simulator
├── include/
│   ├── cdh_types.h             # Packet structures & enums
│   ├── cdh_config.h            # System configuration
│   └── crc16.h                 # Checksum utilities
├── test/
│   └── test_native/            # Unit tests (native platform)
├── platformio.ini              # Build configuration
└── README.md
```

---

## 🔧 Configuration

Key parameters in `include/cdh_config.h`:

```c
#define WATCHDOG_TIMEOUT_MS     15000    // Task liveness timeout
#define TM_GENERATION_PERIOD_MS 500      // Telemetry interval
#define TM_ARCHIVE_DEPTH        100      // On-board storage capacity
#define EPS_CRITICAL_VOLTAGE    3.3f     // Fault threshold
```

---

## 🧰 Future Enhancements

- [ ] **CCSDS Packet Standard** - Migrate to industry-standard TM/TC format
- [ ] **Time Service Integration** - UTC synchronization for timestamping

---

## 📊 Verification Status

| Test Case | Status | Evidence |
|-----------|--------|----------|
| Task scheduling determinism | ✅ Passed | FreeRTOS trace analysis |
| CRC validation accuracy | ✅ Passed | 10,000 packet test |
| Watchdog fault injection | ✅ Passed | Manual task stall test |
| Mode transition correctness | ✅ Passed | State machine verification |
| Queue overflow handling | ✅ Passed | Stress test (100 msg/s) |

---

**This C&DH subsystem is part of a larger modular CubeSat FSW stack. Check out the complete architecture:**

- [ADCS Flight Software](https://github.com/ozgunbkl/CubeSat_ADCS_FSW)
- [EPS Controller](https://github.com/ozgunbkl/EPS_Controller_FSW)
- [FDIR Service](https://github.com/ozgunbkl/CubeSat_FDIR_Project)
- [Communications Subsystem](https://github.com/ozgunbkl/CubeSat_FSW_Comms)
