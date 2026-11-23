# -*- coding: utf-8 -*-
"""
MANSA — Vérificateur HORS Unreal du parcours d'escalade de Djenné (D3-D7).

Construit le graphe des prises de design/data/djenne_greybox.json avec les règles de
design/data/DT_PrisesEscalade.csv (mêmes règles que UMansaClimbingComponent) et vérifie :
  1. qu'un chemin existe du sol (CP1) jusqu'à la terrasse (D01), via l'option A ET via l'option B ;
  2. que la BancoFragile est obligatoire (on ne peut pas la contourner) ;
  3. que la mosquée n'a aucune prise.

Modèle (identique au C++) :
  - point de saisie d'une prise = centre ; d'une Corniche = point le plus proche sur son segment
    (axe = plus grande dimension horizontale) ;
  - saut prise -> prise si écart latéral (horizontal) <= DistanceMaxLaterale et écart vertical
    <= DistanceMaxVerticale (valeurs de la prise VISÉE), normales non opposées ;
  - saisie depuis un sol : prise à une hauteur 0 < h <= HauteurSaisieDepuisSol au-dessus du sol,
    à une distance horizontale <= DistanceSaisieDepuisSol d'un point où Sira peut se tenir, côté
    extérieur du mur ;
  - mantle depuis une Corniche autorisée : arrivée à (rayon + avancée) derrière l'arête, sur le sol
    le plus haut à cet endroit.

Usage : python tools/unreal/verifier_chemin_escalade.py [chemin_json] [chemin_csv]
Code de retour 0 si tout est OK.
"""
import csv
import json
import math
import os
import sys
from collections import deque

RACINE = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
CHEMIN_JSON = sys.argv[1] if len(sys.argv) > 1 else os.path.join(RACINE, "design", "data", "djenne_greybox.json")
CHEMIN_CSV = sys.argv[2] if len(sys.argv) > 2 else os.path.join(RACINE, "design", "data", "DT_PrisesEscalade.csv")

# Réglages du personnage (BP_Sira / UMansaClimbingComponent) : gardés en phase avec le C++.
RAYON_CAPSULE = 42.0
AVANCEE_MANTLE = 60.0
PAS_ECHANTILLON = 10.0
COS_NORMALES_MIN = -0.3


def charger():
    d = json.load(open(CHEMIN_JSON, encoding="utf-8"))
    regles = {}
    with open(CHEMIN_CSV, encoding="utf-8") as f:
        for ligne in csv.DictReader(f):
            nom = ligne.pop("---")
            regles[nom] = {k: (v == "True" if v in ("True", "False") else float(v)) for k, v in ligne.items()}
    return d, regles


# ---------------------------------------------------------------------------
# Géométrie
# ---------------------------------------------------------------------------
class Prise:
    def __init__(self, pr, regles):
        self.id = pr["id"]
        self.type = pr["type"]
        self.r = regles[self.type]
        self.c = tuple(float(v) for v in pr["centre"])
        self.n = tuple(float(v) for v in pr["normale"])
        dx, dy, _ = pr["dims"]
        self.segment = None
        if self.type == "Corniche":
            if dx >= dy:
                self.segment = ((self.c[0] - dx / 2, self.c[1], self.c[2]), (self.c[0] + dx / 2, self.c[1], self.c[2]))
            else:
                self.segment = ((self.c[0], self.c[1] - dy / 2, self.c[2]), (self.c[0], self.c[1] + dy / 2, self.c[2]))
        mantle = pr.get("mantle_autorise")
        self.mantle = self.r["bMantleAutorise"] if mantle is None else bool(mantle)
        self.section = pr["section"]

    def point_proche(self, p):
        if not self.segment:
            return self.c
        a, b = self.segment
        ab = [b[i] - a[i] for i in range(3)]
        l2 = sum(v * v for v in ab)
        t = max(0.0, min(1.0, sum((p[i] - a[i]) * ab[i] for i in range(3)) / l2))
        return tuple(a[i] + ab[i] * t for i in range(3))

    def points_echantillon(self):
        """Points de saisie possibles (centre, ou tout le segment tous les 10 cm)."""
        if not self.segment:
            return [self.c]
        a, b = self.segment
        n = max(1, int(math.dist(a, b) / PAS_ECHANTILLON))
        return [tuple(a[i] + (b[i] - a[i]) * k / n for i in range(3)) for k in range(n + 1)]


