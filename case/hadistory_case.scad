// hadistory - enclosure for the Pico version
// v8 "bump" style: 10 mm corners, 3 mm chamfer on the back edge, 26 mm body and a separate
// pillow-shaped cap over the HAT Dupont block; the v7 "box" style is kept behind style = "box".
//
// Portrait tablet. The Inky Impression 5.7" HAT sits in the back shell, display up, on eight
// small pads one PCB thickness (plus a drop) below the rim; the front is a flat bezel plate
// screwed on top. Raspberry Pi Pico 2 W, TP4056 charger, 103450 LiPo, slide switch and the
// status sphere LED live in the back shell, all wired with Dupont jumpers plugged on the HAT's
// 40-pin header. (A second LED for the mode was tried beside the sphere, v8 drafts: anything
// bigger than a 5 mm LED there blocks the Dupont housings on the Pico's first pins; dropped.)
//
// Coordinates: X right, Y up, Z out of the display, device seen FROM THE FRONT.
// The Pimoroni drawing of the HAT is a front view with the components seen through; it is
// turned 90 degrees so that its left edge (buttons) is the top edge, see hat_pt().
// In this frame the HAT header and its Dupont block hang on the RIGHT, the panel flex is
// on the LEFT edge, the four side buttons are on the TOP edge (they protrude from the PCB).
//
// Back shell layout: Pico standing lower left of centre (USB through the bottom wall, 20 mm
// free on each long side for the Dupont housings on its right-angle pins), TP4056 glued
// components down along the left wall, USB end up, with a window in the floor under its LEDs,
// battery across the top, two sphere LEDs in the chin (white status left, green AI mode right),
// push keys at the junction on the top wall. v9: on the LEFT edge, a story dial (KY-040 encoder
// behind a notched wall, thumbwheel with the numbers 1 to 12 on its rim, printed end stops) and
// under it the slide switch, lying on the floor with its knob through the wall. v10, after the
// first print: the encoder's real lengths (12 mm bushing on a boss behind the wall), its holes
// along the floor edge with the pins towards the bottom edge, and the switch dropped in from the
// front into grooves through an opening of the wall covered by the ON / OFF plate.
//
// Printed parts:  front (bezel), back (shell), cap (bump style), key (x4), wheel (story dial),
//                 plate (key labels, glued on the top edge), swplate (ON / OFF, glued on the left edge over the switch opening),
//                 swbar (screwed behind the switch, holds it down)
// Render:  openscad -D 'part="back"' -o stl/bump/back.stl hadistory_case.scad   (-D 'style="box"' for the v7 box)
// Units: mm. Printing grid 0.4 mm nozzle / 0.2 mm layers: heights on multiples of 0.2,
// thin features on multiples of 0.4; smaller adjustments are not reproduced.

part = "assembly";   // "front" | "back" | "cap" | "key" | "wheel" | "plate" | "swplate" | "swbar" | "assembly" | "closed" (all parts, opaque, for pictures)
                     // | "section" (assembly cut at x = section_x, section_w thick, seen from +X)
section_x = 85.6;    // through the HAT header strip and the cap
section_w = 1000;    // slab thickness kept beyond section_x (e.g. 30 with section_x = 2: the left wall seen from inside)
// "bump": the body is as thin as the Pico allows (26 mm), the HAT Dupont block gets a separate cap on
// the back; big corners, chamfered back edge, rounded bezel edge. "box": the v7 rectangular shell,
// 33 mm deep everywhere. The insides (pads, key guides, switch seat, dial, LED tubes, keys) are the same.
style = "bump";

// ---------------------------------------------------------------- HAT (Pimoroni drawing)
hat_w          = 125.40;   // PCB, landscape frame of the drawing (buttons on its left edge)
hat_h          =  99.50;
hat_pcb_t      =   2.80;   // glass + PCB, measured
hat_dupont_t   =  20.80;   // glass to the top of Dupont housings plugged on the header, measured
disp_w         = 114.90;   // visible area, centred on the PCB
disp_h         =  85.80;
pi_x           =  37.20;   // Pi footprint origin from the PCB left / bottom edges of the drawing
pi_y           =  26.20;
pi_h           =  56.00;
hdr_len        =  51.00;   // 2 x 20 header along the top edge of the Pi footprint
hdr_wid        =   5.10;
hdr_x          = pi_x + 3.5 + 29 - hdr_len/2;
hdr_y          = pi_y + pi_h - 3.5 - hdr_wid/2;
// Side buttons on the drawing's left edge, centres from its bottom (flex) edge: D, C, B, A
// (drawing and product photo agree, confirmed roughly on the printed v3).
btn_from_flex  = [19.83, 41.28, 62.73, 84.18];
btn_n          = len(btn_from_flex);
btn_cap_w      =   4.0;
btn_protrusion =   1.5;    // cap beyond the PCB edge (to measure)
// The switches are on the BACK face of the PCB: the caps sit below the PCB underside, not at
// the glass level (v3 had the keys about 4 mm too high). The key notch starts btn_cap_drop
// below the PCB underside and runs up to the rim; the key fills it, so it presses the cap
// wherever it is in that range.
btn_cap_drop   =   4.4;    // bottom of the key notch below the PCB underside
btn_cap_h      =   2.8;    // cap height, preview only
fpc_x0         =  50.0;    // flex of the panel on the drawing's bottom edge (drawing and photo)
fpc_x1         =  76.0;

// ---------------------------------------------------------------- other parts, measured
pico_l         =  51.0;    // Pico 2 W lying flat, right-angle pins on both long sides
pico_w         =  21.0;
pico_t         =   8.0;    // with pins and micro-USB
pico_hole_dx   =  47.0;    // M2 holes
pico_hole_dy   =  11.4;
pico_standoff  =   8.0;    // raises the Pico so that the pins and housings clear the floor
pico_post_d    =   4.4;    // standoff diameter
pico_post_flat =   1.6;    // flat on the outer face of each standoff, this far from the hole centre: the
                           // pin row is 1.6 mm from the Pico edge, the hole 4.8, so the header plastic
                           // (2.54 wide) comes within 1.9 mm of the hole centre; 4.4 round was too wide
pico_post_relief_h = 3.2;  // height of that flat below the Pico; the rest of the standoff stays round
dupont_room    =  20.0;    // free space beside each row of pins for the housings
usb_cut_w      =  13.0;    // micro-USB opening
usb_cut_h      =   7.0;
tp_l           =  26.0;    // TP4056 (IN+ IN- B+ B-), its own USB unused; lies along the left wall (Y), USB end towards
tp_w           =  17.0;    // the buttons (v10, the user's placement), a long edge 1 mm off the wall
tp_pcb_t       =   1.0;
tp_usb_h       =   3.2;    // micro-USB shell above the PCB: the module lies components DOWN, resting on that shell
                           // and on a rib under its far end, so that its LEDs face the floor (to measure)
tp_rib_l       =   6.0;    // rib under the middle of the B+ B- edge (the lower end), between the pads
tp_lip_h       =   1.2;    // locating lip along its inner long edge, above the PCB level, and an end stop past the USB
tp_lip_t       =   1.2;    // end: with the left wall they set where the module is glued
tp_clear       =   0.2;
tp_led_x       =  15.0;    // charge / full LEDs, this far from the USB edge (measured 2026-10-21, about 15)
tp_win         = [3.0, 10.0];   // window through the floor under them, along the module and across it, centred on its width
bat_l          =  50.0;    // 103450 LiPo
bat_w          =  34.0;
bat_t          =  10.0;
bat_clear      =   1.0;    // per side, inside the ribs
sw_body_l      =  11.6;    // SS12F15 slide switch: body along the wall (Y)
sw_body_w      =   6.0;    // body depth into the case (X), knob face against the wall
sw_body_t      =   5.7;    // body height (Z)
sw_ear_len     =  19.4;    // overall length over the two mounting ears
sw_ear_t       =   0.5;    // ear sheet thickness; the ears are in the plane of the knob face
sw_ear_h       =   4.0;    // ear height (Z), to measure
sw_slot_l      =   8.0;    // opening through the left wall for the knob: this wide, from the knob up to the rim (v10),
sw_slot_h      =   3.4;    // the knob's travel slot being its lowest sw_slot_h; the ON / OFF plate covers the rest
sw_pad_t       =   1.2;    // pad under the body, so that the ears (centred on the body height) clear the floor
sw_y           =  21.5;    // switch centre along the left wall: above the corner boss, under the lower HAT pad (Y 32 to 40,
                           // 11 mm up); the ears end 0.2 mm short of an end block each side
