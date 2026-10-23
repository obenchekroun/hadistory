# Generates pico/img/wiring.svg: Fritzing-like wiring of the hadistory Pico version.
import sys

W, H = 1500, 1260
out = []
def add(s): out.append(s)

# ---------------------------------------------------------------- geometry
PITCH = 24
PY0 = 230                      # y of row 0
def row_y(i): return PY0 + i * PITCH
PICO_X0, PICO_X1 = 760, 920    # Pico body
HAT_XE, HAT_XO = 1180, 1212    # HAT header columns seen from the BACK: even on the left, odd on the right
HAT_X0, HAT_X1 = 1166, 1226    # header block
HAT_XM = (HAT_XE + HAT_XO) // 2  # between the columns, where a wire to an odd pin passes

# Pico pin names, physical 1..40
left = ["GP0", "GP1", "GND", "GP2", "GP3", "GP4", "GP5", "GND", "GP6", "GP7",
        "GP8", "GP9", "GND", "GP10", "GP11", "GP12", "GP13", "GND", "GP14", "GP15"]
right_bottom_up = ["GP16", "GP17", "GND", "GP18", "GP19", "GP20", "GP21", "GND", "GP22", "RUN",
                   "GP26", "GP27", "GND", "GP28", "ADC_VREF", "3V3 OUT", "3V3 EN", "GND", "VSYS", "VBUS"]
def pico_pin(n):               # -> (x, y) of the pin centre, side
    if n <= 20: return (PICO_X0, row_y(n - 1), "L")
    return (PICO_X1, row_y(40 - n), "R")
def pico_name(n): return left[n - 1] if n <= 20 else right_bottom_up[n - 21]

def hat_pin(n):                # back view: odd pins on the right column
    i = (n - 1) // 2
    return (HAT_XO if n % 2 else HAT_XE, row_y(i))
def hat_entry(n):              # points from the gap into the pin: even direct, odd between two even pads then a jog
    x, y = hat_pin(n)
    if n % 2 == 0: return y, [(HAT_XE - 5, y)]
    return y - 12, [(HAT_XM, y - 12), (HAT_XM, y), (HAT_XO - 5, y)]

# Signals on the HAT header (Pi GPIO naming) for the labels
hat_lbl = {1: "3V3", 2: "5V (nc)", 6: "GND", 11: "GPIO17 BUSY", 13: "GPIO27 RESET", 15: "GPIO22 DC",
           18: "GPIO24 bouton D", 19: "GPIO10 MOSI", 23: "GPIO11 SCLK", 24: "GPIO8 CS",
           29: "GPIO5 bouton A", 31: "GPIO6 bouton B", 36: "GPIO16 bouton C", 27: "ID_SD (nc)", 28: "ID_SC (nc)"}

# ---------------------------------------------------------------- helpers
def wire(pts, color, w=3):
    d = " ".join(f"{'M' if i == 0 else 'L'}{x},{y}" for i, (x, y) in enumerate(pts))
    add(f'<path d="{d}" fill="none" stroke="{color}" stroke-width="{w}" stroke-linecap="round" stroke-linejoin="round"/>')
def dot(x, y, color): add(f'<circle cx="{x}" cy="{y}" r="4" fill="{color}"/>')
def text(x, y, s, size=11, anchor="start", color="#222", weight="normal", family="Helvetica, Arial, sans-serif"):
    add(f'<text x="{x}" y="{y}" font-size="{size}" text-anchor="{anchor}" fill="{color}" font-weight="{weight}" font-family="{family}">{s}</text>')
def rect(x, y, w, h, fill, stroke="#333", rx=3, sw=1.5):
    add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"/>')
def pad(x, y, s=12, fill="#d4af37"):
    add(f'<rect x="{x - s/2}" y="{y - s/2}" width="{s}" height="{s}" fill="{fill}" stroke="#555" stroke-width="1"/>')

C = {  # wire colours
    "VBUS": "#e8711a", "VSYS": "#c0392b", "GND": "#111111", "3V3": "#e74c3c",
    "SCLK": "#f1c40f", "MOSI": "#2980b9", "CS": "#27ae60", "DC": "#8e44ad", "RESET": "#8d5524", "BUSY": "#16a085",
    "A": "#7f8c8d", "B": "#2c3e50", "C": "#d35400", "D": "#1abc9c", "LED": "#ff69b4", "DIAL": "#3949ab", "BAT+": "#c0392b", "BAT-": "#111111",
}