def ecarts(p, q):
    return math.hypot(q[0] - p[0], q[1] - p[1]), q[2] - p[2]


def saut_possible(src, dst):
    """Meilleur couple de points de saisie (src sur sa prise, dst sur la cible) et écarts."""
    if sum(src.n[i] * dst.n[i] for i in range(3)) < COS_NORMALES_MIN:
        return None
    meilleur = None
    for p in src.points_echantillon():
        q = dst.point_proche(p)
        lat, vert = ecarts(p, q)
        if lat <= dst.r["DistanceMaxLaterale"] + 1e-6 and abs(vert) <= dst.r["DistanceMaxVerticale"] + 1e-6:
            cout = math.hypot(lat, vert)
            if meilleur is None or cout < meilleur[0]:
                meilleur = (cout, lat, vert)
    return meilleur


class Sols:
    """Boîtes avec collision : le sol en (x, y) est le sommet de la boîte la plus haute qui contient (x, y)."""

    def __init__(self, elements):
        self.boites = []
        for e in elements:
            if e["forme"] != "boite" or not e.get("collision", True) or (e.get("rotation") or {}).get("yaw", 0):
                continue
            (cx, cy, cz), (dx, dy, dz) = e["centre"], e["dims"]
            self.boites.append((e["id"], cx - dx / 2, cx + dx / 2, cy - dy / 2, cy + dy / 2, cz + dz / 2))

    def sol(self, x, y, z_max=1e9):
        meilleur = None
        for b in self.boites:
            if b[1] <= x <= b[2] and b[3] <= y <= b[4] and b[5] <= z_max:
                if meilleur is None or b[5] > meilleur[5]:
                    meilleur = b
        return meilleur

    def debout_possible(self, x, y, boite):
        """La capsule tient debout sur `boite` en (x, y) : aucune boîte plus haute sous son disque."""
        for ox, oy in ((0, 0), (RAYON_CAPSULE, 0), (-RAYON_CAPSULE, 0), (0, RAYON_CAPSULE), (0, -RAYON_CAPSULE)):
            b = self.sol(x + ox, y + oy)
            if b is None or b[5] > boite[5] + 1e-6:
                return False
        b = self.sol(x, y)
        return b is not None and b[0] == boite[0]


def saisie_depuis_sol(sols, boite, prise):
    """Peut-on saisir `prise` depuis le dessus de `boite` ? Renvoie (distance, hauteur) ou None."""
    h = prise.c[2] - boite[5]
    if not (0 < h <= prise.r["HauteurSaisieDepuisSol"] + 1e-6):
        return None
    portee = prise.r["DistanceSaisieDepuisSol"]
    for q in prise.points_echantillon():
        x0, x1 = max(boite[1], q[0] - portee), min(boite[2], q[0] + portee)
        y0, y1 = max(boite[3], q[1] - portee), min(boite[4], q[1] + portee)
        x = x0
        while x <= x1:
            y = y0
            while y <= y1:
                d = math.hypot(x - q[0], y - q[1])
                cote = (x - q[0]) * prise.n[0] + (y - q[1]) * prise.n[1]
                if d <= portee and cote > 0 and sols.debout_possible(x, y, boite):
                    return d, h
                y += PAS_ECHANTILLON
            x += PAS_ECHANTILLON
    return None


def arrivee_mantle(sols, prise, point):
    n = prise.n
    recul = RAYON_CAPSULE + AVANCEE_MANTLE
    x, y = point[0] - n[0] * recul, point[1] - n[1] * recul
    b = sols.sol(x, y, z_max=point[2] + 30)
    if b is None or b[5] < point[2] - 150:
        return None
    return b if sols.debout_possible(x, y, b) else None