sw_lip_t       =   1.6;    // lips over the ears, 0.4 off them, from the floor up to sw_lip_z: the ears slide down behind them
sw_lip_z       =  10.0;
sw_post_dy     =   6.8;    // post behind the body, this far up the wall from the switch centre (clear of the pins' housings),
sw_post_w      =   5.0;    // 5 x 5, top level with the body top; the swbar is screwed on it and overhangs the body (v10b)
sw_bar_t       =   1.6;
led_sphere_d   =   9.9;    // Adafruit dLUX-dLITE sphere LEDs (5431): white = status, green = AI mode
led_tube_t     =   1.2;    // LED tube wall; 1.6 on v4 made the tube overlap the PCB edge by 0.45 mm

// ---------------------------------------------------------------- story dial (v9): KY-040 behind the left wall
dial_y         =  76.0;    // wheel axis along Y: high enough for the Dupont housings on the module's header, 14 mm below
                           // the PCB's lower end, to clear the HAT's back module (to Y 44.4); the notch wall is then in the
                           // panel flex span (63.8 to 89.8), where it is thinned, and the TP4056 moved up to Y 92
dial_notch_d   =   4.5;    // notch in the left edge, from the outer face; the 2 mm wall behind it ends at X 6.5, just
                           // outside the HAT PCB (X 6.9), the wheel sinks into the notch, the nut hides in it
dial_notch_r   =   2.0;    // plan radius of the notch corners
enc_pcb_l      =  26.1;    // KY-040 (HW-040) module PCB, along Y behind the notch wall, encoder towards the wall, the
                           // right-angle header at one end: enc_pins_up false (v10c) puts it at the lower end, the five
                           // pins leaving towards the bottom edge and, the module being what it is, its two mounting
                           // holes along the floor edge (datasheet picture: PCB 26.1 x 18.6), which puts the axis at
                           // 11.6 and allows the 21 mm wheel; true turns it around, holes on the HAT side, axis 9.4,
                           // wheel 18.4 at most (v10b, when the dial sat lower: the housings had to stay clear of the
                           // HAT's back module)
enc_pins_up    = false;
enc_pcb_w      =  18.6;    // across (Z), standing in a recess of the floor, 1.2 mm under the HAT
enc_pcb_t      =   1.6;
enc_body       =  12.0;    // encoder body, square
enc_body_h     =   4.0;    // PCB face to the bushing base (measured on the first print: about 4)
enc_bush_d     =   7.0;    // M7 x 0.75 threaded bushing
enc_bush_h     =  12.0;    // bushing, base to end (measured: about 12, where v9 assumed 7)
enc_bush_out   =   3.4;    // bushing beyond the outer face of the notch wall, inside the wheel's bore (room for the nut, 2.3,
                           // if wheel_nut); the rest of its length goes through the wall and a boss behind it, the body
                           // sits on the boss
enc_shaft_d    =   6.0;    // D shaft, enc_shaft_h long beyond the bushing end; CUT to the length echo()ed below
enc_shaft_h    =  12.0;
enc_cx         =  17.0;    // encoder axis from the pin end of the PCB (9.1 from the other end; to measure)
enc_hole_edge  =   2.6;    // mounting holes (3.3 mm): centre to the near long edge of the PCB, the floor edge here (to measure)
enc_hole_dw    =   7.8;    // and to the axis, across the PCB (measured on the face plate printed earlier)
enc_hole_dl    = [-5.5, 8.5];   // along the PCB from the axis, + towards the pins (same plate: 14 apart, 1.5 off centre)
enc_floor_recess = 0.8;    // the PCB sinks this much into the floor
enc_recess_play  = [0.8, 2.0];   // recess beyond the PCB, each side across and along: the bushing and the screw set
                           // where the PCB is, the recess only has to take it (v10 print: 0.4 and 0.5 were too tight)
enc_boss_w     =  12.0;    // boss behind the notch wall around the bushing, from the floor, as wide as the body
enc_post_w     =   5.0;    // ribs from the floor under the holes, up to the PCB, M3 self-tapping screws from behind the
enc_post_pilot =   2.5;    // PCB. Holes along the floor: a rib under each. Holes on the HAT side (pins up): a rib only
                           // where it clears the encoder body, i.e. under the hole beyond the body (the other hole is
                           // only 5.5 mm from the axis); the nut on the bushing holds the rest
enc_rib_x0     =   7.5;    // ribs start here, clear of the panel flex folded over the PCB edge (X 6.3 to 7.1)
wheel_d        =  21.0;    // thumbwheel: bottom 1.1 mm above the back face with the axis at 11.6 (22.8 would be flush); the
                           // size the user found right for the detents, not to be reduced (18.4 max with enc_pins_up)
wheel_t        =  12.0;    // along X: 4 in the notch (nut recess and bushing end inside), 8 proud of the edge
wheel_nut      = false;    // true: the M7 nut on the bushing, hidden in a round recess of the wheel, the stop groove then
                           // opening into that recess (no room for a wall between them on the 18.4 wheel). false (v10b):
                           // no nut, the encoder is held by the boss, the hole through wall and boss and the PCB screw,
                           // and the stop groove is a closed ring in the wheel face, which hides the mechanism
wheel_nut_d    =  12.4;    // round recess for the M7 nut (10 AF, 11.5 across corners): the wheel turns around it
wheel_nut_h    =   3.0;
wheel_bore_d   =   6.1;    // D bore for the 6 mm D shaft, blind, 1 mm cap: same as the knob cap printed earlier
wheel_bore_flat=   1.7;    // flat this far from the axis (4.75 across from the flat to the round side)
wheel_phase    =   0;      // degrees: turns the numbers and the stop groove about the D flat, so that at a detent
                           // the 1 sits under the arrow; measure the offset on the first print, set, reprint
wheel_numbers  =  12;      // on the rim, one per firmware step, 1 at the lower stop
wheel_step     =  24;      // degrees between numbers = one firmware step. The KY-040 at hand has 30 detents a turn and half a
                           // quadrature cycle per detent: the firmware counts every second detent, two detents per number, 24
                           // degrees, 264 of travel. A 20-detent encoder would take 18 here and DIAL_DETENTS_PER_STEP 1 in the
                           // firmware (two detents per number would need 396 degrees); wheel_18.stl is exported for it
arrow_l        =   5.0;    // arrow engraved in the bezel margin, pointing at the number on top of the wheel: length, width, depth
arrow_w        =   4.8;
arrow_d        =   0.8;
wheel_num_sz   =   2.8;
wheel_num_d    =   0.6;    // engraved
stop_groove_r  = wheel_nut ? [6.4, 8.8] : [5.2, 9.2];   // stop pin on the notch wall, in an arc groove on the inner face of the
stop_pin_d     =   1.6;    // wheel: travel (wheel_numbers - 1) * wheel_step, the groove ends are the stops. Groove inner and
stop_pin_h     =   1.5;    // outer radii, the pin in the middle: no nut, 1.2 mm of wall to the bushing bore, 1.3 of rim outside
stop_groove_d  =   1.6;    // (v10d: the v10c groove was 2 mm wide, 0.2 a side around the pin, the play of the encoder in its
                           // recess kept the pin out of it, so it opens almost to the rim and the inner chamfer thins to 0.4);
                           // nut: open into the nut recess, 0.4 of rim and no chamfer on that face
stop_r         = (stop_groove_r[0] + stop_groove_r[1])/2;   // 7.2
stop_groove_w  = stop_groove_r[1] - stop_groove_r[0];       // 4.0

// ---------------------------------------------------------------- design parameters
wall           =   2.0;
bezel_t        =   2.0;
clear          =   0.4;    // clearance around the HAT PCB (per side); 0.8 on v3 left too much play
clear_y        =   0.1;    // lip gap on the top and bottom edges only: with 0.4 the v5 still had 0.6 mm of play along
                           // the PCB length (the sides are fine at 0.4). The pocket itself keeps clear.
