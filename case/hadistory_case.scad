// hadistory - enclosure for the Pico version
//
// Portrait tablet: the Inky Impression 5.7" HAT is the face (its side buttons on the top
// edge), the Raspberry Pi Pico 2 W, a TP4056 charger, a 103450 LiPo and a slide switch
// live in the back shell. Everything inside is wired with Dupont jumpers plugged on the
// HAT's 40-pin header, which sets the depth of the shell.
//
// Layout of the back shell (looking into it, bottom wall at the bottom):
//   - Pico standing in the lower middle, USB through the bottom wall; its right-angle pins
//     take Dupont housings on both long sides, so 20 mm are kept free left and right of it
//   - the HAT's 40-pin header hangs upper right, with its Dupont block (that is what sets the depth)
//   - battery lying across the top, TP4056 bottom right (its own USB is not used: the Pico
//     USB feeds its IN+), slide switch on the right wall, sphere LED in the chin
//
// Printed parts:
//   front  - bezel frame; the HAT PCB sits in a pocket behind it
//   back   - shell with the component pockets; screwed to the front at the 4 corners,
//            its walls and bosses clamp the HAT PCB edges against the bezel
//   key    - push key for the HAT side buttons (print 4)
//
// Render:  openscad -D 'part="front"' -o front.stl hadistory_case.scad   (also "back", "key")
// Units: mm.

part = "assembly";   // "front" | "back" | "key" | "assembly" (preview)

// ---------------------------------------------------------------- HAT (Pimoroni drawing)
hat_w          = 125.40;   // PCB, landscape frame of the Pimoroni drawing (buttons on the left)
hat_h          =  99.50;
hat_corner_r   =   3.00;   // PCB corner radius (HAT standard)
hat_pcb_t      =   2.80;   // glass + PCB, measured
hat_conn_t     =   8.10;   // glass to the top of the 40-pin header, measured
hat_dupont_t   =  20.80;   // same with Dupont housings plugged in, measured
disp_w         = 114.90;   // visible area, centred on the PCB
disp_h         =  85.80;
pi_x           =  37.20;   // Pi footprint origin from the PCB left / bottom edges
pi_y           =  26.20;
pi_h           =  56.00;
hdr_len        =  51.00;   // 2 x 20 header along the top edge of the Pi footprint
hdr_wid        =   5.10;
hdr_x          = pi_x + 3.5 + 29 - hdr_len/2;
hdr_y          = pi_y + pi_h - 3.5 - hdr_wid/2;
btn_y0         =  19.83;   // side buttons on the PCB left edge, from the bottom edge
btn_pitch      =  21.45;
btn_n          =   4;
btn_cap_w      =   4.0;    // button cap width along the edge
btn_protrusion =   1.5;    // cap beyond the PCB edge (to measure)
fpc_x0         =  60.0;    // flex tab of the panel on the PCB bottom edge
fpc_x1         =  86.0;

// ---------------------------------------------------------------- other parts, measured
pico_l         =  51.0;    // Pico 2 W lying flat, right-angle pins towards the inside
pico_w         =  21.0;
pico_t         =   8.0;    // with pins and micro-USB
pico_hole_dx   =  47.0;    // M2 holes
pico_hole_dy   =  11.4;
dupont_room    =  20.0;    // free space beside each row of right-angle pins for the Dupont housings
tp_l           =  26.0;    // TP4056 (micro-USB, IN+ IN- B+ B-)
tp_w           =  17.0;
tp_t           =   5.0;
bat_l          =  50.0;    // 103450 LiPo
bat_w          =  34.0;
bat_t          =  10.0;
sw_body_l      =  13.0;    // SS12F15 slide switch
sw_body_w      =   6.5;
sw_body_t      =   5.0;
sw_slot_l      =   8.0;    // knob travel slot
sw_slot_w      =   2.6;
led_sphere_d   =   9.9;    // Adafruit dLUX-dLITE sphere LED (product 5431), standard leads

// ---------------------------------------------------------------- design parameters
wall           =   2.0;    // shell and frame walls
bezel_t        =   2.0;    // bezel thickness over the glass edge
clear          =   0.30;   // print clearance around the HAT PCB
win_margin     =   1.0;    // window = visible area + margin
ear_side       =   3.0;    // margin between the PCB pocket and the walls: room for the bosses
ear_top        =   3.0;
ear_bottom     =  12.0;    // chin: room for the sphere LED below the PCB
corner_r       =   5.0;    // outer corner radius
screw_d        =   2.0;    // M2 screws, self tapping in the bosses
boss_d         =   4.6;
key_slot_w     =   8.0;    // slot for the push keys in the top wall
key_slot_h     =   3.2;
cavity_t       = hat_dupont_t - hat_pcb_t + 1.0;   // free depth behind the PCB: Dupont + 1 mm
$fn            = 48;

