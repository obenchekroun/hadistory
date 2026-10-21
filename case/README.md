# hadistory — enclosure for the Pico version

A 3D printed case, portrait, with the Inky Impression 5.7" as the face and the electronics in the back shell: back shell, bezel, a cap over the HAT wiring, four push keys and a label plate. Design inspired by the [YouMakeTech Pico-GB](https://www.youmaketech.com/pico-gb-gameboy-emulator-handheld-for-raspberry-pi-pico/) bottom cover: every part sits in a moulded pocket or on standoffs, and all the wiring is Dupont jumpers.

Two styles in the same model, `style` at the top of the file. **`bump`** (v8, the default): 26 mm body, 10 mm corners, chamfered back edge, rounded bezel edge, and a pillow-shaped cap on the back over the HAT Dupont block, the only place that needs the depth. **`box`** (v7): the rectangular shell, 33 mm deep everywhere, validated by the test prints. The insides are the same.

| ![Bump, front](img/bump/front.png) | ![Bump, back](img/bump/rear.png) |
|:--:|:--:|
| `bump`: front, keys on the top edge | `bump`: the cap over the HAT Dupont block |

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
| `stl/bump/plate.stl` | Label plate for the keys, glued on the top edge of the case, print flat text up (same part in both folders) |
| `stl/bump/swplate_on_left.stl`, `swplate_on_right.stl` | ON / OFF plate for the switch, glued on the bottom edge; print the one matching the wiring (same parts in both folders) |
| `stl/box/back.stl` | `box` back shell (v7 plus the TP4056 posts and slot), print floor down |
| `stl/box/front.stl` | `box` bezel, exported face down |
| `stl/box/key.stl` | `box` push key, print 4, flat on its bottom face |
| `stl/box/plate.stl` | Label plate, same as above |
| `stl/box/swplate_on_left.stl`, `swplate_on_right.stl` | ON / OFF plates, same as above |

Pictures: `img/bump/`, `img/box/`, and the concepts in `img/`.

Regenerate the STL files after a change:

```bash
cd case
for p in back front cap key plate; do openscad -D "part=\"$p\"" -o stl/bump/$p.stl hadistory_case.scad; done
for p in back front key plate; do openscad -D 'style="box"' -D "part=\"$p\"" -o stl/box/$p.stl hadistory_case.scad; done
for d in bump box; do
  openscad -D 'part="swplate"' -D 'sw_on_left=true'  -o stl/$d/swplate_on_left.stl  hadistory_case.scad
  openscad -D 'part="swplate"' -D 'sw_on_left=false' -o stl/$d/swplate_on_right.stl hadistory_case.scad
done
```

Other values of `part`: `assembly` (shell, cap, HAT and parts preview, bezel transparent), `closed` (all parts opaque, for pictures), `section` (assembly cut at `section_x`, to check the clearances: 85.6 goes through the HAT header and the cap, 50 through the Pico).

OpenSCAD on macOS: `brew install --cask openscad@snapshot` (the stable cask is disabled). The `.scad` file also opens in FreeCAD.

## How it goes together

The device is seen from the front in the model: X right, Y up, display towards the viewer. The Pimoroni drawing of the HAT is a front view with the components seen through (a mirrored version was tried in v2 and put the keys and the flex relief in the wrong place). Turned so that the buttons are on top: the 40-pin header and its Dupont block hang on the **right**, the panel flex is on the **left** edge, the four side buttons protrude from the **top** edge.

