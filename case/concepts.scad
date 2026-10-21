// hadistory - design concepts for the enclosure, visual only (no functional pockets).
// Same frame as hadistory_case.scad: device seen from the front, X right, Y up, Z towards
// the viewer, front face at z = T_full. Envelope 113.3 x 148.2; keys, LED, USB, switch and
// window where the functional model puts them.
//
// What drives the thickness: the Dupont housings plugged on the HAT header need 33 mm over a
// 5 x 51 mm strip on the right (X 83 to 88, Y 45 to 96); everything else (Pico on its
// standoffs, battery, TP4056) fits in 24 mm. The three concepts spend that difference
// differently:
//   concept = "grip" : 24 mm body, 33 mm vertical grip band on the right (e-reader style)
//   concept = "bump" : 24 mm body, local rounded bump on the back over the Dupont block only
//   concept = "slim" : 24 mm all over, needs the HAT wired flat (ribbon or soldered) instead
//                      of Dupont housings
// Render: see "Design directions" in README.md (front and back views, same camera, view = "front" | "back")

concept = "grip";
view = "front";   // "front" | "back": the back view turns the model over so the fixed light of OpenSCAD falls on it
$fn = 72;

W = 113.3; H = 148.2;
T_full = 33;      // over the HAT Dupont block
T_body = 24;      // elsewhere: Pico 18 mm from the back face, HAT PCB and bezel
R = 10;           // corner radius in plan (4 on the functional case)
E = 5;            // edge rounding
win = [13.8, 19.6, 85.8, 114.9];      // display window, centred on the PCB
keys = [26.73, 48.18, 69.63, 91.08];  // key centres on the top edge
led = [18, 7.5];
dupont = [83, 45, 5, 51];              // header strip under the Dupont block

module rrect(w, h, r) { translate([r, r]) offset(r = r) square([w - 2*r, h - 2*r]); }
// rounded box with rounded edges, standing on z = 0
module rbox(w, h, t, r, e) {
    hull() for (x = [r, w - r], y = [r, h - r]) translate([x, y, 0]) {
        translate([0, 0, e]) cylinder(r = r, h = t - 2*e);
        for (z = [e, t - e]) translate([0, 0, z]) rotate_extrude() translate([r - e, 0]) circle(e);
    }
}

module face_details() {
    t = T_full;
    color("#1d2b2a") translate([win[0], win[1], t - 0.6]) linear_extrude(0.7) rrect(win[2], win[3], 2);
    color("LimeGreen") translate([led[0], led[1], t + 1]) sphere(d = 9.9);
    for (x = keys) color("Gainsboro") translate([x - 3.9, H - 1, t - 7.6]) cube([7.8, 4, 7.2]);
    color("Black") translate([44, -0.5, 10]) cube([13, 2, 7]);           // Pico micro-USB
    color("Gainsboro") translate([82, -0.5, t - 5.4]) cube([8, 2, 2.6]);  // switch knob
}

module window_cut() { translate([win[0], win[1], T_full - 2]) linear_extrude(5) rrect(win[2], win[3], 2); }
module body() { translate([0, 0, T_full - T_body]) rbox(W, H, T_body, R, E); }

// 24 mm body, thick vertical band on the right over the Dupont block, blended into the body:
// holds like an e-reader, the thumb lands on the band; the keys stay on the top edge.
module concept_grip() {
    band = 40;
    color("#5b7fa6") difference() {
        union() {
            body();
            hull() {
                translate([W - band, 0, 0]) rbox(band, H, T_full, R, E);
                translate([W - band - 16, 0, T_full - T_body - 1]) rbox(band + 16, H, T_body + 1, R, E);
            }
        }
        window_cut();
    }
    face_details();
}

// 24 mm body, pillow-shaped bump on the back over the Dupont block only (X 70 to 113,
// Y 32 to 108), like a camera bump; the rest of the back is flat and thin.
module concept_bump() {
    color("#5b7fa6") difference() {
        union() {
            body();
            hull() {
                translate([70, 32, 0]) rbox(W - 70, 76, T_full, 14, E);
                translate([58, 20, T_full - T_body - 1]) rbox(W - 58, 100, T_body + 1, 14, E);
            }
        }
        window_cut();
    }
    face_details();
}

// 24 mm all over: only possible if the HAT header is wired flat (2x20 IDC ribbon about
// 10 mm high, or wires soldered on the header pins from the back) instead of Dupont housings.
module concept_slim() {
    color("#5b7fa6") difference() { body(); window_cut(); }
    face_details();
}

module scene() {
    if (concept == "grip") concept_grip();
    else if (concept == "bump") concept_bump();
    else concept_slim();
}
if (view == "back") translate([W, 0, T_full]) rotate([0, 180, 0]) scene();
else scene();
