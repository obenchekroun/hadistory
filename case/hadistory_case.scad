// hadistory - enclosure for the Pico version (v4)
//
// Portrait tablet. The Inky Impression 5.7" HAT sits in the back shell, display up, on six
// small pads one PCB thickness (plus a drop) below the rim; the front is a flat bezel plate
// screwed on top. Raspberry Pi Pico 2 W, TP4056 charger, 103450 LiPo and slide switch live
// in the back shell, all wired with Dupont jumpers plugged on the HAT's 40-pin header.
//
// Coordinates: X right, Y up, Z out of the display, device seen FROM THE FRONT.
// The Pimoroni drawing of the HAT is a front view with the components seen through; it is
// turned 90 degrees so that its left edge (buttons) is the top edge, see hat_pt().
// In this frame the HAT header and its Dupont block hang on the RIGHT, the panel flex is
// on the LEFT edge, the four side buttons are on the TOP edge (they protrude from the PCB).
//
// Back shell layout: Pico standing lower left of centre (USB through the bottom wall, 20 mm
// free on each long side for the Dupont housings on its right-angle pins), TP4056 bottom
// right, battery across the top, sphere LED bottom left in the chin, slide switch on the
// bottom wall at the junction with the bezel, push keys at the junction on the top wall.
//
// Printed parts:  front (flat bezel), back (shell), key (x4)
// Render:  openscad -D 'part="back"' -o back.stl hadistory_case.scad
// Units: mm. Printing grid 0.4 mm nozzle / 0.2 mm layers: heights on multiples of 0.2,
// thin features on multiples of 0.4; smaller adjustments are not reproduced.

part = "assembly";   // "front" | "back" | "key" | "assembly"

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
tp_l           =  26.0;    // TP4056 (IN+ IN- B+ B-), its own USB unused
tp_w           =  17.0;
tp_t           =   5.0;
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
sw_slot_l      =   8.0;    // knob travel slot
sw_slot_h      =   3.4;    // knob slot depth below the rim (notch open at the rim)
led_sphere_d   =   9.9;    // Adafruit dLUX-dLITE sphere LED (5431)

// ---------------------------------------------------------------- design parameters
wall           =   2.0;
bezel_t        =   2.0;
clear          =   0.4;    // clearance around the HAT PCB (per side); 0.8 on v3 left too much play
hat_corner_r   =   1.0;
win_margin     =   1.0;
ear_side       =   4.5;    // margins between the PCB pocket and the walls
ear_top        =   6.0;    // room for the key guides behind the protruding buttons
ear_bottom     =  12.0;    // chin: LED and switch
corner_r       =   4.0;
screw_pilot    =   2.6;    // M3 self-tapping, printed pilot hole
screw_d        =   3.0;
boss_d         =   6.4;
boss_inset     =   3.2;
dupont_slack   =  10.0;    // extra depth to handle the Dupont housings on the HAT
shelf_drop     =   0.4;    // PCB underside this much lower than rim - hat_pcb_t (1.0 on v3 was too much)
pad_w          =   8.0;    // support pads along the PCB edges
pad_in         =   1.6;    // how far a pad reaches under the PCB
pad_t          =   3.0;    // pad thickness, plus a 45 degree gusset below
lip_t          =   1.2;    // locating lip on each pad, outside the PCB
key_w          =   8.0;    // key shaft width (X); its height runs from the notch bottom to the rim
key_head_w     =  12.0;
key_head_t     =   2.0;    // head pocket depth (Y)
key_block_w    =  16.0;    // guide block width around each key
cap_room       =   2.4;    // key guide inner face set back from the PCB edge, beyond the button caps
$fn            = 48;