- **Depth** (`bump`): 26.2 mm in all, set by the Pico: floor 2, standoffs 8, Pico with its right-angle pins 8, then `hat_back_room` (3 mm, to measure: the clearance for the components on the back of the HAT PCB over the Pico, the header itself is elsewhere), the PCB 2.8 plus the 0.4 drop, bezel 2. The Dupont housings on the HAT header, 18 mm below the PCB, end 1 mm above the floor; the cap gives them 6 mm more plus the floor thickness, for plugging. In `box`, 33 mm: the housings plus 10 mm of slack everywhere.
- **Outline** (`bump`): 10 mm plan radius, 3 mm chamfer at 45 degrees on the back edge (prints floor down as is, where a fillet would not), and a 2 mm bead at 45 degrees inside along the floor-wall junction so that 2.1 mm of material stays behind the chamfer. The corner bosses move 6 mm in from the faces (3.2 in `box`), and the top margin grows to 7 mm so that the top bosses clear the PCB corners.
- **Cap** (`bump`): a 38 x 67 mm pillow, 6 mm proud of the back, 12 mm plan radius, sides leaning in at 45 degrees and a 3 mm rounded top edge, centred on the HAT header strip (X 83 to 88, Y 45 to 96). It goes in from inside through a hole in the floor (0.2 mm clearance), its 3 mm flange sits on the inner face of the floor and is glued there before the HAT goes in; the flange also makes it captive. Walls 2 mm, the inside closes at 45 degrees so that it prints flange down without support. Its footprint stops short of the TP4056 ribs below and the battery rib above.
- **Back shell**: walls 2 mm, floor 2 mm. The HAT rests on eight small pads, 8 mm wide, reaching 1.6 mm under the PCB edge, 3.2 mm below the rim (`shelf_drop` 0.4 mm: 1 mm on v3 was too much); each pad hangs from the wall on a 45 degree gusset and carries a short locating lip outside the PCB: 0.4 mm on the long sides (`clear`, 0.8 mm on v3 left too much play), 0.1 mm on the top and bottom edges (`clear_y`: at 0.4 the v5 still had 0.6 mm of play along the PCB length). Two pads per long side, two on the bottom edge, two on the top edge between the keys, clear of the protruding caps (v4 had none on top: the PCB could ride up to the key guides, a millimetre or more off the bottom lips, and shifted at each key press). Everything else in the margins stays open: screwdriver access to the Pico, wires everywhere. Margins 4.5 mm on the sides, 6 mm on top, 12 mm chin.
- **Front**: a 2 mm plate with the window, four countersunk M3 screws into the corner bosses, and the conical hole of the sphere LED. It covers the notches of the keys and of the switch knob at the rim, and holds the HAT down on its pads. In `bump` its outer edge is rounded (1.6 mm) and it prints face up, the countersinks and the LED cone being 45 degree features; `box` prints it face down.
- **Keys**: the HAT switches are on the back face of the PCB, at 19.8, 41.3, 62.7 and 84.2 mm from the flex edge (`btn_from_flex`); their caps protrude 1.5 mm from the top edge, below the PCB underside (v3 had the keys at the glass level, about 4 mm too high). Four guide blocks in the top margin, set back 2.4 mm from the PCB edge to clear the caps and hanging from the wall on a 45 degree gusset like the pads (no print support), carry a notch from 4.4 mm below the PCB underside up to the rim (`btn_cap_drop`), and a step for the 12 mm head of the key. The key is 7.2 mm tall and fills the notch, so it cannot ride up and reaches the cap wherever it sits in that range; its shaft stands 3 mm proud of the wall (`key_proud`, 1 mm on v4 was too short to find by feel). A key is dropped in from above before the HAT and is captive once the HAT and bezel are on. Its shaft and head are 0.6 mm narrower than their notch (`key_play`, 0.8 up to v6 felt loose with the 3 mm shaft outside), the head has 0.2 mm of play along the travel in its step (`key_head_play`) and the nub stops 0.3 mm short of the cap with the key fully out (`key_gap`): 0.5 mm of free travel before the click, where the v6 key had 1.1 mm. The cap protrusion (`btn_protrusion`, 1.5 mm) is still an estimate: any error on it adds to or subtracts from that free travel. The shell notches did not change: a new key fits a v6 shell.
- **Label plate**: a 92 x 8 x 1.2 mm plate glued on the outer face of the top wall, 1 mm behind the key notches, centred on the four keys. One word per key with a small pointer towards the key: left to right as seen from the front, BACK, MODE, RESET, NEXT (`plate_labels`; those are keys D, C, B, A, the firmware functions are in `pico/README.md`). Read with the display facing you and the top edge tilted towards you, or with the device lying display up and seen from beyond its top edge: the back of the case is up either way. Text and pointers stand 0.6 mm proud (`plate_relief`, a negative value engraves them instead); glue with cyanoacrylate or a dot of hot glue.
- **ON / OFF plate**: a 30 x 7 x 1.2 mm plate of the same construction, glued on the bottom wall just under the switch knob slot, ON on one side of the knob and OFF on the other. Which side is on depends on how the switch was wired, so there are two variants (`sw_on_left`): `swplate_on_left.stl` and `swplate_on_right.stl`, pick the one that matches. It reads with the display facing you and the bottom edge tilted towards you, display side up.
- **Switch**: SS12F15 standing against the bottom wall at the rim, knob through a notch in the wall, pins pointing inwards. Its two mounting ears slide down into two grooves between the wall and a small rib on each side; the grooves locate it in X and Y, its body rests on a short pad from the wall (`sw_pad_t`, 1.2 mm on a 45 degree gusset, top 0.1 mm above the nominal body underside so the ears float just off the groove bottoms) and the bezel holds it down. Up to v6 the switch hung from its ears alone and rocked in the grooves when the knob was pushed, jamming. No screws; placed from above before closing. Ear thickness and height are parameters (`sw_ear_t`, `sw_ear_h`), to check on the part.
- **Panel flex**: on the left edge, between the two left pads; nothing touches the PCB edge there.
- Outer size 113 x 148 x 33 mm.

