#pragma once

/**
 * @file SafetyChecklist.hpp
 * @brief ISO 26262 Functional Safety — Hazard Analysis and Risk Assessment (HARA)
 *
 * This header embeds the complete HARA table as structured compile-time constants.
 * It covers all 6 monitored safety conditions in this system, their ASIL levels,
 * associated hazards, and safety goals per ISO 26262 Part 3.
 *
 * Standards Reference: ISO 26262:2018 Road vehicles — Functional safety
 *
 * ASIL Determination Methodology:
 *   ASIL = f(Severity, Exposure, Controllability)
 *   Severity (S0-S3), Exposure (E0-E4), Controllability (C0-C3)
 *
 * @author Team C-10
 * @version 1.0
 */

#include <string>
#include <array>

namespace VehicleSystem {
namespace Safety {

// ===========================================================================
// HARA Entry Structure
// ===========================================================================

/**
 * @brief Represents one row of the Hazard Analysis and Risk Assessment table.
 */
struct HARAEntry {
    const char* dtcCode;        ///< OBD-II DTC code (e.g., "P0217")
    const char* hazardousEvent; ///< Description of hazardous event
    const char* severity;       ///< Severity class (S0-S3)
    const char* exposure;       ///< Exposure class (E0-E4)
    const char* controllability;///< Controllability class (C0-C3)
    const char* asilLevel;      ///< Resulting ASIL level (QM, A, B, C, D)
    const char* safetyGoal;     ///< Top-level safety goal (SG-xx)
    const char* safetyGoalText; ///< Full safety goal description
    const char* trigger;        ///< Alert trigger keyword in the system
};

// ===========================================================================
// HARA Table — 6 Safety Conditions
// ===========================================================================

/**
 * @brief Complete HARA table for all monitored vehicle conditions.
 *
 * This is the authoritative reference for all ISO 26262 safety classifications
 * within this Vehicle Monitoring System.
 */
constexpr std::array<HARAEntry, 6> HARA_TABLE = {{

    // -----------------------------------------------------------------------
    // 1. Engine Overheat
    // -----------------------------------------------------------------------
    {
        /* dtcCode          */ "P0217",
        /* hazardousEvent   */ "Engine thermal runaway: undetected overheating leads to engine "
                               "fire or mechanical seizure at any vehicle speed.",
        /* severity         */ "S3",   // Life-threatening injuries
        /* exposure         */ "E3",   // Occasional - occurs on long drives or traffic jams
        /* controllability  */ "C2",   // Normally controllable but not always
        /* asilLevel        */ "ASIL C",
        /* safetyGoal       */ "SG-01",
        /* safetyGoalText   */ "The system shall detect engine coolant temperature exceeding "
                               "the critical threshold and alert the driver within 500ms to "
                               "prevent thermal runaway and engine damage.",
        /* trigger          */ "ENGINE OVERHEAT"
    },

    // -----------------------------------------------------------------------
    // 2. Low Battery Voltage
    // -----------------------------------------------------------------------
    {
        /* dtcCode          */ "P0562",
        /* hazardousEvent   */ "Loss of vehicle electrical power causing failure of safety-"
                               "critical systems (ABS, power steering, lights) while driving.",
        /* severity         */ "S2",   // Severe injuries possible
        /* exposure         */ "E3",   // Occasional
        /* controllability  */ "C2",   // Normally controllable
        /* asilLevel        */ "ASIL B",
        /* safetyGoal       */ "SG-02",
        /* safetyGoalText   */ "The system shall continuously monitor battery voltage and warn "
                               "the driver before voltage drops below the safe operating "
                               "threshold to prevent sudden loss of electrical power.",
        /* trigger          */ "LOW BATTERY"
    },

    // -----------------------------------------------------------------------
    // 3. Overspeed
    // -----------------------------------------------------------------------
    {
        /* dtcCode          */ "P0219",
        /* hazardousEvent   */ "Loss of vehicle control at excessive speed, increasing "
                               "collision probability and severity.",
        /* severity         */ "S2",   // Severe injuries possible
        /* exposure         */ "E4",   // High — speed commonly exceeded
        /* controllability  */ "C3",   // Difficult to control
        /* asilLevel        */ "ASIL B",
        /* safetyGoal       */ "SG-03",
        /* safetyGoalText   */ "The system shall alert the driver immediately when vehicle "
                               "speed exceeds the profile-configured speed limit to encourage "
                               "safe speed reduction.",
        /* trigger          */ "OVERSPEED"
    },

    // -----------------------------------------------------------------------
    // 4. Low Tire Pressure
    // -----------------------------------------------------------------------
    {
        /* dtcCode          */ "C0077",
        /* hazardousEvent   */ "Tire blowout or loss of vehicle stability resulting in rollover "
                               "or uncontrolled lane departure.",
        /* severity         */ "S2",   // Severe injuries possible
        /* exposure         */ "E3",   // Occasional
        /* controllability  */ "C2",   // Normally controllable
        /* asilLevel        */ "ASIL B",
        /* safetyGoal       */ "SG-04",
        /* safetyGoalText   */ "The system shall detect tire pressure below the minimum safe "
                               "threshold (25 PSI) in real time and warn the driver to "
                               "reduce speed and service the vehicle.",
        /* trigger          */ "LOW TIRE PRESSURE"
    },

    // -----------------------------------------------------------------------
    // 5. Door Open While Moving
    // -----------------------------------------------------------------------
    {
        /* dtcCode          */ "B1026",
        /* hazardousEvent   */ "Occupant ejection or injury due to door opening while vehicle "
                               "is in motion above 10 km/h.",
        /* severity         */ "S3",   // Life-threatening injuries
        /* exposure         */ "E2",   // Low probability
        /* controllability  */ "C1",   // Simply controllable
        /* asilLevel        */ "ASIL C",
        /* safetyGoal       */ "SG-05",
        /* safetyGoalText   */ "The system shall immediately raise a CRITICAL alert if any "
                               "door is detected as open while vehicle speed exceeds 10 km/h, "
                               "to prompt immediate safe stop.",
        /* trigger          */ "DOOR OPEN WARNING"
    },

    // -----------------------------------------------------------------------
    // 6. Seatbelt Not Fastened While Moving
    // -----------------------------------------------------------------------
    {
        /* dtcCode          */ "B0075",
        /* hazardousEvent   */ "Severe occupant injury or fatality in a collision due to "
                               "unrestrained occupant.",
        /* severity         */ "S3",   // Life-threatening injuries
        /* exposure         */ "E4",   // High — commonly observed
        /* controllability  */ "C3",   // Difficult to control post-collision
        /* asilLevel        */ "ASIL B",
        /* safetyGoal       */ "SG-06",
        /* safetyGoalText   */ "The system shall continuously alert the driver when the "
                               "seatbelt is unfastened while vehicle speed exceeds 10 km/h "
                               "until the seatbelt is fastened.",
        /* trigger          */ "SEATBELT WARNING"
    }

}};

// ===========================================================================
// Functional Safety Compliance Notes
// ===========================================================================

/**
 * @brief ISO 26262 compliance status for this system.
 *
 * This system implements a monitoring-only (observer) architecture:
 * - All safety alerts are advisory — the system DETECTS and NOTIFIES.
 * - Actuation (e.g., applying brakes) is NOT performed by this system.
 * - Thread safety is ensured via std::mutex on all shared data.
 * - Watchdog monitors all 4 threads with configurable timeout.
 * - All alerts are logged with timestamps to persistent storage.
 * - DTC freeze-frame data is captured at fault occurrence time.
 *
 * ASIL decomposition is not applicable (no redundant channels).
 * Safety goals are achieved via software-only monitoring path.
 */
struct ComplianceNotes {
    static constexpr const char* STANDARD       = "ISO 26262:2018";
    static constexpr const char* SYSTEM_NAME    = "Smart Cabin & Vehicle Health Monitoring System";
    static constexpr const char* VERSION        = "1.0.0";
    static constexpr const char* ARCHITECTURE   = "AUTOSAR-Inspired, C++17, Multi-threaded";
    static constexpr const char* SAFETY_CLASS   = "Observer / Monitor — No direct actuation";
    static constexpr const char* MAX_ASIL       = "ASIL C (Engine Overheat, Door Open Warning)";
    static constexpr const char* THREAD_SAFETY  = "std::mutex on all shared state";
    static constexpr const char* WATCHDOG       = "Thread health monitored with configurable timeout";
    static constexpr const char* LOGGING        = "All alerts logged with timestamp and freeze-frame";
};

} // namespace Safety
} // namespace VehicleSystem
