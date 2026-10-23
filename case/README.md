# hadistory — enclosure for the Pico version

A 3D printed case, portrait, with the Inky Impression 5.7" as the face and the electronics in the back shell: back shell, bezel, a cap over the HAT wiring, four push keys, a story dial thumbwheel and two label plates. Design inspired by the [YouMakeTech Pico-GB](https://www.youmaketech.com/pico-gb-gameboy-emulator-handheld-for-raspberry-pi-pico/) bottom cover: every part sits in a moulded pocket or on standoffs, and all the wiring is Dupont jumpers.

Two styles in the same model, `style` at the top of the file. **`bump`** (v9, the default): 26 mm body, 10 mm corners, chamfered back edge, rounded bezel edge, and a pillow-shaped cap on the back over the HAT Dupont block, the only place that needs the depth. **`box`** (v7): the rectangular shell, 33 mm deep everywhere, validated by the test prints. The insides are the same.

| ![Bump, front](img/bump/front.png) | ![Bump, back](img/bump/rear.png) |
|:--:|:--:|
| `bump`: front, keys on the top edge, white and green spheres in the chin, story dial on the left edge | `bump`: the cap over the HAT Dupont block |

| ![Left edge](img/bump/left.png) | ![Wheel](img/bump/wheel.png) |
|:--:|:--:|
| Left edge: the thumbwheel in its notch, the ON / OFF plate under it | The wheel as printed, numbers engraved on the rim, nut recess and stop groove on the inner face |

| ![Assembly](img/bump/assembly.png) | ![Back shell](img/bump/back.png) |
|:--:|:--:|
| `bump` assembly, seen from the front | `bump` back shell, display side up: hole for the cap on the right |

| ![Box assembly](img/box/assembly.png) | ![Box back shell](img/box/back.png) |
|:--:|:--:|
| `box` (v7) assembly | `box` (v7) back shell |

| ![Label plate](img/plate.png) | ![Top edge with the plate](img/plate_edge.png) |
|:--:|:--:|
| Label plate, as printed | Top edge seen from outside, back of the case up: keys and plate |

| ![ON / OFF plates](img/swplate.png) |
|:--:|
| The two ON / OFF plates for the bottom edge, on left and on right |

## Files

| File | Content |
|---|---|
| `hadistory_case.scad` | Parametric model (OpenSCAD). All dimensions are in the parameter block at the top; `style` picks `bump` or `box`. |
| `stl/bump/back.stl` | `bump` back shell, print floor down |
| `stl/bump/front.stl` | `bump` bezel, exported face up (rounded edge on top) |
| `stl/bump/cap.stl` | `bump` cap over the HAT Dupont block, exported flange down, dome up; glued inside |
| `stl/bump/key.stl` | `bump` push key, print 4 (1 mm longer than the box key: the top margin is 7 mm) |
| `stl/bump/wheel.stl` | Story dial thumbwheel, print outer face down (same part in both folders) |
| `stl/bump/plate.stl` | Label plate for the keys, glued on the top edge of the case, print flat text up (same part in both folders) |
| `stl/bump/swplate_on_left.stl`, `swplate_on_right.stl` | ON / OFF plate for the switch, glued on the left edge above the knob slot; print the one matching the wiring (same parts in both folders) |
| `stl/box/back.stl` | `box` back shell (v7 outline with the v9 insides), print floor down |
| `stl/box/front.stl` | `box` bezel, exported face down |
| `stl/box/key.stl` | `box` push key, print 4, flat on its bottom face |
| `stl/box/wheel.stl` | Thumbwheel, same as above |
| `stl/box/plate.stl` | Label plate, same as above |
| `stl/box/swplate_on_left.stl`, `swplate_on_right.stl` | ON / OFF plates, same as above |

Pictures: `img/bump/`, `img/box/`, and the concepts in `img/`.

Regenerate the STL files after a change:

```bash
cd case
for p in back front cap key wheel plate; do openscad -D "part=\"$p\"" -o stl/bump/$p.stl hadistory_case.scad; done
for p in back front key wheel plate; do openscad -D 'style="box"' -D "part=\"$p\"" -o stl/box/$p.stl hadistory_case.scad; done
for d in bump box; do
  openscad -D 'part="swplate"' -D 'sw_on_left=true'  -o stl/$d/swplate_on_left.stl  hadistory_case.scad
  openscad -D 'part="swplate"' -D 'sw_on_left=false' -o stl/$d/swplate_on_right.stl hadistory_case.scad
done
```

Other values of `part`: `assembly` (shell, cap, HAT and parts preview, bezel transparent), `closed` (all parts opaque, for pictures), `section` (assembly cut at `section_x`, to check the clearances: 85.6 goes through the HAT header and the cap, 50 through the Pico).

OpenSCAD on macOS: `brew install --cask openscad@snapshot` (the stable cask is disabled). The `.scad` file also opens in FreeCAD.

## How it goes together

The device is seen from the front in the model: X right, Y up, display towards the viewer. The Pimoroni drawing of the HAT is a front view with the components seen through (a mirrored version was tried in v2 and put the keys and the flex relief in the wrong place). Turned so that the buttons are on top: the 40-pin header and its Dupont block hang on the **right**, the panel flex is on the **left** edge, the four side buttons protrude from the **top** edge.