hat_corner_r   =   1.0;
win_margin     =   1.0;
ear_side       =   4.5;    // margins between the PCB pocket and the walls
ear_top        = style == "bump" ? 7.0 : 6.0;   // room for the key guides behind the protruding buttons; 7 keeps
                                                 // the 10 mm corner bosses off the PCB corners
ear_bottom     =  12.0;    // chin: LED and switch
corner_r       = style == "bump" ? 10.0 : 4.0;   // plan radius of the outline
back_chamfer   = style == "bump" ?  3.0 : 0;     // 45 degree chamfer on the back edge (prints floor down as is)
bead           = style == "bump" ?  2.0 : 0;     // 45 degree bead inside, along the floor-wall junction, keeps
                                                 // 2.1 mm of material behind the chamfer (0.7 without)
bezel_edge_r   = style == "bump" ?  1.6 : 0;     // rounded front edge of the bezel (printed face up)
screw_pilot    =   2.6;    // M3 self-tapping, printed pilot hole
screw_d        =   3.0;
boss_d         =   6.4;
boss_inset     = style == "bump" ? 6.0 : 3.2;    // corner bosses, from the outer faces; 6 keeps 1.1 mm of wall
                                                 // outside the boss and 3 mm around the pilot hole at the diagonal
dupont_slack   =  10.0;    // box: extra depth to handle the Dupont housings on the HAT
hat_back_room  =   3.0;    // bump: clearance between the Pico top and the HAT underside, for the components on the
                           // back of the HAT PCB (to measure: the header itself is in the cap zone, not over the Pico)
hat_mod        = [4, 4, 10, 26, 4];   // module with a connector on the back of the HAT near its lower left corner (measured):
                           // [from the left PCB edge, from the bottom PCB edge, width along X, length along Y, height
                           // below the PCB]. 4 mm tall, 1 more than hat_back_room: nothing may stand under it, which
                           // is why the story dial sits above Y 44.4
// Cap over the HAT Dupont block (bump style): a pillow on the back, inserted from inside through a hole
// in the floor, its flange glued on the inner face of the floor. Centred on the header strip, which
// runs X 83 to 88, Y 45 to 96; stops short of the battery rib above.
bump_x0        =  66.6;    // footprint at the back face
bump_y0        =  36.0;
bump_w         =  38.0;
bump_len       =  67.0;
bump_r         =  12.0;    // plan radius of the footprint
bump_h         =   6.0;    // beyond the back face; the housings end 1 mm above the floor, this is the slack
bump_edge_r    =   3.0;    // rounding of the top edge
bump_top_in    =   3.0;    // sides lean in this much before the rounding: 45 degrees
cap_wall       =   2.0;
cap_flange     =   3.0;    // flange width around the hole, inside
cap_flange_t   =   1.2;
cap_clear      =   0.2;    // hole to cap, per side
shelf_drop     =   0.8;    // PCB underside this much lower than rim - hat_pcb_t (1.0 on v3 was too much, 0.4 on v7
                           // left the glass 0.6 proud on the right once the bezel was screwed)
pad_w          =   8.0;    // support pads along the PCB edges
pad_in         =   1.6;    // how far a pad reaches under the PCB
pad_t          =   3.0;    // pad thickness, plus a 45 degree gusset below
lip_t          =   1.2;    // locating lip on each pad, outside the PCB
key_w          =   8.0;    // key shaft width (X); its height runs from the notch bottom to the rim
key_head_w     =  12.0;
key_head_t     =   2.0;    // head pocket depth (Y)
key_proud      =   3.0;    // shaft beyond the outer face of the wall (1.0 on v4 was too short to find by feel)
key_play       =   0.6;    // side play of the key shaft and head in their notch (0.8 up to v6, loose)
key_head_play  =   0.2;    // head play along the travel in its step (0.8 up to v6)
key_gap        =   0.3;    // nub to cap with the key fully out; free travel = key_head_play + key_gap
                           // (1.1 mm up to v6, too much), plus the error on btn_protrusion
key_block_w    =  16.0;    // guide block width around each key
cap_room       =   2.4;    // key guide inner face set back from the PCB edge, beyond the button caps
// Label plate glued on the outer face of the top wall, behind the keys: one word per key with a
// small pointer towards it, text raised plate_relief (colour change at plate_t for two colours).
// Labels left to right as seen from the front, i.e. keys D, C, B, A (btn_from_flex order).
plate_labels   = ["BACK", "MODE", "RESET", "NEXT"];
plate_w        =  92.0;    // centred on the four keys; the top wall is flat from X 4 to 109.3
plate_h        =   8.0;    // along the depth of the case (Z): pointer and word
plate_t        =   1.2;    // plate body
plate_relief   =   0.6;    // text and pointers above the body (negative: engraved into it)
plate_gap      =   1.0;    // between the plate and the key notches
plate_font     = "Liberation Sans:style=Bold";   // bundled with OpenSCAD
plate_word_sz  =   4.2;    // "RESET" is then about 15 mm wide for a 21.5 mm key pitch; stems about 0.6 mm
// ON / OFF plate glued on the bottom edge under the switch knob, same construction. Two variants,
// sw_on_left true or false, since which way is "on" depends on how the switch was wired.
sw_on_left     = true;
sw_plate_w     =  30.0;    // centred on the switch, over the wall opening (8 mm wide in the middle)
sw_plate_sz    =   4.0;
$fn            = 48;

// Derived
pocket_w = hat_h + 2*clear;                    // 100.3
pocket_h = hat_w + 2*clear;                    // 126.2
in_w     = pocket_w + 2*ear_side;              // 109.3
in_h     = pocket_h + ear_top + ear_bottom;    // 144.2
out_w    = in_w + 2*wall;                      // 113.3
out_h    = in_h + 2*wall;                      // 148.2
cavity_t = style == "bump" ? pico_standoff + pico_t + hat_back_room + hat_pcb_t + shelf_drop   // 22.6: floor top to rim,
                           : hat_dupont_t - hat_pcb_t + 1 + dupont_slack;   // the Pico sets it, the HAT Dupont block goes into the cap; box: 29
rim_z    = wall + cavity_t;                    // 24.6 | 31
shelf_z  = rim_z - hat_pcb_t - shelf_drop;     // 21.0 | 27.4: PCB underside
key_z0   = shelf_z - btn_cap_drop;             // 16.6 | 23.0: bottom of the key notches
key_h    = rim_z - key_z0 - 0.4;               // 7.6: key height, under the bezel
pcb_x0   = wall + ear_side + clear;            // PCB bottom-left corner
pcb_y0   = wall + ear_bottom + clear;
pcb_x1   = pcb_x0 + hat_h;
pcb_y1   = pcb_y0 + hat_w;

// Drawing point (front view, x right, y up, buttons on the left) -> this frame: turned 90
// degrees clockwise so that the left edge becomes the top edge. Relative to the PCB corner.
function hat_pt(x, y) = [y, hat_w - x];

module rounded_rect(w, h, r) { translate([r, r]) offset(r = r) square([w - 2*r, h - 2*r]); }
function bosses() = [[boss_inset, boss_inset], [out_w - boss_inset, boss_inset],
                     [boss_inset, out_h - boss_inset], [out_w - boss_inset, out_h - boss_inset]];
function btn_x(i) = pcb_x0 + hat_pt(0, btn_from_flex[i])[0];   // key centres along X, from the flex (left) edge
// LED as low in the chin as its bore allows inside the wall; the tube then stays 0.4 clear of the PCB
function led_pos() = [18, pcb_y0 - 0.4 - (led_sphere_d + 0.6)/2 - led_tube_t];   // y 7.55
function btn_mid(i) = (btn_x(i) + btn_x(i + 1))/2;                               // between two keys

// ---------------------------------------------------------------- component positions
function pico_pos() = [40, wall + 1.5];                      // bottom-left corner, 21 wide x 51 tall
function pico_holes() = let(p = pico_pos(), ox = (pico_w - pico_hole_dy)/2, oy = (pico_l - pico_hole_dx)/2)
    [[p[0] + ox, p[1] + oy], [p[0] + ox + pico_hole_dy, p[1] + oy],
     [p[0] + ox, p[1] + oy + pico_hole_dx], [p[0] + ox + pico_hole_dy, p[1] + oy + pico_hole_dx]];