// Derived
pocket_w = hat_h + 2*clear;                    // 100.3
pocket_h = hat_w + 2*clear;                    // 126.2
in_w     = pocket_w + 2*ear_side;              // 109.3
in_h     = pocket_h + ear_top + ear_bottom;    // 144.2
out_w    = in_w + 2*wall;                      // 113.3
out_h    = in_h + 2*wall;                      // 148.2
cavity_t = hat_dupont_t - hat_pcb_t + 1 + dupont_slack;   // 29: PCB underside to floor (nominal)
rim_z    = wall + cavity_t;                    // 31
shelf_z  = rim_z - hat_pcb_t - shelf_drop;     // 27.8: PCB underside
key_z0   = shelf_z - btn_cap_drop;             // 23.4: bottom of the key notches
key_h    = rim_z - key_z0 - 0.4;               // 7.2: key height, under the bezel
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
function led_pos() = [18, wall + ear_bottom/2];

// ---------------------------------------------------------------- component positions
function pico_pos() = [40, wall + 1.5];                      // bottom-left corner, 21 wide x 51 tall
function pico_holes() = let(p = pico_pos(), ox = (pico_w - pico_hole_dy)/2, oy = (pico_l - pico_hole_dx)/2)
    [[p[0] + ox, p[1] + oy], [p[0] + ox + pico_hole_dy, p[1] + oy],
     [p[0] + ox, p[1] + oy + pico_hole_dx], [p[0] + ox + pico_hole_dy, p[1] + oy + pico_hole_dx]];
function tp_pos()  = [92, wall + 1.5];                       // bottom-left corner, 17 wide x 26 tall
function bat_pos() = [26, out_h - wall - 4 - bat_w];         // lying across the top
function sw_x()    = 86;                                     // switch centre on the bottom wall
function hdr_strip() = let(p = hat_pt(hdr_x, hdr_y + hdr_wid/2))
    [pcb_x0 + p[0] - hdr_wid/2, pcb_y0 + p[1] - hdr_len, hdr_wid, hdr_len];
function fpc_y() = [pcb_y0 + hat_pt(fpc_x1, 0)[1], pcb_y0 + hat_pt(fpc_x0, 0)[1]];   // flex span on the left edge
// Pads: [edge, position along the edge]; edges: "L" "R" "B" (none on top: the buttons protrude there)
function pads() = [["L", 28], ["L", 112], ["R", 28], ["R", 112], ["B", 30], ["B", 68]];   // clear of the LED, Pico screws and switch

// ---------------------------------------------------------------- front plate
module front() {
    difference() {
        linear_extrude(bezel_t) rounded_rect(out_w, out_h, corner_r);
        translate([pcb_x0 + (hat_h - disp_h)/2 - win_margin, pcb_y0 + (hat_w - disp_w)/2 - win_margin, -1])
            linear_extrude(bezel_t + 2) rounded_rect(disp_h + 2*win_margin, disp_w + 2*win_margin, 1.5);
        for (b = bosses()) translate([b[0], b[1], -1]) {
            cylinder(d = screw_d + 0.4, h = bezel_t + 2);
            translate([0, 0, bezel_t + 1 - 1.6]) cylinder(d1 = screw_d + 0.4, d2 = 6.2, h = 1.6 + 0.01);
        }
        translate([led_pos()[0], led_pos()[1], -1])
            cylinder(d1 = led_sphere_d + 0.8, d2 = led_sphere_d - 0.5, h = bezel_t + 2);
    }
}

