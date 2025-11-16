/*
 * Flick Detection for Ploopy Madromys Trackball
 * 
 * Implementation of flick gesture detection algorithm
 */

#include "flick_detection.h"
#include "action.h"
#include "timer.h"

// ============================================================================
// Private State Variables
// ============================================================================

// Global flick detector instance
static flick_detector_t g_flick_detector = {
    .state = FLICK_STATE_IDLE,
    .direction = FLICK_DIR_NONE,
    .gesture_start_time = 0,
    .last_update_time = 0,
    .cooldown_start_time = 0,
    .accumulated_x = 0,
    .accumulated_y = 0,
    .last_velocity_x = 0,
    .movement_consistent = true,
    .direction_changes = 0
};

// Runtime enable/disable flag
static bool g_flick_enabled = true;

// Configurable thresholds (can be modified at runtime)
static int16_t g_velocity_threshold = FLICK_VELOCITY_THRESHOLD;
static int16_t g_distance_threshold = FLICK_DISTANCE_THRESHOLD;
static uint16_t g_time_window_ms = FLICK_TIME_WINDOW_MS;
static uint16_t g_cooldown_ms = FLICK_COOLDOWN_MS;
static float g_vertical_tolerance_ratio = FLICK_VERTICAL_TOLERANCE_RATIO;
static int16_t g_decay_threshold = FLICK_VELOCITY_DECAY_THRESHOLD;

// ============================================================================
// Private Helper Functions
// ============================================================================

/**
 * Send a keycode as a tap (press and release)
 * 
 * @param keycode The keycode to send
 */
static void send_keycode_tap(uint16_t keycode) {
    register_code16(keycode);
    wait_ms(10);  // Brief delay to ensure keypress registers
    unregister_code16(keycode);
}

/**
 * Determine if movement indicates a potential flick start
 * 
 * @param velocity_x Current X velocity
 * @return True if velocity exceeds threshold
 */
static bool is_flick_velocity(int16_t velocity_x) {
    // Check if absolute velocity exceeds the threshold
    int16_t abs_velocity = velocity_x < 0 ? -velocity_x : velocity_x;
    return abs_velocity >= g_velocity_threshold;
}

/**
 * Validate if the accumulated movement qualifies as a flick
 * Checks distance, direction consistency, and vertical tolerance
 * 
 * @return True if movement is a valid flick gesture
 */
static bool validate_flick_gesture(void) {
    // Check 1: Has the horizontal distance exceeded the threshold?
    int16_t abs_x = g_flick_detector.accumulated_x < 0 ? 
                    -g_flick_detector.accumulated_x : 
                    g_flick_detector.accumulated_x;
    
    if (abs_x < g_distance_threshold) {
        return false;  // Not enough horizontal movement
    }
    
    // Check 2: Is the vertical movement within acceptable tolerance?
    int16_t abs_y = g_flick_detector.accumulated_y < 0 ? 
                    -g_flick_detector.accumulated_y : 
                    g_flick_detector.accumulated_y;
    
    // Calculate the ratio of vertical to horizontal movement
    // We want primarily horizontal movement
    if (abs_x > 0) {
        float vertical_ratio = (float)abs_y / (float)abs_x;
        if (vertical_ratio > g_vertical_tolerance_ratio) {
            return false;  // Too much vertical movement, not a horizontal flick
        }
    }
    
    // Check 3: Has the movement been consistent (not too many direction changes)?
    // Allow up to 2 direction changes for natural hand wobble
    if (g_flick_detector.direction_changes > 2) {
        return false;  // Too erratic, not a deliberate flick
    }
    
    // Check 4: Verify movement consistency flag
    if (!g_flick_detector.movement_consistent) {
        return false;
    }
    
    // All checks passed - this is a valid flick gesture
    return true;
}

/**
 * Determine flick direction from accumulated movement
 * 
 * @return FLICK_DIR_LEFT or FLICK_DIR_RIGHT
 */
static flick_direction_t determine_flick_direction(void) {
    if (g_flick_detector.accumulated_x < 0) {
        return FLICK_DIR_LEFT;
    } else if (g_flick_detector.accumulated_x > 0) {
        return FLICK_DIR_RIGHT;
    }
    return FLICK_DIR_NONE;
}