// Derived sizes. Portrait: the HAT is turned so that its left edge (buttons) is the top.
pocket_w = hat_h + 2*clear;              // device inner width  (100.1)
pocket_h = hat_w + 2*clear;              // device inner height (126.0)
in_w     = pocket_w + 2*ear_side;        // shell interior
in_h     = pocket_h + ear_top + ear_bottom;
out_w    = in_w + 2*wall;                // 110.1
out_h    = in_h + 2*wall;                // 141.0
front_t  = bezel_t + hat_pcb_t;
back_t   = cavity_t + wall;              // 22 (cavity 19 + floor 2)... see cavity_t
pcb_x0   = wall + ear_side + clear;      // PCB bottom-left corner in part coordinates
pcb_y0   = wall + ear_bottom + clear;

// Drawing frame (x right, y up, buttons left) -> portrait part frame, origin = PCB corner.
// Rotation of 90 degrees clockwise: the left edge of the drawing becomes the top edge.
function P(x, y) = [y, hat_w - x];

module rounded_rect(w, h, r) { translate([r, r]) offset(r = r) square([w - 2*r, h - 2*r]); }
module at_pcb() { translate([pcb_x0, pcb_y0, 0]) children(); }

// Screw bosses: in the ears, at the four corners, outside the PCB pocket
function bosses() = let(o = wall + boss_d/2 + 0.4)
    [[o, o], [out_w - o, o], [o, out_h - o], [out_w - o, out_h - o]];

// ---------------------------------------------------------------- front frame
// Modelled in the assembly orientation: same XY as the back shell (device seen from the
// front), PCB pocket from z = 0 to hat_pcb_t, bezel on top up to front_t. For printing the
// "front" output turns it face down (a rotation, so left/right stay correct).
module front() {
    difference() {
        linear_extrude(front_t) rounded_rect(out_w, out_h, corner_r);
        // PCB pocket, open towards the back (z = 0), with the HAT corner radius
        translate([pcb_x0 - clear, pcb_y0 - clear, -1])
            linear_extrude(hat_pcb_t + 1) rounded_rect(pocket_w, pocket_h, hat_corner_r + clear);
        // Display window
        at_pcb() translate([(hat_h - disp_h)/2 - win_margin, (hat_w - disp_w)/2 - win_margin, -1])
            linear_extrude(front_t + 2) rounded_rect(disp_h + 2*win_margin, disp_w + 2*win_margin, 1.5);
        // Key slots through the top wall and ear, at the HAT side buttons, in the PCB plane
        for (i = [0 : btn_n - 1]) { p = P(0, btn_y0 + i*btn_pitch);
            translate([pcb_x0 + p[0] - key_slot_w/2, pcb_y0 + hat_w - 0.5, hat_pcb_t/2 - key_slot_h/2 - 1])
                cube([key_slot_w, ear_top + wall + clear + 1, key_slot_h + 1]); }
        // Corner screws: clearance hole, countersunk from the front face
        for (b = bosses()) translate([b[0], b[1], -1]) {
            cylinder(d = screw_d + 0.5, h = front_t + 2);
            translate([0, 0, front_t + 1 - 1.4]) cylinder(d1 = screw_d + 0.5, d2 = 4.6, h = 1.4 + 0.01);
        }
        // Sphere LED in the chin, left of the Pico USB: conical hole, narrower than the
        // sphere at the front face so that the sphere seats in it and a dome shows outside
        translate([led_x(), wall + ear_bottom/2, -1])
            cylinder(d1 = led_sphere_d + 0.8, d2 = led_sphere_d - 0.5, h = front_t + 2);
    }
}

