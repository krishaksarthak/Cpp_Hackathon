# ISO 26262 Functional Safety — HARA Document
## Smart Cabin & Vehicle Health Monitoring System
**Version:** 1.0.0 | **Standard:** ISO 26262:2018 | **ASIL Max:** ASIL C

---

## 1. Overview

This document presents the **Hazard Analysis and Risk Assessment (HARA)** for the Smart Cabin & Vehicle Health Monitoring System. The system monitors 6 safety-critical vehicle conditions in real time using a multi-threaded C++17 architecture and generates alerts and Diagnostic Trouble Codes (DTCs) when hazardous conditions are detected.

The system operates as an **observer/monitor** — it detects, logs, and notifies. It does **not** actuate (brake, steer, etc.).

---

## 2. System Description

| Property | Value |
|---|---|
| System Name | Smart Cabin & Vehicle Health Monitoring System |
| Standard | ISO 26262:2018 Road Vehicles – Functional Safety |
| Architecture | AUTOSAR-Inspired, C++17, Multi-threaded |
| Safety Classification | Observer / Monitor — No direct actuation |
| Maximum ASIL | **ASIL C** |
| Thread Safety | `std::mutex` on all shared state |
| Watchdog | Thread health monitored with configurable timeout |
| Logging | All alerts logged with timestamp and freeze-frame data |

---

## 3. ASIL Determination Methodology

Per ISO 26262 Part 3, ASIL is determined by combining three parameters:

| Parameter | Levels | Description |
|---|---|---|
| **Severity (S)** | S0–S3 | Severity of harm to people |
| **Exposure (E)** | E0–E4 | Probability of hazardous situation occurring |
| **Controllability (C)** | C0–C3 | Ability of driver to avoid harm |

**ASIL Lookup Table (ISO 26262-3:2018, Table 4):**

| S \ E,C | E1,C3 | E2,C2 | E3,C2 | E4,C2 | E3,C3 | E4,C3 |
|---|---|---|---|---|---|---|
| S1 | QM | QM | QM | QM | QM | A |
| S2 | QM | QM | A | B | A | B |
| S3 | QM | A | B | C | C | D |

---

## 4. Hazard Analysis and Risk Assessment Table

| # | DTC | Alert | Hazardous Event | S | E | C | **ASIL** | Safety Goal |
|---|---|---|---|---|---|---|---|---|
| 1 | P0217 | ENGINE OVERHEAT | Engine thermal runaway — undetected overheating leads to fire or mechanical seizure | S3 | E3 | C2 | **ASIL C** | SG-01 |
| 2 | P0562 | LOW BATTERY | Loss of vehicle electrical power causing failure of safety-critical systems while driving | S2 | E3 | C2 | **ASIL B** | SG-02 |
| 3 | P0219 | OVERSPEED | Loss of vehicle control at excessive speed, increasing collision probability and severity | S2 | E4 | C3 | **ASIL B** | SG-03 |
| 4 | C0077 | LOW TIRE PRESSURE | Tire blowout or loss of vehicle stability resulting in rollover or lane departure | S2 | E3 | C2 | **ASIL B** | SG-04 |
| 5 | B1026 | DOOR OPEN WARNING | Occupant ejection or injury due to door opening while vehicle is in motion above 10 km/h | S3 | E2 | C1 | **ASIL C** | SG-05 |
| 6 | B0075 | SEATBELT WARNING | Severe occupant injury/fatality in collision due to unrestrained occupant | S3 | E4 | C3 | **ASIL B** | SG-06 |

---

## 5. Safety Goals

### SG-01 — Engine Overheat (ASIL C)
> The system shall detect engine coolant temperature exceeding the critical threshold and alert the driver within 500ms to prevent thermal runaway and engine damage.

- **Monitored Parameter:** Engine coolant temperature (°C)
- **Threshold:** Configurable per driver profile (default: 110°C critical, 100°C warning)
- **Alert Severity:** CRITICAL
- **DTC Generated:** P0217 with freeze-frame snapshot

---

### SG-02 — Low Battery Voltage (ASIL B)
> The system shall continuously monitor battery voltage and warn the driver before voltage drops below the safe operating threshold to prevent sudden loss of electrical power.

