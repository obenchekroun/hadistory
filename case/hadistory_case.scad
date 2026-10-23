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
// components down against the left wall under the battery with a window in the floor under its LEDs,
// battery across the top, two sphere LEDs in the chin (white status left, green AI mode right),
// push keys at the junction on the top wall. v9: on the LEFT edge, a story dial (KY-040 encoder
// behind a notched wall, thumbwheel with the numbers 1 to 12 on its rim, printed end stops) and
// under it the slide switch, lying on the floor with its knob through the wall.
//
// Printed parts:  front (bezel), back (shell), cap (bump style), key (x4), wheel (story dial), plate (key
//                 labels, glued on the top edge), swplate (ON / OFF beside the switch, glued on the left edge)
// Render:  openscad -D 'part="back"' -o stl/bump/back.stl hadistory_case.scad   (-D 'style="box"' for the v7 box)
// Units: mm. Printing grid 0.4 mm nozzle / 0.2 mm layers: heights on multiples of 0.2,
// thin features on multiples of 0.4; smaller adjustments are not reproduced.

part = "assembly";   // "front" | "back" | "cap" | "key" | "wheel" | "plate" | "swplate" | "assembly" | "closed" (all parts, opaque, for pictures)
                     // | "section" (assembly cut at x = section_x, seen from +X)
section_x = 85.6;    // through the HAT header strip and the cap
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
tp_l           =  26.0;    // TP4056 (IN+ IN- B+ B-), its own USB unused; lies along X against the left wall
tp_w           =  17.0;
tp_pcb_t       =   1.0;
tp_usb_h       =   3.2;    // micro-USB shell above the PCB: the module lies components DOWN, resting on that shell
                           // and on a rib under its far end, so that its LEDs face the floor (to measure)
tp_rib_l       =   6.0;    // rib under the middle of the far (B+ B-) edge, between the pads, with an end stop beyond
tp_lip_h       =   1.2;    // locating lip along its lower long edge, above the PCB level: with the left wall, the
tp_lip_t       =   1.2;    // end stop and the battery rib it sets where the module is glued
tp_clear       =   0.2;
tp_led_x       =  15.0;    // charge / full LEDs, this far from the USB edge (measured 2026-10-21, about 15)
tp_win         = [3.0, 10.0];   // window through the floor under them, along X and Y, centred on the module width
tp_usb_out     = true;     // true: USB end away from the wall, B+ B- pads 1 mm off the wall and the LEDs on the wall side
                           // (11 mm from it); false: USB end at the wall. Everything about the module mirrors with it.
bat_l          =  50.0;    // 103450 LiPo
bat_w          =  34.0;
bat_t          =  10.0;
bat_clear      =   1.0;    // per side, inside the ribs
sw_body_l      =  11.6;    // SS12F15 slide switch: body along X
sw_body_w      =   6.0;    // body depth into the case (Y), knob face against the wall
sw_body_t      =   5.7;    // body height (Z)
sw_ear_len     =  19.4;    // overall length over the two mounting ears
sw_ear_t       =   0.5;    // ear sheet thickness; the ears are in the plane of the knob face
sw_ear_h       =   4.0;    // ear height (Z), to measure
sw_slot_l      =   8.0;    // knob travel slot through the left wall
sw_slot_h      =   3.4;    // knob slot height
sw_pad_t       =   1.2;    // pad under the body, so that the ears (centred on the body height) clear the floor
sw_y           =  21.0;    // switch centre along the left wall: above the corner boss, under the lower HAT pad (Y 24 to 32,
                           // 11 mm up), below the dial notch (from Y 33.5)
led_sphere_d   =   9.9;    // Adafruit dLUX-dLITE sphere LEDs (5431): white = status, green = AI mode
led_tube_t     =   1.2;    // LED tube wall; 1.6 on v4 made the tube overlap the PCB edge by 0.45 mm

// ---------------------------------------------------------------- story dial (v9): KY-040 behind the left wall
dial_y         =  47.0;    // wheel axis along Y: the free stretch of the left edge between the lower HAT pad (to 32)
                           // and the panel flex (from 63.8)
dial_notch_d   =   4.5;    // notch in the left edge, from the outer face; the 2 mm wall behind it ends at X 6.5, just
                           // outside the HAT PCB (X 6.9), the wheel sinks into the notch, the nut hides in it