/**
 * Check if velocity has decayed below threshold
 * Indicates gesture may have stopped or slowed down
 * 
 * @param velocity_x Current X velocity
 * @return True if velocity has decayed
 */
static bool has_velocity_decayed(int16_t velocity_x) {
    int16_t abs_velocity = velocity_x < 0 ? -velocity_x : velocity_x;
    
    // Velocity has decayed if it's below the decay threshold
    if (abs_velocity < g_decay_threshold) {
        return true;
    }
    
    // Also check if velocity has reversed significantly
    // This indicates the user stopped and is moving back
    if (g_flick_detector.last_velocity_x != 0) {
        // Check if signs are different (direction reversed)
        bool sign_changed = (g_flick_detector.last_velocity_x > 0) != (velocity_x > 0);
        if (sign_changed && abs_velocity > g_decay_threshold) {
            return true;  // Strong reversal indicates end of gesture
        }
    }
    
    return false;
}

/**
 * Transition the state machine to a new state
 * Handles state-specific initialization
 * 
 * @param new_state The state to transition to
 */
static void transition_state(flick_state_t new_state) {
    flick_state_t old_state = g_flick_detector.state;
    
    // Exit actions for current state
    switch (old_state) {
        case FLICK_STATE_DETECTING:
            // If leaving detection without triggering, reset accumulated values
            if (new_state != FLICK_STATE_TRIGGERED) {
                g_flick_detector.accumulated_x = 0;
                g_flick_detector.accumulated_y = 0;
                g_flick_detector.direction_changes = 0;
            }
            break;
        default:
            break;
    }
    
    // Update state
    g_flick_detector.state = new_state;
    
    // Entry actions for new state
    uint32_t current_time = timer_read32();
    
    switch (new_state) {
        case FLICK_STATE_IDLE:
            g_flick_detector.direction = FLICK_DIR_NONE;
            g_flick_detector.accumulated_x = 0;
            g_flick_detector.accumulated_y = 0;
            g_flick_detector.last_velocity_x = 0;
            g_flick_detector.movement_consistent = true;
            g_flick_detector.direction_changes = 0;
            break;
            
        case FLICK_STATE_DETECTING:
            g_flick_detector.gesture_start_time = current_time;
            g_flick_detector.accumulated_x = 0;
            g_flick_detector.accumulated_y = 0;
            g_flick_detector.direction_changes = 0;
            g_flick_detector.movement_consistent = true;
            break;
            
        case FLICK_STATE_TRIGGERED:
            // Direction should already be set before transitioning here
            break;
            
        case FLICK_STATE_COOLDOWN:
            g_flick_detector.cooldown_start_time = current_time;
            break;
    }
}

/**
 * Handle state machine update for IDLE state
 * Watches for flick initiation
 * 
 * @param velocity_x Current X velocity
 * @param current_time Current timestamp
 */
static void update_idle_state(int16_t velocity_x, uint32_t current_time) {
    // Check if the current velocity exceeds the threshold to start detection
    if (is_flick_velocity(velocity_x)) {
        // Store initial velocity for direction tracking
        g_flick_detector.last_velocity_x = velocity_x;
        
        // Transition to detecting state
        transition_state(FLICK_STATE_DETECTING);
    }
}

/**
 * Handle state machine update for DETECTING state
 * Tracks gesture progress and validates completion
 * 
 * @param velocity_x Current X velocity
 * @param velocity_y Current Y velocity
 * @param current_time Current timestamp
 */
