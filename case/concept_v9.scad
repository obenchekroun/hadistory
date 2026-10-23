// hadistory - v9 concept, visual only: the v8 bump body with
//   - a story dial on the LEFT edge: KY-040 encoder behind the wall, a thumbwheel outside, numbers
//     1 to 12 on its rim, an arrow on the bezel margin pointing at the number in use
//   - the ON / OFF slide switch moved from the bottom edge to the left edge, under the wheel
//   - a second dLUX sphere (green, AI mode) at the right end of the chin, symmetric to the white one
// Same frame as hadistory_case.scad (device seen from the front, X right, Y up, Z towards the viewer).
// Renders: see "Design directions" in README.md.

view = "front";   // "front" | "left" | "wheel"
$fn = 72;

W = 113.3; H = 148.2; T = 26.6;           // v8 bump outline, 0.8 mm display drop included
R = 10; chamfer = 3; edge_r = 1.6;
win = [13.8, 19.6, 85.8, 114.9];          // display window
keys = [26.73, 48.18, 69.63, 91.08];      // key centres on the top edge
led_w = [18, 7.55]; led_g = [95, 7.55];   // white status sphere, green mode sphere
cap = [66.6, 36, 38, 67, 12, 6, 3];       // x0 y0 w l r h edge_r
usb = [44, 13, 7];                        // x0 w h on the bottom edge
wheel_y = 60; wheel_z = T/2;              // dial axis through the left wall
wheel_d = 24; wheel_t = 8; wheel_gap = 0.5;
sw_y = 25;                                // switch knob slot on the left edge

module rrect(w, h, r) { translate([r, r]) offset(r = r) square([w - 2*r, h - 2*r]); }

// body: 45 degree chamfer on the back edge, rounded front edge
module body() {
    hull() {
        translate([0, 0, chamfer]) linear_extrude(T - chamfer - edge_r) rrect(W, H, R);
        linear_extrude(0.01) offset(delta = -chamfer) rrect(W, H, R);
        for (a = [15 : 15 : 90]) translate([0, 0, T - edge_r + edge_r*sin(a) - 0.01])
            linear_extrude(0.01) offset(delta = -edge_r*(1 - cos(a))) rrect(W, H, R);
    }
}
module cap_dome() {
    x0 = cap[0]; y0 = cap[1]; w = cap[2]; l = cap[3]; r = cap[4]; h = cap[5]; e = cap[6];
    translate([0, 0, 0]) mirror([0, 0, 1]) hull() {
        translate([x0, y0, 0]) linear_extrude(0.01) rrect(w, l, r);
        for (a = [0 : 15 : 90]) translate([x0, y0, 0]) translate([0, 0, h - e + e*sin(a) - 0.01])
            linear_extrude(0.01) offset(delta = -3 - e*(1 - cos(a))) rrect(w, l, r);   // 3 mm top inset
    }
}
module sphere_led(p, c) { color(c) translate([p[0], p[1], T - 9.9/2 + 1.5]) sphere(d = 9.9); }

module thumbwheel() {
    // axis along X, the wheel sits outside the left wall; numbers on the rim every 18 degrees,
    // 1 at the counter-clockwise stop; the number facing the viewer (top of the wheel) is in use
    translate([-wheel_t/2 - wheel_gap, wheel_y, wheel_z]) rotate([0, 90, 0]) {   // half of it inside the edge pocket
        color("#2e3a40") difference() {
            cylinder(d = wheel_d, h = wheel_t);
            for (i = [0 : 39]) rotate([0, 0, i*9 + 4.5]) translate([wheel_d/2, 0, -1]) cylinder(d = 1.2, h = wheel_t + 2, $fn = 12);   // grip ridges
        }
    }
    // numbers upright for the viewer, 18 degrees apart on the rim, 1 above and 12 below; the one at
    // the top of the wheel (facing the viewer) is in use: 5 in this picture
    for (i = [0 : 11]) translate([-wheel_gap, wheel_y, wheel_z]) rotate([(i - 4)*18, 0, 0])
        translate([0, 0, wheel_d/2 - 0.3]) color("Gainsboro")
            linear_extrude(0.6) text(str(i + 1), size = 3.2, halign = "center", valign = "center", font = "Helvetica:style=Bold");
}

module face_details() {
    color("#1d2b2a") translate([win[0], win[1], T - 0.6]) linear_extrude(0.7) rrect(win[2], win[3], 2);
    sphere_led(led_w, "White");
    sphere_led(led_g, "LimeGreen");
    for (x = keys) color("Gainsboro") translate([x - 3.9, H - 1, T - 7.6 - 2.4]) cube([7.8, 4, 7.2]);
    color("DimGray") translate([keys[0] - 20, H, T - 11.2]) cube([92, 1.2, 8]);                 // label plate
    color("Black") translate([usb[0], -0.5, 10]) cube([usb[1], 2, usb[2]]);                     // Pico micro-USB
    // arrow on the bezel margin, pointing left at the wheel
    color("Gainsboro") translate([wheel_t/2 + 1.5, wheel_y, T]) linear_extrude(0.5) polygon([[0, 0], [3.2, 1.8], [3.2, -1.8]]);
    // switch on the left edge: knob through a slot, ON / OFF plate under it
    color("Gainsboro") translate([-1.5, sw_y - 1.3, T - 5.4]) cube([2, 2.6, 2.6]);
    color("DimGray") translate([-0.1, sw_y - 15, T - 12.4]) cube([1.2, 30, 7]);
    color("White") translate([0.05, sw_y + 7, T - 9.5]) rotate([90, 0, 90]) linear_extrude(1.2) text("ON", size = 3.2, halign = "center", valign = "center");
    color("White") translate([0.05, sw_y - 7, T - 9.5]) rotate([90, 0, 90]) linear_extrude(1.2) text("OFF", size = 3.2, halign = "center", valign = "center");
}

module scene() {
    color("#5b7fa6") difference() {
        body();
        translate([win[0], win[1], T - 2]) linear_extrude(5) rrect(win[2], win[3], 2);
        translate([-1, wheel_y - wheel_d/2 - 1.5, -1]) cube([wheel_t/2 + 1.5, wheel_d + 3, T + 2]);   // notch through the edge: the wheel sinks half into it
        translate([-1, sw_y - 4, T - 5.6]) cube([3, 8, 6]);                                                             // switch slot
        translate([usb[0], -1, 10]) cube([usb[1], 3, usb[2]]);
    }
    color("#5b7fa6") cap_dome();
    face_details();
    thumbwheel();
}

if (view == "left") translate([0, 0, 0]) scene();
else scene();