dial_notch_r   =   2.0;    // plan radius of the notch corners
enc_pcb_l      =  26.0;    // KY-040 module PCB, standing behind the notch wall, encoder towards the wall, pins up (+Y)
enc_pcb_w      =  19.0;    // along Z: as tall as the room between floor and HAT, hence the floor recess below
enc_pcb_t      =   1.6;
enc_body       =  12.0;    // encoder body, square
enc_body_h     =   6.5;    // PCB face to the bushing base (to measure)
enc_bush_d     =   7.0;    // M7 x 0.75 threaded bushing, 7 long: 2 in the wall, 5 in the notch under the nut
enc_bush_h     =   7.0;
enc_shaft_d    =   6.0;    // D shaft, 13 long from the bushing end; CUT to the length echo()ed below
enc_cx         =  17.0;    // encoder axis from the pin end of the PCB (to measure; 9 from the other end)
enc_floor_recess = 0.8;    // the PCB sinks this much into the floor: 19 mm wide for 19 mm of room
enc_hole_dy    =   7.8;    // the two mounting holes of the module: this far from the axis towards the far (no pin) end,
enc_hole_dz    =  14.0;    // this far apart across the PCB (measured on the face plate printed earlier: 3.3 mm holes)
enc_post_d     =   5.0;    // posts from the notch wall to the PCB, M3 self-tapping screws from behind the PCB
enc_post_pilot =   2.5;
wheel_d        =  21.0;    // thumbwheel: as big as the axis height allows, its bottom flush with the back face
wheel_t        =  12.0;    // along X: 4 in the notch (nut recess and bushing inside), 8 proud of the edge
wheel_nut_d    =  12.4;    // round recess for the M7 nut (10 AF, 11.5 across corners): the wheel turns around it
wheel_nut_h    =   3.0;
wheel_bore_d   =   6.1;    // D bore for the 6 mm D shaft, blind, 1 mm cap: same as the knob cap printed earlier
wheel_bore_flat=   1.7;    // flat this far from the axis (4.75 across from the flat to the round side)
wheel_phase    =   0;      // degrees: turns the numbers and the stop groove about the D flat, so that at a detent
                           // the 1 sits under the arrow; measure the offset on the first print, set, reprint
wheel_numbers  =  12;      // on the rim, every detent (18 degrees on a 20-detent KY-040), 1 at the lower stop
wheel_step     =  18;
wheel_num_sz   =   2.8;
wheel_num_d    =   0.6;    // engraved
stop_r         =   8.15;   // stop pin on the notch wall, in an arc groove on the inner face of the wheel: travel
stop_pin_d     =   2.0;    // (wheel_numbers - 1) * wheel_step, the groove ends are the stops
stop_pin_h     =   1.5;
stop_groove_w  =   2.8;    // between the nut recess (r 6.2) and the rim (r 10.5)
stop_groove_d  =   1.6;

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
sw_plate_w     =  30.0;    // centred on the switch; the knob slot is 8 mm wide in the middle
sw_plate_h     =   7.0;
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
function tp_pos()  = [wall + 1.0, 88];                       // bottom-left corner, 26 along X (one end 1 mm off the left
                                                             // wall, see tp_usb_out), 17 along Y, under the battery
function bat_pos() = [26, out_h - wall - 4 - bat_w];         // lying across the top
function led2_pos() = [95, led_pos()[1]];                                           // green sphere, right end of the chin
function led_positions() = [led_pos(), led2_pos()];
dial_z   = wall - enc_floor_recess + enc_pcb_w/2;    // 10.7: dial axis height, set by the encoder PCB standing on the recessed floor
notch_l  = wheel_d + 6;                              // 27: notch length along the edge
function enc_x()  = dial_notch_d + wall + enc_body_h;   // 13: wall-side face of the encoder PCB
function enc_y0() = dial_y - (enc_pcb_l - enc_cx);      // 38: PCB lower end
function sw_slot_z0() = wall + sw_pad_t + (sw_body_t - sw_slot_h)/2;   // 4.35: knob slot bottom
function hdr_strip() = let(p = hat_pt(hdr_x, hdr_y + hdr_wid/2))
    [pcb_x0 + p[0] - hdr_wid/2, pcb_y0 + p[1] - hdr_len, hdr_wid, hdr_len];
