// ===========   GENERAL   ==========
typedef byte int8;
#include "strats.h"
#define PRESS_DELAY 300
#define FLOAT_POINTS 6
#define ACCEL_TIME_OUT 100
#define ACCEL_FACT_DIV 140
#define OK_DELAY 500

// ==========   DISPLAY   ==========
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#define I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;
#define OLED_FONT System5x7


// ==========   ENCODER   ==========
#include <Encoder.h>
Encoder enc(3, 2);
long oldPos  = -999;
long newPos;


// ==========   BUTTONS   ===========
static const int8 okPin = 5;
static const int8 backPin = 6;


// ============   GUI   =============
#include "gui.h"
const String botMenu[] = {"Mini Sumo", "mBot Line"};
const String miniSumoMenu[] = {"Strategy", "Dohyo"};
const String stratMenu[] = {"Send & Run", "Send"};


// ============   SD   ==============
#include "sd.h"

void setup() {
  Serial.begin(9600);

  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.setFont(OLED_FONT);

  int8 cardStatus =  initSD();
  if (cardStatus > 0) {
    throwError("SD Error", cardStatus);
  }


  buildMenu(2, botMenu, true);
  printMenu();
  menuState = 0;
}



void loop() {
  int8 inputChange = readInputs();

  // Start from robot selection (menuState: 0)
  if (menuState == 0) {
    if (inputChange == 1) {
       printMenu();
    } else if(inputChange == 2) {
      // Mini Sumo is selected:
      if (menuPos == 0) {
        buildMenu(2, miniSumoMenu, true);
        menuState = 1;
        printMenu();
        delay(PRESS_DELAY);
      }
      // mBot Line is selected:
      else if (menuPos == 1) {

      }
    }
  }

  // Mini Sumo menu
  else if (menuState == 1) {
    if (inputChange == 1) {
      lastChange = millis();
      printMenu();
    } else if (inputChange == 2) {
      // Load strategy list
      int8 r = loadStratList();
      if(r == 0) {
        buildMenu(stCount, strats, false);
        menuState = 2;
        printMenu();
        delay(PRESS_DELAY);
      } else {
        throwError("SD ", r);
        printMenu();
        delay(PRESS_DELAY);
      }
    } else if (inputChange == 4) {
      buildMenu(2, botMenu, true);
      menuState = 0;
      printMenu();
      delay(PRESS_DELAY);
    }
  }

  // Mini Sumo Strategies
  else if (menuState == 2) {
    if (inputChange == 1) {
       printMenu();
    } else if (inputChange == 2) {
      // Load strategy
      int8 r = loadStrat(menuPos);
      if (r == 0 ) {
        selectedSt = menuPos;
        buildMenu(2, stratMenu, false);
        appendMenu(vars, varNames);
        addVariables(2, vars+1);
        menuState = 3;
        printMenu();
        delay(PRESS_DELAY);
      } else {
        throwError("SD ", r);
        printMenu();
        delay(PRESS_DELAY);
      }

    } else if (inputChange == 4) {
      buildMenu(2, miniSumoMenu, true);
      menuState = 1;
      printMenu();
      delay(PRESS_DELAY);
    }
  }

  // Mini Sumo Strategy Screen
  else if (menuState == 3) {
    if (inputChange == 1) {
       printMenu();
    } else if (inputChange == 2) {
      buildEditor(menuPos - 2);
      menuState = 10;
      printEditor();
      delay(PRESS_DELAY);
    } else if (inputChange == 4) {
      buildMenu(stCount, strats, false);
      menuState = 2;
      printMenu();
      delay(PRESS_DELAY);
    }


  // Variable editor
  } else if (menuState == 10) {
    if (inputChange == 1) {
      if (updateValue()) printEditor();
    } else if (inputChange == 2 || inputChange == 4) {
      if (inputChange == 2) {
        if (type == 3) {
          floatSelected = !floatSelected;
          printEditor();
        } else {
          saveValue();
          writeToFile(selectedSt);

          menuState = 3;
          resumeMenu();
          printMenu();
        }
      }
      delay(PRESS_DELAY);
    }
  }


}


/*
 * Inputs number values:
 * 0 - nothing
 * 1 - encoder change
 * 2 - OK Button pressed
 * 4 - Back Button pressed
 */

int8 readInputs() {
  int8 r = 0;

  if (!editorOpen) {
    // Encoder output
    newPos = enc.read() / 4;
    if (newPos != oldPos) {
      if (newPos > menuPosMax){
        enc.write(menuPosMax * 4);
        newPos = menuPosMax;
      } else if (newPos < 0){
        enc.write(0);
        newPos = 0;
      } else r += 1;

      menuPos = newPos;
      oldPos = newPos;
    }


    // OK Button
    if (digitalRead(okPin)) r += 2;
  } else {
    newPos = enc.read() / 4;

    if (newPos != oldPos) {
      if (lastChange + ACCEL_TIME_OUT < millis()) {
        accelerationFactor = 1;
      } else {
        accelerationFactor = ACCEL_FACT_DIV / (millis() - lastChange);
      }

      offset = (newPos - oldPos) * accelerationFactor;

      lastChange = millis();
      oldPos = newPos;

      r += 1;
    }

    // OK BUTTON PRESS n HOLD
    if (digitalRead(okPin)) {
      long okTime = millis();
      while (digitalRead(okPin));
      if (millis() - okTime > OK_DELAY) r+= 8;
      else r += 2;
    }
  }


  // Back Button
  if (digitalRead(backPin)) r += 4;

  return r;
}
