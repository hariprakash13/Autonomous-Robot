# ENGR7023 – Autonomous Corridor Navigation Robot
### Real-time Embedded Systems | Oxford Brookes University | S1 2025–26

---

## Overview

This project implements an autonomous corridor navigation system on an **STM32 Nucleo-64** microcontroller using the **Mbed platform**. The robot navigates dynamic corridor environments, avoids both static and dynamic obstacles, and recovers from dead ends — all without human intervention.

The control system is built around a **5-state Finite State Machine (FSM)**, continuously reading distance data from two HC-SR04 ultrasonic sensors and adjusting steering and speed in real time.

---

## Hardware

| Component | Description | Pin(s) |
|-----------|-------------|--------|
| STM32 Nucleo-64 | Central microcontroller | — |
| HC-SR04 Ultrasonic (Left) | Left wall distance measurement | TRIG: PA_10 / ECHO: PB_3 |
| HC-SR04 Ultrasonic (Right) | Right wall distance measurement | TRIG: PA_8 / ECHO: PA_9 |
| Steering Servo | Directional control via PWM | PB_0 |
| Motor (Forward) | Forward propulsion via PWM | PB_10 |
| Motor (Reverse) | Reverse propulsion via PWM | PB_4 |

---



### State Descriptions

| State | Behaviour | Trigger Condition |
|-------|-----------|-------------------|
| **FORWARD** | Cruise forward; steer away from close wall | Default state |
| **STOPPED** | Halt motors; wait 300 ms | Both sensors < 150 mm |
| **REVERSING** | Reverse straight 600 ms; choose turn direction | After STOPPED dwell |
| **TURNING** | Full lock steering 800 ms toward open side | After REVERSING |
| **DEAD_END** | Reverse 2 s then halt permanently | Terminal fallback |

---

## Sensor Distance Thresholds

| Threshold | Condition | Response |
|-----------|-----------|----------|
| < 200 mm (one side) | Robot drifting toward wall | Steer away; reduce speed |
| < 150 mm (both sides) | Dead end / head-on obstacle | Trigger STOPPED → recovery |
| 500 mm (timeout) | No echo within 30 ms | Safe default; no action |

Distance is calculated as:
```
distance (mm) = (echo_duration_us × 0.343) / 2
```

---

## Steering & Motor PWM Parameters

| Parameter | Value | Description |
|-----------|-------|-------------|
| PWM Period | 20 ms | Standard RC servo/ESC period |
| Steer Centre | 1500 µs | Straight ahead |
| Steer Left | 1900 µs | Full left lock |
| Steer Right | 1100 µs | Full right lock |
| Forward Cruise | 10 ms pulse | Normal speed |
| Forward Corrective | 8 ms pulse | Reduced speed during turns |
| Reverse | 8 ms pulse | Recovery manoeuvre |

---

## Control Loop Timing

| Phase | Duration |
|-------|----------|
| Left sensor read | up to 60 ms |
| Right sensor read | up to 60 ms |
| FSM evaluation | ~5 ms |
| Actuator write | ~5 ms |
| sleep_for delay | 50 ms |
| **Worst-case cycle** | **~170–210 ms** |

Effective control rate: **~5–6 Hz** (worst case) / **~14 Hz** (normal)

---

## Test Scenarios & Results

| Scenario | Runs | Successful | Collisions |
|----------|------|------------|------------|
| Straight Corridor – Static Obstacles | 5 | 5 (100%) | 0 |
| Zig-Zag Corridor – Varying Widths | 3 | 2 (67%) | 0 |
| Dynamic Corridor – Moving Obstacle | 5 | 5 (100%) | 0 |

---

## Evidence of Test-Driven Development

The following changes were made iteratively based on test results:

- `CHANGE 1`: Dead-end threshold reduced from **200 mm → 150 mm** after false triggers in narrow sections during Scenario 2 testing
- `CHANGE 2`: Turn duration increased from **600 ms → 800 ms** after Scenario 2 testing showed insufficient heading change
- `CHANGE 3`: STOPPED dwell increased from **200 ms → 300 ms** to allow dynamic obstacles to partially clear before reversing
- `CHANGE 4`: Sensor timeout set to **30 ms** after testing showed 25 ms caused missed echoes on angled surfaces
- `TEST 1`: Verified `L < 200` triggers `steer_right()` at measured wall distance ~180 mm ✅
- `TEST 2`: Verified dual `L < 150 & R < 150` correctly triggers STOPPED state ✅
- `TEST 3`: Verified 800 ms turn duration achieves ~90° heading change on hard floor ✅
- `TEST 4`: Verified 30 ms timeout safely returns 500 mm on open space ✅
- `TEST 5`: Verified recovery sequence (STOPPED → REVERSING → TURNING → FORWARD) completes without collision ✅

---

## Repository Contents

| File | Description |
|------|-------------|
| `main.cpp` | Full navigation source code with inline comments |
| `README.md` | Project documentation |

---

## How to Build & Flash

1. Open **Keil µVision** or **Mbed Studio**
2. Create a new Mbed project targeting **NUCLEO-F446RE**
3. Add `main.cpp` to the project source
4. Connect the Nucleo board via USB
5. Build and flash — the robot initialises after a **2-second startup delay**
6. Serial debug output available at **9600 baud** via USB:
   ```
   State:0  L:245  R:312
   ```

---

## Dependencies

- [Mbed OS 6](https://os.mbed.com/docs/mbed-os/v6.16/)
- STM32 Nucleo-64 (NUCLEO-F446RE or compatible)
- HC-SR04 Ultrasonic Sensor Library (standard Mbed DigitalIn/DigitalOut)

---

## Author

Student Number: [HariPrakash]  
Module: ENGR7023 – Real-time Embedded Systems  
Oxford Brookes University | Semester 1, 2025–26
