# -*- coding: utf-8 -*-
"""Contrôle hors Unreal du JSON du greybox Djenné. Usage : python verifier_djenne_greybox.py [chemin_json]"""
import json, os, sys

chemin = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "design", "data", "djenne_greybox.json")
d = json.load(open(chemin, encoding="utf-8"))
echecs = []


def verif(cond, msg):
    print(("OK    " if cond else "ECHEC ") + msg)
    if not cond:
        echecs.append(msg)


els, prs, gp = d["elements"], d["prises"], d["gameplay"]
verif(len(els) == d["meta"]["nb_elements_attendu"], f"{len(els)} éléments (attendu {d['meta']['nb_elements_attendu']})")
verif(len(prs) == 23, f"{len(prs)} prises (attendu 23)")
sec = {s: sum(p["section"] == s for p in prs) for s in ("D1", "D2", "D3")}
verif(sec == {"D1": 5, "D2": 8, "D3": 10}, f"répartition des prises {sec}")
fr = [p["id"] for p in prs if p["type"] == "BancoFragile"]
verif(len(fr) == 1, f"BancoFragile unique : {fr}")
ids = [x["id"] for x in els + prs + gp]
verif(len(ids) == len(set(ids)), "identifiants uniques")
nul = [x["id"] for x in els + prs if any(v <= 0 for v in x["dims"])]
nul += [g["id"] for g in gp if "dims" in g and any(v <= 0 for v in g["dims"])]
verif(not nul, f"aucune dimension nulle ou négative {nul}")

# Boîte englobante de la mosquée (union des éléments C_Mosquee)
mos = [e for e in els if e["dossier"] == "C_Mosquee"]
lo = [min(e["centre"][i] - e["dims"][i] / 2 for e in mos) for i in range(3)]
hi = [max(e["centre"][i] + e["dims"][i] / 2 for e in mos) for i in range(3)]
print(f"      boîte mosquée : min {lo} max {hi}")
dedans = [p["id"] for p in prs if all(lo[i] <= p["centre"][i] <= hi[i] for i in range(3))]
verif(not dedans, f"aucune prise dans la boîte de la mosquée {dedans}")

mats = set(d["materiaux"])
inconnus = sorted({x["materiau"] for x in els + prs} - mats)
verif(not inconnus, f"matériaux référencés tous définis {inconnus}")
verif(all(m["couleur"] in d["palette"] for m in d["materiaux"].values()), "couleurs des matériaux présentes dans la palette")
verif(all(x["forme"] in d["config"]["meshes"] for x in els + prs), "formes connues")
verif(all(x.get("axe", "Z") in ("X", "Y", "Z") for x in els + prs), "axes valides")
verif(sum(g["type"] == "PlayerStart" for g in gp) == 1, "un seul PlayerStart")

print("\n" + ("TOUT EST OK" if not echecs else f"{len(echecs)} ÉCHEC(S)"))
sys.exit(1 if echecs else 0)