// ---------------------------------------------------------------- back shell
module back() {
    difference() {
        union() {
            difference() {   // walls and floor
                linear_extrude(rim_z) rounded_rect(out_w, out_h, corner_r);
                translate([wall, wall, wall]) linear_extrude(rim_z) rounded_rect(in_w, in_h, corner_r - wall);
            }
            for (b = bosses()) translate([b[0], b[1], 0]) cylinder(d = boss_d, h = rim_z);
            for (p = pads()) pad(p[0], p[1]);
            for (i = [0 : btn_n - 1]) key_guide(btn_x(i));
            pico_mounts();
            battery_ribs();
            tp_ribs();
            switch_cradle();
            led_tube();
        }
        for (b = bosses()) translate([b[0], b[1], 2]) cylinder(d = screw_pilot, h = rim_z);
        for (h = pico_holes()) translate([h[0], h[1], 1]) cylinder(d = 2.0, h = wall + pico_standoff + 1);
        // Pico micro-USB through the bottom wall, at the Pico PCB level
        translate([pico_pos()[0] + pico_w/2 - usb_cut_w/2, -1, wall + pico_standoff - 1]) cube([usb_cut_w, wall + 2, usb_cut_h]);
        // Switch knob notch in the bottom wall, open at the rim
        translate([sw_x() - sw_slot_l/2, -1, rim_z - sw_slot_h]) cube([sw_slot_l, wall + 2, sw_slot_h + 1]);
        // Key notches through the top wall and guides, from below the caps up to the rim, with the head step
        for (i = [0 : btn_n - 1]) {
            translate([btn_x(i) - key_w/2 - 0.2, pcb_y1 + cap_room - 1, key_z0 - 0.2]) cube([key_w + 0.4, ear_top + wall + 2, rim_z]);
            translate([btn_x(i) - key_head_w/2 - 0.2, pcb_y1 + cap_room - 0.01, key_z0 - 0.2]) cube([key_head_w + 0.4, key_head_t + 0.4, rim_z]);
        }
        // LED bore and wire exit of the tube towards the inside
        translate([led_pos()[0], led_pos()[1], wall]) cylinder(d = led_sphere_d + 0.6, h = rim_z);
        translate([led_pos()[0] - (led_sphere_d + 4)/2, led_pos()[1], wall]) cube([led_sphere_d + 4, led_sphere_d, shelf_z - led_sphere_d - wall - 1]);
    }
}

// Support pad under the PCB edge, cantilevered from the wall with a 45 degree gusset,
// plus a locating lip outside the PCB up to the rim
module pad(edge, pos) {
    reach = (edge == "B") ? ear_bottom + clear + pad_in : ear_side + clear + pad_in;   // from the wall inner face
    module shape() {   // in a local frame: X along the edge, Y from the wall inwards, Z up
        // pad
        translate([-pad_w/2, 0, shelf_z - pad_t]) cube([pad_w, reach, pad_t]);
        // gusset: 45 degrees under the pad
        hull() {
            translate([-pad_w/2, 0, shelf_z - pad_t - reach]) cube([pad_w, 0.01, 0.01]);
            translate([-pad_w/2, 0, shelf_z - pad_t - 0.01]) cube([pad_w, reach, 0.01]);
        }
        // locating lip, outside the PCB outline, up to the rim
        translate([-pad_w/2, reach - pad_in - clear - lip_t, shelf_z - pad_t]) cube([pad_w, lip_t, rim_z - shelf_z + pad_t]);
    }
    if (edge == "L") translate([wall, pos, 0]) rotate([0, 0, -90]) mirror([1, 0, 0]) shape();
    else if (edge == "R") translate([out_w - wall, pos, 0]) rotate([0, 0, 90]) shape();
    else translate([pos, wall, 0]) shape();
}

