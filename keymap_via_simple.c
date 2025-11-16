/*
 * Simple VIA Integration Example for Flick Detection
 * 
 * This is a simplified approach that uses VIA's keycode system
 * to toggle flick detection and adjust sensitivity without
 * needing custom VIA menus or HID commands.
 * 
 * Users can map custom keycodes in VIA's UI to control flick detection.
 */

#include QMK_KEYBOARD_H
#include "flick_detection.h"
#include "flick_detection_via.h"

// ============================================================================
// Custom Keycodes for VIA
// ============================================================================

enum custom_keycodes {
    FLICK_TOGGLE = QK_USER_0,      // Toggle flick detection on/off
    FLICK_SENS_UP,                  // Increase sensitivity (lower threshold)
    FLICK_SENS_DOWN,                // Decrease sensitivity (higher threshold)
    FLICK_DIST_UP,                  // Increase distance threshold
    FLICK_DIST_DOWN,                // Decrease distance threshold
    FLICK_RESET                     // Reset to defaults
};

// ============================================================================
// Keymap
// ============================================================================

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Layer 0: Default buttons
    [0] = LAYOUT(
        KC_BTN1, KC_BTN2, KC_BTN3,
        KC_BTN4, KC_BTN5
    ),
    
    // Layer 1: Function layer with flick controls
    [1] = LAYOUT(
        FLICK_TOGGLE, FLICK_SENS_UP, FLICK_SENS_DOWN,
        FLICK_DIST_UP, FLICK_DIST_DOWN
    ),
    
    // Layer 2: Additional layer
    [2] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, FLICK_RESET
    ),
    
    // Layer 3: Empty layer
    [3] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS
    )
};

// ============================================================================
// Initialization
// ============================================================================

void keyboard_post_init_user(void) {
    // Initialize flick detection
    flick_detection_init();
    
    // Load saved VIA settings from EEPROM
    flick_detection_via_init();
    
    #ifdef CONSOLE_ENABLE
        debug_enable = true;
        uprintf("Flick detection with VIA initialized\n");
    #endif
}

// ============================================================================
// Pointing Device Integration
// ============================================================================

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // Process flick detection
    mouse_report = flick_detection_process(mouse_report);
    return mouse_report;
}

// ============================================================================
// Custom Keycode Handler
// ============================================================================

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;  // Only act on key press, not release
    }
    
    switch (keycode) {
        case FLICK_TOGGLE: {
            // Toggle flick detection on/off
            bool enabled = flick_detection_is_enabled();
            flick_detection_set_enabled(!enabled);
            flick_detection_via_save();  // Save to EEPROM
            
            #ifdef CONSOLE_ENABLE
                uprintf("Flick detection: %s\n", enabled ? "DISABLED" : "ENABLED");
            #endif
            return false;
        }
        
        case FLICK_SENS_UP: {
            // Increase sensitivity (lower velocity threshold)
            // Current threshold - 5, minimum 10
            int16_t current = flick_detection_get_velocity_threshold();
            int16_t new_val = current - 5;
            if (new_val < 10) new_val = 10;
            
            flick_detection_set_velocity_threshold(new_val);
            flick_detection_via_save();
            
            #ifdef CONSOLE_ENABLE
                uprintf("Velocity threshold decreased to: %d\n", new_val);
            #endif
            return false;
        }
        
        case FLICK_SENS_DOWN: {
            // Decrease sensitivity (higher velocity threshold)
            // Current threshold + 5, maximum 100
            int16_t current = flick_detection_get_velocity_threshold();
            int16_t new_val = current + 5;
            if (new_val > 100) new_val = 100;
            
            flick_detection_set_velocity_threshold(new_val);
            flick_detection_via_save();
            
            #ifdef CONSOLE_ENABLE
                uprintf("Velocity threshold increased to: %d\n", new_val);
            #endif
            return false;
        }
        
        case FLICK_DIST_UP: {
            // Increase distance threshold
            int16_t current = flick_detection_get_distance_threshold();
            int16_t new_val = current + 10;
            if (new_val > 200) new_val = 200;
            
            flick_detection_set_distance_threshold(new_val);
            flick_detection_via_save();
            
            #ifdef CONSOLE_ENABLE
                uprintf("Distance threshold increased to: %d\n", new_val);
            #endif
            return false;
        }
        
        case FLICK_DIST_DOWN: {
            // Decrease distance threshold
            int16_t current = flick_detection_get_distance_threshold();
            int16_t new_val = current - 10;
            if (new_val < 40) new_val = 40;
            
            flick_detection_set_distance_threshold(new_val);
            flick_detection_via_save();
            
            #ifdef CONSOLE_ENABLE
                uprintf("Distance threshold decreased to: %d\n", new_val);
            #endif
            return false;
        }
        
        case FLICK_RESET: {
            // Reset all settings to defaults
            flick_detection_via_reset_defaults();
            
            #ifdef CONSOLE_ENABLE
                uprintf("Flick detection reset to defaults\n");
            #endif
            return false;
        }
    }
    
    return true;
}

// ============================================================================
// VIA Custom Value Handler (for advanced users)
// ============================================================================

#ifdef VIA_ENABLE
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    uint8_t *command_id = &(data[0]);
    uint8_t *channel_id = &(data[1]);
    uint8_t *value_id = &(data[2]);
    uint8_t *value_data = &(data[3]);
    
    // Handle flick detection channel
    if (*channel_id == FLICK_VIA_CHANNEL) {
        switch (*command_id) {
            case id_custom_set_value:
                flick_detection_via_set_value(*value_id, value_data);
                break;
            case id_custom_get_value:
                flick_detection_via_get_value(*value_id, value_data);
                break;
            case id_custom_save:
                flick_detection_via_save();
                break;
            default:
                *command_id = id_unhandled;
                break;
        }
        return;
    }
    
    // Let VIA handle other channels
    *command_id = id_unhandled;
}
#endif