function fpc_y() = [pcb_y0 + hat_pt(fpc_x1, 0)[1], pcb_y0 + hat_pt(fpc_x0, 0)[1]];   // flex span on the left edge
// Pads: [edge, position along the edge]; edges: "L" "R" "B" "T". On top, between the keys, clear of
// the protruding caps (v4 had none there: the PCB could ride up to the key guides, 1 mm or more off the
// bottom lips, and shift at each key press)
function pads() = [["L", 28], ["L", 112], ["R", 28], ["R", 112], ["B", 30], ["B", 68],   // clear of the LED, Pico screws and switch
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
        translate([0, 0, bezel_t - 0.6]) linear_extrude(1) polygon([[dial_notch_d + 1.2, dial_y], [dial_notch_d + 4.4, dial_y + 1.8], [dial_notch_d + 4.4, dial_y - 1.8]]);
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
            enc_posts();
            for (p = led_positions()) led_tube(p);
        }
        for (b = bosses()) translate([b[0], b[1], 2]) cylinder(d = screw_pilot, h = rim_z);
        for (h = pico_holes()) translate([h[0], h[1], 1]) cylinder(d = 2.0, h = wall + pico_standoff + 1);
        // Pico micro-USB through the bottom wall, at the Pico PCB level
        translate([pico_pos()[0] + pico_w/2 - usb_cut_w/2, -1, wall + pico_standoff - 1]) cube([usb_cut_w, wall + 2, usb_cut_h]);
        // Switch knob slot through the left wall
        translate([-1, sw_y - sw_slot_l/2, sw_slot_z0()]) cube([wall + 2, sw_slot_l, sw_slot_h]);
        // Story dial: notch in the left edge, bushing hole through its wall, floor recess under the encoder PCB
        translate([0, 0, -1]) linear_extrude(rim_z + 2) notch2d();
        translate([dial_notch_d - 1, dial_y, dial_z]) rotate([0, 90, 0]) cylinder(d = enc_bush_d + 0.4, h = wall + 2);
        translate([enc_x() - 0.4, enc_y0() - 0.5, wall - enc_floor_recess]) cube([enc_pcb_t + 0.8, enc_pcb_l + 1, enc_floor_recess + 0.01]);
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
                translate([p[0] - w/2, p[1], wall]) cube([w, led_sphere_d, h - wall]);
                translate([p[0] - 0.01, p[1], h + w/2 - 0.01]) cube([0.02, led_sphere_d, 0.02]);
            }
        }
        // TP4056: window through the floor under its LEDs (module glued components down)
        tp_frame() translate([tp_pos()[0] + tp_led_x - tp_win[0]/2, tp_pos()[1] + tp_w/2 - tp_win[1]/2, -1]) cube([tp_win[0], tp_win[1], wall + 2]);
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
// tp_frame(): the module drawn with its USB end at the wall (X from the USB edge), mirrored about the
// module centre when tp_usb_out is set
module tp_frame() {
    cx = tp_pos()[0] + tp_l/2;
    translate([cx, 0, 0]) mirror([tp_usb_out ? 1 : 0, 0, 0]) translate([-cx, 0, 0]) children();
}
module tp_lip() {
    t = tp_pos(); z = wall + tp_usb_h;
    translate([t[0], t[1] - tp_clear - tp_lip_t, 0]) cube([tp_l, tp_lip_t, z + tp_lip_h]);                     // lip
    tp_frame() translate([t[0] + tp_l - 1.6, t[1] + tp_w/2 - tp_rib_l/2, 0]) cube([1.6, tp_rib_l, z]);          // rib under the B+ B- edge
    translate([t[0] + tp_l + tp_clear, t[1] + tp_w/2 - tp_rib_l/2, 0]) cube([1.2, tp_rib_l, z + tp_lip_h]);    // end stop, away from the wall
}

module led_tube(p) { translate([p[0], p[1], 0]) cylinder(d = led_sphere_d + 0.6 + 2*led_tube_t, h = rim_z); }

