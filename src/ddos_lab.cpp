#include "modes.h"

#include <HTTPClient.h>
#include <WiFi.h>

#include "config.h"
#include "ddos_lab.h"

// Démo labo coopérative : quelques pulses HTTP étiquetés, pas un flood.
// Le serveur PC augmente volontairement sa jauge.

namespace {

enum class LabPhase : uint8_t { Idle, Connecting, Running, Done, Error };

LabPhase phase = LabPhase::Idle;
bool running = false;
bool done = false;
uint16_t pulsesSent = 0;
uint8_t remoteLoad = 0;
String remotePhase = "idle";
String statusLine = "Pret";
uint32_t lastPulseMs = 0;
uint32_t lastDrawMs = 0;

bool isPrivateIp(const IPAddress& ip) {
    // 10.0.0.0/8
    if (ip[0] == 10) return true;
    // 192.168.0.0/16
    if (ip[0] == 192 && ip[1] == 168) return true;
    // 172.16.0.0/12
    if (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) return true;
    return false;
}

bool connectLabWifi() {
    statusLine = "WiFi labo...";
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(100);
    WiFi.begin(LAB_SSID, LAB_PASSWORD);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
        delay(200);
        M5.update();
    }
    return WiFi.status() == WL_CONNECTED;
}

bool sendPulse() {
    if (WiFi.status() != WL_CONNECTED) return false;

    IPAddress victim;
    if (!victim.fromString(LAB_VICTIM_HOST)) {
        statusLine = "IP invalide";
        phase = LabPhase::Error;
        return false;
    }
    if (!isPrivateIp(victim)) {
        statusLine = "IP non privee!";
        phase = LabPhase::Error;
        Serial.println("[DDOS_LAB] Refuse: cible hors plage privee");
        return false;
    }

    HTTPClient http;
    String url = String("http://") + LAB_VICTIM_HOST + ":" + String(LAB_VICTIM_PORT) + "/demo/pulse";
    http.setTimeout(1500);
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("X-Demo", "cyber-awareness");

    String body = String("token=") + LAB_DEMO_TOKEN + "&source=m5stick";
    int code = http.POST(body);

    if (code == 200) {
        String resp = http.getString();
        // parse load / phase rapidement
        int li = resp.indexOf("\"load\":");
        if (li >= 0) {
            remoteLoad = (uint8_t)resp.substring(li + 7).toInt();
        }
        int pi = resp.indexOf("\"phase\":\"");
        if (pi >= 0) {
            int end = resp.indexOf('"', pi + 9);
            if (end > pi) remotePhase = resp.substring(pi + 9, end);
        }
        pulsesSent++;
        statusLine = "Pulse OK";
        http.end();
        return true;
    }

    statusLine = String("HTTP ") + code;
    http.end();
    return false;
}

}  // namespace

void ddosLabInit() {
    phase = LabPhase::Idle;
    running = false;
    done = false;
    pulsesSent = 0;
    remoteLoad = 0;
    remotePhase = "idle";
    statusLine = "A: lancer";
    lastPulseMs = 0;
    lastDrawMs = 0;
}

void ddosLabDraw() {
    auto& dsp = M5.Display;
    dsp.fillScreen(TFT_BLACK);
    drawHeader("DDoS lab");

    dsp.setTextColor(TFT_ORANGE);
    dsp.setTextSize(1);
    dsp.setCursor(4, 28);
    dsp.println("SANS FLOOD");

    dsp.setTextColor(TFT_WHITE);
    dsp.setCursor(4, 42);
    dsp.printf("%s:%u", LAB_VICTIM_HOST, LAB_VICTIM_PORT);

    dsp.setCursor(4, 56);
    dsp.printf("pulses %u/%u", pulsesSent, LAB_DEMO_MAX_PULSES);

    dsp.setCursor(4, 70);
    dsp.printf("charge %u%%", remoteLoad);
    dsp.drawRect(4, 82, 72, 8, TFT_WHITE);
    uint16_t c = remoteLoad > 90 ? TFT_RED : (remoteLoad > 60 ? TFT_ORANGE : TFT_GREEN);
    dsp.fillRect(4, 82, (72 * remoteLoad) / 100, 8, c);

    dsp.setTextColor(TFT_CYAN);
    dsp.setCursor(4, 96);
    String st = statusLine;
    if (st.length() > 22) st = st.substring(0, 19) + "...";
    dsp.println(st.c_str());

    if (phase == LabPhase::Done || remotePhase == "down") {
        dsp.setTextColor(TFT_RED);
        dsp.setCursor(4, 112);
        dsp.println("Victime DOWN (sim)");
        drawFooter("A: relancer  B: menu");
    } else if (phase == LabPhase::Error) {
        dsp.setTextColor(TFT_RED);
        dsp.setCursor(4, 112);
        dsp.println("Erreur — check IP");
        drawFooter("A: retry  B: menu");
    } else {
        drawFooter(running ? "A: pause  B: menu" : "A: lancer  B: menu");
    }
}

void ddosLabLoop() {
    if (!running || phase == LabPhase::Done || phase == LabPhase::Error) return;

    if (phase == LabPhase::Connecting) {
        if (!connectLabWifi()) {
            statusLine = "WiFi echec";
            phase = LabPhase::Error;
            running = false;
            ddosLabDraw();
            return;
        }
        statusLine = "Connecte";
        phase = LabPhase::Running;
        ddosLabDraw();
        return;
    }

    if (phase != LabPhase::Running) return;

    uint32_t now = millis();
    if (now - lastPulseMs < LAB_DEMO_PULSE_MS) return;
    lastPulseMs = now;

    if (pulsesSent >= LAB_DEMO_MAX_PULSES || remotePhase == "down") {
        running = false;
        done = true;
        phase = LabPhase::Done;
        statusLine = "Demo terminee";
        Serial.println("[DDOS_LAB] Fin — saturation simulee cote serveur");
        ddosLabDraw();
        return;
    }

    sendPulse();
    if (millis() - lastDrawMs > 200) {
        lastDrawMs = millis();
        ddosLabDraw();
    }
}

void ddosLabStop() {
    running = false;
    WiFi.disconnect(true);
}

void ddosLabStartOrPause() {
    if (phase == LabPhase::Done || phase == LabPhase::Error) {
        ddosLabStop();
        ddosLabInit();
        phase = LabPhase::Connecting;
        running = true;
        ddosLabDraw();
        return;
    }

    if (!running) {
        if (phase == LabPhase::Idle) {
            phase = LabPhase::Connecting;
        }
        running = true;
    } else {
        running = false;
        statusLine = "Pause";
    }
    ddosLabDraw();
}

bool ddosLabIsDone() { return done; }
