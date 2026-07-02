#include "modes.h"

namespace {

String pcStatus = "En attente du PC...";
String pcGuess = "";
String pcFound = "";
uint32_t pcCurrent = 0;
uint32_t pcTotal = 100;
bool pcDone = false;

void parseLine(const String& line) {
    if (line.startsWith("STATUS ")) {
        pcStatus = line.substring(7);
    } else if (line.startsWith("PROGRESS ")) {
        int s1 = line.indexOf(' ', 9);
        int s2 = line.indexOf(' ', s1 + 1);
        if (s1 > 0 && s2 > s1) {
            pcCurrent = line.substring(9, s1).toInt();
            pcTotal = line.substring(s1 + 1, s2).toInt();
            pcGuess = line.substring(s2 + 1);
            pcDone = false;
        }
    } else if (line.startsWith("FOUND ")) {
        pcFound = line.substring(6);
        pcDone = true;
        pcStatus = "Mot de passe trouve !";
        Serial.printf("[PC_BRIDGE] FOUND: %s\n", pcFound.c_str());
    } else if (line.startsWith("RESET")) {
        pcStatus = "En attente du PC...";
        pcGuess = "";
        pcFound = "";
        pcCurrent = 0;
        pcTotal = 100;
        pcDone = false;
    }
}

}  // namespace

void pcBridgeInit() {
    pcStatus = "En attente du PC...";
    pcGuess = "";
    pcFound = "";
    pcCurrent = 0;
    pcTotal = 100;
    pcDone = false;

    while (Serial.available()) Serial.read();
    Serial.println("[M5] PC_BRIDGE_READY");
}

void pcBridgeDraw() {
    auto& dsp = M5.Display;
    dsp.fillScreen(TFT_BLACK);
    drawHeader("Mode PC (Hashcat)");

    dsp.setTextColor(TFT_MAGENTA);
    dsp.setTextSize(1);
    dsp.setCursor(4, 28);
    String st = pcStatus;
    if (st.length() > 22) st = st.substring(0, 19) + "...";
    dsp.println(st.c_str());

    if (pcTotal > 0) {
        dsp.setTextColor(TFT_WHITE);
        dsp.setCursor(4, 46);
        dsp.printf("%u / %u", pcCurrent, pcTotal);

        int barW = 72;
        int fill = (int)((pcCurrent * barW) / pcTotal);
        dsp.drawRect(4, 58, barW, 8, TFT_WHITE);
        dsp.fillRect(4, 58, fill, 8, TFT_MAGENTA);
    }

    if (pcGuess.length() > 0) {
        dsp.setTextColor(TFT_CYAN);
        dsp.setCursor(4, 74);
        String g = pcGuess;
        if (g.length() > 22) g = g.substring(0, 19) + "...";
        dsp.printf("> %s", g.c_str());
    }

    if (pcDone && pcFound.length() > 0) {
        dsp.setTextColor(TFT_RED);
        dsp.setCursor(4, 96);
        dsp.println("CRACKE !");
        dsp.setTextColor(TFT_YELLOW);
        dsp.setCursor(4, 108);
        String f = pcFound;
        if (f.length() > 20) f = f.substring(0, 17) + "...";
        dsp.printf("\"%s\"", f.c_str());
    }

    drawFooter("B: menu");
}

void pcBridgeLoop() {
    static String buffer;
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (buffer.length() > 0) {
                parseLine(buffer);
                buffer = "";
                pcBridgeDraw();
            }
        } else {
            buffer += c;
            if (buffer.length() > 120) buffer = "";
        }
    }
}