function tp_pos()  = [wall + 1.0, 92];                       // bottom-left corner: 17 along X from 1 mm off the left wall,
                                                             // 26 along Y, above the dial's notch wall (to 91.5), beside the battery
function bat_pos() = [26, out_h - wall - 4 - bat_w];         // lying across the top
function led2_pos() = [95, led_pos()[1]];                                           // green sphere, right end of the chin
function led_positions() = [led_pos(), led2_pos()];
enc_z0   = wall - enc_floor_recess;                  // 1.2: encoder PCB bottom edge, in the floor recess; top edge 19.8
enc_hole_z = enc_pins_up ? enc_z0 + enc_pcb_w - enc_hole_edge : enc_z0 + enc_hole_edge;   // 17.2 | 3.8
dial_z   = enc_pins_up ? enc_hole_z - enc_hole_dw : enc_hole_z + enc_hole_dw;             // 9.4 | 11.6: dial axis height
notch_l  = wheel_d + 6;                              // 24.4: notch length along the edge
function enc_boss_x1() = dial_notch_d + enc_bush_h - enc_bush_out;   // 13.1: face of the boss, the encoder body against it
function enc_x()  = enc_boss_x1() + enc_body_h;         // 17.1: wall-side face of the encoder PCB
function enc_y0() = enc_pins_up ? dial_y + enc_cx - enc_pcb_l : dial_y - enc_cx;   // 45.5: PCB lower end
function enc_y1() = enc_y0() + enc_pcb_l;               // 71.6
function hat_mod_box() = [[pcb_x0 + hat_mod[0], pcb_y0 + hat_mod[1], shelf_z - hat_mod[4]], [hat_mod[2], hat_mod[3], hat_mod[4]]];
function enc_housings_y() = enc_pins_up ? [enc_y1(), enc_y1() + 14] : [enc_y0() - 14, enc_y0()];   // Dupont housings on the header
function sw_slot_z0() = wall + sw_pad_t + (sw_body_t - sw_slot_h)/2;   // 4.35: knob slot bottom
sw_plate_h = rim_z - 0.2 - sw_slot_z0() - sw_slot_h - plate_gap;    // 15.65 (bump): ON / OFF plate from plate_gap above the knob slot to the rim
function hdr_strip() = let(p = hat_pt(hdr_x, hdr_y + hdr_wid/2))
    [pcb_x0 + p[0] - hdr_wid/2, pcb_y0 + p[1] - hdr_len, hdr_wid, hdr_len];
function fpc_y() = [pcb_y0 + hat_pt(fpc_x1, 0)[1], pcb_y0 + hat_pt(fpc_x0, 0)[1]];   // flex span on the left edge
// Pads: [edge, position along the edge]; edges: "L" "R" "B" "T". On top, between the keys, clear of
// the protruding caps (v4 had none there: the PCB could ride up to the key guides, 1 mm or more off the
// bottom lips, and shift at each key press)
function pads() = [["L", 36], ["L", 112], ["R", 28], ["R", 112], ["B", 30], ["B", 68],   // clear of the LED, Pico screws, switch
                   // (the lower left pad at 36, not 28: there it would be over the switch, which drops in from the front)
                   ["T", btn_mid(0)], ["T", btn_mid(2)]];

// ---------------------------------------------------------------- front plate
// Outline with a rounded top edge (radius r, quarter circle in 6 steps), from z = 0 to h
module rounded_top(h, r) {
    if (r <= 0) linear_extrude(h) rounded_rect(out_w, out_h, corner_r);
    else hull() {
        linear_extrude(h - r) rounded_rect(out_w, out_h, corner_r);
        for (a = [15 : 15 : 90]) translate([0, 0, h - r + r*sin(a) - 0.01])
            linear_extrude(0.01) offset(delta = -r*(1 - cos(a))) rounded_rect(out_w, out_h, corner_r);
    }
}
// Outer envelope of the shell: the back edge chamfered at 45 degrees over back_chamfer
module envelope(h) {
    if (back_chamfer <= 0) linear_extrude(h) rounded_rect(out_w, out_h, corner_r);
    else hull() {
        linear_extrude(0.01) offset(delta = -back_chamfer) rounded_rect(out_w, out_h, corner_r);
        translate([0, 0, back_chamfer]) linear_extrude(h - back_chamfer) rounded_rect(out_w, out_h, corner_r);
    }
}
// Inner cavity, with the 45 degree bead along the floor
module cavity() {
    translate([wall, wall, wall]) hull() {
        linear_extrude(0.01) offset(delta = -bead) rounded_rect(in_w, in_h, corner_r - wall);
        translate([0, 0, bead]) linear_extrude(rim_z) rounded_rect(in_w, in_h, corner_r - wall);
    }
}

module front() {
    difference() {
        rounded_top(bezel_t, bezel_edge_r);
        translate([pcb_x0 + (hat_h - disp_h)/2 - win_margin, pcb_y0 + (hat_w - disp_w)/2 - win_margin, -1])
            linear_extrude(bezel_t + 2) rounded_rect(disp_h + 2*win_margin, disp_w + 2*win_margin, 1.5);
        for (b = bosses()) translate([b[0], b[1], -1]) {
            cylinder(d = screw_d + 0.4, h = bezel_t + 2);
            translate([0, 0, bezel_t + 1 - 1.6]) cylinder(d1 = screw_d + 0.4, d2 = 6.2, h = 1.6 + 0.01);
        }
        for (p = led_positions()) translate([p[0], p[1], -1])
            cylinder(d1 = led_sphere_d + 0.8, d2 = led_sphere_d - 0.5, h = bezel_t + 2);
        // Story dial: the notch, and the arrow pointing at the number on top of the wheel
        translate([0, 0, -1]) linear_extrude(bezel_t + 2) notch2d();
        translate([0, 0, bezel_t - arrow_d]) linear_extrude(1) polygon([[dial_notch_d + 1.0, dial_y], [dial_notch_d + 1.0 + arrow_l, dial_y + arrow_w/2], [dial_notch_d + 1.0 + arrow_l, dial_y - arrow_w/2]]);
    }
}

