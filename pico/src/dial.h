/* Story dial (built with STORY_DIAL=ON): a KY-040 rotary encoder on the left edge of the
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
 */
#ifndef HADISTORY_DIAL_H
#define HADISTORY_DIAL_H

void dial_init(int position, int detents);   // restores the saved state (the detent count when consistent with the
                                  // position, else the position alone, clamped), starts counting
int  dial_position(void);         // 1 .. DIAL_POSITIONS
int  dial_detents(void);          // detents from the lower stop (saved with the settings, restored at boot)
void dial_set_position(int p);    // re-arm: the wheel is at that position (RESET: 1, the lower stop); clears moved
bool dial_moved(void);            // the wheel moved since the last dial_set_moved(false) / dial_set_position()
void dial_set_moved(bool moved);  // cleared when a story starts; restored from the settings at boot
int  dial_take_steps(void);       // net detents counted since the last call (0 = nothing moved)

#endif
