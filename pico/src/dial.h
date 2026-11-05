/* Story dial (built with STORY_DIAL=KY040 or AS5600), two sensors behind one interface.
 *
 * STORY_DIAL=KY040 (dial.cpp; ON means this): a KY-040 rotary encoder on the left edge of the
 * enclosure, a thumbwheel with the numbers 1 to DIAL_POSITIONS on its rim and an arrow on the
 * case. The wheel's travel is limited by printed end stops to DIAL_POSITIONS - 1 steps of
 * DIAL_DETENTS_PER_STEP detents (two on the 30-detent KY-040, one on a 20-detent one), the
 * firmware counts the detents from the lower stop and saturates at both ends, so the stops are
 * the absolute reference of an otherwise incremental encoder.
 *
 * The module's "+" pin may be fed by a GPIO driven high (DIAL_PIN_VCC): it only feeds pull-ups.
 * The position is counted by interrupt while the board is awake (nothing counts during the deep
 * sleep, by choice: battery life). The ritual therefore re-arms it: wheel turned down to the stop,
 * RESET (dial_set_position(1)), wheel turned up to the story, GO. The dial is only read when a
 * story starts, and only if the wheel moved since the last RESET or story start (dial_moved());
 * otherwise the story is picked at random, as without a dial. It never changes the story being read.
 *
 * Every detent counts, whichever rest of the lines it is at (both high or both low: they alternate
 * on an encoder with half a cycle per detent, the KY-040 at hand), and the position is the detent
 * count divided by DIAL_DETENTS_PER_STEP, rounded down: the numbers sit on the even detents from
 * the stop, and a stray detent at the stop (wheel pressed past its detent at RESET, a glitch of
 * the lines when the wheel is pushed) never shifts them.
 *
 * STORY_DIAL=AS5600 (dial_as5600.cpp): the same wheel on a printed axle carrying a magnet, over an
 * AS5600 magnetic angle sensor on I2C. The angle is absolute: the position is read, not counted,
 * at boot and while awake, so it is right after a deep sleep and there is no ritual. RESET only
 * clears the "moved" flag; a wheel found on another number at boot than the saved one counts as
 * moved. No detents and no push button: button B keeps its wake-up slot. The angle of the "1" is
 * learnt once (dial_calibrate(): the 1 under the arrow, RESET held 3 s) and saved.
 */
#ifndef HADISTORY_DIAL_H
#define HADISTORY_DIAL_H

void dial_init(int position, int detents, int zero);   // KY-040: restores the saved state (the detent count when
                                  // consistent with the position, else the position alone, clamped), starts counting.
                                  // AS5600: powers the sensor, takes the saved zero and reads the position
int  dial_position(void);         // 1 .. DIAL_POSITIONS
int  dial_detents(void);          // KY-040: detents from the lower stop (saved with the settings, restored at boot)
int  dial_zero(void);             // AS5600: raw angle of the "1" (saved with the settings), -1 = not calibrated
void dial_set_position(int p);    // RESET. KY-040: re-arm, the wheel is at that position (1, the lower stop); AS5600:
                                  // the position is the wheel's. Both clear moved
bool dial_calibrate(void);        // AS5600: the 1 is under the arrow now; false without sensor or magnet. KY-040: re-arm at 1
void dial_sleep(void);            // before the deep sleep: AS5600 powered off (nothing to do for the KY-040)
bool dial_moved(void);            // the wheel moved since the last dial_set_moved(false) / dial_set_position()
void dial_set_moved(bool moved);  // cleared when a story starts; restored from the settings at boot
int  dial_take_steps(void);       // net detents counted since the last call (0 = nothing moved)

#endif
