#include "modes.h"

#include "config.h"

namespace {

const char* TIPS[] = {
    "Ne te connecte pas aux Wi-Fi gratuits sans VPN",
    "Verifie le nom exact du reseau avec le lieu",
    "Mot de passe: 12+ caracteres, unique",
    "Active la 2FA partout ou c'est possible",
    "Un portail qui demande ton mail = alerte",
    "Ne reutilise jamais le meme mot de passe",
    "Mets a jour tes appareils regulierement",
    "Mefie-toi des USB inconnues (Bad USB)",
    "Phishing: l'urgence est un signal d'arnaque",
    "Utilise un gestionnaire de mots de passe",
};

constexpr size_t TIP_COUNT = sizeof(TIPS) / sizeof(TIPS[0]);

size_t tipIndex = 0;
uint32_t tipLastMs = 0;

}  // namespace

void awarenessInit() {
    tipIndex = 0;
    tipLastMs = millis();
}

void awarenessDraw() {
    auto& dsp = M5.Display;
    dsp.fillScreen(TFT_BLACK);
    drawHeader("Bonnes pratiques");

    dsp.setTextColor(TFT_GREEN);
    dsp.setTextSize(1);
    dsp.setCursor(4, 32);
    dsp.printf("[%u/%u]", tipIndex + 1, TIP_COUNT);

    dsp.setTextColor(TFT_WHITE);
    dsp.setCursor(4, 48);

    // Retour a la ligne simple pour petit ecran
    const char* tip = TIPS[tipIndex];
    String text = tip;
    int y = 48;
    while (text.length() > 0) {
        int chunk = 22;
        if ((int)text.length() <= chunk) {
            dsp.setCursor(4, y);
            dsp.println(text.c_str());
            break;
        }
        String line = text.substring(0, chunk);
        int sp = line.lastIndexOf(' ');
        if (sp > 8) {
            line = text.substring(0, sp);
            text = text.substring(sp + 1);
        } else {
            text = text.substring(chunk);
        }
        dsp.setCursor(4, y);
        dsp.println(line.c_str());
        y += 14;
        if (y > 120) break;
    }

    drawFooter("A: suivant  B: menu");
}

void awarenessLoop() {
    if (millis() - tipLastMs >= AWARENESS_TIP_MS) {
        tipLastMs = millis();
        tipIndex = (tipIndex + 1) % TIP_COUNT;
        awarenessDraw();
    }
}

void awarenessNext() {
    tipIndex = (tipIndex + 1) % TIP_COUNT;
    tipLastMs = millis();
    awarenessDraw();
}
