/* Story dial (built with STORY_DIAL=ON): a KY-040 rotary encoder on the left edge of the
 * enclosure, a thumbwheel with the numbers 1 to DIAL_POSITIONS on its rim and an arrow on the
 * case. The wheel's travel is limited by printed end stops to DIAL_POSITIONS - 1 steps (a step is a
 * full quadrature cycle: one detent on a 20-detent encoder, two on a 30-detent one), the
 * firmware saturates its position at both ends, so the stops are the absolute reference of an
 * otherwise incremental encoder.
 *
 * The module's "+" pin may be fed by a GPIO driven high (DIAL_PIN_VCC): it only feeds pull-ups.
 * The position is counted by interrupt while the board is awake (nothing counts during the deep
 * sleep, by choice: battery life). The ritual therefore re-arms it: wheel turned down to the stop,
 * RESET (dial_set_position(1)), wheel turned up to the story, GO. The dial is only read when a
 * story starts, and only if the wheel moved since the last RESET or story start (dial_moved());
 * otherwise the story is picked at random, as without a dial. It never changes the story being read.
 *
 * On an encoder with half a cycle per detent (30 detents, the KY-040 at hand) the detents alternate
 * between the both-high and the both-low rest of the lines, and a step is one full cycle, every
 * second detent: the detent at the stop must be one that counts, or every number sits a detent
 * off the arrow. So the re-arm reads the lines: the rest state found at the stop is the one the
 * decoder counts at from then on (DIAL_DETENTS_PER_CYCLE 1 only).
 */
#ifndef HADISTORY_DIAL_H
#define HADISTORY_DIAL_H

void dial_init(int position);     // restores the saved position (clamped), starts counting
int  dial_position(void);         // 1 .. DIAL_POSITIONS
void dial_set_position(int p);    // re-arm: the wheel is at that position (RESET: 1, the lower stop); clears moved, and
                                  // the rest state the lines show there (both low or both high) becomes the one that counts
bool dial_rest_low(void);         // which rest state counts (saved with the settings, restored at boot)
void dial_set_rest_low(bool low);
bool dial_moved(void);            // the wheel moved since the last dial_set_moved(false) / dial_set_position()
void dial_set_moved(bool moved);  // cleared when a story starts; restored from the settings at boot
int  dial_take_steps(void);       // net detents counted since the last call (0 = nothing moved)

#endif