- **Depth** (`bump`): 26.6 mm in all, set by the Pico: floor 2, standoffs 8, Pico with its right-angle pins 8, then `hat_back_room` (3 mm, to measure: the clearance for the components on the back of the HAT PCB over the Pico, the header itself is elsewhere), the PCB 2.8 plus the 0.8 drop, bezel 2. The Dupont housings on the HAT header, 18 mm below the PCB, end 1 mm above the floor; the cap gives them 6 mm more plus the floor thickness, for plugging. In `box`, 33 mm: the housings plus 10 mm of slack everywhere.
- **Outline** (`bump`): 10 mm plan radius, 3 mm chamfer at 45 degrees on the back edge (prints floor down as is, where a fillet would not), and a 2 mm bead at 45 degrees inside along the floor-wall junction so that 2.1 mm of material stays behind the chamfer. The corner bosses move 6 mm in from the faces (3.2 in `box`), and the top margin grows to 7 mm so that the top bosses clear the PCB corners.
- **Cap** (`bump`): a 38 x 67 mm pillow, 6 mm proud of the back, 12 mm plan radius, sides leaning in at 45 degrees and a 3 mm rounded top edge, centred on the HAT header strip (X 83 to 88, Y 45 to 96). It goes in from inside through a hole in the floor (0.2 mm clearance), its 3 mm flange sits on the inner face of the floor and is glued there before the HAT goes in; the flange also makes it captive. Walls 2 mm, the inside closes at 45 degrees so that it prints flange down without support. Its footprint stops short of the battery rib above and leaves the switch cradle clear below.
- **Back shell**: walls 2 mm, floor 2 mm. The HAT rests on eight small pads, 8 mm wide, reaching 1.6 mm under the PCB edge, 3.6 mm below the rim (`shelf_drop` 0.8 mm: 1 mm on v3 was too much, and with 0.4 the v7 glass stood 0.6 mm proud on the right side when the bezel was screwed); each pad hangs from the wall on a 45 degree gusset and carries a short locating lip outside the PCB: 0.4 mm on the long sides (`clear`, 0.8 mm on v3 left too much play), 0.1 mm on the top and bottom edges (`clear_y`: at 0.4 the v5 still had 0.6 mm of play along the PCB length). Two pads per long side, two on the bottom edge, two on the top edge between the keys, clear of the protruding caps (v4 had none on top: the PCB could ride up to the key guides, a millimetre or more off the bottom lips, and shifted at each key press). Everything else in the margins stays open: screwdriver access to the Pico, wires everywhere. Margins 4.5 mm on the sides, 6 mm on top, 12 mm chin.
- **Front**: a 2 mm plate with the window, four countersunk M3 screws into the corner bosses, and the conical hole of the sphere LED. It covers the notches of the keys and of the switch knob at the rim, and holds the HAT down on its pads. In `bump` its outer edge is rounded (1.6 mm) and it prints face up, the countersinks and the LED cone being 45 degree features; `box` prints it face down.
- **Keys**: the HAT switches are on the back face of the PCB, at 19.8, 41.3, 62.7 and 84.2 mm from the flex edge (`btn_from_flex`); their caps protrude 1.5 mm from the top edge, below the PCB underside (v3 had the keys at the glass level, about 4 mm too high). Four guide blocks in the top margin, set back 2.4 mm from the PCB edge to clear the caps and hanging from the wall on a 45 degree gusset like the pads (no print support), carry a notch from 4.4 mm below the PCB underside up to the rim (`btn_cap_drop`), and a step for the 12 mm head of the key. The key is 7.6 mm tall and fills the notch, so it cannot ride up and reaches the cap wherever it sits in that range; its shaft stands 3 mm proud of the wall (`key_proud`, 1 mm on v4 was too short to find by feel). A key is dropped in from above before the HAT and is captive once the HAT and bezel are on. Its shaft and head are 0.6 mm narrower than their notch (`key_play`, 0.8 up to v6 felt loose with the 3 mm shaft outside), the head has 0.2 mm of play along the travel in its step (`key_head_play`) and the nub stops 0.3 mm short of the cap with the key fully out (`key_gap`): 0.5 mm of free travel before the click, where the v6 key had 1.1 mm. The cap protrusion (`btn_protrusion`, 1.5 mm) is still an estimate: any error on it adds to or subtracts from that free travel. The shell notches did not change: a new key fits a v6 shell.
- **Label plate**: a 92 x 8 x 1.2 mm plate glued on the outer face of the top wall, 1 mm behind the key notches, centred on the four keys. One word per key with a small pointer towards the key: left to right as seen from the front, BACK, MODE, RESET, NEXT (`plate_labels`; those are keys D, C, B, A, the firmware functions are in `pico/README.md`). Read with the display facing you and the top edge tilted towards you, or with the device lying display up and seen from beyond its top edge: the back of the case is up either way. Text and pointers stand 0.6 mm proud (`plate_relief`, a negative value engraves them instead); glue with cyanoacrylate or a dot of hot glue.
- **ON / OFF plate**: a 30 x 7 x 1.2 mm plate of the same construction, glued on the bottom wall just under the switch knob slot, ON on one side of the knob and OFF on the other. Which side is on depends on how the switch was wired, so there are two variants (`sw_on_left`): `swplate_on_left.stl` and `swplate_on_right.stl`, pick the one that matches. It reads with the display facing you and the bottom edge tilted towards you, display side up.
- **Switch** (v9): SS12F15 lying on the floor against the left wall, Y 21, knob through an 8 x 3.4 mm slot in the wall, pins pointing inwards. Its ears are in the plane of the knob face, so they lie against the wall inner face; the body sits on a 1.2 mm pad so that the ears clear the floor (`sw_pad_t`), between a U of three 1.6 mm ribs 3 mm tall (the end ribs start 0.9 mm from the wall, beside the ears), and a drop of glue keeps it there: nothing holds it from above, the HAT is 12 mm higher. Up to v8 the switch stood at the rim of the bottom edge, held down by the bezel; that spot went to the green sphere. Ear thickness and height are parameters (`sw_ear_t`, `sw_ear_h`), to check on the part. The ON / OFF plate is glued on the left edge above the slot and reads with the device face up.
- **Story dial** (v9): a KY-040 module standing behind the left wall, encoder towards the wall and pins towards the top of the case, in the free stretch of the left edge between the lower HAT pad (Y 24 to 32) and the panel flex (from Y 63.8). The left edge gets a notch, 4.5 mm deep and 27 mm long (Y 33.5 to 60.5, 2 mm corner radius), through the shell and the bezel; the 2 mm wall behind it ends at X 6.5, just outside the HAT PCB, and carries the M7 bushing hole and a 2 mm stop pin. The module's 19 mm wide PCB is as tall as the room between floor and HAT, so it sinks 0.8 mm into a recess of the floor (`enc_floor_recess`); that sets the axis 10.7 mm above the back face and the wheel diameter, 21 mm, bottom flush with the back. The thumbwheel is 12 mm thick: 4 mm inside the notch, hiding the nut (round recess, the wheel turns around it) and the bushing end, 8 mm proud of the edge. Its shaft bore is a blind D, 6.1 mm with the flat 1.7 mm from the axis (4.75 across), the shape of the knob cap printed earlier for this encoder, so the wheel presses on without glue; the shaft is **cut to 6.5 mm beyond the bushing end** (echoed by the model). The D fixes the wheel's angle on the shaft, and the detents are fixed on the shaft too, so where the 1 lands at the lower stop is a property of the encoder: `wheel_phase` turns the numbers and the groove about the flat, to be set from the first print (angle between the 1 and the arrow at the stop detent, 18 degrees per detent). Numbers 1 to 12 are engraved on the rim every 18 degrees (one detent of a 20-detent encoder), upright for the reader, 1 on top at the lower stop; an arrow engraved in the bezel margin points at the number in use. On the inner face of the wheel, a 1.6 mm deep arc groove between the nut recess and the rim takes the stop pin: 198 degrees of travel plus the pin, its ends are the stops. The firmware saturates its count at both ends, see `pico/README.md`.
- **Panel flex**: on the left edge, between the two left pads; nothing touches the PCB edge there.
- Outer size 113 x 148 x 33 mm.

