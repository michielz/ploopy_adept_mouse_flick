# Flick Detection Testing and Validation Guide

This guide helps you test and validate the flick detection firmware to ensure it's working correctly.

## Quick Test Checklist

- [ ] Firmware compiles without errors
- [ ] Flick left triggers browser back
- [ ] Flick right triggers browser forward
- [ ] Normal mouse movement works
- [ ] No accidental triggers during regular use
- [ ] Cooldown prevents double-triggers
- [ ] Flicks don't cause cursor jumps

## Testing Environment Setup

### Browser Setup

1. **Open a test browser** (Chrome, Firefox, etc.)
2. **Navigate through several pages** to build history:
   - Open Google
   - Click on a link
   - Click on another link
   - You should now have forward/back history

### Debug Console Setup (Optional but Recommended)

1. Enable console in `rules.mk`:
   ```makefile
   CONSOLE_ENABLE = yes
   ```

2. Add debug flag in `config.h`:
   ```c
   #define FLICK_DETECTION_DEBUG
   ```

3. Open QMK Toolbox or use `hid_listen` to view console output

4. In your `keymap.c`, add a debug toggle key:
   ```c
   enum custom_keycodes {
       DEBUG_FLICK = SAFE_RANGE
   };
   
   bool process_record_user(uint16_t keycode, keyrecord_t *record) {
       if (keycode == DEBUG_FLICK && record->event.pressed) {
           flick_detection_print_debug();
       }
       return true;
   }
   ```

## Basic Functionality Tests

### Test 1: Left Flick (Browser Back)

**Objective:** Verify that a quick leftward swipe triggers browser back

**Steps:**
1. Open browser with some history
2. Navigate to the second or third page
3. Place hand on trackball
4. Quickly roll the ball to the left with moderate speed
5. Release

**Expected Result:**
- Browser should navigate back one page
- Cursor should not jump significantly
- Only one page should go back (not multiple)

**If it doesn't work:**
- Try rolling faster
- Check console for debug output
- Verify you moved far enough (check `FLICK_DISTANCE_THRESHOLD`)

### Test 2: Right Flick (Browser Forward)

**Objective:** Verify that a quick rightward swipe triggers browser forward

**Steps:**
1. After going back (Test 1), you should have forward history
2. Quickly roll the ball to the right
3. Release

**Expected Result:**
- Browser should navigate forward one page
- Consistent behavior with left flick

### Test 3: Normal Mouse Movement

**Objective:** Ensure regular mouse usage isn't affected

**Steps:**
1. Move the mouse cursor slowly around the screen
2. Make small circular motions
3. Move diagonally
4. Move vertically

**Expected Result:**
- Cursor moves normally
- No unexpected browser navigation
- No lag or stuttering

### Test 4: Slow Horizontal Movement

**Objective:** Verify that slow horizontal movement doesn't trigger flicks

**Steps:**
1. Slowly roll the ball left
2. Slowly roll the ball right
3. Use deliberate, slow movements

**Expected Result:**
- NO browser navigation
- Cursor moves normally
- Flicks only trigger on fast movements

## Edge Case Tests

### Test 5: Cooldown Period

**Objective:** Verify that rapid repeated flicks are prevented

**Steps:**
1. Trigger a left flick
2. Immediately try to trigger another left flick
3. Try within 500ms (or your `FLICK_COOLDOWN_MS` value)

**Expected Result:**
- First flick works
- Second flick is ignored during cooldown
- Third flick works after cooldown expires

### Test 6: Diagonal Movement

**Objective:** Ensure diagonal swipes don't trigger flicks

**Steps:**
1. Quickly swipe diagonally (up-left)
2. Quickly swipe diagonally (down-right)
3. Try various diagonal angles

**Expected Result:**
- NO browser navigation if vertical component is too large
- Only triggers if movement is primarily horizontal

### Test 7: Direction Reversal

**Objective:** Verify that changing direction mid-gesture cancels detection

**Steps:**
1. Start rolling left quickly
2. Mid-motion, reverse and go right
3. Complete the rightward motion

**Expected Result:**
- Gesture is likely cancelled due to direction change
- May trigger if the reversal is at the very end
- Should feel natural and predictable

### Test 8: Short Quick Movements

**Objective:** Test minimum distance threshold

**Steps:**
1. Make very quick but very short leftward movements
2. Repeat multiple times
3. Gradually increase distance

**Expected Result:**
- Short movements below threshold don't trigger
- Once you exceed `FLICK_DISTANCE_THRESHOLD`, it triggers
- Helps understand the minimum gesture size

## Configuration Tuning Tests

### Test 9: Sensitivity Tuning

If flicks are too hard or too easy to trigger, adjust these values:

