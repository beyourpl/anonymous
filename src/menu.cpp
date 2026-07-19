#include "modes.h"

namespace {

struct MenuItem {
    const char* label;
    AppMode mode;
};

const MenuItem ITEMS[] = {
    {"1 Faux Wi-Fi", AppMode::FakeWifi},
    {"2 Brute force sim", AppMode::BruteSim},
    {"3 Brute force reel", AppMode::BruteReal},
    {"4 Mode PC", AppMode::PcBridge},
    {"5 Quiz phishing", AppMode::PhishingQuiz},
    {"6 Sensibilisation", AppMode::Awareness},
};

constexpr size_t ITEM_COUNT = sizeof(ITEMS) / sizeof(ITEMS[0]);
size_t selected = 0;

}  // namespace

void menuInit() { selected = 0; }

void menuDraw() {
    auto& dsp = M5.Display;
    dsp.fillScreen(TFT_BLACK);

    dsp.setTextColor(TFT_YELLOW);
    dsp.setTextSize(2);
    dsp.setCursor(4, 2);
    dsp.println("Cyber");

    // 6 entrees: espacement serre pour tenir sur l'ecran
    constexpr int rowH = 14;
    constexpr int startY = 24;

    dsp.setTextSize(1);
    for (size_t i = 0; i < ITEM_COUNT; i++) {
        int y = startY + (int)i * rowH;
        if (i == selected) {
            dsp.fillRect(2, y - 1, 76, rowH, TFT_GREEN);
            dsp.setTextColor(TFT_BLACK);
        } else {
            dsp.setTextColor(TFT_WHITE);
        }
        dsp.setCursor(4, y);
        dsp.println(ITEMS[i].label);
    }

    drawFooter("A: bas  B: ok");
}

AppMode menuUpdate() {
    return ITEMS[selected].mode;
}

void menuMoveDown() {
    selected = (selected + 1) % ITEM_COUNT;
    menuDraw();
}

AppMode menuGetSelection() {
    return ITEMS[selected].mode;
}
