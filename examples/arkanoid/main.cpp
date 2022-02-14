#include <EspNowJoystick.hpp>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Preferences.h>
#include "battery.hpp"
#include "hal.hpp"
#include "bmp.h"

#define TFT_GREY 0x5AEB
#define lightblue 0x2D18
#define orange 0xFB60
#define purple 0xFB9B

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

EspNowJoystick joystick;
TelemetryMessage tm;

float ys = 1;
float x = random(30, 100);  //coordinates of ball
float y = 70;
int ny = y;  //coordinates of previous position
int nx = x;
float px = 45;  //67 je sredina    pozicija igrača
int pxn = px;
int vrije[2] = {1, -1};
int enx[16] = {8, 33, 58, 83, 108, 8, 33, 58, 83, 108, 22, 47, 72, 97, 47, 72};
int eny[16] = {37, 37, 37, 37, 37, 45, 45, 45, 45, 45, 53, 53, 53, 53, 61, 61};
int enc[16] = {TFT_RED, TFT_RED, TFT_RED, TFT_RED, TFT_RED, TFT_GREEN, TFT_GREEN, TFT_GREEN, TFT_GREEN, TFT_GREEN, orange, orange, orange, orange, lightblue, lightblue};
int score = 0;
int level = 1;
float amount[4] = {0.25, 0.50, 0.75, 1};
float xs = amount[random(4)] * vrije[random(2)];
int fase = 0;
float xstep = 1;
int spe = 0;
int pom = 0;
int gameSpeed = 7000;

uint32_t suspendCount = 0;

Preferences preferences;
const char * app_name = "breakout";

void setSpeed(int16_t Vtx, int16_t Vty, int16_t Wt) {
    Wt = (Wt > 100) ? 100 : Wt;
    Wt = (Wt < -100) ? -100 : Wt;
    Vty = (Vty > 100) ? 100 : Vty;
    Vty = (Vty < -100) ? -100 : Vty;

    int turn = map(abs(Wt), 0, 100, 0, 5);
    
    // Serial.printf("[Vtx:%04d Vty:%04d Wt:%04d ]\n",Vtx, Vty, Wt);
    // Serial.println(debugCount++);

    int dir = (Wt>0) ? 0 : 1; // 0 left, 1 right

    if (turn > 0) {
        if (dir == 0) {  // turn left
            px = px - turn;
        } else {  // turn right
            px = px + turn;
        }
    }

}

void resetVars() {
    tft.setCursor(99, 0, 2);
    tft.println("LVL" + String(level));
    y = 75;
    ys = 1;
    x = random(30, 100);

    int enx2[16] = {8, 33, 58, 83, 108, 8, 33, 58, 83, 108, 22, 47, 72, 97, 47, 72};
    for (int n = 0; n < 16; n++) {
        enx[n] = enx2[n];
    }
}

void newLevel() {
    score = score + 1;
    delay(3000);
    gameSpeed = gameSpeed - 500;
    level = level + 1;
    resetVars();
}

void espDelay(int ms) {
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_light_sleep_start();
}

void showSplash() {
    tft.setRotation(0);
    tft.setSwapBytes(true);
    tft.pushImage(0, 0, 135, 240, bootlogo);
}

void displayBoard() {
    tft.fillScreen(TFT_BLACK);
    tft.drawLine(0, 17, 0, 240, TFT_GREY);
    tft.drawLine(0, 17, 135, 17, TFT_GREY);
    tft.drawLine(134, 17, 134, 240, TFT_GREY);

    tft.setCursor(3, 3, 2);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    tft.setCursor(0, 0, 2);
    tft.println("SCORE " + String(score));

    tft.setCursor(99, 0, 2);
    tft.println("LVL" + String(level));
}

void suspend() {
    tm.e1 = false;   // notify to joystick that we are sleeping
    joystick.sendTelemetryMsg(tm);

    suspendCount = 0;
    showSplash();
    espDelay(3000);
    int r = digitalRead(TFT_BL);
    digitalWrite(TFT_BL, !r);
    espDelay(1000);
    tft.writecommand(TFT_DISPOFF);
    tft.writecommand(TFT_SLPIN);
    //After using light sleep, you need to disable timer wake, because here use external IO port to wake up
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    // esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    delay(200);
    esp_deep_sleep_start();
}

void saveRecord(int level, int score) {
    preferences.begin(app_name,false);
    preferences.putInt("level_record",level);
    preferences.putInt("score_record",score);
    preferences.end();
}

int getLevelRecord() {
    preferences.begin(app_name, false);
    int record = preferences.getInt("level_record",0);
    preferences.end();
    return record;
}

int getScoreRecord() {
    preferences.begin(app_name, false);
    int record = preferences.getInt("score_record",0);
    preferences.end();
    return record;
}

void showBatteryStatus() {
    tft.setTextSize(1);
    float volts = battGetVoltage();
    String batt = "";
    if (battIsCharging()) {
        tft.setTextColor(TFT_GREEN);
        batt = "BAT:" + String(battCalcPercentage(volts)) + "%";
    }
    else {
        int batt_value = battCalcPercentage(volts);
        if (batt_value < 30) tft.setTextColor(TFT_RED);
        batt = "BAT: " + String(batt_value) + "%";
    } 
    tft.setTextDatum(TL_DATUM);
    tft.drawString(batt, 0, 0);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(TC_DATUM);
    String voltage = "" + String(volts) + "v";
    tft.setTextDatum(TR_DATUM);
    tft.drawString(voltage, tft.width(), 0);
}