## Electronics inside the back shell

| Part | Where | Fixing |
|---|---|---|
| Pico 2 W, right-angle headers on both long sides | Lower middle left, standing, USB towards the bottom wall through a 13 x 7 mm opening; 20 mm kept free on each side for the Dupont housings | 4 M2 screws on 8 mm standoffs of 4.4 mm (2 mm pilot holes), each with a flat on its outer face 1.6 mm from the hole over the top 3.2 mm, where the header plastic sits next to the holes (`pico_post_flat`, `pico_post_relief_h`); nothing above the screws |
| TP4056 (IN+ IN- B+ B-), its micro-USB unused | Against the left wall under the battery, USB end 1 mm from the wall, B+/B- end towards the battery | Two M2 posts, 3 mm high, under its mounting holes (`tp_holes`, to measure on the module); a 14 x 1.6 mm slot through the left wall at the level of its top face, in front of the charge / full LEDs (`tp_led_x`, to measure), to read them from outside |
| 103450 LiPo 2000 mAh | Across the top of the shell | Ribs with 1 mm clearance, 12 mm openings at both ends for the wires |
| SS12F15 slide switch | Bottom wall, right of the Pico USB, at the rim | Ears in two grooves, no screws |
| Mode LED, red 5 mm, + 220 ohm | Chin, beside the sphere (X 31) | Small tube in the shell, 8 mm outside: the LED drops in from above, its flange rests on a step 6.1 mm below the bezel face so that the dome stands 1.5 mm proud like the sphere, the legs go through a square slot into the open lower half where the wires are soldered; 5.4 mm hole in the bezel |
| dLUX-dLITE sphere LED (Adafruit 5431) + 220 ohm | Chin, bottom left | Tube in the shell (1.2 mm wall, 0.4 mm clear of the PCB edge: the 1.6 mm tube of v4 overlapped it), open towards the inside for the wires up to 8 mm above the floor, the opening closing with a 45 degree roof so that it prints without support, conical hole in the bezel |

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

Version 8 (`bump` style: new outline, depth and cap; TP4056 moved to the left wall on posts with an LED slot, in both styles; otherwise the insides of v7) after test prints of versions 1 to 6 of the box (v4: keys, LED, switch and Pico all good, the HAT had play towards the chin; v5: 0.6 mm of play left along the PCB length, keys too short to find by feel; v6: label plate, keys loose with 1.1 mm of free travel, switch rocking in its grooves). To check with the parts placed, before wiring:

1. HAT: drops onto the eight pads with little play (`clear`) in both directions, glass 0.4 mm below the rim (`shelf_drop`), buttons free above the top margin, flex free on the left edge.
2. Keys: centred on the button caps in X and covering them in depth (`btn_from_flex`, `btn_cap_drop`), slide in their notches, heads seated in the step, about 0.5 mm of free travel before the click (`key_gap`, `key_head_play`; more means `btn_protrusion` is overestimated, to measure on the HAT).
3. Pico: screwed on its standoffs, housings on the pins clear of the floor, USB centred in its opening (`pico_standoff`, `pico_pos()`).
4. Dupont housings on the HAT header: nothing touches the floor with the HAT in place (`dupont_slack`).
5. Switch: body in its pocket, knob sliding in the notch, bezel closes over it.
6. Screws: M3 bite in the bosses without splitting them.
7. Mode LED: drops into its tube and seats on the step, dome 1.5 mm proud of the bezel like the sphere (`led2_body_h`, `led2_flange_t`: 7.6 and 1.0 on a standard 5 mm LED, to check on the part); legs reachable through the opening towards the inside.
8. TP4056: its holes land on the two posts (`tp_holes`) and its LEDs show through the slot (`tp_led_x`); both are measured on the module, the defaults are guesses.
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

Mode indication, under discussion (2026-10-21): the firmware will draw a robot pictogram in the top-left corner of every AI page, where the story mode draws its page counter, so that the panel itself tells the mode, even asleep or off. Whether a second, green LED for the AI mode joins the white status sphere is open; `img/led_options.png` compares the placements (A: 10 mm green sphere beside the white one, B: 5 mm green LED beside it, C: green sphere at the right end of the chin with the switch moved, D: badge only), `img/robot_badge.png` is the pictogram. If a second LED comes, each LED answers one question only: white = activity (solid ready, breathing busy, fast blink error), green = AI mode (on in AI mode, blinking while the WiFi joins), and the mode bursts of the white LED go away.

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
openscad -D 'part="section"' -o section.png --imgsize=1400,420 --camera=85.6,74.6,13,90,0,270,170 --projection=o --colorscheme=Tomorrow hadistory_case.scad
```