// Guide block for a key, in the top ear, set back from the PCB edge to clear the button cap
module key_guide(x) {
    translate([x - key_block_w/2, pcb_y1 + cap_room, key_z0 - 2]) cube([key_block_w, out_h - wall - (pcb_y1 + cap_room), rim_z - key_z0 + 2]);
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

module tp_ribs() {
    t = tp_pos(); r = 1.6; h = wall + 3;
    for (p = [[t[0] - r, t[1], r, tp_l], [t[0] + tp_w, t[1], r, tp_l], [t[0] - r, t[1] + tp_l, tp_w + 2*r, r]])
        translate([p[0], p[1], 0]) cube([p[2], p[3], h]);
}

// Slide switch holder on the bottom wall, at the rim: the switch stands knob towards the
// wall, its two mounting ears slide down into two grooves between the wall and a rib, which
// hold it in X and Y; the bezel holds it down. No screws. Pins point inwards for the wires.
module switch_cradle() {
    x = sw_x();
    groove_w = sw_ear_t + 0.4;              // ear groove, between the wall and the rib
    groove_d = sw_ear_h + 0.4;              // groove depth below the rim
    rib_t = 1.6;
    rib_h = groove_d + 1.6;                 // rib goes 1.6 below the groove bottom to close it
    rib_x0 = sw_body_l/2 + 0.4;             // ribs start just beyond the body
    rib_x1 = sw_ear_len/2 + 0.4 + 1.6;      // and end beyond the ears, forming the end stops
    for (sgn = [-1, 1]) {
        xa = x + sgn * rib_x0; xb = x + sgn * rib_x1;
        x0 = min(xa, xb); w = abs(xb - xa);
        translate([x0, wall, rim_z - rib_h]) difference() {
            union() {
                cube([w, groove_w + rib_t, rib_h]);
                hull() { cube([w, 0.01, 0.01]); translate([0, 0, -(groove_w + rib_t)]) cube([w, 0.01, 0.01]); translate([0, groove_w + rib_t, 0]) cube([w, 0.01, 0.01]); }
            }
            // the groove: open at the rim and towards the body, closed at its far end (end stop)
            ear_run = sw_ear_len/2 + 0.4 - rib_x0;
            translate([sgn < 0 ? w - ear_run : 0, -1, rib_h - groove_d]) cube([ear_run, groove_w + 1, groove_d + 1]);
        }
    }
}

module led_tube() { translate([led_pos()[0], led_pos()[1], 0]) cylinder(d = led_sphere_d + 0.6 + 3.2, h = rim_z); }

// ---------------------------------------------------------------- push key
// Shaft through the guide and the wall, 1 mm proud outside; head in the step on the inner face of
// the guide; nub towards the cap. Full height from the notch bottom to the bezel, so that it
// cannot ride up and reaches the cap wherever it sits below the PCB. Printed as modelled.
module key() {
    shaft_l = out_h - wall - (pcb_y1 + cap_room) + wall - key_head_t + 1.0;   // through the guide and wall, 1 mm proud
    nub     = cap_room - btn_protrusion - 0.3;                               // from the guide face to 0.3 mm short of the cap
    cube([key_w - 0.4, shaft_l, key_h]);
    translate([-(key_head_w - key_w)/2, shaft_l, 0]) cube([key_head_w - 0.4, key_head_t - 0.4, key_h]);   // head
    translate([(key_w - 4)/2 - 0.2, shaft_l + key_head_t - 0.4, 0]) cube([4, max(0.4, nub), key_h]);       // nub
}

// ---------------------------------------------------------------- previews
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
    t = tp_pos();    color("Blue")  translate([t[0], t[1], wall]) cube([tp_w, tp_l, tp_t]);
    color("White") translate([sw_x() - sw_body_l/2, wall + sw_ear_t + 0.2, rim_z - sw_body_t - 0.4]) cube([sw_body_l, sw_body_w, sw_body_t]);
    color("Silver") translate([sw_x() - sw_ear_len/2, wall + 0.2, rim_z - sw_ear_h - 0.4]) cube([sw_ear_len, sw_ear_t, sw_ear_h]);
    color("LimeGreen") translate([led_pos()[0], led_pos()[1], rim_z + bezel_t - led_sphere_d/2 + 1.5]) sphere(d = led_sphere_d);
    for (i = [0 : btn_n - 1]) color("Yellow") translate([btn_x(i) - key_w/2 + 0.2, out_h + 1.0, key_z0 + 0.2]) mirror([0, 1, 0]) key();   // shaft 1 mm proud of the wall
}

// ---------------------------------------------------------------- output
if (part == "front") translate([0, out_h, bezel_t]) rotate([180, 0, 0]) front();   // face down for printing
else if (part == "back") back();
else if (part == "key") key();   // flat on its bottom face
else {
    back();
    %parts_preview();
    hat();
    %translate([0, 0, rim_z]) front();
    echo(out = [out_w, out_h, rim_z + bezel_t], shelf_z = shelf_z, key_z0 = key_z0, key_h = key_h, header_strip = hdr_strip(), flex_y = fpc_y(), keys = [for (i = [0:3]) btn_x(i)], pico_holes = pico_holes());
}
