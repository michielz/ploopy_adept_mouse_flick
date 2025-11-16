/*
 * VIA Integration for Flick Detection
 * 
 * Implementation of VIA custom values for runtime configuration
 */

#include "flick_detection_via.h"
#include "flick_detection.h"
#include "eeconfig.h"
#include "eeprom.h"

// ============================================================================
// EEPROM Address Management
// ============================================================================

// VIA uses addresses starting at a specific offset
// We'll use the user EEPROM space which starts after VIA's reserved space
#define FLICK_EEPROM_ADDR (EECONFIG_SIZE + 64)  // Start after VIA's space

// Magic number to verify EEPROM data is valid
#define FLICK_EEPROM_MAGIC 0xF1C4

// ============================================================================
// Default Values
// ============================================================================

static const flick_detection_config_t default_config = {
    .enabled = 1,
    .velocity_threshold = FLICK_VELOCITY_THRESHOLD,
    .distance_threshold = FLICK_DISTANCE_THRESHOLD,
    .time_window_ms = FLICK_TIME_WINDOW_MS,
    .cooldown_ms = FLICK_COOLDOWN_MS,
    .vertical_tolerance = (uint8_t)(FLICK_VERTICAL_TOLERANCE_RATIO * 100),
    .decay_threshold = FLICK_VELOCITY_DECAY_THRESHOLD,
    .reserved = {0}
};

// Current configuration in RAM
static flick_detection_config_t current_config;

// ============================================================================
// EEPROM Helper Functions
// ============================================================================

/**
 * Check if EEPROM contains valid flick detection configuration
 */
static bool is_eeprom_valid(void) {
    uint16_t magic;
    eeprom_read_block(&magic, (void*)(FLICK_EEPROM_ADDR - 2), sizeof(magic));
    return magic == FLICK_EEPROM_MAGIC;
}

/**
 * Write magic number to EEPROM
 */
static void write_eeprom_magic(void) {
    uint16_t magic = FLICK_EEPROM_MAGIC;
    eeprom_update_block(&magic, (void*)(FLICK_EEPROM_ADDR - 2), sizeof(magic));
}

/**
 * Apply configuration to the flick detection system
 */
static void apply_config(void) {
    flick_detection_set_enabled(current_config.enabled);
    flick_detection_set_velocity_threshold(current_config.velocity_threshold);
    flick_detection_set_distance_threshold(current_config.distance_threshold);
    
    // Note: time_window_ms, cooldown_ms, vertical_tolerance, and decay_threshold
    // would require additional setter functions in flick_detection.h
    // For now, we store them but can't apply them dynamically
    // You could extend the API to add these setters
}

// ============================================================================
// Public API Implementation
// ============================================================================

void flick_detection_via_init(void) {
    // Try to load configuration from EEPROM
    if (is_eeprom_valid()) {
        flick_detection_via_load();
    } else {
        // First time init - use defaults and save them
        current_config = default_config;
        flick_detection_via_save();
    }
    
    // Apply the loaded configuration
    apply_config();
}

void flick_detection_via_load(void) {
    eeprom_read_block(&current_config, (void*)FLICK_EEPROM_ADDR, sizeof(current_config));
}

void flick_detection_via_save(void) {
    write_eeprom_magic();
    eeprom_update_block(&current_config, (void*)FLICK_EEPROM_ADDR, sizeof(current_config));
}

void flick_detection_via_reset_defaults(void) {
    current_config = default_config;
    apply_config();
    flick_detection_via_save();
}

void flick_detection_via_get_value(uint8_t value_id, uint8_t *value) {
    switch (value_id) {
        case FLICK_VIA_ENABLED:
            *value = current_config.enabled;
            break;
            
        case FLICK_VIA_VELOCITY_THRESHOLD:
            // Return high byte
            *(uint16_t*)value = current_config.velocity_threshold;
            break;
            
        case FLICK_VIA_DISTANCE_THRESHOLD:
            *(uint16_t*)value = current_config.distance_threshold;
            break;
            
        case FLICK_VIA_TIME_WINDOW:
            *(uint16_t*)value = current_config.time_window_ms;
            break;
            
        case FLICK_VIA_COOLDOWN:
            *(uint16_t*)value = current_config.cooldown_ms;
            break;
            
        case FLICK_VIA_VERTICAL_TOLERANCE:
            *value = current_config.vertical_tolerance;
            break;
            
        case FLICK_VIA_DECAY_THRESHOLD:
            *(uint16_t*)value = current_config.decay_threshold;
            break;
            
        default:
            *value = 0;
            break;
    }
}

void flick_detection_via_set_value(uint8_t value_id, uint8_t *value) {
    bool needs_save = false;
    
    switch (value_id) {
        case FLICK_VIA_ENABLED:
            current_config.enabled = *value ? 1 : 0;
            flick_detection_set_enabled(current_config.enabled);
            needs_save = true;
            break;
            
        case FLICK_VIA_VELOCITY_THRESHOLD:
            current_config.velocity_threshold = *(uint16_t*)value;
            flick_detection_set_velocity_threshold(current_config.velocity_threshold);
            needs_save = true;
            break;
            
        case FLICK_VIA_DISTANCE_THRESHOLD:
            current_config.distance_threshold = *(uint16_t*)value;
            flick_detection_set_distance_threshold(current_config.distance_threshold);
            needs_save = true;
            break;
            
        case FLICK_VIA_TIME_WINDOW:
            current_config.time_window_ms = *(uint16_t*)value;
            // Would need a setter function in flick_detection.h
            needs_save = true;
            break;
            
        case FLICK_VIA_COOLDOWN:
            current_config.cooldown_ms = *(uint16_t*)value;
            // Would need a setter function in flick_detection.h
            needs_save = true;
            break;
            
        case FLICK_VIA_VERTICAL_TOLERANCE:
            current_config.vertical_tolerance = *value;
            // Would need a setter function in flick_detection.h
            needs_save = true;
            break;
            
        case FLICK_VIA_DECAY_THRESHOLD:
            current_config.decay_threshold = *(uint16_t*)value;
            // Would need a setter function in flick_detection.h
            needs_save = true;
            break;
            
        case FLICK_VIA_RESET_TO_DEFAULTS:
            flick_detection_via_reset_defaults();
            needs_save = false;  // reset_defaults handles saving
            break;
    }
    
    if (needs_save) {
        flick_detection_via_save();
    }
}
