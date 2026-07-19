#pragma once

#include <M5Unified.h>

enum class AppMode : uint8_t {
    Menu = 0,
    FakeWifi,
    BruteSim,
    BruteReal,
    Awareness,
    PcBridge,
    PhishingQuiz,
    DdosSim,
    DdosLab,
};

void menuInit();
void menuDraw();
AppMode menuUpdate();

void fakeWifiInit();
void fakeWifiDraw();
void fakeWifiLoop();

void bruteSimInit();
void bruteSimDraw();
void bruteSimLoop();

void bruteRealInit();
void bruteRealDraw();
void bruteRealLoop();

void awarenessInit();
void awarenessDraw();
void awarenessLoop();

void pcBridgeInit();
void pcBridgeDraw();
void pcBridgeLoop();

void phishingQuizInit();
void phishingQuizDraw();
void phishingQuizLoop();

void ddosSimInit();
void ddosSimDraw();
void ddosSimLoop();

void ddosLabInit();
void ddosLabDraw();
void ddosLabLoop();

void drawHeader(const char* title);
void drawFooter(const char* hint);
