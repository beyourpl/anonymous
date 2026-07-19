#pragma once

// ── Réseau de LABO uniquement (brute force réel) ──────────────────────────
// Ne jamais pointer vers un réseau que vous ne contrôlez pas.
#define LAB_SSID     "LABO_CYBER_DEMO"
#define LAB_PASSWORD "12345678"   // mot de passe volontairement faible pour la démo

// ── Faux point d'accès (evil twin pédagogique) ────────────────────────────
#define FAKE_AP_SSID     "Starbucks_Free"
#define FAKE_AP_PASSWORD ""       // réseau ouvert = plus réaliste pour la démo

// ── Portail captif ──────────────────────────────────────────────────────────
#define CAPTIVE_PORTAL_TITLE "Wi-Fi Public — Connexion requise"

// ── Brute force ─────────────────────────────────────────────────────────────
#define BRUTE_CONNECT_TIMEOUT_MS 4000
#define BRUTE_DELAY_BETWEEN_MS   300

// ── Affichage ───────────────────────────────────────────────────────────────
#define AWARENESS_TIP_MS 5000

// ── Démo DDoS labo (sans flood) ─────────────────────────────────────────────
// IP privée du PC qui lance : python3 pc-tools/ddos_victim_server.py
// Le firmware refuse toute IP hors plages privées (10/8, 172.16/12, 192.168/16).
#define LAB_VICTIM_HOST      "192.168.1.50"
#define LAB_VICTIM_PORT      8080
#define LAB_DEMO_TOKEN       "CYBER_DEMO_LAB"
#define LAB_DEMO_MAX_PULSES  40      // plafond dur — pas un flood
#define LAB_DEMO_PULSE_MS    500     // 2 pulses / seconde max
