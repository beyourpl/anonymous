#include "modes.h"

#include "config.h"

namespace {

const char* SIM_WORDLIST[] = {
    "password", "12345678", "azerty", "qwerty", "admin",
    "letmein", "welcome", "starbucks", "wifi1234", "00000000",
    "password1", "iloveyou", "monmotdepasse", "123456789", "guest",
    "football", "dragon", "master", "sunshine", "princess",
    "login", "passw0rd", "solo", "pass", "starwars",
    "freewifi", "internet", "changeme", "secret", "default",
    "11111111", "abc123", "test", "demo", "cyber",
    "hotel", "guest123", "wifi", "free", "open",
    "bienvenue", "france", "paris", "coffee", "1234",
    LAB_PASSWORD,  // le mot de passe cible tombe ici
};

constexpr size_t SIM_WORDLIST_LEN = sizeof(SIM_WORDLIST) / sizeof(SIM_WORDLIST[0]);
constexpr size_t TARGET_INDEX = SIM_WORDLIST_LEN - 1;

uint32_t simIndex = 0;
uint32_t simTick = 0;
bool simDone = false;
bool simRunning = false;
String simFound = "";

uint32_t simDelayMs() {
    // Accélère au début, ralentit près de la cible pour le suspense
    if (simIndex > TARGET_INDEX - 3) return 800;
    if (simIndex > TARGET_INDEX / 2) return 200;
    return 80;
}

}  // namespace

void bruteSimInit() {
    simIndex = 0;
    simTick = 0;
    simDone = false;
    simRunning = true;
    simFound = "";
}

void bruteSimDraw() {
    auto& dsp = M5.Display;
    dsp.fillScreen(TFT_BLACK);
    drawHeader("Brute force (sim)");

    dsp.setTextColor(TFT_WHITE);
    dsp.setTextSize(1);
    dsp.setCursor(4, 30);
    dsp.printf("Cible: %s", LAB_SSID);

    dsp.setCursor(4, 48);
    dsp.printf("Essai %u / %u", simIndex + 1, SIM_WORDLIST_LEN);

  // Barre de progression
    int barW = 72;
    int fill = (int)((simIndex * barW) / SIM_WORDLIST_LEN);
    dsp.drawRect(4, 62, barW, 8, TFT_WHITE);
    dsp.fillRect(4, 62, fill, 8, TFT_BLUE);

    dsp.setCursor(4, 78);
    dsp.setTextColor(TFT_CYAN);
    if (simIndex < SIM_WORDLIST_LEN) {
        dsp.printf("> %s", SIM_WORDLIST[simIndex]);
    }

    if (simDone) {
        dsp.setTextColor(TFT_RED);
        dsp.setCursor(4, 100);
        dsp.println("MOT DE PASSE TROUVE !");
        dsp.setTextColor(TFT_YELLOW);
        dsp.setCursor(4, 112);
        dsp.printf("\"%s\"", simFound.c_str());
        dsp.setTextColor(TFT_WHITE);
        dsp.setCursor(4, 128);
        dsp.println("Trop faible = crack rapide");
        drawFooter("A: sensibilisation");
    } else {
        drawFooter("A: pause  B: quitter");
    }
}

void bruteSimLoop() {
    if (!simRunning || simDone) return;

    uint32_t now = millis();
    if (now - simTick < simDelayMs()) return;
    simTick = now;

    if (simIndex >= TARGET_INDEX) {
        simFound = SIM_WORDLIST[TARGET_INDEX];
        simDone = true;
        simRunning = false;
        Serial.printf("[BRUTE_SIM] Mot de passe trouve: %s\n", simFound.c_str());
        bruteSimDraw();
        return;
    }

    simIndex++;
    bruteSimDraw();
}

void bruteSimTogglePause() {
    if (simDone) return;
    simRunning = !simRunning;
    bruteSimDraw();
}

bool bruteSimIsDone() { return simDone; }
