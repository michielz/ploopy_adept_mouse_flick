/*
 * VIA Integration for Flick Detection
 * 
 * This module provides VIA integration for runtime configuration
 * of flick detection parameters through the VIA application.
 */

#pragma once

#include "quantum.h"

// ============================================================================
// VIA Custom Value IDs
// ============================================================================

// Channel ID for flick detection custom values
// VIA uses channels 0-7, we'll use channel 1 for flick detection
#define FLICK_VIA_CHANNEL 1

// Custom value IDs for each parameter
enum flick_via_value_ids {
    FLICK_VIA_ENABLED = 1,              // Enable/disable flick detection
    FLICK_VIA_VELOCITY_THRESHOLD,       // Velocity threshold
    FLICK_VIA_DISTANCE_THRESHOLD,       // Distance threshold
    FLICK_VIA_TIME_WINDOW,              // Time window in ms
    FLICK_VIA_COOLDOWN,                 // Cooldown period in ms
    FLICK_VIA_VERTICAL_TOLERANCE,       // Vertical tolerance (0-100 representing 0.0-1.0)
    FLICK_VIA_DECAY_THRESHOLD,          // Velocity decay threshold
    FLICK_VIA_RESET_TO_DEFAULTS         // Write-only: reset all to defaults
};

// ============================================================================
// VIA Integration Functions
// ============================================================================

/**
 * Initialize VIA integration for flick detection
 * Should be called after flick_detection_init()
 */
void flick_detection_via_init(void);

/**
 * Handle VIA custom value get requests
 * Called by via_custom_value_command
 * 
 * @param value_id The ID of the value being requested
 * @param value Pointer to store the retrieved value
 */
void flick_detection_via_get_value(uint8_t value_id, uint8_t *value);

/**
 * Handle VIA custom value set requests
 * Called by via_custom_value_command
 * 
 * @param value_id The ID of the value being set
 * @param value Pointer to the new value
 */
void flick_detection_via_set_value(uint8_t value_id, uint8_t *value);

/**
 * Save current flick detection settings to EEPROM
 * This allows settings to persist across power cycles
 */
void flick_detection_via_save(void);

/**
 * Load flick detection settings from EEPROM
 * Called during initialization to restore saved settings
 */
void flick_detection_via_load(void);

/**
 * Reset all flick detection settings to defaults
 */
void flick_detection_via_reset_defaults(void);

// ============================================================================
// EEPROM Configuration
// ============================================================================

// EEPROM memory layout for flick detection settings
// We need to reserve space in EEPROM for persistent storage
typedef struct {
    uint8_t enabled;                    // 0 = disabled, 1 = enabled
    uint16_t velocity_threshold;        // Velocity threshold value
    uint16_t distance_threshold;        // Distance threshold value
    uint16_t time_window_ms;            // Time window in milliseconds
    uint16_t cooldown_ms;               // Cooldown period in milliseconds
    uint8_t vertical_tolerance;         // Vertical tolerance (0-100)
    uint16_t decay_threshold;           // Velocity decay threshold
    uint8_t reserved[8];                // Reserved for future use
} flick_detection_config_t;

// Total size: 22 bytes (with 8 bytes reserved for future features)

#endif // FLICK_DETECTION_VIA_H