# ---------------------------------------------------------------------------
# Graphe
# ---------------------------------------------------------------------------
def construire(d, regles, exclues=()):
    prises = [Prise(p, regles) for p in d["prises"] if p["id"] not in exclues and regles[p["type"]]["bSaisissable"]]
    sols = Sols(d["elements"])
    aretes = {}

    def ajouter(a, b, info):
        aretes.setdefault(a, []).append((b, info))

    noms_sols = {b[0]: b for b in sols.boites}
    for p in prises:
        for q in prises:
            if p is not q:
                s = saut_possible(p, q)
                if s:
                    ajouter(p.id, q.id, "saut lat %.0f / vert %+.0f" % (s[1], s[2]))
        for b in sols.boites:
            s = saisie_depuis_sol(sols, b, p)
            if s:
                ajouter("SOL:" + b[0], p.id, "saisie depuis le sol (dist %.0f, haut %.0f)" % s)
        if p.mantle:
            for pt in p.points_echantillon():
                b = arrivee_mantle(sols, p, pt)
                if b:
                    ajouter(p.id, "SOL:" + b[0], "mantle -> %s (Z %.0f)" % (b[0], b[5]))
                    break
    # Marcher : on suppose qu'on peut marcher partout sur le dessus d'une même boîte.
    return prises, sols, aretes, noms_sols


def chemin(aretes, depart, but):
    prec = {depart: None}
    file = deque([depart])
    while file:
        u = file.popleft()
        if u == but:
            break
        for v, info in aretes.get(u, []):
            if v not in prec:
                prec[v] = (u, info)
                file.append(v)
    if but not in prec:
        return None
    etapes, u = [], but
    while prec[u]:
        u0, info = prec[u]
        etapes.append((u0, u, info))
        u = u0
    return list(reversed(etapes))


def main():
    d, regles = charger()
    echecs = []

    def verif(cond, msg):
        print(("OK    " if cond else "ECHEC ") + msg)
        if not cond:
            echecs.append(msg)

    cp1 = next(g for g in d["gameplay"] if g["id"] == "CP1")["position"]
    _, sols, _, _ = construire(d, regles)
    sol_depart = sols.sol(cp1[0], cp1[1], z_max=cp1[2] + 10)
    depart = "SOL:" + sol_depart[0]
    but = "SOL:D01"
    print("Départ : %s (CP1), but : %s (terrasse)\n" % (depart, but))

    ids_a = [p["id"] for p in d["prises"] if p["id"].startswith("D3-A")]
    ids_b = [p["id"] for p in d["prises"] if p["id"].startswith("D3-B")]
    for nom, exclues in (("complet", ()), ("option A (sans D3-B*)", ids_b), ("option B (sans D3-A*)", ids_a)):
        _, _, aretes, _ = construire(d, regles, exclues)
        c = chemin(aretes, depart, but)
        verif(c is not None, "chemin %s du sol au sommet" % nom)
        if c:
            for u, v, info in c:
                print("        %-10s -> %-10s %s" % (u, v, info))

    fragile = [p["id"] for p in d["prises"] if p["type"] == "BancoFragile"]
    _, _, aretes, _ = construire(d, regles, fragile)
    verif(chemin(aretes, depart, but) is None, "la BancoFragile %s est obligatoire (aucun chemin sans elle)" % fragile)

    _, _, aretes, _ = construire(d, regles)
    secours = [v for v, _ in aretes.get(fragile[0], [])] if fragile else []
    verif(any(v == "D2-06" for v in secours), "toron de secours D2-06 atteignable depuis la prise fragile (%s)" % secours)

    mos = [e for e in d["elements"] if e["dossier"] == "C_Mosquee"]
    lo = [min(e["centre"][i] - e["dims"][i] / 2 for e in mos) for i in range(3)]
    hi = [max(e["centre"][i] + e["dims"][i] / 2 for e in mos) for i in range(3)]
    dedans = [p["id"] for p in d["prises"] if all(lo[i] <= p["centre"][i] <= hi[i] for i in range(3))]
    verif(not dedans, "aucune prise sur la mosquée %s" % dedans)

    print("\n" + ("TOUT EST OK" if not echecs else "%d ÉCHEC(S)" % len(echecs)))
    return 1 if echecs else 0


if __name__ == "__main__":
    sys.exit(main())