void showGameOver(){
    tft.fillScreen(TFT_BLACK);

    tft.setCursor(13, 83, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    tft.println("GAME OVER");

    tft.setCursor(13, 103, 4);
    tft.println("SCORE:" + String(score));

    tft.setCursor(13, 130, 4);
    tft.println("LEVEL:" + String(level));

    tft.drawLine(0, 155, 135, 155, TFT_GREY);

    int level_record = getLevelRecord();
    int score_record = getScoreRecord();

    tft.setTextSize(1);
    tft.setCursor(13, 164, 2);

    if (level >= level_record && score > score_record) {
        saveRecord(level, score);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.println("!! NEW RECORD !!");
    } else {
        tft.println("RECORD:");
    }

    tft.setCursor(13, 185, 2);
    tft.println("Score: " + String(getScoreRecord()));

    tft.setCursor(13, 200, 2);
    tft.println("Level: " + String(getLevelRecord()));
}

void sendHeartbeat() {
    static uint_least32_t timeStamp = 0;
    if (millis() - timeStamp > 500) {
        timeStamp = millis();
        tm.btv = battGetVoltage();
        tm.btl = battCalcPercentage(tm.btv);
        tm.e1 = true;
        joystick.sendTelemetryMsg(tm);
    }
}

class MyJoystickCallback : public EspNowJoystickCallbacks {
    void onJoystickMsg(JoystickMessage jm){
        if (jm.ck == 0x01) {
            setSpeed(jm.ax - 100, jm.ay - 100, jm.az - 100);
        } else {
            setSpeed(0, 0, 0);
        }
    };
    void onError(){
        setSpeed(0, 0, 0);
        Serial.println("Error");
    };
};

void setup(void) {
    Serial.begin(115200);
    Serial.println("\n-->[SETUP] init:");
    Serial.begin(115200);
    delay(100);
    joystick.setJoystickCallbacks(new MyJoystickCallback());
    tm = joystick.newTelemetryMsg();
    joystick.init();
    pinMode(BUTTON_L, INPUT_PULLUP);
    pinMode(BUTTON_R, INPUT_PULLUP);
    tft.begin();
    setupBattery();              // init battery ADC.
    setupBattADC();
    showSplash();
}

void loop() {
    if (suspendCount++ > 10000000) suspend();  //auto suspend after ~1 minute
    sendHeartbeat();
    if (fase == 0) {
        if (digitalRead(0) == 0 || digitalRead(35) == 0 || px != 45) {
            if (pom == 0) {
                displayBoard();
                fase = fase + 1;
                pom = 1;
            }
        } else {
            pom = 0;
        }
    }

    if (fase == 1) {
        if (y != ny) {
            tft.fillEllipse(nx, ny, 2, 2, TFT_BLACK);  //brisanje loptice
            ny = y;
            nx = x;
        }
        if (int(px) != pxn) {
            tft.fillRect(pxn, 234, 24, 4, TFT_BLACK);  //briasnje igraca
            pxn = px;
        }

        // spe=spe+1;

        if (px >= 2 && px <= 109) {
            if (digitalRead(BUTTON_L) == 0)
                px = px - 1;
            if (digitalRead(BUTTON_R) == 0)
                px = px + 1;
        }
        if (px <= 3)
            px = 4;

        if (px >= 108)
            px = 107;

        if (y > 232 && x > px && x < px + 24) {  ///brisati kasnije
            ys = ys * -1;

            xs = amount[random(4)] * vrije[random(2)];
        }

        for (int j = 0; j < 16; j++) {
            if (x > enx[j] && x < enx[j] + 20 && y > eny[j] && y < eny[j] + 5) {
                tft.fillRect(enx[j], eny[j], 20, 4, TFT_BLACK);
                enx[j] = 400;
                ys = ys * -1;
                xs = amount[random(4)] * vrije[random(2)];

                score = score + 1;

                tft.setCursor(0, 0, 2);
                tft.println("SCORE " + String(score));
            }
        }

        if (y < 21)
            ys = ys * -1.00;

        if (y > 240)
            fase = fase + 1;

        if (x >= 130)
            xs = xs * -1.00;

        if (x <= 4)
            xs = xs * -1.00;

        for (int i = 0; i < 16; i++)  //draw enemies
            tft.fillRect(enx[i], eny[i], 20, 4, enc[i]);

        tft.fillEllipse(int(x), y, 2, 2, TFT_WHITE);  // draw ball

        //if(spe>10){  //change coordinates of ball
        y = y + ys;
        x = x + xs;
        //spe=0;
        //}

        tft.fillRect(px, 234, 24, 4, TFT_WHITE);

        if (score == 16 || score == 33 || score == 50 || score == 67 || score == 84 || score == 101 || score == 118 || score == 135 || score == 152 || score == 169)
            newLevel();

        suspendCount = 0;
        delayMicroseconds(gameSpeed);
    }
    if (fase == 2) {
        px = 45;
        tm.e1 = 1;
        joystick.sendTelemetryMsg(tm);
        showGameOver();
        showBatteryStatus();
        setupBattADC();
        delay(500);
        fase++;
    }

    // reset the game with right button
    if (fase == 3 && (digitalRead(BUTTON_R) == 0 || px != 45)) {
        suspendCount = 0;
        score = 0;
        level = 1;
        fase = 0;
        pom = 0;
        gameSpeed = 7000;
        resetVars();
    }
}
