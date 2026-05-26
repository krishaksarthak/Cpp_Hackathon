# Test Execution Scenarios

## Scenario 1: Quick Smoke Test (5 minutes)

**Purpose**: Verify basic functionality after each build

**Steps**:
1. Build project: `./build.sh`
2. Run application: `./run.sh`
3. Let run for 30 seconds
4. Press Ctrl+C to exit

**Expected Results**:
- ✅ Application starts without errors
- ✅ All 6 sensors display values
- ✅ Dashboard refreshes periodically
- ✅ Clean shutdown

---

## Scenario 2: Alert Generation Test (10 minutes)

**Purpose**: Verify all 6 alert types can be triggered

**Steps**:
1. Start application
2. Wait for random sensor values to trigger alerts:
   - Engine temp > 110°C
   - Battery < 10V
   - Tire pressure < 25 PSI
   - Speed > 120 km/h
   - Door open while moving
   - Seatbelt unlocked while moving

**Expected Results**:
- ✅ At least 3 different alert types appear within 5 minutes
- ✅ Alerts shown on dashboard
- ✅ Alerts logged to file
- ✅ DTC codes generated

---

## Scenario 3: Profile Switching Test (5 minutes)

**Purpose**: Verify driver profile switching works

**Steps**:
1. Start application with eco_mode
2. Note speed threshold (100 km/h)
3. Stop application
4. Edit `data/config.json`: `"active_profile": "sport_mode"`
5. Restart application
6. Note new speed threshold (180 km/h)

**Expected Results**:
- ✅ Eco mode has lower thresholds
- ✅ Sport mode has higher thresholds
- ✅ Alert behavior changes accordingly

---

## Scenario 4: Threading & Watchdog Test (15 minutes)

**Purpose**: Verify thread safety and watchdog recovery

**Steps**:
1. Start application
2. Open another terminal: `ps aux | grep Vehicle`
3. Note thread count (should be 5: main + 4 workers)
4. Identify sensor thread PID
5. Kill sensor thread: `kill -9 <PID>`
6. Wait 5 seconds
7. Check if watchdog restarted it

**Expected Results**:
- ✅ 5 threads initially
- ✅ Watchdog detects thread failure
- ✅ Thread restarted automatically
- ✅ System continues operating

---

## Scenario 5: Stress Test (30 minutes)

**Purpose**: Verify stability under extended operation

**Steps**:
1. Start application
2. Let run for 30 minutes
3. Monitor CPU/memory usage
4. Check log file size
5. Graceful shutdown

**Expected Results**:
- ✅ No crashes
- ✅ Memory usage stable (no leaks)
- ✅ Log file grows linearly
- ✅ All threads join cleanly on exit

---

## Scenario 6: Memory Leak Test (10 minutes)

**Purpose**: Verify no memory leaks

**Steps**:
1. Build with debug symbols: `cmake -DCMAKE_BUILD_TYPE=Debug ..`
2. Run with valgrind: `./scripts/check_memory_leaks.sh`
3. Let run for 2 minutes
4. Stop and check report

**Expected Results**:
- ✅ Valgrind report shows 0 bytes leaked
- ✅ All heap blocks freed

---

## Scenario 7: Exception Handling Test (5 minutes)

**Purpose**: Verify graceful error handling

**Steps**:
1. Remove write permissions: `chmod 000 logs/`
2. Start application
3. Trigger an alert
4. Verify exception caught

**Expected Results**:
- ✅ Application doesn't crash
- ✅ Error logged to console
- ✅ System continues with degraded logging

---

## Scenario 8: Configuration Error Test (5 minutes)

**Purpose**: Verify handling of invalid config

**Steps**:
1. Edit `data/config.json` - remove a closing bracket
2. Start application
3. Verify fallback to defaults

**Expected Results**:
- ✅ JSON parse error caught
- ✅ Default values used
- ✅ Warning message displayed
- ✅ Application runs normally

---

## Scenario 9: Log Search Test (5 minutes)

**Purpose**: Verify lambda-based log filtering

**Steps**:
1. Run application for 5 minutes
2. Generate multiple alerts
3. Stop application
4. Search logs:
```bash
   cat logs/vehicle_events.log | grep "CRITICAL"
   cat logs/vehicle_events.log | grep "DTC"
```

**Expected Results**:
- ✅ Can filter by severity
- ✅ Can filter by keyword
- ✅ Timestamps present
- ✅ All critical events captured

---

## Scenario 10: Full Demo Run (15 minutes)

**Purpose**: Complete demonstration for hackathon judges

**Steps**:
1. Start with clean logs: `rm logs/*.log`
2. Run application
3. Show dashboard updates
4. Wait for alerts to trigger
5. Show DTC generation
6. Switch profiles (edit config, reload)
7. Show log file contents
8. Demonstrate thread safety
9. Graceful shutdown
10. Show memory leak check results

**Expected Results**:
- ✅ All features demonstrated
- ✅ No errors or crashes
- ✅ Professional output formatting
- ✅ Clean metrics