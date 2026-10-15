# hadistory — enclosure for the Pico version

A two-part 3D printed case, portrait, with the Inky Impression 5.7" as the face and the electronics in the back shell. Design inspired by the [YouMakeTech Pico-GB](https://www.youmaketech.com/pico-gb-gameboy-emulator-handheld-for-raspberry-pi-pico/) bottom cover: every part is screwed or held in moulded pockets, and all the wiring is Dupont jumpers.

| ![Assembly](img/assembly.png) | ![Back shell](img/back.png) |
|:--:|:--:|
| Assembly, seen from the front (keys on top, LED bottom left) | Back shell, component pockets |

## Files

| File | Content |
|---|---|
| `hadistory_case.scad` | Parametric model (OpenSCAD). All dimensions are in the parameter block at the top. |
| `stl/hadistory_front.stl` | Bezel frame, exported face down ready to print |
| `stl/hadistory_back.stl` | Back shell, print floor down |
| `stl/hadistory_key.stl` | Push key for the HAT side buttons, print 4 |

Regenerate the STL files after a change:

```bash
cd case
openscad -D 'part="front"' -o stl/hadistory_front.stl hadistory_case.scad
openscad -D 'part="back"'  -o stl/hadistory_back.stl  hadistory_case.scad
openscad -D 'part="key"'   -o stl/hadistory_key.stl   hadistory_case.scad
```

OpenSCAD on macOS: `brew install --cask openscad@snapshot` (the stable cask is disabled). The `.scad` file also opens in FreeCAD.

## Outline

- Outer size 110 x 145 x 26 mm. The depth is set by the Dupont housings standing on the HAT's 40-pin header (20.8 mm measured from the glass).
- The HAT PCB sits in a pocket of the bezel and rests on a 1.2 mm lip of the back shell that runs all around the pocket, floor to PCB plane: the lip locates the PCB (0.3 mm clearance, `clear`) and the screws clamp it between lip and bezel. The lip is interrupted where the Pico reaches the bottom wall and notched for the LED wires. No screw goes into the HAT.
- The two parts are joined by 4 M2 self-tapping screws from the front, countersunk, into the back shell bosses. The bosses sit in a 3 mm margin around the PCB (8 mm at the bottom, the "chin").
- Side buttons of the HAT are on the top edge: four slots through the wall take the printed keys, which push the HAT's own tactile buttons.
- Bottom wall: Pico micro-USB only (centre): it flashes, gives the console and charges the battery through the TP4056. Right wall: slide switch knob, just above the end of the HAT header.
- Chin (12 mm): the Adafruit dLUX-dLITE sphere LED, 9.9 mm, seats in a conical hole of the bezel, bottom left, guided by a tube in the back shell that is open towards the inside below the sphere so that the leads and wires run to the Pico; about a third of the sphere shows outside.

## Electronics inside the back shell

| Part | Where | Fixing |
|---|---|---|
| Pico 2 W, right-angle headers on both long sides | Lower middle, USB towards the bottom wall, 20 mm kept free on each side for the Dupont housings | 4 M2 screws on 2.5 mm standoffs |
| TP4056 (IN+ IN- B+ B-), its micro-USB unused | Bottom right | Ribs on three sides |
| 103450 LiPo 2000 mAh | Across the top of the shell, above the HAT header | Ribs, add a piece of foam |
| SS12F15 slide switch | Right wall, between the HAT header and the battery | Pocket, knob through the slot |
| dLUX-dLITE sphere LED (Adafruit 5431) + 220 ohm or more | Chin, bottom left | Tube in the shell, conical hole in the bezel |

Power wiring (the HAT's 5V pin is not needed: on this HAT it only feeds the small expansion header, the panel runs from 3V3):

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

PLA or PETG, 0.4 mm nozzle, 0.2 mm layers, 3 walls. The model keeps heights on multiples of 0.2 mm and thin features on multiples of 0.4 mm: when adjusting a parameter, move by at least one layer (0.2 mm) or one line (0.4 mm), smaller changes are not reproduced. Front frame face down (the countersinks print as overhangs, fine at 45 degrees). Back shell floor down, no supports needed: the wall openings are small. Keys flat on their head.

## First print checklist

The model is a first version drawn from the Pimoroni drawing and a few measurements; expect to adjust:

1. HAT pocket: the PCB must drop in with little play (`clear`), the glass flush with the bezel window. Vertical play between lip and bezel is taken up by a strip of thin adhesive foam on the lip rather than by 0.1 mm tweaks, which the printer cannot reproduce; `hat_pcb_t` only moves by 0.2 mm steps.
2. Key slots: the keys must slide freely and reach the button caps (`btn_protrusion`, measure it).
3. Pico standoffs: the right-angle pins must clear the floor; raise the standoffs if not.
4. USB openings: centred on the connectors; move `pico_pos()` / `tp_pos()` if needed.
5. Dupont housings: nothing may touch the floor once closed; `cavity_t` gives 1 mm of margin.
