# Demo Script (15 minutes)

## Opening (2 minutes) - Member 5
"Good [morning/afternoon], we are Team [Name]. We've built a production-grade Smart Cabin & Vehicle Health Monitoring System that demonstrates all mandatory C++ concepts plus 5 bonus features..."

**Show slide**: Team photo + roles

---

## Architecture Overview (3 minutes) - Member 1
**Show**: Class diagram

"Our system follows Adaptive AUTOSAR principles with 5 main modules..."

1. Sensor Framework (inheritance, polymorphism)
2. Alert Management (enum classes, operator overloading)
3. Dashboard & Statistics (STL containers, lambdas)
4. Event Logger (file I/O, RAII)
5. Thread Manager (concurrency, mutexes)

---

## Live Demo (7 minutes)

### Scenario 1: Normal Operation (1 min) - Member 3
```bash
./run.sh
```
Show: All 6 sensors updating, no alerts

---

### Scenario 2: Profile Switching (1 min) - Member 5
Edit config.json live:
```json
"active_profile": "sport_mode"
```
Show: Thresholds change, restart not needed

---

### Scenario 3: Alert Generation (2 min) - Member 2
Wait for random high temperature spike OR manually inject

Show:
1. Alert appears on dashboard
2. DTC generated (P0217)
3. Event logged to file
4. Statistics updated

---

### Scenario 4: Threading Demo (2 min) - Member 4
```bash
ps aux | grep Vehicle  # Show 4+ threads
```

Show thread safety:
1. Kill sensor thread process
2. Watchdog detects & restarts
3. System continues running

---

### Scenario 5: Log Search (1 min) - Member 3
```bash
cat logs/vehicle_events.log | grep CRITICAL
```
Show lambda-based filtering working

---

## Technical Deep Dive (2 minutes) - All

**Q: Why unique_ptr vs shared_ptr?**  
Member 1: "Sensors have exclusive ownership..."

**Q: How did you prevent race conditions?**  
Member 4: "Every shared data structure protected by mutex..."

**Q: STL container choices?**  
Member 3: "vector for sensors (index access), map for DTC lookup..."

---

## Closing (1 minute) - Member 5

**Accomplishments**:
- ✅ 100% mandatory requirements
- ✅ 5 bonus features (JSON, profiles, stats, DTC, watchdog)
- ✅ Production-grade code quality
- ✅ Full documentation

"Thank you! Questions?"