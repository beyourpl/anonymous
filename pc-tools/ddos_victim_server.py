#!/usr/bin/env python3
"""
Serveur « victime » pédagogique pour la démo DDoS labo (sans flood).

Le M5Stick envoie quelques requêtes étiquetées DEMO. Ce serveur augmente
volontairement une jauge de charge et affiche une page web pour le public.

Ce n'est PAS un service à attaquer : la saturation est simulée côté serveur.

Usage (PC de labo, même Wi-Fi que le M5Stick) :
  python3 pc-tools/ddos_victim_server.py
  # puis ouvrir http://<IP_DU_PC>:8080

Configurer l'IP du PC dans include/config.h (LAB_VICTIM_HOST).
"""

from __future__ import annotations

import argparse
import json
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs

DEMO_TOKEN = "CYBER_DEMO_LAB"
load = 0.0
pulses = 0
phase = "idle"  # idle | rising | down | recovery
last_pulse = 0.0
lock = threading.Lock()


def decay_loop() -> None:
    global load, phase
    while True:
        time.sleep(0.2)
        with lock:
            if phase == "rising":
                load = max(0.0, load - 0.4)
            elif phase == "recovery":
                load = max(0.0, load - 1.5)
                if load <= 0:
                    phase = "idle"
                    load = 0.0


class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt: str, *args) -> None:
        print(f"[victim] {self.address_string()} {fmt % args}")

    def _send(self, code: int, body: bytes, content_type: str) -> None:
        self.send_response(code)
        self.send_header("Content-Type", content_type)
        self.send_header("Cache-Control", "no-store")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self) -> None:
        if self.path.startswith("/api/status"):
            with lock:
                payload = {
                    "load": int(load),
                    "pulses": pulses,
                    "phase": phase,
                    "demo": True,
                    "note": "Saturation simulee — pas un vrai DDoS",
                }
            self._send(200, json.dumps(payload).encode(), "application/json")
            return

        if self.path in ("/", "/index.html"):
            self._send(200, DASHBOARD_HTML.encode("utf-8"), "text/html; charset=utf-8")
            return

        self._send(404, b"not found", "text/plain")

    def do_POST(self) -> None:
        global load, pulses, phase, last_pulse

        if self.path != "/demo/pulse":
            self._send(404, b"not found", "text/plain")
            return

        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(max(0, length)).decode("utf-8", errors="ignore")
        # accepte form ou token brut
        token = ""
        if "token=" in raw:
            token = parse_qs(raw).get("token", [""])[0]
        else:
            token = raw.strip()

        if token != DEMO_TOKEN:
            self._send(403, b'{"error":"bad token"}', "application/json")
            return

        with lock:
            now = time.time()
            # anti-abus leger: ignore si > 5 pulses / seconde depuis la meme source logique
            if now - last_pulse < 0.15:
                payload = {"load": int(load), "phase": phase, "accepted": False}
                self._send(200, json.dumps(payload).encode(), "application/json")
                return

            last_pulse = now
            pulses += 1
            # Chaque pulse augmente volontairement la jauge (cooperation pedagogique)
            load = min(100.0, load + 8.0)
            if load >= 100.0:
                phase = "down"
            else:
                phase = "rising"
            payload = {
                "load": int(load),
                "pulses": pulses,
                "phase": phase,
                "accepted": True,
                "demo": True,
            }

        self._send(200, json.dumps(payload).encode(), "application/json")

    def do_PUT(self) -> None:
        """PUT /demo/reset — remet la demo a zero (formateur)."""
        global load, pulses, phase
        if self.path != "/demo/reset":
            self._send(404, b"not found", "text/plain")
            return
        with lock:
            load = 0.0
            pulses = 0
            phase = "idle"
        self._send(200, b'{"ok":true}', "application/json")


DASHBOARD_HTML = """<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Labo — Victime DDoS (démo)</title>
  <style>
    :root { --bg:#0f1419; --fg:#e7ecf1; --ok:#3dd68c; --warn:#f5a524; --bad:#f31260; }
    body { margin:0; font-family: ui-sans-serif, system-ui, sans-serif; background:var(--bg); color:var(--fg);
           min-height:100vh; display:flex; align-items:center; justify-content:center; }
    .wrap { width:min(560px, 92vw); }
    h1 { font-size:1.4rem; margin:0 0 .25rem; }
    .tag { color:#8b9bb4; font-size:.85rem; margin-bottom:1.5rem; }
    .gauge-bg { height:28px; background:#1c2430; border-radius:8px; overflow:hidden; border:1px solid #2a3544; }
    .gauge { height:100%; width:0%; background:var(--ok); transition: width .2s, background .2s; }
    .stats { display:flex; justify-content:space-between; margin-top:.75rem; font-variant-numeric:tabular-nums; }
    .big { font-size:3rem; font-weight:700; margin:1.2rem 0 .4rem; }
    .down { color:var(--bad); animation: pulse 1s infinite; }
    @keyframes pulse { 50% { opacity:.5; } }
    .note { margin-top:1.5rem; padding:1rem; background:#1c2430; border-radius:8px; font-size:.9rem; color:#a8b3c4; }
  </style>
</head>
<body>
  <div class="wrap">
    <h1>Serveur labo — démo DDoS</h1>
    <p class="tag">Saturation volontaire · aucun flood réel · atelier cybersécurité</p>
    <div class="big" id="pct">0%</div>
    <div class="gauge-bg"><div class="gauge" id="bar"></div></div>
    <div class="stats">
      <span>Phase: <strong id="phase">idle</strong></span>
      <span>Pulses: <strong id="pulses">0</strong></span>
    </div>
    <p class="note" id="msg">En attente du M5Stick (POST /demo/pulse)…</p>
  </div>
  <script>
    async function tick() {
      try {
        const r = await fetch('/api/status');
        const j = await r.json();
        const el = document.getElementById('pct');
        const bar = document.getElementById('bar');
        el.textContent = j.load + '%';
        el.className = 'big' + (j.phase === 'down' ? ' down' : '');
        bar.style.width = j.load + '%';
        bar.style.background = j.load > 90 ? 'var(--bad)' : (j.load > 60 ? 'var(--warn)' : 'var(--ok)');
        document.getElementById('phase').textContent = j.phase;
        document.getElementById('pulses').textContent = j.pulses;
        document.getElementById('msg').textContent =
          j.phase === 'down'
            ? 'SERVICE DOWN (simulé) — en vrai, le site serait inaccessible pour les clients.'
            : (j.phase === 'rising'
               ? 'Charge en hausse (le serveur augmente volontairement la jauge).'
               : 'En attente du M5Stick (POST /demo/pulse)…');
      } catch (e) {}
    }
    setInterval(tick, 300);
    tick();
  </script>
</body>
</html>
"""


def main() -> None:
    parser = argparse.ArgumentParser(description="Victime DDoS pédagogique (labo)")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()

    threading.Thread(target=decay_loop, daemon=True).start()
    httpd = ThreadingHTTPServer((args.host, args.port), Handler)
    print("=== DDoS lab victim (DEMO, no flood) ===")
    print(f"Dashboard : http://<IP_DE_CE_PC>:{args.port}/")
    print(f"Token     : {DEMO_TOKEN}")
    print("Configurez LAB_VICTIM_HOST dans include/config.h")
    print("Ctrl+C pour arrêter.\n")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nArrêt.")
        httpd.server_close()


if __name__ == "__main__":
    main()