// Slide switch lying on the floor against the left wall (v9: the right end of the chin went to the green
// sphere, and at the rim the body would meet the HAT PCB, the side margin being 4.5 mm). Same principle
// as the v7 cradle, turned on its side and lowered: the knob through the wall, the two ears sliding down
// into grooves between the wall and a rib on each side, closed at their far ends (X, Y and rotation
// located), the body on a pad one layer above its nominal underside so that the ears float just off the
// groove bottoms. The bezel cannot hold it down from 12 mm up, so a lip on the back rib snaps over the
// body's top edge instead (the rib is 1.2 thick to flex when the switch is pressed in). No glue, no screw.
module switch_seat() {
    x0 = wall + sw_ear_t + 0.2;                       // knob face of the body
    z0 = wall + sw_pad_t;                             // body underside
    ear_z0 = z0 + sw_body_t/2 - sw_ear_h/2;           // ear bottom
    groove_w = sw_ear_t + 0.4; rib_t = 1.6;
    ear_run = (sw_ear_len - sw_body_l)/2 + 0.4;       // groove length beyond the body
    translate([x0 - 0.2, sw_y - sw_body_l/2 - 0.2, 0]) cube([sw_body_w + 0.4, sw_body_l + 0.4, z0]);   // pad
    for (sgn = [-1, 1]) {                             // ear grooves
        ya = sw_y + sgn*(sw_body_l/2 + 0.4); yb = sw_y + sgn*(sw_body_l/2 + 0.4 + ear_run + rib_t);
        translate([wall - 0.01, min(ya, yb), 0]) difference() {
            cube([groove_w + rib_t + 0.01, abs(yb - ya), z0 + sw_body_t]);
            translate([-1, sgn < 0 ? rib_t : 0, ear_z0 - 0.05]) cube([groove_w + 1.01, ear_run, sw_body_t + 2]);
        }
    }
    xb = x0 + sw_body_w + 0.2;                        // back rib, with the snap lip
    translate([xb, sw_y - sw_body_l/2 - 0.2, 0]) cube([1.2, sw_body_l + 0.4, z0 + sw_body_t + 1.2]);
    translate([xb - 0.5, sw_y - sw_body_l/2 + 1.5, z0 + sw_body_t + 0.2]) hull() {
        cube([0.51, sw_body_l - 3, 0.4]);
        translate([0.5, 0, 0]) cube([0.01, sw_body_l - 3, 1.0]);
    }
}

