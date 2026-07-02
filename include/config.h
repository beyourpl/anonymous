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