add(f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">')
add(f'<rect width="{W}" height="{H}" fill="#f7f7f2"/>')
text(40, 1246, "hadistory · câblage de la version Pico 2 W (brochage par défaut de hadistory_config.h). Fils : couleur = signal, voir la légende. Broches = numéros physiques.", 12, color="#555")

# ---------------------------------------------------------------- Pico 2 W
rect(PICO_X0, 206, PICO_X1 - PICO_X0, row_y(19) + 24 - 206, "#2e7d32", "#1b5e20", rx=6)
rect(PICO_X0 + 55, 196, 50, 16, "#bbb", "#777", rx=2)        # micro-USB
text((PICO_X0 + PICO_X1) / 2, 192, "micro-USB : charge + console", 10, "middle", "#555")
text((PICO_X0 + PICO_X1) / 2, 720, "Raspberry Pi Pico 2 W (vu de dessus)", 11, "middle", "#1b5e20", "bold")
rect(PICO_X0 + 67, 300, 26, 26, "#9e9e9e", "#616161", rx=2)    # RP2350
text(PICO_X0 + 80, 342, "RP2350", 7, "middle", "#cfd8dc")
rect(PICO_X0 + 67, 560, 26, 40, "#9e9e9e", "#616161", rx=2)    # CYW43 can
text(PICO_X0 + 80, 614, "CYW43", 7, "middle", "#cfd8dc")
for n in range(1, 41):
    x, y, side = pico_pin(n)
    pad(x, y, 11, "#e0c46c")
    name = pico_name(n)
    power = name in ("GND", "VSYS", "VBUS", "3V3 OUT")
    col = "#fff" if not power else "#ffe082"
    if side == "L":
        text(x + 10, y + 4, f"{n}", 8, "start", "#cfd8dc")
        text(x + 24, y + 4, name, 9.5, "start", col, "bold" if power else "normal")
    else:
        text(x - 10, y + 4, f"{n}", 8, "end", "#cfd8dc")
        text(x - 24, y + 4, name, 9.5, "end", col, "bold" if power else "normal")

# ---------------------------------------------------------------- HAT header (Inky Impression, 40 pins)
rect(HAT_X0, 212, HAT_X1 - HAT_X0, row_y(19) + 18 - 212, "#1a1a1a", "#000", rx=2)
text((HAT_X0 + HAT_X1) / 2, 206, "header 40 broches du display HAT, vu de dos", 10, "middle", "#333", "bold")
text((HAT_X0 + HAT_X1) / 2, 196, "(côté où l'on branche les fils : broche 1 = 3V3 en haut à droite, impaires à droite)", 9, "middle", "#555")
for n in range(1, 41):
    x, y = hat_pin(n)
    used = n in hat_lbl and "(nc)" not in hat_lbl[n]
    pad(x, y, 10, "#d4af37" if used else "#555")
for i in range(20):
    o, e = 2 * i + 1, 2 * i + 2
    lo = hat_lbl.get(o, ""); le = hat_lbl.get(e, "")
    s = f"{e}" + (f" {le}" if le else "") + "  ·  " + f"{o}" + (f" {lo}" if lo else "")
    text(1300, row_y(i) + 4, s, 10, "start", "#222" if (lo or le) else "#999")
text(1300, 218, "pair (gauche) · impair (droite)", 9, "start", "#555")

# ---------------------------------------------------------------- signal wires Pico -> HAT (left column, from the gap)
gap = lambda k: 990 + 14 * k
def pico_to_hat(pn, hn, k, sig):
    px, py, _ = pico_pin(pn); ey, entry = hat_entry(hn)
    wire([(px, py), (gap(k), py), (gap(k), ey)] + entry, C[sig])
# gap channels inner -> outer, in the order of the Pico pins (top to bottom) so that nothing crosses;
# channel 6 is kept for button D, the only wire out of order (3 crossings, unavoidable)
pico_to_hat(36, 1, 0, "3V3")
pico_to_hat(33, 6, 1, "GND")      # a GND of its own for the HAT, pin 38 goes to the TP4056
pico_to_hat(29, 11, 2, "BUSY")
pico_to_hat(27, 13, 3, "RESET")
pico_to_hat(26, 15, 4, "DC")
pico_to_hat(25, 19, 5, "MOSI")
pico_to_hat(24, 23, 7, "SCLK")
pico_to_hat(22, 24, 8, "CS")

# ---------------------------------------------------------------- buttons: Pico left pins 4..7 -> HAT, around the bottom
def button(pn, hn, xl, yb, xg, sig):
    px, py, _ = pico_pin(pn); ey, entry = hat_entry(hn)
    wire([(px, py), (xl, py), (xl, yb), (xg, yb), (xg, ey)] + entry, C[sig])
button(4, 29, 692, 876, gap(11), "A")     # A: pin 4 -> HAT 29
button(5, 31, 704, 864, gap(10), "B")     # B: pin 5 -> HAT 31
button(6, 36, 716, 852, gap(9), "C")      # C: pin 6 -> HAT 36
button(7, 18, 728, 840, gap(6), "D")      # D: pin 7 -> HAT 18, between MOSI and SCLK

# ---------------------------------------------------------------- power: VBUS / VSYS / GND up and to the left column
px40 = pico_pin(40); px39 = pico_pin(39); px38 = pico_pin(38)
TP_X0, TP_Y0, TP_W, TP_H = 300, 380, 180, 80
TP_INP, TP_INM = 400, 440          # top pads
TP_BP, TP_BM = 360, 420            # bottom pads
wire([(px39[0], px39[1]), (955, px39[1]), (955, 40), (120, 40), (120, 150)], C["VSYS"])            # VSYS -> diode cathode
wire([(px40[0], px40[1]), (940, px40[1]), (940, 60), (TP_INP, 60), (TP_INP, TP_Y0)], C["VBUS"])    # VBUS -> IN+
wire([(px38[0], px38[1]), (970, px38[1]), (970, 80), (TP_INM, 80), (TP_INM, TP_Y0)], C["GND"])     # GND -> IN-
text(600, 36, "VSYS ← diode ← switch ← batterie", 10, "middle", C["VSYS"])
text(600, 56, "VBUS (5 V de l'USB du Pico) → TP4056 IN+", 10, "middle", C["VBUS"])
text(600, 76, "GND commun → TP4056 IN-", 10, "middle", C["GND"])

# TP4056
rect(TP_X0, TP_Y0, TP_W, TP_H, "#1f5fa8", "#123c6b", rx=4)
rect(TP_X0 + 8, TP_Y0 + 8, 60, 16, "#bbb", "#777", rx=2); text(TP_X0 + 38, TP_Y0 + 20, "USB inutilisé", 8, "middle", "#333")
text(TP_X0 + TP_W / 2, TP_Y0 + 44, "TP4056", 13, "middle", "#fff", "bold")
for x, lbl in ((TP_INP, "IN+"), (TP_INM, "IN-")):
    pad(x, TP_Y0); text(x, TP_Y0 + 24, lbl, 10, "middle", "#fff", "bold")
for x, lbl in ((TP_BP, "B+"), (TP_BM, "B-")):
    pad(x, TP_Y0 + TP_H); text(x, TP_Y0 + TP_H - 12, lbl, 10, "middle", "#fff", "bold")
text(TP_X0 + TP_W + 8, TP_Y0 + 56, "module avec OUT+ / OUT- :", 9, "start", "#555")
text(TP_X0 + TP_W + 8, TP_Y0 + 68, "OUT+ vers le switch à la place de B+", 9, "start", "#555")

# Battery
BAT_Y0 = 540
rect(TP_X0, BAT_Y0, TP_W, 80, "#9e9e9e", "#616161", rx=6)
text(TP_X0 + TP_W / 2, BAT_Y0 + 46, "LiPo 103450  3,7 V", 12, "middle", "#111", "bold")
text(TP_X0 + TP_W / 2, BAT_Y0 + 64, "(protection intégrée à la cellule)", 9, "middle", "#333")
pad(TP_BP, BAT_Y0, 12, "#c0392b"); text(TP_BP, BAT_Y0 - 16, "+", 14, "middle", C["BAT+"], "bold")
pad(TP_BM, BAT_Y0, 12, "#111");    text(TP_BM, BAT_Y0 - 16, "−", 14, "middle", "#111", "bold")
wire([(TP_BP, TP_Y0 + TP_H), (TP_BP, BAT_Y0)], C["BAT+"])
wire([(TP_BM, TP_Y0 + TP_H), (TP_BM, BAT_Y0)], C["BAT-"])

# Switch SS12F15 (pins on its top edge: left, centre, right)
SW_X0, SW_Y0 = 60, 300
rect(SW_X0, SW_Y0, 140, 50, "#ececec", "#555", rx=4)
rect(SW_X0 + 85, SW_Y0 + 14, 30, 22, "#333", "#111", rx=2)     # knob
text(SW_X0 + 70, SW_Y0 + 72, "switch SS12F15", 11, "middle", "#222", "bold")
text(SW_X0 + 70, SW_Y0 + 86, "milieu = commun, un côté = sortie", 9, "middle", "#555")
for x in (95, 130, 165): pad(x, SW_Y0, 10, "#bbb")
# B+ -> switch centre (around the left of the TP4056)
dot(TP_BP, 500, C["BAT+"])
wire([(TP_BP, 500), (240, 500), (240, 280), (130, 280), (130, SW_Y0)], C["BAT+"])
# switch left -> diode anode
wire([(95, SW_Y0), (95, 260), (120, 260), (120, 210)], C["VSYS"])
# Diode, vertical at x=120: cathode (band) at the top towards VSYS
add('<polygon points="109,196 131,196 120,174" fill="#333"/>')          # triangle points to the cathode (up, towards VSYS)
add('<line x1="108" y1="171" x2="132" y2="171" stroke="#333" stroke-width="3"/>')   # band = cathode
wire([(120, 150), (120, 170)], C["VSYS"]); wire([(120, 195), (120, 210)], C["VSYS"])
text(140, 178, "diode Schottky 1 A", 10, "start", "#222", "bold")
text(140, 191, "1N5817 / 1N5819 / SS14, bague vers VSYS", 9, "start", "#555")
text(140, 204, "(pas de 1N5711 : diode de signal)", 9, "start", "#555")

# ---------------------------------------------------------------- status LED on GP8 (pin 11), GND pin 13
p11 = pico_pin(11); p13 = pico_pin(13)
wire([(p11[0], p11[1]), (736, p11[1]), (736, 780), (790, 780)], C["LED"])            # GP8 -> resistor
rect(790, 774, 50, 12, "#f5deb3", "#8d5524", rx=2); text(815, 770, "220 Ω", 9, "middle", "#222")
wire([(840, 780), (868, 780)], C["LED"])
add('<circle cx="880" cy="780" r="12" fill="#ffb3c6" stroke="#c2185b" stroke-width="1.5"/>')   # LED
add('<polygon points="874,773 874,787 887,780" fill="#c2185b"/>'); add('<line x1="887" y1="773" x2="887" y2="787" stroke="#c2185b" stroke-width="2"/>')
text(880, 806, "LED sphère dLUX-dLITE", 9, "middle", "#222"); text(880, 818, "anode (patte longue) côté résistance", 9, "middle", "#555")
wire([(892, 780), (930, 780), (930, 758), (748, 758), (748, p13[1]), (p13[0], p13[1])], C["GND"])   # cathode -> GND pin 13

# ---------------------------------------------------------------- story dial, option STORY_DIAL: KY-040 on GP6 / GP7 / GP9, "+" from GP14, GND pin 8
# Drawn as a schematic: the D shaft turns two contacts (A = CLK, B = DT) that close to GND, 20 detents a turn;
# the module adds a 10 k pull-up from each of them to its "+" pin; the push button SW closes to GND (no pull-up).
KX, KY, KW, KH = 400, 640, 280, 170
rect(KX, KY, KW, KH, "#1f5fa8", "#123c6b", rx=4)
text(KX + 12, KY + 22, "KY-040", 12, "start", "#fff", "bold")
text(KX + 12, KY + 36, "encodeur rotatif", 8, "start", "#cfd8dc")
# the shaft: knob seen from the top, D flat
kc = (KX + 55, KY + 100)
add(f'<circle cx="{kc[0]}" cy="{kc[1]}" r="28" fill="#2e3a40" stroke="#111" stroke-width="1.5"/>')
add(f'<path d="M{kc[0]-9},{kc[1]-7} A11,11 0 1 1 {kc[0]+9},{kc[1]-7} Z" fill="#9e9e9e" stroke="#555" stroke-width="1"/>')   # D shaft
add(f'<path d="M{kc[0]-36},{kc[1]} A36,36 0 0 1 {kc[0]},{kc[1]-36}" fill="none" stroke="#cfd8dc" stroke-width="1.2" stroke-dasharray="3,3"/>')
text(kc[0], kc[1] - 40, "axe D, 20 crans / tour", 7.5, "middle", "#cfd8dc")
add(f'<line x1="{kc[0]+30}" y1="{kc[1]}" x2="{KX + 140}" y2="{kc[1]}" stroke="#cfd8dc" stroke-width="1" stroke-dasharray="2,3"/>')   # mechanical link
# pads on the top edge (external wires) and the internal nodes under them
kp = {"GND": KX + 148, "CLK": KX + 191, "DT": KX + 234, "SW": KX + 277}
for lbl, x in kp.items():
    pad(x, KY, 10); text(x, KY + 20, lbl, 7.5, "middle", "#fff")
GY = KY + 132                                                                  # GND bus
add(f'<line x1="{kp["GND"]}" y1="{KY}" x2="{kp["GND"]}" y2="{GY}" stroke="#fff" stroke-width="1.5"/>')
add(f'<line x1="{kp["GND"]}" y1="{GY}" x2="{kp["SW"]}" y2="{GY}" stroke="#fff" stroke-width="1.5"/>')
for lbl, name in (("CLK", "A"), ("DT", "B"), ("SW", "poussoir")):
    x = kp[lbl]; ny = KY + 62                                                  # node under the pad
    add(f'<line x1="{x}" y1="{KY}" x2="{x}" y2="{ny}" stroke="#fff" stroke-width="1.5"/>')
    add(f'<circle cx="{x}" cy="{ny}" r="2.2" fill="#fff"/>')
    add(f'<line x1="{x}" y1="{ny}" x2="{x + 9}" y2="{ny + 34}" stroke="#fff" stroke-width="1.5"/>')     # contact blade, open
    add(f'<line x1="{x}" y1="{ny + 40}" x2="{x}" y2="{GY}" stroke="#fff" stroke-width="1.5"/>')
    add(f'<circle cx="{x}" cy="{ny + 40}" r="2.2" fill="#fff"/>')
    text(x + 13, ny + 22, name, 7.5, "start", "#cfd8dc")
    if lbl != "SW":                                                            # 10 k pull-up to "+", drawn as a stub with a flag
        add(f'<line x1="{x}" y1="{ny}" x2="{x - 8}" y2="{ny}" stroke="#fff" stroke-width="1.5"/>')
        rect(x - 24, ny - 4, 16, 8, "#f5deb3", "#8d5524", rx=1, sw=1)
        add(f'<line x1="{x - 24}" y1="{ny}" x2="{x - 30}" y2="{ny}" stroke="#fff" stroke-width="1.5"/>')
        text(x - 32, ny + 3, "+", 9, "end", "#ffe082", "bold")
        text(x - 16, ny - 7, "10 k", 6.5, "middle", "#fff")
pad(KX + KW, KY + 100, 10); text(KX + KW - 9, KY + 103, "+", 9, "end", "#ffe082", "bold")               # "+" pad, right edge
text(KX + KW - 14, KY + 115, "3,3 V des pull-ups", 6.5, "end", "#cfd8dc")
text(KX + 12, KY + KH - 22, "cadran des histoires, option STORY_DIAL", 8, "start", "#cfd8dc")
text(KX + 12, KY + KH - 8, "ordre réel des broches du module : GND  +  SW  DT  CLK", 7, "start", "#cfd8dc")
p8 = pico_pin(8); p9 = pico_pin(9); p10 = pico_pin(10); p12 = pico_pin(12); p19 = pico_pin(19)
wire([(p8[0], p8[1]), (kp["GND"], p8[1]), (kp["GND"], KY)], C["GND"])                       # GND pin 8
wire([(p9[0], p9[1]), (kp["CLK"], p9[1]), (kp["CLK"], KY)], C["DIAL"])                      # CLK GP6
wire([(p10[0], p10[1]), (kp["DT"], p10[1]), (kp["DT"], KY)], C["DIAL"])                     # DT GP7
wire([(p12[0], p12[1]), (kp["SW"], p12[1]), (kp["SW"], KY)], C["DIAL"])                     # SW GP9
wire([(p19[0], p19[1]), (745, p19[1]), (745, KY + 100), (KX + KW, KY + 100)], C["DIAL"])   # "+" from GP14 (pin 19), driven high

# ---------------------------------------------------------------- legend
LX, LY = 40, 925
text(LX, LY, "Légende", 13, "start", "#222", "bold")
rows = [
    ("3V3", "3V3 OUT", "Pico 36", "HAT 1", "alimentation du HAT (le 5V du HAT reste libre)"),
    ("GND", "GND", "Pico 33 / 38 / 13 / 8", "HAT 6 / TP4056 IN- et B- / LED / KY-040", "masse commune, un fil par broche"),
    ("SCLK", "SCLK", "Pico 24 (GP18)", "HAT 23 (GPIO11)", "SPI0 horloge"),
    ("MOSI", "MOSI", "Pico 25 (GP19)", "HAT 19 (GPIO10)", "SPI0 données"),
    ("CS", "CS", "Pico 22 (GP17)", "HAT 24 (GPIO8)", "sélection"),
    ("DC", "DC", "Pico 26 (GP20)", "HAT 15 (GPIO22)", "données / commande"),
    ("RESET", "RESET", "Pico 27 (GP21)", "HAT 13 (GPIO27)", ""),
    ("BUSY", "BUSY", "Pico 29 (GP22)", "HAT 11 (GPIO17)", ""),
    ("A", "bouton A", "Pico 4 (GP2)", "HAT 29 (GPIO5)", "page suivante / exécuter"),
    ("B", "bouton B", "Pico 5 (GP3)", "HAT 31 (GPIO6)", "nouvelle histoire"),
    ("C", "bouton C", "Pico 6 (GP4)", "HAT 36 (GPIO16)", "changer de mode"),
    ("D", "bouton D", "Pico 7 (GP5)", "HAT 18 (GPIO24)", "page précédente"),
    ("LED", "LED", "Pico 11 (GP8)", "résistance 220 Ω, LED, Pico 13 (GND)", "LED d'état : fixe prête, respire occupée, clignote erreur"),
    ("DIAL", "cadran", "Pico 9 / 10 / 12 / 19", "KY-040 CLK (GP6) / DT (GP7) / SW (GP9) / + (GP14 à 1)", "option STORY_DIAL : numéro d'histoire ; le poussoir vaut RESET"),
    ("VBUS", "VBUS", "Pico 40", "TP4056 IN+", "5 V de l'USB du Pico : charge la batterie"),
    ("VSYS", "VSYS", "Pico 39", "diode ← switch ← TP4056 B+", "alimentation sur batterie"),
]
for i, (k, sig, a, b, note) in enumerate(rows):
    y = LY + 18 + i * 16
    add(f'<line x1="{LX}" y1="{y - 4}" x2="{LX + 30}" y2="{y - 4}" stroke="{C[k]}" stroke-width="4" stroke-linecap="round"/>')
    text(LX + 40, y, sig, 10, "start", "#222", "bold")
    text(LX + 110, y, a, 10); text(LX + 240, y, "→  " + b, 10); text(LX + 520, y, note, 10, "start", "#555")

text(LX, LY + 18 + len(rows) * 16 + 10, "Non dessiné : le breakout microSD de STORAGE=SDCARD (SPI0 sur GP10 à GP13, 3V3, GND), voir les tableaux du README. Les boutons se ferment vers la masse du HAT, pull-ups internes du Pico.", 10, "start", "#555")
add('</svg>')
open(sys.argv[1], "w").write("\n".join(out))
print("ok")