## Electronics inside the back shell

| Part | Where | Fixing |
|---|---|---|
| Pico 2 W, right-angle headers on both long sides | Lower middle left, standing, USB towards the bottom wall through a 13 x 7 mm opening; 20 mm kept free on each side for the Dupont housings | 4 M2 screws on 8 mm standoffs of 4.4 mm (2 mm pilot holes), each with a flat on its outer face 1.6 mm from the hole over the top 3.2 mm, where the header plastic sits next to the holes (`pico_post_flat`, `pico_post_relief_h`); nothing above the screws |
| TP4056 (IN+ IN- B+ B-), its micro-USB unused | Against the left wall under the battery, B+/B- end 1 mm from the wall, USB end towards the Pico (`tp_usb_out`; false puts the USB at the wall, everything about the module mirrors) | No mounting holes on the module and its LEDs sit 15 mm in from the USB edge, so it is glued components DOWN: the micro-USB shell rests on the floor at the Pico end, a 1.6 x 6 mm rib at the wall carries the B+/B- edge between its pads (`tp_usb_h`, 3.2 mm, to check), wires soldered on the bare side. A 1.2 mm lip along its lower long edge and an end stop past the USB end, with the left wall and the battery rib, set where it goes (`tp_clear`); a 3 x 10 mm window through the floor under the charge / full LEDs, 11 mm from the wall (`tp_led_x` from the USB edge, `tp_win`), shows them from the back |
| KY-040 rotary encoder module (story dial) | Behind the left wall, Y 38 to 64, pins up | Its M7 bushing through the notch wall, nut in the notch; the PCB stands in a 0.8 mm recess of the floor (`enc_floor_recess`); five Dupont wires on its header, towards the Pico |
| 103450 LiPo 2000 mAh | Across the top of the shell | Ribs with 1 mm clearance, 12 mm openings at both ends for the wires |
| SS12F15 slide switch | Left wall, Y 21, lying on the floor, knob through the wall | Pad and three ribs around the body, ears against the wall, a drop of glue (`sw_y`) |
| dLUX-dLITE sphere LEDs (Adafruit 5431) + 220 ohm, white = status at X 18, green = AI mode at X 95 | Chin, both ends | Tubes in the shell (1.2 mm wall, 0.4 mm clear of the PCB edge: the 1.6 mm tube of v4 overlapped it), open towards the inside for the wires up to 8 mm above the floor, the opening closing with a 45 degree roof so that it prints without support, conical hole in the bezel |

