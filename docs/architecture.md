# Electron Microscope Control Board — Architecture

## Overview

This project simulates the firmware/software stack that bridges an electron microscope's
control electronics board with a host PC.  All components run in-process but are separated
by thread-safe channels, mirroring a real RS-232 / USB-UART deployment.

```
┌──────────────────────────────────────────────────────────┐
│  Host PC                                                 │
│  HostSystem                                              │
│    · set_beam_voltage() / set_stage_x() / acquire() …   │
│    · serialises Frame → raw bytes                        │
└────────────────────┬─────────────────────────────────────┘
                     │ CommunicationChannel::Pipe (host→board)
                     ▼
┌──────────────────────────────────────────────────────────┐
│  ControlBoard  (background thread)                       │
│  ┌─────────────────────┐  ┌──────────────────────────┐  │
│  │   MessageParser     │  │    CommandHandler        │  │
│  │  byte-stream FSM    │→ │  command dispatch table  │  │
│  └─────────────────────┘  └──────────┬───────────────┘  │
│                                      │                   │
│              ┌───────────────────────┼────────────────┐  │
│              ▼                       ▼                ▼  │
│       BeamController        StageController    VacuumSystem│
│       voltage/current/      X/Y/Z/tilt         pressure   │
│       focus/enable          positioning        simulation  │
│              └───────────────────────┼────────────────┘  │
│                            MicroscopeController           │
│                            (aggregates subsystems,        │
│                             owns SystemState)             │
└────────────────────┬─────────────────────────────────────┘
                     │ CommunicationChannel::Pipe (board→host)
                     ▼
              ResponseCode + payload back to HostSystem
```

## Wire Protocol

```
Offset  Bytes  Field
──────  ─────  ──────────────────────────────────────────────
0       2      Magic: 0x45 0x4D  ('E' 'M')
2       1      MessageType  (COMMAND=0x01, RESPONSE=0x02, …)
3       1      CommandId
4       2      Payload length  (big-endian)
6       N      Payload bytes
6+N     2      CRC-16/CCITT-FALSE  (covers bytes 0…6+N-1)
```

All multi-byte integers are big-endian.  Floats are IEEE-754 single-precision,
transmitted as their raw 32-bit representation in big-endian order.

### CRC Algorithm

CRC-16/CCITT-FALSE: polynomial 0x1021, initial value 0xFFFF,
no input/output bit-reversal.

## Subsystems

### BeamController
Manages the electron beam: accelerating voltage (0.1–30 kV), probe current
(0.1–100 nA), and working-distance focus (1–50 mm).  The beam cannot be enabled
until the vacuum system reports pressure below the safe threshold.

### StageController
Five-axis sample stage: X/Y translation (±50 000 µm), Z height (0–40 000 µm),
and tilt angle (±70 °).  All moves are validated against physical travel limits
before being applied.

### VacuumSystem
Models exponential pump-down from atmospheric pressure.  The beam-safe threshold
is 1×10⁻³ Pa.  The CommandHandler enforces that `ENABLE_BEAM` is rejected unless
`VacuumState::READY`.

### MicroscopeController
Owns the top-level `SystemState` FSM:

```
IDLE → READY ↔ IMAGING
                  ↓ (EMERGENCY_STOP)
               EMERGENCY → READY  (after RESET)
```

## Building

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
./build/emcb_simulator
```

## Testing

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

Three test suites:
- **MessageParser** — frame encoding, CRC validation, multi-frame streams, byte-by-byte feed
- **CommandHandler** — per-command acceptance/rejection, emergency-stop FSM, imaging flow
- **Integration** — full round-trip through ControlBoard thread and HostSystem over the channel

## Debugging with GDB

```bash
gdb ./build/emcb_simulator
(gdb) break em::CommandHandler::handle
(gdb) break em::MessageParser::feed
(gdb) run
```

Valgrind for memory analysis:

```bash
valgrind --leak-check=full --track-origins=yes ./build/emcb_simulator
```