// ---------------------------------------------------------------- back shell
module back() {
  intersection() {
    envelope(rim_z);   // trims the corner bosses to the chamfer
    union() {
    stop_pin();        // added after the notch cut, it stands in the notch
    difference() {
        union() {
            difference() {   // walls and floor
                linear_extrude(rim_z) rounded_rect(out_w, out_h, corner_r);
                cavity();
            }
            for (b = bosses()) translate([b[0], b[1], 0]) cylinder(d = boss_d, h = rim_z);
            for (p = pads()) pad(p[0], p[1]);
            for (i = [0 : btn_n - 1]) key_guide(btn_x(i));
            pico_mounts();
            battery_ribs();
            tp_lip();
            switch_seat();
            notch_wall();
            enc_boss();
            enc_posts();
            for (p = led_positions()) led_tube(p);
        }
        for (b = bosses()) translate([b[0], b[1], 2]) cylinder(d = screw_pilot, h = rim_z);
        for (h = pico_holes()) translate([h[0], h[1], 1]) cylinder(d = 2.0, h = wall + pico_standoff + 1);
        // Pico micro-USB through the bottom wall, at the Pico PCB level
        translate([pico_pos()[0] + pico_w/2 - usb_cut_w/2, -1, wall + pico_standoff - 1]) cube([usb_cut_w, wall + 2, usb_cut_h]);
        // Switch opening through the left wall: the knob slot, open up to the rim so that the switch drops in from the front
        translate([-1, sw_y - sw_slot_l/2, sw_slot_z0()]) cube([wall + 2, sw_slot_l, rim_z]);
        translate([sw_post()[0] + sw_post_w/2, sw_post()[1] + sw_post_w/2, sw_post_top() - 7]) cylinder(d = screw_pilot, h = 8);   // swbar screw pilot
        // Story dial: notch in the left edge, bushing hole through its wall and the boss (teardrop: it prints lying),
        // floor recess under the encoder PCB
        translate([0, 0, -1]) linear_extrude(rim_z + 2) notch2d();
        translate([dial_notch_d - 1, dial_y, dial_z]) rotate([0, 90, 0]) linear_extrude(enc_bush_h - enc_bush_out + 1.01) teardrop2d(enc_bush_d + 0.4);
        translate([enc_x() - enc_recess_play[0], enc_y0() - enc_recess_play[1], wall - enc_floor_recess])
            cube([enc_pcb_t + 2*enc_recess_play[0], enc_pcb_l + 2*enc_recess_play[1], enc_floor_recess + 0.01]);
        for (h = enc_holes()) translate([enc_x() - 6.0, h[0], h[1]]) rotate([0, 90, 0]) cylinder(d = enc_post_pilot, h = 7);   // pilot holes in the posts
        // Key notches through the top wall and guides, from below the caps up to the rim, with the head step
        for (i = [0 : btn_n - 1]) {
            translate([btn_x(i) - key_w/2 - 0.2, pcb_y1 + cap_room - 1, key_z0 - 0.2]) cube([key_w + 0.4, ear_top + wall + 2, rim_z]);
            translate([btn_x(i) - key_head_w/2 - 0.2, pcb_y1 + cap_room - 0.01, key_z0 - 0.2]) cube([key_head_w + 0.4, key_head_t + 0.4, rim_z]);
        }
        // LED bores and wire exits of the tubes towards the inside
        for (p = led_positions()) {
            translate([p[0], p[1], wall]) cylinder(d = led_sphere_d + 0.6, h = rim_z);
            hull() {   // open towards the inside up to h, closing with a 45 degree roof above (no support, printed floor down)
                w = led_sphere_d + 4; h = shelf_z - led_sphere_d - wall - 1;
                translate([p[0] - w/2, p[1], wall]) cube([w, 6, h - wall]);          // 6 deep: opens the inner half of the tube
                translate([p[0] - 0.01, p[1], h + w/2 - 0.01]) cube([0.02, 6, 0.02]);
            }
        }
        // TP4056: window through the floor under its LEDs (module glued components down)
        tp_frame() translate([tp_led_x - tp_win[0]/2, tp_w/2 - tp_win[1]/2, -1]) cube([tp_win[0], tp_win[1], wall + 2]);
        // Hole in the floor for the cap over the HAT Dupont block
        if (style == "bump") translate([0, 0, -1]) linear_extrude(wall + 2) bump_fp(cap_clear);
    }
    }
  }
}

// ---------------------------------------------------------------- cap over the HAT Dupont block
// Footprint of the cap at the back face, offset by o (negative inwards)
module bump_fp(o) { translate([bump_x0 - o, bump_y0 - o]) rounded_rect(bump_w + 2*o, bump_len + 2*o, bump_r + o); }

// Modelled in place: dome outside the back face (z < 0), skirt through the floor, flange on the
// inner face of the floor. Hollow, 2 mm walls, the inside closing at 45 degrees so that it prints
// flange down without support: the housings sit under the flat part of the ceiling.
module cap() {
    d_side = bump_h - bump_edge_r;   // depth of the straight, leaning side
    difference() {
        union() {
            hull() {   // dome
                translate([0, 0, -0.01]) linear_extrude(0.01) bump_fp(0);
                for (a = [0 : 15 : 90]) translate([0, 0, -(d_side + bump_edge_r*sin(a))])
                    linear_extrude(0.01) bump_fp(-bump_top_in - bump_edge_r*(1 - cos(a)));
            }
            linear_extrude(wall) bump_fp(0);                                             // skirt, through the floor
            translate([0, 0, wall]) linear_extrude(cap_flange_t) bump_fp(cap_flange);  // flange, inside
        }
        hull() {   // inside: 45 degree ceiling, flat from bump_fp(-cap_wall - inner_d) inwards
            inner_d = bump_h - cap_wall;
            translate([0, 0, -inner_d]) linear_extrude(0.01) bump_fp(-cap_wall - 0.8 - inner_d);
            translate([0, 0, -0.01]) linear_extrude(wall + cap_flange_t + 0.03) bump_fp(-cap_wall - 0.8);
        }
    }
}

// Support pad under the PCB edge, cantilevered from the wall with a 45 degree gusset,
// plus a locating lip outside the PCB up to the rim
module pad(edge, pos) {
    reach = (edge == "B") ? ear_bottom + clear + pad_in     // from the wall inner face
          : (edge == "T") ? ear_top + clear + pad_in
          : ear_side + clear + pad_in;
    gap   = (edge == "B" || edge == "T") ? clear_y : clear;   // lip to PCB edge
    module shape() {   // in a local frame: X along the edge, Y from the wall inwards, Z up
        // pad
        translate([-pad_w/2, 0, shelf_z - pad_t]) cube([pad_w, reach, pad_t]);
        // gusset: 45 degrees under the pad
        hull() {
            translate([-pad_w/2, 0, shelf_z - pad_t - reach]) cube([pad_w, 0.01, 0.01]);
            translate([-pad_w/2, 0, shelf_z - pad_t - 0.01]) cube([pad_w, reach, 0.01]);
        }
        // locating lip, outside the PCB outline, up to the rim
        translate([-pad_w/2, reach - pad_in - gap - lip_t, shelf_z - pad_t]) cube([pad_w, lip_t, rim_z - shelf_z + pad_t]);
    }
    if (edge == "L") translate([wall, pos, 0]) rotate([0, 0, -90]) mirror([1, 0, 0]) shape();
    else if (edge == "R") translate([out_w - wall, pos, 0]) rotate([0, 0, 90]) shape();
    else if (edge == "T") translate([pos, out_h - wall, 0]) mirror([0, 1, 0]) shape();
    else translate([pos, wall, 0]) shape();
}

// Guide block for a key, in the top ear, set back from the PCB edge to clear the button cap
module key_guide(x) {
    y0 = pcb_y1 + cap_room; d = out_h - wall - y0; z0 = key_z0 - 2;   // 4 deep from the wall, 21.4 up
    translate([x - key_block_w/2, y0, z0]) cube([key_block_w, d, rim_z - z0]);
    // 45 degree gusset under the block, hanging from the wall: no support when printed upright
    hull() {
        translate([x - key_block_w/2, y0, z0 - 0.01]) cube([key_block_w, d, 0.01]);
        translate([x - key_block_w/2, y0 + d - 0.01, z0 - d]) cube([key_block_w, 0.01, 0.01]);
    }
}

// Pico standoffs: round, with a flat face towards the nearer long edge of the Pico over the top
// pico_post_relief_h, where the header plastic and pin tails sit next to the holes
module pico_mounts() {
    cx = pico_pos()[0] + pico_w/2; top = wall + pico_standoff;
    for (h = pico_holes()) {
        sgn = (h[0] < cx) ? -1 : 1;
        translate([h[0], h[1], 0]) difference() {
            cylinder(d = pico_post_d, h = top);
            translate([sgn > 0 ? pico_post_flat : -pico_post_flat - pico_post_d, -pico_post_d, top - pico_post_relief_h])
                cube([pico_post_d, 2*pico_post_d, pico_post_relief_h + 1]);
        }
    }
}

// Battery across the top: ribs with clearance, openings in both short sides for the wires
module battery_ribs() {
    b = bat_pos(); r = 1.6; c = bat_clear; h = wall + 5;
    difference() {
        union() {
            translate([b[0] - c - r, b[1] - c - r, 0]) cube([bat_l + 2*c + 2*r, r, h]);
            translate([b[0] - c - r, b[1] + bat_w + c, 0]) cube([bat_l + 2*c + 2*r, r, h]);
            translate([b[0] - c - r, b[1] - c - r, 0]) cube([r, bat_w + 2*c + 2*r, h]);
            translate([b[0] + bat_l + c, b[1] - c - r, 0]) cube([r, bat_w + 2*c + 2*r, h]);
        }
        for (x = [b[0] - c - r - 1, b[0] + bat_l + c - 1]) translate([x, b[1] + bat_w/2 - 6, wall]) cube([r + 2, 12, h]);
    }
}