Power wiring (the HAT's 5V pin is not needed: on this HAT it only feeds the small expansion header, the panel runs from 3V3; the whole wiring is drawn in `pico/img/wiring.png`):

| From | To |
|---|---|
| Battery + / - | TP4056 B+ / B- |
| Pico VBUS (pin 40) | TP4056 IN+ : the Pico's USB charges the battery |
| TP4056 IN- / B- | Pico GND |
| TP4056 B+ | Slide switch, centre pin |
| Slide switch, side pin | Anode of a 1 A Schottky diode (1N5817, 1N5819, SS14; a 1N4007 also works with a larger drop). Not a 1N5711, it is a 15 mA signal diode. |
| Diode cathode | Pico VSYS (pin 39) |

The diode keeps the USB 5V from reaching the battery through the Pico when the USB is plugged with the switch on. A TP4056 module without B+/B- protection relies on the protection circuit of the cell itself (most 103450 pouch cells have one).

Signal wiring: see the wiring tables in `pico/README.md` (display on GP17 to GP22, buttons on GP2 to GP5, LED on GP8), with the HAT's 3V3 and GND.

## Printing

PLA or PETG, 0.4 mm nozzle, 0.2 mm layers, 3 walls, no supports: every overhang in the shell (pads, key guides, switch pad) sits on a 45 degree gusset, the back chamfer and the inner bead are at 45 degrees, the cap prints flange down with its dome narrowing upwards and a 45 degree ceiling inside, the `bump` bezel prints face up. The back shell always prints floor down; a rounded back would have needed supports or the shell printed rim down, which would have left the Pico standoffs and the ribs in the air, hence the chamfer and the separate cap. The model keeps heights on multiples of 0.2 mm and thin features on multiples of 0.4 mm: when adjusting a parameter, move by at least one layer (0.2 mm) or one line (0.4 mm), smaller changes are not reproduced. Printed holes come out smaller than modelled: the M3 pilot holes are 2.6 mm, the Pico M2 holes 2.0 mm.

Label plates (keys and ON / OFF): flat, text up, 0.2 mm layers (0.1 mm looks better on the letters). The strokes of the 4 mm words are about 0.6 mm wide, so enable the slicer's thin wall / gap fill option (PrusaSlicer "Detect thin walls", Cura "Print thin walls"), or print at 0.4 mm line width with a single perimeter for the top 0.6 mm. For two colours, change the filament at 1.2 mm, where the text starts; or set `plate_relief` to -0.6 and fill the engraved letters with paint.

## Print checklist

Version 9 (story dial and switch on the left edge, green sphere at the right of the chin, on top of v8: `bump` outline, depth and cap, TP4056 glued components down with an LED window in the floor; same insides in both styles) after test prints of versions 1 to 6 of the box (v4: keys, LED, switch and Pico all good, the HAT had play towards the chin; v5: 0.6 mm of play left along the PCB length, keys too short to find by feel; v6: label plate, keys loose with 1.1 mm of free travel, switch rocking in its grooves; v7: glass and switch 0.6 mm proud on the right side when screwing the bezel, hence `shelf_drop` 0.8 and `sw_drop` 0.4). To check with the parts placed, before wiring:

1. HAT: drops onto the eight pads with little play (`clear`) in both directions, glass 0.8 mm below the rim (`shelf_drop`), buttons free above the top margin, flex free on the left edge.
2. Keys: centred on the button caps in X and covering them in depth (`btn_from_flex`, `btn_cap_drop`), slide in their notches, heads seated in the step, about 0.5 mm of free travel before the click (`key_gap`, `key_head_play`; more means `btn_protrusion` is overestimated, to measure on the HAT).
3. Pico: screwed on its standoffs, housings on the pins clear of the floor, USB centred in its opening (`pico_standoff`, `pico_pos()`).
4. Dupont housings on the HAT header: nothing touches the floor with the HAT in place (`dupont_slack`).
5. Switch: lies on its pad between the ribs with the ears against the wall, knob sliding in the slot (`sw_y`, `sw_pad_t`, `sw_ear_h`); glue once checked.
6. Screws: M3 bite in the bosses without splitting them.
7. TP4056: components down, B+/B- edge at the wall, it sits level on its USB shell and the wall-side rib (`tp_usb_h`), between the lip, the end stop, the left wall and the battery rib (`tp_l`, `tp_w`, `tp_clear`); its LEDs show through the floor window from the back (`tp_led_x`, `tp_win`); glue it only after this check, with the wires already soldered on the bare side.
8. Story dial: the KY-040 PCB stands in its floor recess with the bushing through the wall and the nut tightened in the notch (`enc_body_h`, `enc_cx`: measure the module, the PCB must not touch the HAT); the pins and their housings clear the Pico's left housings and the flex connector; shaft cut to the echoed length; the wheel pressed on its D turns freely in the notch, its groove over the pin, 11 detents between the stops; at the lower stop the 1 is under the arrow, else measure the offset, set `wheel_phase`, reprint the wheel.
9. `bump` only: the HAT underside clears the Pico (`hat_back_room`, 3 mm nominal, measure the tallest component on the back of the HAT over the Pico area); the cap slides into its hole from inside and its flange sits flat on the floor; the Dupont housings on the HAT header stay clear of the cap ceiling.

## Design directions

`concepts.scad` holds the visual-only concepts drawn before v8 on the real envelope, `concept = "grip" | "bump" | "slim"`, `view = "front" | "back"`. The thickness was the question: the Dupont housings plugged on the HAT header need 33 mm over a 5 x 51 mm strip on the right of the device, everything else fits in about 26 mm. **`bump` was chosen (2026-10-20) and is now the functional model above.** The other two stay for reference:

| ![Grip](img/concept_grip.png) |
|:--:|
| `grip`: thin body, vertical band on the right over the Dupont block, e-reader style. |

| ![Bump](img/concept_bump.png) |
|:--:|
| `bump`: thin body, rounded bump on the back over the Dupont block only. In the functional model the bump is a separate cap, since a bump printed with the shell would need supports. |

| ![Slim](img/concept_slim.png) |
|:--:|
| `slim`: thin all over, needs the HAT wired flat: a 2x20 right-angle male header plugged into the HAT's female header lays the Dupont housings flat (about 14 mm instead of 18), wires soldered on a straight header give about 11.5 mm. Still an option to shrink the cap later. |

Mode indication (decided 2026-10-21): the firmware draws a robot pictogram in the top-left corner of every AI page, where the story mode draws its page counter, so that the panel itself tells the mode, even asleep or off (`img/robot_badge.png`); the status LED keeps the mode bursts of the Pi version. A second LED for the AI mode was tried beside the white sphere and dropped: `img/led_options.png` compares the placements (A: 10 mm green sphere beside the white one, B: 5 mm green LED beside it, C: green sphere at the right end of the chin with the switch moved, D: badge only). A 5 mm LED at X 31 fits, but a second dLUX sphere there needs a 12.9 mm tube that reaches Y 14 in the chin and blocks the Dupont housing on Pico pin 4 (button A); option C has no room unless the switch moves into the corner boss. Only a 5 mm LED, or moving the four button wires two pins up the Pico (GP4 to GP7), would bring it back.

**v9 (2026-10-23)**: story dial on the left edge and the second sphere back. Drawn first as a concept, `concept_v9.scad`, visual only on the v8 bump body, then built into the functional model above. The concept pictures:

| ![v9 front](img/concept_v9_front.png) | ![v9 left edge](img/concept_v9_left.png) | ![v9 wheel](img/concept_v9_wheel.png) |
|:--:|:--:|:--:|
| Two spheres in the chin: white status left, green AI mode right, the switch gone from the bottom edge | Left edge: thumbwheel in a notch, ON / OFF switch under it | The number in use faces the viewer, under an arrow on the bezel margin |

- **Story dial**: a KY-040 encoder standing behind the left wall, its threaded bushing through the wall. Outside, a thumbwheel half sunk in a notch of the edge, the numbers 1 to 12 upright on its rim 18 degrees apart (one detent of a 20-detent encoder), an arrow on the bezel pointing at the one in use. A pin in an arc groove limits the travel to 11 detents: the end stops are the absolute reference of an otherwise incremental encoder, the firmware saturates its index at 1 and 12. In the functional model the wheel ended up 21 x 12 mm (the encoder PCB sets the axis height, the bushing and nut need the thickness), at Y 47 to stay clear of the HAT pad below and the panel flex above. Selection is blind by design (no scrolling on a 30 s panel) and nothing counts during the deep sleep, so the ritual is: wheel down to the stop, press the wheel (RESET, re-arms the count at 1), turn up to the number, GO.
- **Switch** moved to the left edge under the wheel, with its ON / OFF plate; the bottom edge keeps only the Pico USB. In the functional model it lies on the floor: at the rim, as before, its 6 mm body would have met the HAT PCB, the left margin being 4.5 mm where the chin was 12.
- **Second sphere** at the right end of the chin (X 95), where the switch was: no conflict with the Dupont housings, no button wire moves. White = activity, green = AI mode with the mode bursts.
- Firmware side: done behind the `STORY_DIAL` build option (`pico/README.md`), deep sleep kept as it is: the wheel is counted awake only, hence the re-arm by RESET. Counting asleep (dormant mode) stays a possible later step.

```bash
openscad -o img/concept_v9_front.png --imgsize=1000,900 --camera=-123,-186,316,56.65,74.6,13 --viewall --projection=p --colorscheme=Tomorrow concept_v9.scad
openscad -o img/concept_v9_left.png  --imgsize=1000,800 --camera=30,74,13,65,0,240,300 --projection=p --colorscheme=Tomorrow concept_v9.scad
openscad -o img/concept_v9_wheel.png --imgsize=800,800 --camera=6,60,13,0,0,0,60 --projection=o --colorscheme=Tomorrow concept_v9.scad
```

Render a concept (front and back views, same camera, the back view turns the model over; combined with ImageMagick):

```bash
openscad -D 'concept="grip"' -D 'view="front"' -o grip_front.png --imgsize=900,900 --camera=-123,-186,316,56.65,74.1,16.5 --viewall --projection=p --colorscheme=Tomorrow concepts.scad
openscad -D 'concept="grip"' -D 'view="back"'  -o grip_back.png  --imgsize=900,900 --camera=-123,-186,316,56.65,74.1,16.5 --viewall --projection=p --colorscheme=Tomorrow concepts.scad
magick grip_front.png grip_back.png +append -resize 1400x img/concept_grip.png
```

Pictures of the functional model:

```bash
openscad -o img/bump/assembly.png --imgsize=1000,1250 --camera=57,74,0,0,0,0,440 --projection=o --colorscheme=Tomorrow hadistory_case.scad
openscad -D 'part="back"' -o img/bump/back.png --imgsize=1000,1250 --camera=57,74,0,0,0,0,440 --projection=o --colorscheme=Tomorrow hadistory_case.scad
openscad -D 'part="closed"' -o img/bump/front.png --imgsize=1000,900 --camera=-123,-186,316,56.65,74.6,13 --viewall --projection=p --colorscheme=Tomorrow hadistory_case.scad
openscad -D 'part="closed"' -o img/bump/rear.png  --imgsize=1000,900 --camera=257,-186,-284,56.65,74.6,13 --viewall --projection=p --colorscheme=Tomorrow hadistory_case.scad
openscad -D 'part="closed"' -o img/bump/left.png  --imgsize=1000,800 --camera=30,74,13,65,0,240,300 --projection=p --colorscheme=Tomorrow hadistory_case.scad
openscad -D 'part="wheel"'  -o img/bump/wheel.png --imgsize=800,800 --camera=0,0,6,70,0,30,70 --projection=p --colorscheme=Tomorrow hadistory_case.scad
openscad -D 'part="section"' -o section.png --imgsize=1400,420 --camera=85.6,74.6,13,90,0,270,170 --projection=o --colorscheme=Tomorrow hadistory_case.scad
```