- **Monitored Parameter:** Battery voltage (V)
- **Threshold:** Configurable (default: < 10.0V)
- **Alert Severity:** WARNING
- **DTC Generated:** P0562 with freeze-frame snapshot

---

### SG-03 — Overspeed (ASIL B)
> The system shall alert the driver immediately when vehicle speed exceeds the profile-configured speed limit to encourage safe speed reduction.

- **Monitored Parameter:** Vehicle speed (km/h)
- **Threshold:** Driver profile dependent (Eco: 100 km/h, Sport: 180 km/h, Comfort: 120 km/h)
- **Alert Severity:** WARNING
- **DTC Generated:** P0219 with freeze-frame snapshot

---

### SG-04 — Low Tire Pressure (ASIL B)
> The system shall detect tire pressure below the minimum safe threshold in real time and warn the driver to reduce speed and service the vehicle.

- **Monitored Parameter:** Tire pressure (PSI)
- **Threshold:** Configurable (default: < 25 PSI)
- **Alert Severity:** WARNING
- **DTC Generated:** C0077 with freeze-frame snapshot

---

### SG-05 — Door Open While Moving (ASIL C)
> The system shall immediately raise a CRITICAL alert if any door is detected as open while vehicle speed exceeds 10 km/h, to prompt immediate safe stop.

- **Monitored Parameters:** Door state (OPEN/CLOSED) + Vehicle speed (km/h)
- **Condition:** Door OPEN AND speed > 10 km/h
- **Alert Severity:** CRITICAL
- **DTC Generated:** B1026 with freeze-frame snapshot

---

### SG-06 — Seatbelt Not Fastened While Moving (ASIL B)
> The system shall continuously alert the driver when the seatbelt is unfastened while vehicle speed exceeds 10 km/h until the seatbelt is fastened.

- **Monitored Parameters:** Seatbelt state (LOCKED/UNLOCKED) + Vehicle speed (km/h)
- **Condition:** Seatbelt UNLOCKED AND speed > 10 km/h
- **Alert Severity:** WARNING
- **DTC Generated:** B0075 with freeze-frame snapshot

---

## 6. Safety Architecture Notes

### 6.1 Thread Safety
All shared alert state is protected by `std::mutex`. The `AlertManager` uses a dedicated config mutex to separate threshold reads from alert reads, preventing priority inversion.

### 6.2 Watchdog Health Monitor
A `Watchdog` class monitors all 4 worker threads with configurable timeout (default: 5000ms). Threads must send periodic heartbeats; missed heartbeats are logged and flagged.

### 6.3 Freeze Frame Data Capture
When a DTC is generated, a complete sensor snapshot (freeze frame) is captured at that instant. This enables post-incident analysis consistent with OBD-II diagnostic standards.

### 6.4 Alert Deduplication
Each alert condition uses prefix-matching deduplication to ensure no duplicate alerts for the same ongoing condition. Alerts are resolved automatically when the condition clears.

### 6.5 Persistent Logging
All alerts, state changes, and system events are logged asynchronously to `logs/vehicle_log.txt` with ISO 8601 timestamps. Logs are append-only and survive process restarts.

---

## 7. Embedded Code Reference

The HARA table is also embedded directly in the codebase as compile-time constants for traceability:

```cpp
// include/common/SafetyChecklist.hpp
VehicleSystem::Safety::HARA_TABLE  // std::array<HARAEntry, 6>
```

ASIL levels are enforced at the alert construction level:
```cpp
// src/alerts/AlertManager.cpp
Alert alert(AlertSeverity::CRITICAL, "ENGINE OVERHEAT ...",
            SensorType::ENGINE_TEMPERATURE, engineTemp,
            ASILLevel::C);  // ISO 26262 ASIL C
```

DTC entries carry ASIL, hazard, and safety goal strings loaded from `data/dtc_codes.json`.

---

*This document is generated and maintained as part of the Vehicle Monitoring System codebase. For audit purposes, refer to `include/common/SafetyChecklist.hpp` and `data/dtc_codes.json`.*