// TP4056 seat: the module has no mounting holes and its LEDs are on the component side, so it lies
// components down, resting on its micro-USB shell at the wall end and on a short rib under the far
// edge, LEDs over a window in the floor. A lip along its lower long edge and an end stop beyond the
// end away from the wall, with the left wall and the battery rib, fix where it is glued.
// tp_frame(): the module's own frame, x along it from the USB edge, y across it from the wall side,
// z as the case; placed along the left wall with the USB end at the top (local x becomes -Y)
module tp_frame() {
    t = tp_pos();
    translate([t[0], t[1] + tp_l, 0]) rotate([0, 0, -90]) children();
}
module tp_lip() {
    z = wall + tp_usb_h;
    tp_frame() {
        translate([0, tp_w + tp_clear, 0]) cube([tp_l, tp_lip_t, z + tp_lip_h]);                       // lip along the inner long edge
        translate([tp_l - 1.6, tp_w/2 - tp_rib_l/2, 0]) cube([1.6, tp_rib_l, z]);                    // rib under the B+ B- edge
        translate([-tp_clear - 1.2, tp_w/2 - tp_rib_l/2, 0]) cube([1.2, tp_rib_l, z + tp_lip_h]);    // end stop past the USB end
    }
}

module led_tube(p) { translate([p[0], p[1], 0]) cylinder(d = led_sphere_d + 0.6 + 2*led_tube_t, h = rim_z); }

// Slide switch lying on the floor against the left wall (v9: the right end of the chin went to the green
// sphere), knob through the wall. v10: it drops in from the front, like the v7 cradle, through the wall
// opening (as wide as the knob slot, open up to the rim): the ears slide down between the wall and a lip
// at each end, an end block each side stops them along the wall, 0.2 mm each side, the body rests on its
// pad. The bezel covers the opening at the rim, the ON / OFF
// plate glued over it keeps the knob, hence the switch, from riding up. No glue on the switch, no screw;
// the pins and their housings are free behind the body.
function sw_post() = [wall + sw_ear_t + 0.2 + sw_body_w + 0.2, sw_y + sw_post_dy - sw_post_w/2];   // post corner (X, Y), 0.2 behind the body
function sw_post_top() = wall + sw_pad_t + sw_body_t + 0.1;                                          // 9.0: level with the body top
module switch_seat() {
    x0 = wall + sw_ear_t + 0.2;                       // knob face of the body
    z0 = wall + sw_pad_t;                             // body underside
    xl = wall + 0.2 + sw_ear_t + 0.4;                 // lips, 0.4 off the ears
    p = sw_post(); translate([p[0], p[1], 0]) cube([sw_post_w, sw_post_w, sw_post_top()]);   // post for the swbar screw
    translate([x0 - 0.2, sw_y - sw_body_l/2 - 0.2, 0]) cube([sw_body_w + 0.4, sw_body_l + 0.4, z0]);   // pad
    for (sgn = [-1, 1]) {
        ye = sw_y + sgn*(sw_ear_len/2 + 0.2);                                                           // ear end, plus play
        yl = sw_y + sgn*(sw_body_l/2 + 0.4);                                                            // lip, from 0.4 beyond the body
        translate([xl, min(ye, yl), 0]) cube([sw_lip_t, abs(ye - yl), sw_lip_z]);                       // lip over the ear run
        translate([wall - 0.01, min(ye, ye + sgn*1.6), 0]) cube([xl + sw_lip_t - wall + 0.01, 1.6, sw_lip_z]);   // end block
    }
}

// Bar screwed on the post behind the switch, overhanging the body's upper end by 3.7 mm, 0.1 above it: it
// keeps the switch from riding up (the ON / OFF plate did that alone in the first v10). Flat, in case XY.
module sw_bar() {
    p = sw_post(); xb = wall + 0.2 + sw_ear_t + 0.4 + sw_lip_t + 0.3;   // from 0.3 beyond the lips
    yb = sw_y + sw_body_l/2 - 4.8;                                      // 4.8 mm of body under the tongue
    difference() {
        linear_extrude(sw_bar_t) polygon([[xb, yb], [p[0] - 0.2, yb], [p[0] - 0.2, p[1]], [p[0] + sw_post_w, p[1]],
                                          [p[0] + sw_post_w, p[1] + sw_post_w], [xb, p[1] + sw_post_w]]);
        translate([p[0] + sw_post_w/2, p[1] + sw_post_w/2, -1]) cylinder(d = 3.2, h = sw_bar_t + 2);
    }
}
module sw_bar_in_place() { translate([0, 0, sw_post_top()]) sw_bar(); }

