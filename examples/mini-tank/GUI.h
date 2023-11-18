#include "GUIIcons.h"

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE);
int dw = 0;  // display width
int dh = 0;  // display height

int lastDrawedLine = 0;

void showWelcome() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(0, 0, "MiniTank v001");
  u8g2.sendBuffer();
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(dw - 18, 1, String(SRC_REV).c_str());
  u8g2.drawLine(0, 9, dw - 1, 9);
  lastDrawedLine = 10;
  // only for first screen
  u8g2.sendBuffer();
}

void showWelcomeMessage(String msg) {
  if (lastDrawedLine >= dh - 6) {
    delay(500);
    showWelcome();
  }
  u8g2.setFont(u8g2_font_4x6_tf);
  if (dh == 32) {
    if (lastDrawedLine < 32) {
      u8g2.drawStr(0, lastDrawedLine, msg.c_str());
    } else {
      u8g2.drawStr(72, lastDrawedLine - 20, msg.c_str());
    }
  } else
    u8g2.drawStr(0, lastDrawedLine, msg.c_str());
  lastDrawedLine = lastDrawedLine + 7;
  u8g2.sendBuffer();
}

void displayEmoticonLabel(int cursor, String msg) {
  u8g2.setFont(u8g2_font_unifont_t_emoticons);
  u8g2.drawGlyph(76, 12, cursor);
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.setCursor(77, 17);
  u8g2.print(msg);
}

void displayEmoticon(int speed) {
  if(speed == 0) return;
  static uint_least32_t smileTStamp = 0;
  if (millis() - smileTStamp > 500) {
    smileTStamp = millis();
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    if (speed > 60)
      u8g2.drawXBM(15, 2, 32, 32, SmileFaceGood);
    else if (speed > 0)
      u8g2.drawXBM(15, 2, 32, 32, SmileFaceModerate);
    else if (speed < -70)
      u8g2.drawXBM(15, 2, 32, 32, SmileFaceHazardous);
    else if (speed < -50)
      u8g2.drawXBM(15, 2, 32, 32, SmileFaceVeryUnhealthy);
    else if (speed < -40)
      u8g2.drawXBM(15, 2, 32, 32, SmileFaceUnhealthySGroups);
    else if (speed < -20)
      u8g2.drawXBM(15, 2, 32, 32, SmileFaceUnhealthy);
    else
      u8g2.drawXBM(15, 2, 32, 32, SmileFaceModerate);
    u8g2.sendBuffer();
  }
}

void displayBigMsg(String msg) {
  u8g2.setFont(u8g2_font_inb19_mn);
  int strw = u8g2.getStrWidth(msg.c_str());
  u8g2.setCursor((dw - strw) / 2, 1);
  u8g2.print(msg.c_str());
}

void displayBottomLine(String msg) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  int strw = u8g2.getStrWidth(msg.c_str());
  u8g2.setCursor((dw - strw) / 2, 25);
  u8g2.print(msg.c_str());
  u8g2.sendBuffer();
}

void displayInit() {
  u8g2.setBusClock(100000);
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setContrast(128);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  u8g2.setFontMode(0);
  dw = u8g2.getDisplayWidth();
  dh = u8g2.getDisplayHeight();
  Serial.println("-->[OGUI] display config ready.");
}