**Too Difficult (flicks don't register):**

```c
// In config.h, reduce thresholds:
#define FLICK_VELOCITY_THRESHOLD 20      // Lower = easier
#define FLICK_DISTANCE_THRESHOLD 60      // Lower = shorter swipes
#define FLICK_TIME_WINDOW_MS 200         // Longer = more time
```

**Too Easy (accidental triggers):**

```c
// In config.h, increase thresholds:
#define FLICK_VELOCITY_THRESHOLD 40      // Higher = need faster motion
#define FLICK_DISTANCE_THRESHOLD 100     // Higher = need longer swipes
#define FLICK_COOLDOWN_MS 700            // Longer cooldown
```

### Test 10: Vertical Tolerance

If diagonal movements trigger flicks when they shouldn't:

```c
// In config.h:
#define FLICK_VERTICAL_TOLERANCE_RATIO 0.3  // Stricter (was 0.5)
```

## Stress Tests

### Test 11: Rapid Alternating Flicks

**Steps:**
1. Trigger left flick
2. Wait for cooldown
3. Trigger right flick
4. Repeat 10 times

**Expected Result:**
- All flicks should register correctly
- No missed detections
- No stuck states

### Test 12: Extended Use Test

**Steps:**
1. Use trackball normally for 10 minutes
2. Mix regular cursor movement with occasional flicks
3. Try various speeds and directions

**Expected Result:**
- Everything works smoothly
- No unexpected behavior
- No firmware crashes or freezes

### Test 13: Interrupt Test

**Steps:**
1. Start a flick gesture
2. Press a mouse button mid-gesture
3. Try clicking while flicking

**Expected Result:**
- Mouse buttons work normally
- Gesture may cancel or complete depending on timing
- No conflicts between features

## Debug Output Interpretation

When `FLICK_DETECTION_DEBUG` is enabled, you'll see output like:

```
Flick Detector Debug:
  State: DETECTING
  Direction: NONE
  Accumulated X: 45
  Accumulated Y: 12
  Last Velocity X: 25
  Direction Changes: 0
  Movement Consistent: YES
  Enabled: YES
  Velocity Threshold: 30
  Distance Threshold: 80
```

**What to look for:**

- **State IDLE**: Waiting for flick
- **State DETECTING**: Gesture in progress
  - Check `Accumulated X` to see progress toward threshold
  - `Direction Changes` should be 0-2 for valid flicks
- **State TRIGGERED**: Flick detected, sending keycode
- **State COOLDOWN**: Preventing repeat triggers

## Common Issues and Solutions

### Issue: Flicks trigger too easily

**Solution:**
1. Increase `FLICK_VELOCITY_THRESHOLD` to 35-40
2. Increase `FLICK_DISTANCE_THRESHOLD` to 100-120
3. Decrease `FLICK_VERTICAL_TOLERANCE_RATIO` to 0.3

### Issue: Flicks never trigger

**Solution:**
1. Decrease `FLICK_VELOCITY_THRESHOLD` to 20-25
2. Decrease `FLICK_DISTANCE_THRESHOLD` to 60-70
3. Increase `FLICK_TIME_WINDOW_MS` to 200-250

### Issue: Diagonal movements trigger flicks

**Solution:**
1. Decrease `FLICK_VERTICAL_TOLERANCE_RATIO` to 0.3-0.4
2. This makes the detection more strict about horizontal-only movement

### Issue: Double triggers

**Solution:**
1. Increase `FLICK_COOLDOWN_MS` to 600-800
2. This gives more time between flicks

### Issue: Cursor jumps during flick

**Explanation:** This is intentional! The firmware consumes movement during flick detection to prevent cursor jumping. The browser navigation happens instead of cursor movement.

**If you want cursor to move:**
Modify `flick_detection_process()` in `flick_detection.c` to not zero out the report during detection (not recommended).

## Performance Validation

### Responsiveness Test

1. Measure time from flick gesture to browser action
2. Should feel instant (< 50ms)
3. No noticeable delay in browser response

### CPU Usage Test

1. Monitor QMK's scan rate (if available)
2. Flick detection should not impact performance
3. Mouse movement should remain smooth

## Final Validation Checklist

Before considering the implementation complete, verify:

- [x] **All basic functionality tests pass**
- [x] **No accidental triggers during normal use**
- [x] **Gesture feels natural and reliable**
- [x] **Configuration values are appropriate for your preference**
- [x] **Debug mode works (if enabled)**
- [x] **No firmware errors or crashes**
- [x] **Performance is acceptable**
- [x] **Documentation is clear and helpful**

## Reporting Issues

If you find bugs or unexpected behavior, document:

1. **Exact steps to reproduce**
2. **Expected vs actual behavior**
3. **Configuration values** (from config.h)
4. **Debug output** (if available)
5. **QMK version** and firmware build date
6. **Trackball model** and sensor type

## Success Metrics

The implementation is successful when:

1. ✅ 90%+ of intentional flicks are detected
2. ✅ <5% false positive rate during normal use
3. ✅ Gesture feels natural and predictable
4. ✅ No negative impact on regular mouse usage
5. ✅ Users can easily tune to their preferences

## Next Steps

Once testing is complete:

1. Document your preferred configuration values
2. Share your settings with the community
3. Consider contributing improvements back to the codebase
4. Enjoy your gesture-enabled trackball!