// ---------------------------------------------------------------- story dial
// Notch in the left edge (plan), through the shell and the bezel; the wheel sinks into it
module notch2d() { translate([-5, dial_y - notch_l/2]) rounded_rect(5 + dial_notch_d, notch_l, dial_notch_r); }
// The wall behind the notch, X 4.5 to 6.5, closing the notch on three sides (the outer wall is cut away)
// Where the notch wall runs into the span of the panel flex (folded over the PCB edge, X 6.3 to 7.1, from 4 mm
// under the PCB) it is thinned to 1.2 mm above Z 16, leaving 1.2 mm beyond the PCB edge for the fold
module notch_wall() {
    difference() {
        translate([wall - 0.01, dial_y - notch_l/2 - wall, 0]) cube([dial_notch_d + 0.01, notch_l + 2*wall, rim_z]);
        translate([wall + 3.7, fpc_y()[0] - 0.5, 16]) cube([dial_notch_d, out_h, rim_z]);
    }
}
// Boss behind the notch wall, from the floor, taking the bushing length that the wall and the nut do not:
// the encoder body sits on its face, the bushing hole runs through wall and boss
module enc_boss() {
    difference() {
        translate([dial_notch_d + wall - 0.01, dial_y - enc_boss_w/2, 0]) cube([enc_boss_x1() - dial_notch_d - wall + 0.01, enc_boss_w, dial_z + enc_body/2]);
        translate([wall + 3.7, fpc_y()[0] - 0.5, 16]) cube([enc_rib_x0 - wall - 3.7, out_h, rim_z]);   // flex fold room, as the notch wall
    }
}
// Hole section for a horizontal hole printed without support: a circle with a 45 degree roof (apex towards -X,
// which rotate([0, 90, 0]) turns into +Z)
module teardrop2d(d) { hull() { circle(d = d); translate([-d/2*sqrt(2), 0]) square(0.01, center = true); } }
// Two ribs from the floor under the mounting holes of the module (both along its floor edge), from the
// notch wall to the PCB, with the pilot holes; the screws come from behind the PCB
function enc_holes() = [for (dl = enc_hole_dl) [dial_y + (enc_pins_up ? dl : -dl), enc_hole_z]];
module enc_posts() {
    for (h = enc_holes()) {
        d = h[0] - dial_y;                                                   // hole along the PCB from the axis
        // rib span along Y: the hole plus enc_post_w/2 each side, kept 0.2 clear of the body when the holes are on its level
        lo = enc_pins_up ? max(abs(d) - enc_post_w/2, enc_body/2 + 0.2) : abs(d) - enc_post_w/2;
        hi = abs(d) + enc_post_w/2;
        if (hi - lo >= 3.6) translate([enc_rib_x0, dial_y + (d > 0 ? lo : -hi), 0]) cube([enc_x() - enc_rib_x0, hi - lo, h[1] + enc_post_w/2]);
    }
}
// Stop pin on the notch wall, at the bottom of the wheel, inside the arc groove of the wheel
module stop_pin() {
    translate([dial_notch_d - stop_pin_h, dial_y, dial_z - stop_r]) rotate([0, 90, 0]) cylinder(d = stop_pin_d, h = stop_pin_h + 0.5);
}
// Annulus sector, angles in degrees, counter-clockwise
module arc2d(r0, r1, a0, a1) {
    n = max(3, ceil((a1 - a0)/4));
    polygon(concat([for (i = [0 : n]) let(a = a0 + (a1 - a0)*i/n) [r1*cos(a), r1*sin(a)]],
                   [for (i = [n : -1 : 0]) let(a = a0 + (a1 - a0)*i/n) [r0*cos(a), r0*sin(a)]]));
}
// Thumbwheel, in its printing frame: axis Z, outer face on the bed (z = 0), inner face up (z = wheel_t)
// with the nut recess, the bushing clearance, the blind shaft bore and the arc groove all opening upwards.
// Numbers engraved on the rim, 1 on top (local -X) when the wheel is at the lower stop; the pin then sits
// at the groove end at local angle 0, the groove runs counter-clockwise to the upper stop.
// Placed in the case with rotate([0, 90, 0]) at X = dial_notch_d - 0.5 - wheel_t: local -X becomes +Z.
stop_half = asin(stop_pin_d/2/stop_r) + 2;                 // half pin angle plus 0.3 mm of clearance
wheel_bush_z = (dial_notch_d - enc_bush_out) - (dial_notch_d - 0.5 - wheel_t);   // 9.1: bushing end, in the wheel frame
shaft_keep   = wheel_bush_z - 1.0 - 0.3;                   // 7.8: shaft to keep beyond the bushing end, 0.3 short of the bore's cap
module wheel() {
    r = wheel_d/2; t = wheel_t; travel = (wheel_numbers - 1)*wheel_step; in_ch = wheel_nut ? 0 : 0.4;
    difference() {
        union() {
            translate([0, 0, 0.8]) cylinder(r = r, h = t - 0.8 - in_ch);
            cylinder(r1 = r - 0.8, r2 = r, h = 0.8);   // chamfer on the outer face
            if (in_ch > 0) translate([0, 0, t - in_ch]) cylinder(r1 = r, r2 = r - in_ch, h = in_ch);   // and a smaller one on the inner face, the rim there is 1.3 mm
        }
        if (wheel_nut) translate([0, 0, t - wheel_nut_h]) cylinder(d = wheel_nut_d, h = wheel_nut_h + 1);  // nut recess
        translate([0, 0, wheel_bush_z - 0.7]) cylinder(d = enc_bush_d + 1.0, h = t);                       // bushing clearance
        translate([0, 0, 1.0]) linear_extrude(t) difference() {                                            // D shaft bore, 1 mm cap
            circle(d = wheel_bore_d);
            translate([-wheel_bore_d, -wheel_bore_d - wheel_bore_flat]) square([2*wheel_bore_d, wheel_bore_d]);   // flat towards local -Y
        }
        rotate([0, 0, wheel_phase]) {   // markings and groove, set relative to the D flat
            translate([0, 0, t - stop_groove_d]) linear_extrude(stop_groove_d + 1)
                arc2d(stop_r - stop_groove_w/2, stop_r + stop_groove_w/2, -stop_half, travel + stop_half);  // stop groove
            for (i = [1 : wheel_numbers]) rotate([0, 0, (i - 1)*wheel_step])
                translate([-(r - wheel_num_d), 0, t/2]) rotate([0, -90, 0]) linear_extrude(wheel_num_d + 0.2)
                    text(str(i), size = wheel_num_sz, font = plate_font, halign = "center", valign = "center");
        }
    }
}
module wheel_in_place() { translate([dial_notch_d - 0.5 - wheel_t, dial_y, dial_z]) rotate([0, 90, 0]) wheel(); }
// KY-040 module and the switch, for the previews
module encoder_preview() {
    color("#1f5fa8") translate([enc_x(), enc_y0(), wall - enc_floor_recess]) cube([enc_pcb_t, enc_pcb_l, enc_pcb_w]);
    color("Silver") translate([enc_boss_x1(), dial_y - enc_body/2, dial_z - enc_body/2]) cube([enc_body_h, enc_body, enc_body]);
    color("Silver") translate([dial_notch_d - enc_bush_out, dial_y, dial_z]) rotate([0, 90, 0]) cylinder(d = enc_bush_d, h = enc_bush_h);
    if (wheel_nut) color("Silver") translate([dial_notch_d - 2.3, dial_y, dial_z]) rotate([0, 90, 0]) cylinder(d = 11.5, h = 2.3, $fn = 6);   // nut
    color("Gainsboro") translate([dial_notch_d - enc_bush_out - shaft_keep, dial_y, dial_z]) rotate([0, 90, 0]) cylinder(d = enc_shaft_d, h = shaft_keep);   // shaft, cut
    for (i = [0 : 4]) color("Black")                                       // right-angle header: pins and housings along the PCB, away from it
        translate([enc_x() - 2.6, enc_pins_up ? enc_y1() : enc_y0() - 14, enc_pins_up ? enc_z0 + 1.3 + 2.54*i - 1.2 : enc_z0 + enc_pcb_w - 1.3 - 2.54*i - 1.2]) cube([2.6, 14, 2.4]);
}
module switch_preview() {
    color("White") translate([wall + sw_ear_t + 0.2, sw_y - sw_body_l/2, wall + sw_pad_t]) cube([sw_body_w, sw_body_l, sw_body_t]);
    color("Silver") translate([wall + 0.2, sw_y - sw_ear_len/2, wall + sw_pad_t + sw_body_t/2 - sw_ear_h/2]) cube([sw_ear_t, sw_ear_len, sw_ear_h]);
    color("Gainsboro") translate([-1.5, sw_y - 1.3, sw_slot_z0() + 0.4]) cube([wall + 2, 2.6, 2.6]);   // knob
}
// ON / OFF plate glued on the left edge over the switch opening, above the knob slot, text facing out and reading with the device face up
module sw_plate_in_place() {
    translate([0, sw_y + sw_plate_w/2, sw_plate_z0()]) multmatrix([[0, 0, -1, 0], [-1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 1]]) sw_plate();
}

// ---------------------------------------------------------------- push key
// Shaft through the guide and the wall, key_proud outside; head in the step on the inner face of
// the guide; nub towards the cap. Full height from the notch bottom to the bezel, so that it
// cannot ride up and reaches the cap wherever it sits below the PCB. Printed as modelled.
// Modelled with the key fully out (head against the outer end of its step), y from the tip.
function key_shaft_w() = key_w + 0.4 - key_play;   // in a key_w + 0.4 notch
module key() {
    w       = key_shaft_w();
    hw      = key_head_w + 0.4 - key_play;
    step    = key_head_t + 0.4;                              // depth of the head step cut in the guide
    head_t  = step - key_head_play;
    tip_to_guide = key_proud + out_h - (pcb_y1 + cap_room);  // tip to the guide inner face
    shaft_l = tip_to_guide - step;
    nub     = cap_room - btn_protrusion - key_gap + key_head_play;   // head inner face to key_gap short of the cap
    cube([w, shaft_l, key_h]);
    translate([-(hw - w)/2, shaft_l, 0]) cube([hw, head_t, key_h]);          // head
    translate([(w - 4)/2, shaft_l + head_t, 0]) cube([4, max(0.4, nub), key_h]);   // nub
}

// ---------------------------------------------------------------- previews
// Label plate, flat, text up (local frame: X along the case, Y towards the back of the case
// = text up, Z out of the wall). Glued on the top wall with its lower edge plate_gap behind the
// key notches: seen from the top edge with the display facing you, the words sit right behind
// their keys and read with the back of the case up.
// A label plate w x h with the 2D marks() children raised (or engraved) on it
module label_plate(w, h) {
    difference() {
        linear_extrude(plate_t) rounded_rect(w, h, 1.5);
        if (plate_relief < 0) translate([0, 0, plate_t + plate_relief]) linear_extrude(1) children();
    }
    if (plate_relief > 0) translate([0, 0, plate_t]) linear_extrude(plate_relief) children();
}
module plate() {
    label_plate(plate_w, plate_h) for (i = [0 : btn_n - 1]) translate([btn_x(i) - plate_x0(), 0]) {
        polygon([[-1.6, 2.6], [1.6, 2.6], [0, 0.6]]);                                // pointer towards the key
        translate([0, 3.6]) text(plate_labels[i], size = plate_word_sz, font = plate_font,
                                 halign = "center", valign = "baseline");
    }
}
// ON / OFF plate: glued on the left wall over the switch opening, from plate_gap above the knob slot
// to the rim; it reads with the device face up, i.e. local Y = +Z of the case.
module sw_plate(on_left = sw_on_left) {
    label_plate(sw_plate_w, sw_plate_h) for (p = [[-9, on_left ? "ON" : "OFF"], [9, on_left ? "OFF" : "ON"]])
        translate([sw_plate_w/2 + p[0], 1.6]) text(p[1], size = sw_plate_sz, font = plate_font,
                                                   halign = "center", valign = "baseline");
}
function sw_plate_z0() = sw_slot_z0() + sw_slot_h + plate_gap;   // above the knob slot, on the left edge
function plate_x0() = (btn_x(0) + btn_x(btn_n - 1))/2 - plate_w/2;   // case X of the plate's left edge, centred on the keys

