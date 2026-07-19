#include <M5Unified.h>

#include "awareness.h"
#include "bruteforce_real.h"
#include "bruteforce_sim.h"
#include "config.h"
#include "fake_wifi.h"
#include "menu.h"
#include "modes.h"
#include "phishing_quiz.h"

namespace {

AppMode currentMode = AppMode::Menu;

void enterMode(AppMode mode) {
    if (currentMode == AppMode::FakeWifi && mode != AppMode::FakeWifi) {
        fakeWifiStop();
    }

    currentMode = mode;

    switch (mode) {
        case AppMode::Menu:
            menuInit();
            menuDraw();
            break;
        case AppMode::FakeWifi:
            fakeWifiInit();
            fakeWifiDraw();
            break;
        case AppMode::BruteSim:
            bruteSimInit();
            bruteSimDraw();
            break;
        case AppMode::BruteReal:
            bruteRealInit();
            bruteRealDraw();
            break;
        case AppMode::Awareness:
            awarenessInit();
            awarenessDraw();
            break;
        case AppMode::PcBridge:
            pcBridgeInit();
            pcBridgeDraw();
            break;
        case AppMode::PhishingQuiz:
            phishingQuizInit();
            phishingQuizDraw();
            break;
    }
}

void handleButtons() {
    bool a = M5.BtnA.wasPressed();
    bool b = M5.BtnB.wasPressed();

    switch (currentMode) {
        case AppMode::Menu:
            if (a) menuMoveDown();
            if (b) enterMode(menuGetSelection());
            break;

        case AppMode::FakeWifi:
            if (b) enterMode(AppMode::Menu);
            break;

        case AppMode::BruteSim:
            if (a) {
                if (bruteSimIsDone()) {
                    enterMode(AppMode::Awareness);
                } else {
                    bruteSimTogglePause();
                }
            }
            if (b) enterMode(AppMode::Menu);
            break;

        case AppMode::BruteReal:
            if (a) {
                if (bruteRealIsDone()) {
                    enterMode(AppMode::Awareness);
                } else {
                    bruteRealTogglePause();
                }
            }
            if (b) enterMode(AppMode::Menu);
            break;

        case AppMode::Awareness:
            if (a) awarenessNext();
            if (b) enterMode(AppMode::Menu);
            break;

        case AppMode::PcBridge:
            if (b) enterMode(AppMode::Menu);
            break;

        case AppMode::PhishingQuiz:
            if (phishingIsDone()) {
                if (a) phishingNext();  // recommencer
                if (b) enterMode(AppMode::Menu);
            } else if (phishingIsAnswered()) {
                if (a) phishingNext();
                if (b) enterMode(AppMode::Menu);
            } else {
                if (a) phishingAnswer(true);   // arnaque
                if (b) phishingAnswer(false);  // legitime
            }
            break;
    }
}

}  // namespace

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1);
    M5.Display.setTextSize(1);
    M5.Display.fillScreen(TFT_BLACK);

    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("=== M5Stick Cyber Awareness ===");
    Serial.println("Atelier pedagogique — reseau labo uniquement");
    Serial.printf("SSID labo: %s\n", LAB_SSID);

    enterMode(AppMode::Menu);
}

void loop() {
    M5.update();
    handleButtons();

    switch (currentMode) {
        case AppMode::FakeWifi:
            fakeWifiLoop();
            break;
        case AppMode::BruteSim:
            bruteSimLoop();
            break;
        case AppMode::BruteReal:
            bruteRealLoop();
            break;
        case AppMode::Awareness:
            awarenessLoop();
            break;
        case AppMode::PcBridge:
            pcBridgeLoop();
            break;
        default:
            break;
    }

    delay(10);
}
