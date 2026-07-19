#include "modes.h"

void drawHeader(const char* title) {
    auto& dsp = M5.Display;
    dsp.setTextColor(TFT_YELLOW);
    dsp.setTextSize(2);
    dsp.setCursor(4, 4);
    dsp.println(title);
}

void drawFooter(const char* hint) {
    auto& dsp = M5.Display;
    dsp.setTextColor(TFT_DARKGREY);
    dsp.setTextSize(1);
    dsp.setCursor(4, 148);
    dsp.println(hint);
}