module hat() {
    color("DarkSlateGray") translate([pcb_x0, pcb_y0, shelf_z]) linear_extrude(hat_pcb_t) rounded_rect(hat_h, hat_w, 1);
    color("Black") translate([pcb_x0 + (hat_h - disp_h)/2, pcb_y0 + (hat_w - disp_w)/2, shelf_z + hat_pcb_t - 0.3]) cube([disp_h, disp_w, 0.3]);
    s = hdr_strip();
    color("Gray") translate([s[0], s[1], shelf_z - 5.3]) cube([s[2], s[3], 5.3]);
    color("Silver", 0.5) translate([s[0], s[1], shelf_z - (hat_dupont_t - hat_pcb_t)]) cube([s[2], s[3], hat_dupont_t - hat_pcb_t]);
    for (i = [0 : btn_n - 1]) color("Red") translate([btn_x(i) - btn_cap_w/2, pcb_y1, shelf_z - 0.6 - btn_cap_h]) cube([btn_cap_w, btn_protrusion, btn_cap_h]);
    f = fpc_y(); color("Orange", 0.6) translate([pcb_x0 - 0.6, f[0], shelf_z - 4]) cube([0.8, f[1] - f[0], 4 + hat_pcb_t]);
    m = hat_mod_box(); color("Crimson", 0.7) translate(m[0]) cube(m[1]);   // module on the back of the HAT, lower left
}
module parts_preview() {
    pp = pico_pos(); color("Green") translate([pp[0], pp[1], wall + pico_standoff]) cube([pico_w, pico_l, pico_t]);
    color("Silver", 0.4) translate([pp[0] - dupont_room, pp[1] + 2, wall + pico_standoff]) cube([dupont_room, pico_l - 4, 6]);
    color("Silver", 0.4) translate([pp[0] + pico_w, pp[1] + 2, wall + pico_standoff]) cube([dupont_room, pico_l - 4, 6]);
    b = bat_pos();   color("Gold")  translate([b[0], b[1], wall]) cube([bat_l, bat_w, bat_t]);
    tp_frame() {
        color("Blue") translate([0, 0, wall + tp_usb_h]) cube([tp_l, tp_w, tp_pcb_t]);                              // PCB, components down
        color("Silver") translate([0, tp_w/2 - 3.8, wall]) cube([6, 7.6, tp_usb_h]);                                 // micro-USB shell on the floor
        for (c = [["Red", -2], ["DodgerBlue", 1]]) color(c[0]) translate([tp_led_x - 0.6, tp_w/2 + c[1], wall + tp_usb_h - 0.8]) cube([1.2, 1.2, 0.8]);
    }
    hs = hdr_strip(); color("Silver", 0.4) translate([hs[0] - 2.5, hs[1], shelf_z - (hat_dupont_t - hat_pcb_t)])
        cube([hs[2] + 5, hs[3], hat_dupont_t - hat_pcb_t]);   // Dupont housings on the HAT header, 18 mm below the PCB
    switch_preview();
    encoder_preview();
    color("#2e3a40") wheel_in_place();
    color("White")     translate([led_pos()[0],  led_pos()[1],  rim_z + bezel_t - led_sphere_d/2 + 1.5]) sphere(d = led_sphere_d);
    color("LimeGreen") translate([led2_pos()[0], led2_pos()[1], rim_z + bezel_t - led_sphere_d/2 + 1.5]) sphere(d = led_sphere_d);
    for (i = [0 : btn_n - 1]) color("Yellow") translate([btn_x(i) - key_shaft_w()/2, out_h + key_proud, key_z0 + 0.2]) mirror([0, 1, 0]) key();   // fully out, shaft key_proud of the wall
    color("DimGray") translate([plate_x0(), out_h, key_z0 - 0.2 - plate_gap]) rotate([-90, 0, 0]) plate();   // label plate on the top wall, behind the keys
    color("DimGray") sw_plate_in_place();                                                                  // ON / OFF plate above the knob slot
    color("Orange") sw_bar_in_place();
}

// ---------------------------------------------------------------- output
if (part == "front") {
    if (bezel_edge_r > 0) front();                                             // bump: face up (rounded edge on top)
    else translate([0, out_h, bezel_t]) rotate([180, 0, 0]) front();          // box: face down
}
else if (part == "back") back();
else if (part == "cap") translate([0, 0, wall + cap_flange_t]) rotate([180, 0, 0]) cap();   // flange down, dome up
else if (part == "key") key();   // flat on its bottom face
else if (part == "wheel") wheel();   // outer face down
else if (part == "plate") plate();   // flat, text up
else if (part == "swplate") sw_plate();   // flat, text up; -D sw_on_left=false for the other variant
else if (part == "swbar") sw_bar();   // flat
else if (part == "closed") {
    back();
    if (style == "bump") color("SteelBlue") cap();
    translate([0, 0, rim_z]) front();
    color("White")     translate([led_pos()[0],  led_pos()[1],  rim_z + bezel_t - led_sphere_d/2 + 1.5]) sphere(d = led_sphere_d);
    color("LimeGreen") translate([led2_pos()[0], led2_pos()[1], rim_z + bezel_t - led_sphere_d/2 + 1.5]) sphere(d = led_sphere_d);
    color("#2e3a40") wheel_in_place();
    color("Gainsboro") translate([-1.5, sw_y - 1.3, sw_slot_z0() + 0.4]) cube([wall + 2, 2.6, 2.6]);
    for (i = [0 : btn_n - 1]) color("Gainsboro") translate([btn_x(i) - key_shaft_w()/2, out_h + key_proud, key_z0 + 0.2]) mirror([0, 1, 0]) key();
    color("DimGray") translate([plate_x0(), out_h, key_z0 - 0.2 - plate_gap]) rotate([-90, 0, 0]) plate();
    color("DimGray") sw_plate_in_place();
    color("#1d2b2a") translate([pcb_x0, pcb_y0, rim_z - 0.4]) cube([hat_h, hat_w, 0.4]);   // glass
}
else if (part == "section") intersection() {
    union() {
        back();
        if (style == "bump") color("SteelBlue") cap();
        parts_preview();
        hat();
        translate([0, 0, rim_z]) front();
    }
    translate([section_x, -1, -20]) cube([section_w, out_h + 2, 80]);
}
else {
    back();
    if (style == "bump") color("SteelBlue") cap();
    %parts_preview();
    hat();
    %translate([0, 0, rim_z]) front();
    echo(shaft_keep_mm = shaft_keep, shaft_cut_off_mm = enc_shaft_h - shaft_keep, dial_z = dial_z, notch = [dial_y - notch_l/2, dial_y + notch_l/2],
         wheel_travel_deg = (wheel_numbers - 1)*wheel_step + 2*stop_half, hat_module_to_encoder_pcb_mm = enc_y0() - (hat_mod_box()[0][1] + hat_mod[3]),
         hat_module_to_dial_housings_mm = enc_housings_y()[0] - (hat_mod_box()[0][1] + hat_mod[3]), dial_housings_y = enc_housings_y(), tp_y = [tp_pos()[1], tp_pos()[1] + tp_l],
         enc_pcb_y = [enc_y0(), enc_y0() + enc_pcb_l], enc_pcb_x = enc_x(), sw_to_notch_wall_mm = (dial_y - notch_l/2 - wall) - (sw_y + sw_ear_len/2));
    echo(out = [out_w, out_h, rim_z + bezel_t], shelf_z = shelf_z, key_z0 = key_z0, key_h = key_h, header_strip = hdr_strip(), flex_y = fpc_y(), keys = [for (i = [0:3]) btn_x(i)], pico_holes = pico_holes());
}