// ---------------------------------------------------------------- back shell
module back() {
    difference() {
        union() {
            difference() {
                linear_extrude(back_t) rounded_rect(out_w, out_h, corner_r);
                translate([wall, wall, wall]) linear_extrude(cavity_t + 1) rounded_rect(in_w, in_h, corner_r - wall);
            }
            // Corner bosses, up to the PCB plane, screwed to the front frame
            for (b = bosses()) translate([b[0], b[1], 0]) cylinder(d = boss_d + 2.4, h = back_t);
            // Inner lip around the PCB pocket, floor to PCB plane: the HAT PCB rests on its top
            // face all around and is located by it (same outline as the pocket of the front frame)
            pcb_lip();
            pico_mounts();
            battery_ribs();
            tp_ribs();
            switch_pocket();
            led_holder();
        }
        // Screws
        for (b = bosses()) translate([b[0], b[1], 1.5]) cylinder(d = screw_d - 0.2, h = back_t);
        for (h = pico_holes()) translate([h[0], h[1], wall - 1]) cylinder(d = 1.7, h = 5);
        // Pico micro-USB through the bottom wall (the Pico stands along the left wall, USB down)
        translate([pico_pos()[0] + pico_w/2 - 5, -1, wall + 1]) cube([10, wall + 2, 5]);
        // Slide switch knob slot through the right wall
        translate([out_w - wall - 1, sw_pos()[1] - sw_slot_l/2, wall + 1.5]) cube([wall + 2, sw_slot_l, sw_slot_w]);
        // Relief for the panel flex at the left edge of the PCB, just below the PCB plane
        at_pcb() translate([-clear - 1.3, P(fpc_x1, 0)[1], back_t - 2.5]) cube([4, fpc_x1 - fpc_x0, 3]);
        // Key slots continue into the top wall of the shell (keys straddle both parts)
        for (i = [0 : btn_n - 1]) { p = P(0, btn_y0 + i*btn_pitch);
            translate([pcb_x0 + p[0] - key_slot_w/2, out_h - wall - ear_top - clear - 0.5, back_t - key_slot_h/2])
                cube([key_slot_w, ear_top + wall + clear + 1, key_slot_h]); }
    }
}

// Lip supporting the HAT PCB, interrupted where the Pico reaches the bottom wall and at the LED wires
lip_t = 1.2;
module pcb_lip() {
    difference() {
        translate([pcb_x0 - clear - lip_t, pcb_y0 - clear - lip_t, 0])
            linear_extrude(back_t) rounded_rect(pocket_w + 2*lip_t, pocket_h + 2*lip_t, hat_corner_r + clear + lip_t);
        translate([pcb_x0 - clear, pcb_y0 - clear, -1])
            linear_extrude(back_t + 2) rounded_rect(pocket_w, pocket_h, hat_corner_r + clear);
        // Pico gap: the Pico and its Dupont housings cross the bottom lip
        translate([pico_pos()[0] - 2, 0, -1]) cube([pico_w + 4, pcb_y0 + 1, back_t + 2]);
        // LED wires notch, at the floor
        translate([led_x() - 3, 0, wall]) cube([6, pcb_y0 + 1, 5]);
    }
}

// Pico: standing in the lower middle, long side vertical, USB towards the bottom wall,
// dupont_room free on both long sides for the housings on the right-angle pins.
// Standoffs 2.5 mm so the pins clear the floor.
function pico_pos() = [(out_w - pico_w)/2 - 6, wall + 1.5];   // bottom-left corner (21 wide, 51 tall)
function pico_holes() = let(p = pico_pos(), ox = (pico_w - pico_hole_dy)/2, oy = (pico_l - pico_hole_dx)/2)
    [[p[0] + ox, p[1] + oy], [p[0] + ox + pico_hole_dy, p[1] + oy],
     [p[0] + ox, p[1] + oy + pico_hole_dx], [p[0] + ox + pico_hole_dy, p[1] + oy + pico_hole_dx]];
module pico_mounts() { for (h = pico_holes()) translate([h[0], h[1], 0]) cylinder(d = 4.5, h = wall + 2.5); }

// Battery: lying across the top of the shell, above the 40-pin header strip, long side along X
function bat_pos() = [(out_w - bat_l)/2 - 8, out_h - wall - 3 - bat_w];
module battery_ribs() {
    b = bat_pos(); r = 1.5; h = wall + 5;
    for (p = [[b[0] - r, b[1] - r, bat_l + 2*r, r], [b[0] - r, b[1] + bat_w, bat_l + 2*r, r],
              [b[0] - r, b[1] - r, r, bat_w + 2*r], [b[0] + bat_l, b[1] - r, r, bat_w + 2*r]])
        translate([p[0], p[1], 0]) cube([p[2], p[3], h]);
}

// TP4056: along the bottom wall, micro-USB towards the wall, right of the Pico
function tp_pos() = [out_w - wall - 2 - tp_w, wall + 0.5];   // bottom-left corner (17 wide, 26 tall)
module tp_ribs() {
    t = tp_pos(); r = 1.5; h = wall + 3;
    for (p = [[t[0] - r, t[1], r, tp_l], [t[0] + tp_w, t[1], r, tp_l], [t[0] - r, t[1] + tp_l, tp_w + 2*r, r]])
        translate([p[0], p[1], 0]) cube([p[2], p[3], h]);
}

