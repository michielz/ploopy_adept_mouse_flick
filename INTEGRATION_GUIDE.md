# Flick Detection Integration Guide

This guide explains how to integrate the flick detection firmware into your Ploopy Madromys QMK configuration.

## File Structure

After integration, your keyboard directory should contain:
```
keyboards/ploopyco/madromys/keymaps/your_keymap/
├── keymap.c
├── rules.mk
├── config.h (optional)
├── flick_detection.h
└── flick_detection.c
```

## Step 1: Copy Files

1. Copy `flick_detection.h` to your keymap directory
2. Copy `flick_detection.c` to your keymap directory

## Step 2: Modify `rules.mk`

Add the flick detection source file to your `rules.mk`:

```makefile
# Enable flick detection
SRC += flick_detection.c

# Optional: Enable console for debugging
# CONSOLE_ENABLE = yes
```

## Step 3: Modify `keymap.c`

Add the following to your `keymap.c` file:

### Include the Header

At the top of your `keymap.c`, add:

```c
#include "flick_detection.h"
```

### Initialize in `keyboard_post_init_user()`

Add or modify the `keyboard_post_init_user()` function:

```c
void keyboard_post_init_user(void) {
    // Initialize flick detection
    flick_detection_init();
    
    #ifdef CONSOLE_ENABLE
        debug_enable = true;
        debug_mouse = true;
    #endif
}
```

### Integrate with `pointing_device_task_user()`

Add or modify the `pointing_device_task_user()` function to process flick gestures:

```c
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // Process flick detection
    mouse_report = flick_detection_process(mouse_report);
    
    // Your other custom mouse processing here...
    
    return mouse_report;
}
```

## Complete Example `keymap.c`

Here's a complete minimal example:

```c
#include QMK_KEYBOARD_H
#include "flick_detection.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_BTN1, KC_BTN2, KC_BTN3,
        KC_BTN4, KC_BTN5
    )
};

void keyboard_post_init_user(void) {
    // Initialize flick detection
    flick_detection_init();
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // Process flick detection - this will detect flicks and modify
    // the mouse report to consume movement during gesture detection
    mouse_report = flick_detection_process(mouse_report);
    
    return mouse_report;
}
```

## Step 4: Configuration (Optional)

If you want to customize the flick detection parameters, create or modify `config.h` in your keymap directory:

```c
#pragma once

// Flick Detection Configuration
// All values are optional - defaults will be used if not specified

// Minimum velocity to trigger flick detection (counts per sample)
// Higher = need faster movement
// Default: 30
#define FLICK_VELOCITY_THRESHOLD 35

// Maximum time window for flick completion (milliseconds)
// Default: 150
#define FLICK_TIME_WINDOW_MS 200

// Minimum distance to register as flick (counts)
// Higher = need longer movement
// Default: 80
#define FLICK_DISTANCE_THRESHOLD 100

// Cooldown period after flick (milliseconds)
// Prevents accidental double-triggers
// Default: 500
#define FLICK_COOLDOWN_MS 400

// Maximum vertical/horizontal movement ratio
// 0.5 = allow 50% vertical movement
// Default: 0.5
#define FLICK_VERTICAL_TOLERANCE_RATIO 0.4

// Velocity decay threshold (counts per sample)
// When velocity drops below this, gesture ends
// Default: 5
#define FLICK_VELOCITY_DECAY_THRESHOLD 8

// Enable debug output (requires CONSOLE_ENABLE = yes in rules.mk)
// #define FLICK_DETECTION_DEBUG
```

## Step 5: Compile and Flash

Compile your firmware:

```bash
qmk compile -kb ploopyco/madromys -km your_keymap
```

Flash to your device:

```bash
qmk flash -kb ploopyco/madromys -km your_keymap
```

## Runtime Control

You can control flick detection at runtime using these functions:

### Enable/Disable

```c
// In your keymap.c, you can add custom keycodes to toggle flick detection
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case MY_FLICK_TOGGLE:
            if (record->event.pressed) {
                // Toggle flick detection
                bool current = flick_detection_is_enabled();
                flick_detection_set_enabled(!current);
            }
            return false;
    }
    return true;
}
```

### Adjust Sensitivity

```c
// Make flicks easier to trigger (lower threshold)
flick_detection_set_velocity_threshold(20);

// Make flicks require more distance
flick_detection_set_distance_threshold(120);
```

## Troubleshooting

### Flicks Not Detected

1. **Too sensitive?** Increase `FLICK_VELOCITY_THRESHOLD`
2. **Need more distance?** Increase `FLICK_DISTANCE_THRESHOLD`
3. **Timing out?** Increase `FLICK_TIME_WINDOW_MS`

### Accidental Triggers

1. **Random movements trigger flicks?** Increase `FLICK_VELOCITY_THRESHOLD`
2. **Double triggers?** Increase `FLICK_COOLDOWN_MS`
3. **Diagonal movements trigger?** Decrease `FLICK_VERTICAL_TOLERANCE_RATIO`

### Cursor Jumps During Flick

This is expected behavior. During flick detection, the firmware consumes mouse movement to prevent cursor jumping. If you prefer to see the cursor move during flicks, you can modify the movement consumption logic in `flick_detection_process()`.

### Debug Mode

Enable debug mode to see what's happening:

1. In `config.h`: Add `#define FLICK_DETECTION_DEBUG`
2. In `rules.mk`: Add `CONSOLE_ENABLE = yes`
3. Use QMK Toolbox or another serial console to view debug output
4. Call `flick_detection_print_debug()` to see current state

## Advanced Usage

### Custom Actions

You can detect flicks but perform custom actions instead of browser navigation:

```c
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // Get the state before processing
    flick_state_t old_state = flick_detection_get_state();
    
    // Process normally
    mouse_report = flick_detection_process(mouse_report);
    
    // Check if we just triggered
    flick_state_t new_state = flick_detection_get_state();
    if (old_state == FLICK_STATE_DETECTING && new_state == FLICK_STATE_TRIGGERED) {
        // A flick was just detected!
        // You can add custom logic here
    }
    
    return mouse_report;
}
```

### Layer-Dependent Flicks

```c
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // Only enable flicks on certain layers
    if (get_highest_layer(layer_state) == 0) {
        mouse_report = flick_detection_process(mouse_report);
    }
    
    return mouse_report;
}
```

## Testing Recommendations

1. **Start with defaults** - Don't change any configuration values initially
2. **Test in a browser** - Open a few pages and try flicking left/right
3. **Practice the gesture** - Quick horizontal swipes work best
4. **Adjust if needed** - Use the troubleshooting guide above
5. **Find your preference** - Everyone's flick style is different

## Performance Notes

- Flick detection adds minimal overhead to mouse processing
- Movement is consumed during detection to prevent cursor jumps
- Cooldown period prevents accidental repeated triggers
- All timing is handled by QMK's built-in timer functions

## Support

If you encounter issues:
1. Enable debug mode and check console output
2. Verify your `rules.mk` includes `flick_detection.c`
3. Check that `pointing_device_task_user()` is properly integrated
4. Try adjusting configuration values in `config.h`