static void update_detecting_state(int16_t velocity_x, int16_t velocity_y, uint32_t current_time) {
    // Check 1: Has the time window expired?
    uint32_t elapsed_time = current_time - g_flick_detector.gesture_start_time;
    if (elapsed_time > g_time_window_ms) {
        // Timeout - check if we have a valid gesture before giving up
        if (validate_flick_gesture()) {
            // We have enough movement, trigger the flick
            g_flick_detector.direction = determine_flick_direction();
            transition_state(FLICK_STATE_TRIGGERED);
        } else {
            // Not a valid flick, return to idle
            transition_state(FLICK_STATE_IDLE);
        }
        return;
    }
    
    // Check 2: Has velocity decayed (gesture ended)?
    if (has_velocity_decayed(velocity_x)) {
        // Gesture has ended, check if it's valid
        if (validate_flick_gesture()) {
            // Valid flick detected!
            g_flick_detector.direction = determine_flick_direction();
            transition_state(FLICK_STATE_TRIGGERED);
        } else {
            // Not enough movement or invalid gesture
            transition_state(FLICK_STATE_IDLE);
        }
        return;
    }
    
    // Check 3: Track direction changes
    if (g_flick_detector.last_velocity_x != 0 && velocity_x != 0) {
        // Check if direction reversed
        bool previous_positive = g_flick_detector.last_velocity_x > 0;
        bool current_positive = velocity_x > 0;
        
        if (previous_positive != current_positive) {
            g_flick_detector.direction_changes++;
            
            // If too many direction changes, mark as inconsistent
            if (g_flick_detector.direction_changes > 2) {
                g_flick_detector.movement_consistent = false;
            }
        }
    }
    
    // Accumulate movement
    g_flick_detector.accumulated_x += velocity_x;
    g_flick_detector.accumulated_y += velocity_y;
    
    // Update last velocity for next iteration
    g_flick_detector.last_velocity_x = velocity_x;
    g_flick_detector.last_update_time = current_time;
}

/**
 * Handle state machine update for TRIGGERED state
 * Manages the keycode sending
 * 
 * @param current_time Current timestamp
 */
static void update_triggered_state(uint32_t current_time) {
    // Send the appropriate keycode based on direction
    uint16_t keycode = KC_NO;
    
    switch (g_flick_detector.direction) {
        case FLICK_DIR_LEFT:
            keycode = KC_WWW_BACK;
            break;
        case FLICK_DIR_RIGHT:
            keycode = KC_WWW_FORWARD;
            break;
        default:
            // Invalid direction, just go to cooldown
            transition_state(FLICK_STATE_COOLDOWN);
            return;
    }
    
    // Send the keycode
    send_keycode_tap(keycode);
    
    // Transition to cooldown state
    transition_state(FLICK_STATE_COOLDOWN);
}

/**
 * Handle state machine update for COOLDOWN state
 * Waits for cooldown period to expire
 * 
 * @param current_time Current timestamp
 */
static void update_cooldown_state(uint32_t current_time) {
    // Check if cooldown period has elapsed
    uint32_t elapsed_time = current_time - g_flick_detector.cooldown_start_time;
    
    if (elapsed_time >= g_cooldown_ms) {
        // Cooldown complete, return to idle state
        transition_state(FLICK_STATE_IDLE);
    }
    
    // Otherwise, remain in cooldown state
}

// ============================================================================
// Public API Implementation
// ============================================================================

void flick_detection_init(void) {
    // Initialize the detector to a clean state
    flick_detection_reset();
}

void flick_detection_reset(void) {
    g_flick_detector.state = FLICK_STATE_IDLE;
    g_flick_detector.direction = FLICK_DIR_NONE;
    g_flick_detector.gesture_start_time = 0;
    g_flick_detector.last_update_time = 0;
    g_flick_detector.cooldown_start_time = 0;
    g_flick_detector.accumulated_x = 0;
    g_flick_detector.accumulated_y = 0;
    g_flick_detector.last_velocity_x = 0;
    g_flick_detector.movement_consistent = true;
    g_flick_detector.direction_changes = 0;
}

