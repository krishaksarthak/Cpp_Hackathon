# Team Roles & Responsibilities

## Member 1: Architecture Lead
**Name**: Nischal NV  

### Primary Responsibilities
- Design base `Sensor` class hierarchy
- Implement all 6 sensor classes
- Create sensor update thread
- Define interfaces for other modules

### Deliverables
- `include/sensors/*.hpp`
- `src/sensors/*.cpp`
- Sensor framework documentation

### Dependencies
- None (foundation module)

---

## Member 2: Alert Specialist
**Name**: Sarthak Krishak

### Primary Responsibilities
- Implement Alert & AlertManager classes
- Create all 6 alert detection conditions
- Build DTC generation system
- Monitoring thread implementation

### Deliverables
- `include/alerts/*.hpp`
- `src/alerts/*.cpp`
- Alert detection logic

### Dependencies
- Sensor interfaces from Member 1

---

## Member 3: Dashboard Developer
**Name**: Payel S

### Primary Responsibilities
- Implement Dashboard class
- Create EventLogger with file I/O
- Build VehicleStatistics module
- Dashboard & logger threads

### Deliverables
- `include/dashboard/*.hpp`
- `include/logging/*.hpp`
- `src/dashboard/*.cpp`
- `src/logging/*.cpp`

### Dependencies
- Alert data from Member 2
- Sensor data from Member 1

---

## Member 4: Threading Expert
**Name**: Mohammad A

### Primary Responsibilities
- Design thread-safe data structures
- Implement ThreadManager
- Create Watchdog health monitor
- Ensure race condition prevention

### Deliverables
- `include/threading/*.hpp`
- `src/threading/*.cpp`
- Thread synchronization documentation

### Dependencies
- All modules (integration role)

---

## Member 5: Integration Lead
**Name**: Priyanshu P

### Primary Responsibilities
- JSON configuration system
- Driver profile management
- Exception handling
- Documentation & demo prep

### Deliverables
- `include/config/*.hpp`
- `src/config/*.cpp`
- README.md
- Architecture diagrams
- Demo script

### Dependencies
- Configuration points from all modules

---

## Collaboration Matrix

| Hour | Pair Programming |
|------|------------------|
| 2-3  | Member 1 + 2 (Sensor interfaces) |
| 4-5  | Member 2 + 3 (Alert integration) |
| 6-7  | Member 1 + 4 (Thread safety) |
| 8-9  | Member 3 + 5 (Config + logging) |
| 10-11| Member 4 + All (Integration) |
| 12-13| Member 5 + 3 (Exception handling) |
| 14   | All (Testing) |
| 15   | All (Demo) |