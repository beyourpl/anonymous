#!/usr/bin/env python3
"""
Outil PC pour le mode « M5Stick + PC » — atelier cybersécurité.

Envoie la progression du crack (réel ou simulé) au M5Stick via USB série.
Le M5Stick doit être en mode « 4 Mode PC » dans le menu.

Protocole série (115200 baud) :
  STATUS <message>
  PROGRESS <current> <total> <guess>
  FOUND <password>
  RESET

Usage :
  # Simulation (sans hashcat, pour tester l'affichage)
  python3 pc-tools/crack_display.py --simulate

  # Avec hashcat (fichier .cap ou .hc22000 de VOTRE labo)
  python3 pc-tools/crack_display.py --hashcat capture.hc22000 wordlist.txt

  # Port série explicite
  python3 pc-tools/crack_display.py --port /dev/ttyUSB0 --simulate
"""

from __future__ import annotations

import argparse
import glob
import re
import subprocess
import sys
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("Installer pyserial : pip install pyserial")
    sys.exit(1)


def find_m5_port() -> str | None:
    """Cherche un port USB série probable (CP210x / CH340 / M5Stack)."""
    hints = ("usb", "serial", "cp210", "ch340", "slab", "m5", "uart")
    for port in serial.tools.list_ports.comports():
        desc = (port.device + port.description + (port.manufacturer or "")).lower()
        if any(h in desc for h in hints):
            return port.device
    # Fallback Linux common
    for pattern in ("/dev/ttyUSB*", "/dev/ttyACM*"):
        matches = sorted(glob.glob(pattern))
        if matches:
            return matches[0]
    return None


def open_serial(port: str | None, baud: int) -> serial.Serial:
    p = port or find_m5_port()
    if not p:
        print("Port série introuvable. Utilisez --port /dev/ttyUSB0")
        sys.exit(1)
    print(f"Connexion série : {p} @ {baud}")
    ser = serial.Serial(p, baud, timeout=0.5)
    time.sleep(2)  # reset ESP32 après ouverture port
    # Attendre le handshake
    deadline = time.time() + 8
    while time.time() < deadline:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if line:
            print(f"  [{line}]")
        if "PC_BRIDGE_READY" in line:
            break
    return ser


def send(ser: serial.Serial, cmd: str) -> None:
    print(f"  -> {cmd}")
    ser.write((cmd + "\n").encode("utf-8"))
    ser.flush()


def run_simulation(ser: serial.Serial, wordlist: list[str], target: str) -> None:
    send(ser, "RESET")
    send(ser, "STATUS Simulation hashcat...")
    total = len(wordlist)
    time.sleep(0.5)

    for i, guess in enumerate(wordlist, start=1):
        send(ser, f"PROGRESS {i} {total} {guess}")
        time.sleep(0.15 if i < total - 3 else 0.6)
        if guess == target:
            send(ser, f"FOUND {guess}")
            send(ser, "STATUS Crack termine — debrief!")
            return

    send(ser, "STATUS Mot de passe non trouve dans la liste")


def run_hashcat(ser: serial.Serial, cap_file: str, wordlist: str) -> None:
    send(ser, "RESET")
    send(ser, "STATUS Hashcat en cours...")

    cmd = [
        "hashcat",
        "-m", "22000",
        "-a", "0",
        "--status",
        "--status-timer", "1",
        cap_file,
        wordlist,
    ]
    print("Lancement :", " ".join(cmd))
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )

    progress_re = re.compile(r"Progress\.+:\s*(\d+)/(\d+)")
    found_re = re.compile(r":([^:]+)$")

    with open(wordlist, encoding="utf-8", errors="ignore") as wl:
        words = [w.strip() for w in wl if w.strip()]

    guess_idx = 0
    assert proc.stdout is not None
    for line in proc.stdout:
        line = line.strip()
        if not line:
            continue
        print(line)

        m = progress_re.search(line)
        if m and words:
            cur = int(m.group(1))
            total = int(m.group(2))
            guess = words[min(guess_idx, len(words) - 1)]
            guess_idx = min(guess_idx + 1, len(words) - 1)
            send(ser, f"PROGRESS {cur} {total} {guess}")

        if "STATUS" in line and "Cracked" in line:
            send(ser, "STATUS Hashcat : hash cracké")

        if "All hashes found" in line or "Recovered........" in line:
            break

    proc.wait()
    if proc.returncode == 0:
        # Lire potfile / stdout pour le mot de passe
        show = subprocess.run(
            ["hashcat", "-m", "22000", cap_file, "--show"],
            capture_output=True,
            text=True,
        )
        for show_line in show.stdout.splitlines():
            parts = show_line.split(":")
            if len(parts) >= 2:
                pwd = parts[-1]
                send(ser, f"FOUND {pwd}")
                send(ser, "STATUS Debrief sensibilisation")
                return

    send(ser, "STATUS Hashcat termine (voir console PC)")


def default_wordlist() -> list[str]:
    return [
        "password", "12345678", "azerty", "qwerty", "admin",
        "letmein", "welcome", "starbucks", "wifi1234", "00000000",
        "password1", "iloveyou", "monmotdepasse", "123456789", "guest",
        "football", "dragon", "master", "sunshine", "princess",
        "login", "passw0rd", "solo", "pass", "starwars",
        "freewifi", "internet", "changeme", "secret", "default",
        "11111111", "abc123", "test", "demo", "cyber",
        "hotel", "guest123", "wifi", "free", "open",
        "bienvenue", "france", "paris", "coffee", "1234",
        "12345678",
    ]


def main() -> None:
    parser = argparse.ArgumentParser(description="Bridge PC -> M5Stick cyber demo")
    parser.add_argument("--port", help="Port série (ex: /dev/ttyUSB0, COM3)")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--simulate", action="store_true", help="Simulation sans hashcat")
    parser.add_argument("--target", default="12345678", help="Mot de passe cible (simulation)")
    parser.add_argument("cap_file", nargs="?", help="Fichier .hc22000 / .cap pour hashcat")
    parser.add_argument("wordlist", nargs="?", help="Wordlist pour hashcat")
    args = parser.parse_args()

    ser = open_serial(args.port, args.baud)

    if args.simulate or not args.cap_file:
        words = default_wordlist()
        if args.target not in words:
            words.append(args.target)
        print("\n=== Mode simulation ===")
        print("Appuyez sur B pour quitter le mode PC sur le M5Stick.\n")
        run_simulation(ser, words, args.target)
    else:
        if not args.wordlist:
            print("Fournir une wordlist : crack_display.py capture.hc22000 wordlist.txt")
            sys.exit(1)
        print("\n=== Mode hashcat (labo uniquement) ===\n")
        run_hashcat(ser, args.cap_file, args.wordlist)

    ser.close()
    print("\nTerminé.")


if __name__ == "__main__":
    main()
