// hadistory - design concepts for the enclosure, visual only (no functional pockets).
// concept = "grip"  : thin body, thick vertical grip on the left over the HAT Dupont block
// concept = "wedge" : body thicker at the bottom, thin at the top, doubles as a stand
// Dimensions follow hadistory_case.scad: 114 x 148 face, 33 mm where the Dupont block is.

concept = "grip";
$fn = 64;

W = 114; H = 148;
T_thin = 19;      // thin part of the back
T_thick = 33;     // over the HAT Dupont block
R = 10;           // corner radius
E = 4;            // edge rounding
win_x = 14; win_y = 20; win_w = 86; win_h = 115;   // display window (approx.)
led = [33, 9];

module rbox(w, h, t, r, e) {   // rounded box, rounded edges e, standing on z = 0
    hull() for (x = [r, w - r], y = [r, h - r], z = [e, t - e]) translate([x, y, z]) sphere(e);
    hull() for (x = [r, w - r], y = [r, h - r]) translate([x, y, 0]) {
        translate([0, 0, e]) cylinder(r = r, h = t - 2*e);
    }
}
module rrect(w, h, r) { translate([r, r]) offset(r = r) square([w - 2*r, h - 2*r]); }

module face_details(t) {
    // display window, dark glass just under the bezel
    color("#1d2b2a") translate([win_x, win_y, t - 0.4]) linear_extrude(0.5) rrect(win_w, win_h, 2);
    color("LimeGreen") translate([led[0], led[1], t + 1]) sphere(d = 9.9);
    // keys on the top edge
    for (i = [0 : 3]) color("Gainsboro") translate([24 + i*21.45 - 4, H - 1, t - 4.2]) rbox(8, 3, 2.8, 1, 0.6);
    // USB and switch on the bottom edge
    color("Black") translate([64, -0.5, 10]) cube([13, 2, 7]);
    color("Gainsboro") translate([96, -0.5, t - 5]) cube([8, 2, 2.6]);
}

// The front face is flat at z = T_thick for the whole device; the back steps up (towards
// the face) outside the grip, so the thin part is T_thin thick and the grip T_thick.
module concept_grip() {
    color("#5b7fa6") difference() {
        union() {
            translate([0, 0, T_thick - T_thin]) rbox(W, H, T_thin, R, E);
            // grip: thick vertical band on the left, blended into the body
            hull() {
                rbox(34, H, T_thick, R, E);
                translate([0, 0, T_thick - T_thin - 2]) rbox(48, H, T_thin + 2, R, E);
            }
        }
        translate([win_x, win_y, T_thick - 2]) linear_extrude(5) rrect(win_w, win_h, 2);
    }
    face_details(T_thick);
}

module concept_wedge() {
    // thick at the bottom (y = 0), thin at the top
    color("#5b7fa6") difference() {
        hull() {
            translate([0, 0, 0]) rbox(W, 40, T_thick, R, E);
            translate([0, H - 40, 0]) rbox(W, 40, 16, R, E);
        }
        // the face is not flat in a plain wedge: keep the front flat by cutting a plane
        translate([-1, -1, T_thick]) cube([W + 2, H + 2, 40]);
        translate([win_x, win_y, T_thick - 2]) linear_extrude(5) rrect(win_w, win_h, 2);
    }
    face_details(T_thick);
}

// For the wedge the front must stay flat: the thickness runs on the back. Model it as a
// flat-faced block whose back slopes.
module concept_wedge_flat() {
    color("#5b7fa6") difference() {
        intersection() {
            rbox(W, H, T_thick, R, E);
            // sloping back: keep what is above the plane going from z = 0 (top of device) to z = T_thick - 16 (bottom)
            hull() {
                translate([-1, -1, T_thick - 16]) cube([W + 2, 1, 40]);
                translate([-1, H, 0]) cube([W + 2, 1, 40]);
            }
        }
        translate([win_x, win_y, T_thick - 2]) linear_extrude(5) rrect(win_w, win_h, 2);
    }
    face_details(T_thick);
}

if (concept == "grip") concept_grip();
else concept_wedge_flat();
