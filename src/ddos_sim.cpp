#include "modes.h"

#include "ddos_sim.h"

// Simulation visuelle uniquement — aucun paquet reseau n'est envoye.
// Un vrai DDoS est illegal (France: art. 323-2 Code penal).

namespace {

enum class Phase : uint8_t { Flood, Down, Tips };

Phase phase = Phase::Flood;
bool running = false;
bool done = false;
uint32_t reqCount = 0;
uint32_t lastTick = 0;
uint8_t loadPct = 0;
uint8_t tipIndex = 0;
uint32_t tipLastMs = 0;
uint8_t animFrame = 0;

const char* TIPS[] = {
    "DDoS = saturer un service avec trop de trafic",
    "Souvent via un botnet (PC/IoT pirates)",
    "Illegal sans autorisation (art. 323-2)",
    "Defense: CDN, rate-limit, filtrage amont",
    "Un M5Stick ne peut pas 'coucher' Internet",
    "Cette demo est 100% simulee (0 paquet)",
};

constexpr size_t TIP_COUNT = sizeof(TIPS) / sizeof(TIPS[0]);
constexpr uint32_t TARGET_REQS = 12000;

void drawServer(int load) {
    auto& dsp = M5.Display;
    // Serveur schematique a droite
    int x = 52;
    int y = 48;
    uint16_t color = TFT_GREEN;
    if (load > 70) color = TFT_ORANGE;
    if (load > 90 || phase == Phase::Down) color = TFT_RED;

    dsp.drawRect(x, y, 24, 36, color);
    dsp.fillRect(x + 2, y + 4, 20, 6, color);
    dsp.fillRect(x + 2, y + 14, 20, 6, color);
    dsp.fillRect(x + 2, y + 24, 20, 6, color);

    if (phase == Phase::Down) {
        dsp.setTextColor(TFT_RED);
        dsp.setCursor(x - 4, y + 40);
        dsp.println("DOWN");
    }
}

void drawPackets() {
    auto& dsp = M5.Display;
    // Petits "paquets" animes vers le serveur
    for (int i = 0; i < 5; i++) {
        int px = 4 + ((animFrame * 3 + i * 12) % 44);
        int py = 52 + (i * 7) % 28;
        dsp.fillRect(px, py, 3, 2, TFT_CYAN);
    }
}

}  // namespace

void ddosSimInit() {
    phase = Phase::Flood;
    running = true;
    done = false;
    reqCount = 0;
    loadPct = 0;
    tipIndex = 0;
    lastTick = millis();
    tipLastMs = millis();
    animFrame = 0;
}

void ddosSimDraw() {
    auto& dsp = M5.Display;
    dsp.fillScreen(TFT_BLACK);
    drawHeader("DDoS sim");

    dsp.setTextColor(TFT_ORANGE);
    dsp.setTextSize(1);
    dsp.setCursor(4, 28);
    dsp.println("SIMULATION");

    if (phase == Phase::Tips) {
        dsp.setTextColor(TFT_WHITE);
        dsp.setCursor(4, 44);
        dsp.printf("Conseil %u/%u", tipIndex + 1, TIP_COUNT);

        String t = TIPS[tipIndex];
        int y = 60;
        while (t.length() > 0 && y < 130) {
            int chunk = 22;
            if ((int)t.length() <= chunk) {
                dsp.setCursor(4, y);
                dsp.println(t.c_str());
                break;
            }
            String line = t.substring(0, chunk);
            int sp = line.lastIndexOf(' ');
            if (sp > 8) {
                line = t.substring(0, sp);
                t = t.substring(sp + 1);
            } else {
                t = t.substring(chunk);
            }
            dsp.setCursor(4, y);
            dsp.println(line.c_str());
            y += 12;
        }
        drawFooter("A: suivant  B: menu");
        return;
    }

    dsp.setTextColor(TFT_WHITE);
    dsp.setCursor(4, 40);
    dsp.printf("reqs: %lu", (unsigned long)reqCount);

    // Jauge de charge
    dsp.setCursor(4, 52);
    dsp.printf("charge %u%%", loadPct);
    dsp.drawRect(4, 64, 44, 8, TFT_WHITE);
    uint16_t barColor = loadPct > 90 ? TFT_RED : (loadPct > 70 ? TFT_ORANGE : TFT_GREEN);
    dsp.fillRect(4, 64, (44 * loadPct) / 100, 8, barColor);

    drawPackets();
    drawServer(loadPct);

    if (phase == Phase::Down) {
        dsp.setTextColor(TFT_RED);
        dsp.setCursor(4, 100);
        dsp.println("Service sature!");
        dsp.setTextColor(TFT_YELLOW);
        dsp.setCursor(4, 114);
        dsp.println("0 paquet envoye");
        drawFooter("A: conseils  B: menu");
    } else {
        drawFooter(running ? "A: pause  B: menu" : "A: reprise");
    }
}

void ddosSimLoop() {
    if (!running || phase == Phase::Tips) return;

    uint32_t now = millis();
    if (now - lastTick < 40) return;
    lastTick = now;
    animFrame++;

    if (phase == Phase::Flood) {
        // Accelere progressivement
        uint32_t burst = 20 + (reqCount / 200);
        if (burst > 180) burst = 180;
        reqCount += burst;

        loadPct = (uint8_t)((reqCount * 100) / TARGET_REQS);
        if (loadPct > 100) loadPct = 100;

        if (reqCount >= TARGET_REQS) {
            phase = Phase::Down;
            running = false;
            done = true;
            Serial.println("[DDOS_SIM] Simulation terminee — aucun trafic reseau");
        }
        ddosSimDraw();
    }
}

void ddosSimTogglePause() {
    if (phase == Phase::Down) {
        phase = Phase::Tips;
        tipIndex = 0;
        tipLastMs = millis();
        running = false;
        ddosSimDraw();
        return;
    }
    if (phase == Phase::Tips) {
        tipIndex = (tipIndex + 1) % TIP_COUNT;
        tipLastMs = millis();
        ddosSimDraw();
        return;
    }
    running = !running;
    ddosSimDraw();
}

bool ddosSimIsDone() { return done; }

void ddosSimRestart() {
    ddosSimInit();
    ddosSimDraw();
}