report_mouse_t flick_detection_process(report_mouse_t report) {
    // Early exit if disabled
    if (!g_flick_enabled) {
        return report;
    }
    
    // Extract movement data
    int16_t velocity_x = report.x;
    int16_t velocity_y = report.y;
    uint32_t current_time = timer_read32();
    
    // State machine dispatcher
    // Call appropriate update function based on current state
    switch (g_flick_detector.state) {
        case FLICK_STATE_IDLE:
            update_idle_state(velocity_x, current_time);
            break;
            
        case FLICK_STATE_DETECTING:
            update_detecting_state(velocity_x, velocity_y, current_time);
            break;
            
        case FLICK_STATE_TRIGGERED:
            update_triggered_state(current_time);
            break;
            
        case FLICK_STATE_COOLDOWN:
            update_cooldown_state(current_time);
            break;
    }
    
    // Movement consumption logic:
    // - During DETECTING: consume movement to prevent cursor jump during gesture
    // - During TRIGGERED: consume movement (the flick action is already sent)
    // - During COOLDOWN: consume movement to prevent residual motion
    // - During IDLE: pass through normally
    
    if (g_flick_detector.state == FLICK_STATE_DETECTING ||
        g_flick_detector.state == FLICK_STATE_TRIGGERED ||
        g_flick_detector.state == FLICK_STATE_COOLDOWN) {
        // Consume the movement by zeroing it out
        report.x = 0;
        report.y = 0;
    }
    
    return report;
}

flick_state_t flick_detection_get_state(void) {
    return g_flick_detector.state;
}

bool flick_detection_is_active(void) {
    return g_flick_detector.state != FLICK_STATE_IDLE;
}

void flick_detection_trigger(flick_direction_t direction) {
    if (direction != FLICK_DIR_LEFT && direction != FLICK_DIR_RIGHT) {
        return;
    }
    
    // Set the direction
    g_flick_detector.direction = direction;
    
    // Transition to triggered state to send the keycode
    transition_state(FLICK_STATE_TRIGGERED);
}

// ============================================================================
// Configuration Helpers Implementation
// ============================================================================

void flick_detection_set_enabled(bool enabled) {
    g_flick_enabled = enabled;
    if (!enabled) {
        flick_detection_reset();
    }
}

bool flick_detection_is_enabled(void) {
    return g_flick_enabled;
}

void flick_detection_set_velocity_threshold(int16_t threshold) {
    if (threshold > 0) {
        g_velocity_threshold = threshold;
    }
}

int16_t flick_detection_get_velocity_threshold(void) {
    return g_velocity_threshold;
}

void flick_detection_set_distance_threshold(int16_t threshold) {
    if (threshold > 0) {
        g_distance_threshold = threshold;
    }
}

int16_t flick_detection_get_distance_threshold(void) {
    return g_distance_threshold;
}

void flick_detection_set_time_window(uint16_t time_ms) {
    if (time_ms > 0) {
        g_time_window_ms = time_ms;
    }
}

void flick_detection_set_cooldown(uint16_t cooldown_ms) {
    if (cooldown_ms > 0) {
        g_cooldown_ms = cooldown_ms;
    }
}

void flick_detection_set_vertical_tolerance(float ratio) {
    if (ratio >= 0.0f && ratio <= 1.0f) {
        g_vertical_tolerance_ratio = ratio;
    }
}

void flick_detection_set_decay_threshold(int16_t threshold) {
    if (threshold > 0) {
        g_decay_threshold = threshold;
    }
}

// ============================================================================
// Debug Functions Implementation
// ============================================================================

#ifdef FLICK_DETECTION_DEBUG
#include "print.h"

void flick_detection_print_debug(void) {
    // Print current state
    const char* state_names[] = {
        "IDLE",
        "DETECTING",
        "TRIGGERED",
        "COOLDOWN"
    };
    
    const char* dir_names[] = {
        "NONE",
        "LEFT",
        "RIGHT"
    };
    
    uprintf("Flick Detector Debug:\n");
    uprintf("  State: %s\n", state_names[g_flick_detector.state]);
    uprintf("  Direction: %s\n", dir_names[g_flick_detector.direction]);
    uprintf("  Accumulated X: %d\n", g_flick_detector.accumulated_x);
    uprintf("  Accumulated Y: %d\n", g_flick_detector.accumulated_y);
    uprintf("  Last Velocity X: %d\n", g_flick_detector.last_velocity_x);
    uprintf("  Direction Changes: %d\n", g_flick_detector.direction_changes);
    uprintf("  Movement Consistent: %s\n", g_flick_detector.movement_consistent ? "YES" : "NO");
    uprintf("  Enabled: %s\n", g_flick_enabled ? "YES" : "NO");
    uprintf("  Velocity Threshold: %d\n", g_velocity_threshold);
    uprintf("  Distance Threshold: %d\n", g_distance_threshold);
}
#endif
