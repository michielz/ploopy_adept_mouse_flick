/*
 * Flick Detection for Ploopy Madromys Trackball
 * 
 * This module provides horizontal flick gesture detection for the Ploopy Madromys.
 * A flick to the left triggers KC_WWW_BACK, a flick to the right triggers KC_WWW_FORWARD.
 */

#pragma once

#include "quantum.h"
#include "pointing_device.h"

// ============================================================================
// Configuration Defines
// ============================================================================

/**
 * Minimum velocity threshold to register as a flick (counts per sample)
 * Higher values require faster movement to trigger a flick
 */
#ifndef FLICK_VELOCITY_THRESHOLD
#    define FLICK_VELOCITY_THRESHOLD 30
#endif

/**
 * Maximum time window for flick detection (milliseconds)
 * Flick must complete within this time window
 */
#ifndef FLICK_TIME_WINDOW_MS
#    define FLICK_TIME_WINDOW_MS 150
#endif

/**
 * Minimum distance threshold for flick detection (counts)
 * Total horizontal distance must exceed this value
 */
#ifndef FLICK_DISTANCE_THRESHOLD
#    define FLICK_DISTANCE_THRESHOLD 80
#endif

/**
 * Cooldown period after a flick is detected (milliseconds)
 * Prevents accidental repeated flicks
 */
#ifndef FLICK_COOLDOWN_MS
#    define FLICK_COOLDOWN_MS 500
#endif

/**
 * Maximum vertical tolerance ratio for flick detection
 * Ratio of vertical to horizontal movement (0.5 = 50% vertical allowed)
 */
#ifndef FLICK_VERTICAL_TOLERANCE_RATIO
#    define FLICK_VERTICAL_TOLERANCE_RATIO 0.5
#endif

/**
 * Velocity decay threshold (counts per sample)
 * If velocity drops below this, the flick gesture is cancelled
 */
#ifndef FLICK_VELOCITY_DECAY_THRESHOLD
#    define FLICK_VELOCITY_DECAY_THRESHOLD 5
#endif

// ============================================================================
// State Machine Definitions
// ============================================================================

/**
 * Flick detection state machine states
 */
typedef enum {
    FLICK_STATE_IDLE = 0,      // No flick in progress, waiting for trigger
    FLICK_STATE_DETECTING,     // Flick motion detected, tracking velocity
    FLICK_STATE_TRIGGERED,     // Flick confirmed, key being sent
    FLICK_STATE_COOLDOWN       // Cooldown period after flick
} flick_state_t;

/**
 * Direction of detected flick
 */
typedef enum {
    FLICK_DIR_NONE = 0,
    FLICK_DIR_LEFT,
    FLICK_DIR_RIGHT
} flick_direction_t;

/**
 * Flick detection data structure
 * Contains all state information for the flick detection algorithm
 */
typedef struct {
    // State machine
    flick_state_t state;              // Current state of the detector
    flick_direction_t direction;      // Direction of current/last flick
    
    // Timing information
    uint32_t gesture_start_time;      // Timestamp when potential flick started
    uint32_t last_update_time;        // Timestamp of last movement update
    uint32_t cooldown_start_time;     // Timestamp when cooldown period started
    
    // Movement tracking
    int16_t accumulated_x;            // Total horizontal movement
    int16_t accumulated_y;            // Total vertical movement
    int16_t last_velocity_x;          // Previous frame's X velocity
    
    // Gesture validation
    bool movement_consistent;         // True if movement direction is consistent
    uint8_t direction_changes;        // Count of direction reversals
    
} flick_detector_t;

// ============================================================================
// Public API Functions
// ============================================================================

/**
 * Initialize the flick detection system
 * Should be called once during keyboard initialization
 */
void flick_detection_init(void);

/**
 * Reset the flick detector to idle state
 * Useful for cancelling detection or after an error
 */
void flick_detection_reset(void);

/**
 * Process mouse movement and detect flick gestures
 * This is the main processing function that should be called from pointing_device_task_user
 * 
 * @param report Pointer to the mouse report containing movement data
 * @return Modified mouse report (may have movement zeroed if consumed by flick)
 */
report_mouse_t flick_detection_process(report_mouse_t report);

/**
 * Get the current state of the flick detector
 * Useful for debugging or custom UI indicators
 * 
 * @return Current flick detector state
 */
flick_state_t flick_detection_get_state(void);

/**
 * Check if flick detection is currently active
 * 
 * @return True if detecting or in cooldown, false if idle
 */
bool flick_detection_is_active(void);

/**
 * Manually trigger a flick in a specific direction
 * Useful for testing or custom gestures
 * 
 * @param direction Direction to trigger (FLICK_DIR_LEFT or FLICK_DIR_RIGHT)
 */
void flick_detection_trigger(flick_direction_t direction);

// ============================================================================
// Configuration Helpers
// ============================================================================

/**
 * Enable or disable flick detection at runtime
 * 
 * @param enabled True to enable, false to disable
 */
void flick_detection_set_enabled(bool enabled);

/**
 * Check if flick detection is enabled
 * 
 * @return True if enabled, false if disabled
 */
bool flick_detection_is_enabled(void);

/**
 * Set custom velocity threshold
 * 
 * @param threshold New velocity threshold value
 */
void flick_detection_set_velocity_threshold(int16_t threshold);

/**
 * Get current velocity threshold
 * 
 * @return Current velocity threshold value
 */
int16_t flick_detection_get_velocity_threshold(void);

/**
 * Set custom distance threshold
 * 
 * @param threshold New distance threshold value
 */
void flick_detection_set_distance_threshold(int16_t threshold);

/**
 * Get current distance threshold
 * 
 * @return Current distance threshold value
 */
int16_t flick_detection_get_distance_threshold(void);

/**
 * Set custom time window for flick detection
 * 
 * @param time_ms New time window in milliseconds
 */
void flick_detection_set_time_window(uint16_t time_ms);

/**
 * Set custom cooldown period
 * 
 * @param cooldown_ms New cooldown period in milliseconds
 */
void flick_detection_set_cooldown(uint16_t cooldown_ms);

/**
 * Set custom vertical tolerance ratio
 * 
 * @param ratio New vertical tolerance ratio (0.0 to 1.0)
 */
void flick_detection_set_vertical_tolerance(float ratio);

/**
 * Set custom velocity decay threshold
 * 
 * @param threshold New decay threshold value
 */
void flick_detection_set_decay_threshold(int16_t threshold);

// ============================================================================
// Debug Functions (optional, for development)
// ============================================================================

#ifdef FLICK_DETECTION_DEBUG
/**
 * Get debug information about the current flick detector state
 * Only available when FLICK_DETECTION_DEBUG is defined
 */
void flick_detection_print_debug(void);
#endif