// Slide switch: against the right wall, above the battery, knob through the wall
function sw_pos() = [out_w - wall - sw_body_w - 0.3, pcb_y0 + hat_w - hdr_x + 6];   // just above the end of the header strip
module switch_pocket() {
    s = sw_pos(); r = 1.5;
    difference() {
        translate([s[0] - r, s[1] - sw_body_l/2 - r, 0]) cube([sw_body_w + r + 0.3, sw_body_l + 2*r, wall + sw_body_t]);
        translate([s[0], s[1] - sw_body_l/2, wall]) cube([sw_body_w + 1, sw_body_l, sw_body_t + 1]);
    }
}

// LED: a short tube in the chin of the shell, in line with the bezel hole, the LED pushed in from inside
function led_x() = out_w * 0.22;
// Tube that guides the sphere LED up to the bezel hole. Open towards the inside of the
// shell below the sphere seat, so that the leads and wires run to the Pico inside the case.
module led_holder() {
    seat = back_t - led_sphere_d;   // below this the tube is open on the inner side
    translate([led_x(), wall + ear_bottom/2, 0]) difference() {
        cylinder(d = led_sphere_d + 3.6, h = back_t);
        translate([0, 0, wall]) cylinder(d = led_sphere_d + 0.6, h = back_t);
        translate([-(led_sphere_d + 4)/2, 0, wall]) cube([led_sphere_d + 4, led_sphere_d, seat - wall]);
    }
}

// ---------------------------------------------------------------- push key
// Shaft through the slot, head inside the case, face against the HAT button cap
module key() {
    shaft_l = ear_top + wall + clear + 1.0;      // through the wall and the ear, 1 mm proud
    translate([0, 0, 0]) cube([key_slot_w - 0.4, shaft_l, key_slot_h - 0.4]);
    translate([-2, -1.5, -1]) cube([key_slot_w - 0.4 + 4, 1.5, key_slot_h - 0.4 + 2]);   // head
}

// ---------------------------------------------------------------- previews
module hat() {
    color("DarkSlateGray") at_pcb() linear_extrude(hat_pcb_t) rounded_rect(hat_h, hat_w, hat_corner_r);
    color("Black") at_pcb() translate([(hat_h - disp_h)/2, (hat_w - disp_w)/2, -0.01]) cube([disp_h, disp_w, 0.4]);
    p = P(hdr_x, hdr_y + hdr_wid/2);
    color("Gray") at_pcb() translate([p[0] - hdr_wid/2, p[1] - hdr_len, -(hat_conn_t - hat_pcb_t)]) cube([hdr_wid, hdr_len, hat_conn_t - hat_pcb_t]);
    color("Silver", 0.5) at_pcb() translate([p[0] - hdr_wid/2, p[1] - hdr_len, -(hat_dupont_t - hat_pcb_t)]) cube([hdr_wid, hdr_len, hat_dupont_t - hat_pcb_t]);
    for (i = [0 : btn_n - 1]) { q = P(0, btn_y0 + i*btn_pitch);
        color("Red") at_pcb() translate([q[0] - btn_cap_w/2, hat_w, 0.3]) cube([btn_cap_w, btn_protrusion, 2]); }
}
module parts_preview() {
    pp = pico_pos(); color("Green") translate([pp[0], pp[1], wall + 2.5]) cube([pico_w, pico_l, pico_t]);
    b = bat_pos();   color("Gold")  translate([b[0], b[1], wall]) cube([bat_l, bat_w, bat_t]);
    // Dupont housings on the Pico pins, both sides
    color("Silver", 0.4) translate([pp[0] - dupont_room, pp[1] + 2, wall + 2.5]) cube([dupont_room, pico_l - 4, 6]);
    color("Silver", 0.4) translate([pp[0] + pico_w, pp[1] + 2, wall + 2.5]) cube([dupont_room, pico_l - 4, 6]);
    t = tp_pos();    color("Blue")  translate([t[0], t[1], wall]) cube([tp_w, tp_l, tp_t]);
    s = sw_pos();    color("White") translate([s[0], s[1] - sw_body_l/2, wall]) cube([sw_body_w, sw_body_l, sw_body_t]);
    color("LimeGreen") translate([led_x(), wall + ear_bottom/2, back_t + hat_pcb_t + bezel_t - led_sphere_d/2 + 1.5]) sphere(d = led_sphere_d);
}

// ---------------------------------------------------------------- output
if (part == "front") translate([0, out_h, front_t]) rotate([180, 0, 0]) front();   // face down for printing
else if (part == "back") back();
else if (part == "key") key();
else {
    // Assembly preview: back shell at z = 0 (floor down), HAT in the PCB plane, bezel on top
    back();
    %parts_preview();
    translate([0, 0, back_t]) hat();
    %translate([0, 0, back_t]) front();
    echo(pico_holes = pico_holes(), pico_usb_x = pico_pos()[0] + pico_w/2, led_x = led_x(), out = [out_w, out_h, back_t + front_t]);
}
