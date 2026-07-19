#include "modes.h"

#include "phishing_quiz.h"

namespace {

struct Question {
    const char* text;   // scenario court
    bool isPhishing;    // true = arnaque
    const char* tip;    // explication apres reponse
};

const Question QUESTIONS[] = {
    {"SMS: \"Votre colis "
     "est bloque. Cliquez "
     "bit.ly/colis99\"",
     true, "Lien raccourci + urgence = phishing"},
    {"Mail RH interne "
     "sans piece jointe, "
     "demande de vacances",
     false, "Message habituel, pas d'urgence"},
    {"Mail banque: "
     "\"Compte suspendu! "
     "Validez ici ASAP\"",
     true, "Banque ne demande jamais ca par mail"},
    {"Notification OS: "
     "mise a jour dispo "
     "via reglages",
     false, "Mise a jour via l'OS = OK"},
    {"WhatsApp: patron "
     "demande un virement "
     "urgent discret",
     true, "Verifiez par un autre canal"},
    {"Mail collegue avec "
     "PJ .exe \"facture\"",
     true, "Piece jointe executable = danger"},
};

constexpr size_t Q_COUNT = sizeof(QUESTIONS) / sizeof(QUESTIONS[0]);

size_t qIndex = 0;
uint8_t score = 0;
bool answered = false;
bool lastCorrect = false;
bool done = false;

void wrapPrint(int x, int y, const char* text, uint16_t color) {
    auto& dsp = M5.Display;
    dsp.setTextColor(color);
    dsp.setTextSize(1);
    String t = text;
    while (t.length() > 0 && y < 130) {
        int chunk = 22;
        if ((int)t.length() <= chunk) {
            dsp.setCursor(x, y);
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
        dsp.setCursor(x, y);
        dsp.println(line.c_str());
        y += 12;
    }
}

}  // namespace

void phishingQuizInit() {
    qIndex = 0;
    score = 0;
    answered = false;
    lastCorrect = false;
    done = false;
}

void phishingQuizDraw() {
    auto& dsp = M5.Display;
    dsp.fillScreen(TFT_BLACK);
    drawHeader("Quiz");

    if (done) {
        dsp.setTextColor(TFT_WHITE);
        dsp.setTextSize(1);
        dsp.setCursor(4, 36);
        dsp.printf("Score: %u / %u", score, Q_COUNT);

        dsp.setTextColor(score >= Q_COUNT * 2 / 3 ? TFT_GREEN : TFT_ORANGE);
        dsp.setCursor(4, 56);
        if (score == Q_COUNT) {
            dsp.println("Parfait !");
        } else if (score >= Q_COUNT * 2 / 3) {
            dsp.println("Bien joue");
        } else {
            dsp.println("A revoir...");
        }

        wrapPrint(4, 80, "Astuce: l'urgence et les liens suspects = alerte", TFT_CYAN);
        drawFooter("A: recommencer  B: menu");
        return;
    }

    dsp.setTextColor(TFT_DARKGREY);
    dsp.setTextSize(1);
    dsp.setCursor(4, 28);
    dsp.printf("Q%u/%u  score %u", qIndex + 1, Q_COUNT, score);

    wrapPrint(4, 42, QUESTIONS[qIndex].text, TFT_WHITE);

    if (answered) {
        dsp.setTextColor(lastCorrect ? TFT_GREEN : TFT_RED);
        dsp.setCursor(4, 100);
        dsp.println(lastCorrect ? "Correct !" : "Rate !");
        wrapPrint(4, 114, QUESTIONS[qIndex].tip, TFT_YELLOW);
        drawFooter("A: suivant  B: menu");
    } else {
        drawFooter("A: arnaque  B: legit");
    }
}

void phishingQuizLoop() {
    // rien a animer
}

void phishingAnswer(bool sayPhishing) {
    if (done || answered) return;

    lastCorrect = (sayPhishing == QUESTIONS[qIndex].isPhishing);
    if (lastCorrect) score++;
    answered = true;
    Serial.printf("[QUIZ] Q%u %s (attendu=%s)\n", qIndex + 1,
                  lastCorrect ? "OK" : "KO",
                  QUESTIONS[qIndex].isPhishing ? "phishing" : "legitime");
    phishingQuizDraw();
}

void phishingNext() {
    if (done) {
        phishingQuizInit();
        phishingQuizDraw();
        return;
    }
    if (!answered) return;

    qIndex++;
    answered = false;
    if (qIndex >= Q_COUNT) {
        done = true;
        Serial.printf("[QUIZ] Termine: %u/%u\n", score, Q_COUNT);
    }
    phishingQuizDraw();
}

bool phishingIsDone() { return done; }
bool phishingIsAnswered() { return answered; }