// ---------------------------------------------------------------- story dial
// Notch in the left edge (plan), through the shell and the bezel; the wheel sinks into it
module notch2d() { translate([-5, dial_y - notch_l/2]) rounded_rect(5 + dial_notch_d, notch_l, dial_notch_r); }
// The wall behind the notch, X 4.5 to 6.5, closing the notch on three sides (the outer wall is cut away)
module notch_wall() { translate([wall - 0.01, dial_y - notch_l/2 - wall, 0]) cube([dial_notch_d + 0.01, notch_l + 2*wall, rim_z]); }
// Two posts from the notch wall to the encoder PCB, under its mounting holes, with a flat towards the
// encoder body (the holes are only 0.3 mm beyond it); the screws come from behind the PCB
function enc_holes() = [[dial_y - enc_hole_dy, dial_z - enc_hole_dz/2], [dial_y - enc_hole_dy, dial_z + enc_hole_dz/2]];
module enc_posts() {
    for (h = enc_holes()) translate([dial_notch_d + wall - 0.01, h[0], h[1]]) difference() {
        rotate([0, 90, 0]) cylinder(d = enc_post_d, h = enc_body_h + 0.01);
        translate([-1, enc_hole_dy - enc_body/2 - 0.2, -enc_post_d]) cube([enc_body_h + 2, enc_post_d, 2*enc_post_d]);   // flat, clear of the body
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
wheel_bush_z = (dial_notch_d + wall - enc_bush_h) - (dial_notch_d - 0.5 - wheel_t);   // bushing end, in the wheel frame
module wheel() {
    r = wheel_d/2; t = wheel_t; travel = (wheel_numbers - 1)*wheel_step;
    difference() {
        union() {
            translate([0, 0, 0.8]) cylinder(r = r, h = t - 1.6);
            cylinder(r1 = r - 0.8, r2 = r, h = 0.8);
            translate([0, 0, t - 0.8]) cylinder(r1 = r, r2 = r - 0.8, h = 0.8);
        }
        translate([0, 0, t - wheel_nut_h]) cylinder(d = wheel_nut_d, h = wheel_nut_h + 1);                 // nut recess
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
    color("Silver") translate([dial_notch_d + wall, dial_y - enc_body/2, dial_z - enc_body/2]) cube([enc_body_h, enc_body, enc_body]);
    color("Silver") translate([dial_notch_d + wall - enc_bush_h, dial_y, dial_z]) rotate([0, 90, 0]) cylinder(d = enc_bush_d, h = enc_bush_h);
    color("Silver") translate([dial_notch_d - 2.3, dial_y, dial_z]) rotate([0, 90, 0]) cylinder(d = 11.5, h = 2.3, $fn = 6);   // nut
    color("Gainsboro") translate([dial_notch_d - 0.5 - wheel_t + 1.0, dial_y, dial_z]) rotate([0, 90, 0]) cylinder(d = enc_shaft_d, h = wheel_t - 1.0 + wall + 2);
    for (i = [0 : 4]) color("Black") translate([enc_x() + enc_pcb_t, enc_y0() + enc_pcb_l - 3.5, dial_z - 5.08 + 2.54*i - 1.2]) cube([6 + 14, 2.4, 2.4]);   // pins and housings
}
module switch_preview() {
    color("White") translate([wall + sw_ear_t + 0.2, sw_y - sw_body_l/2, wall + sw_pad_t]) cube([sw_body_w, sw_body_l, sw_body_t]);
    color("Silver") translate([wall + 0.2, sw_y - sw_ear_len/2, wall + sw_pad_t + sw_body_t/2 - sw_ear_h/2]) cube([sw_ear_t, sw_ear_len, sw_ear_h]);
    color("Gainsboro") translate([-1.5, sw_y - 1.3, sw_slot_z0() + 0.4]) cube([wall + 2, 2.6, 2.6]);   // knob
}
// ON / OFF plate glued on the left edge above the knob slot, text facing out and reading with the device face up
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
// ON / OFF plate: glued on the bottom wall under the knob slot, read with the display facing you
// and the bottom edge tilted towards you, i.e. the display side up: local Y = +Z of the case.
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
}
module parts_preview() {
    pp = pico_pos(); color("Green") translate([pp[0], pp[1], wall + pico_standoff]) cube([pico_w, pico_l, pico_t]);
    color("Silver", 0.4) translate([pp[0] - dupont_room, pp[1] + 2, wall + pico_standoff]) cube([dupont_room, pico_l - 4, 6]);
    color("Silver", 0.4) translate([pp[0] + pico_w, pp[1] + 2, wall + pico_standoff]) cube([dupont_room, pico_l - 4, 6]);
    b = bat_pos();   color("Gold")  translate([b[0], b[1], wall]) cube([bat_l, bat_w, bat_t]);
    t = tp_pos();    color("Blue")  translate([t[0], t[1], wall + tp_usb_h]) cube([tp_l, tp_w, tp_pcb_t]);          // PCB, components down
    tp_frame() {
        color("Silver") translate([t[0], t[1] + tp_w/2 - 3.8, wall]) cube([6, 7.6, tp_usb_h]);                       // micro-USB shell on the floor
        for (c = [["Red", -2], ["DodgerBlue", 1]]) color(c[0]) translate([t[0] + tp_led_x - 0.6, t[1] + tp_w/2 + c[1], wall + tp_usb_h - 0.8]) cube([1.2, 1.2, 0.8]);
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
    translate([section_x, -1, -20]) cube([out_w, out_h + 2, 80]);
}
else {
    back();
    if (style == "bump") color("SteelBlue") cap();
    %parts_preview();
    hat();
    %translate([0, 0, rim_z]) front();
    echo(shaft_cut_mm = wheel_bush_z - 1.0, dial_z = dial_z, notch = [dial_y - notch_l/2, dial_y + notch_l/2]);   // shaft length to keep beyond the bushing end
    echo(out = [out_w, out_h, rim_z + bezel_t], shelf_z = shelf_z, key_z0 = key_z0, key_h = key_h, header_strip = hdr_strip(), flex_y = fpc_y(), keys = [for (i = [0:3]) btn_x(i)], pico_holes = pico_holes());
}
