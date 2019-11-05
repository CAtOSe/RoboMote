// ===========   GENERAL   ==========
typedef byte int8;
#include "config.h"
#include "strats.h"

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


// ============   GUI   =============
#include "gui.h"
int8 inputChange = 0;
const String botMenu[] = {"Mini Sumo", "mBot Line"};
const String miniSumoMenu[] = {"Strategy", "Dohyo"};
const String miniSumoMenu_NoSD[1] = {"Dohyo"};
const String stratMenu[] = {"Send", "Send & Run"};


// ============   SD   ==============
String bot;
bool sdError = false;
#include "sd.h"


// ============   IR   ==============
#include "ir.h"

void setup() {
  pinMode(okPin, INPUT);
  pinMode(backPin, INPUT);
  pinMode(startPin, INPUT);
  pinMode(stopPin, INPUT);
  pinMode(programPin, INPUT);

  // Serial.begin(9600);

  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.setFont(OLED_FONT);

  int8 cardStatus =  initSD();
  if (cardStatus > 0) {
    sdError = true;
    throwError("SD Error", cardStatus);
  }

  buildMenu(2, botMenu, true);
  printMenu();
  menuState = 0;
}



void loop() {
  if (inputChange > 1 && inputChange < 15) delay(300);
  inputChange = readInputs();

  if (inputChange > 15 && bot.equals(MINI_SUMO_DIR)) {
    if (inputChange == 16) {
      startMiniSumo();
    } else if (inputChange == 32) {
      stopMiniSumo();
    } else if (inputChange == 64) {
      programMiniSumo();
    }
  } else if (inputChange > 15 && bot.equals(MBOT_DIR)) {
    if (inputChange == 16) {
      sendEvent(MBOT_START);
    } else if (inputChange == 32) {
      sendEvent(MBOT_STOP);
    } else if (inputChange == 64) {
      sendEvent(MBOT_PROGRAM);
    }
  }

  // Start from robot selection (menuState: 0)
  if (menuState == 0) {
    if (inputChange == 1) {
       printMenu();
    } else if(inputChange == 2) {
      // Mini Sumo is selected:
      bot = MINI_SUMO_DIR;
      if (menuPos == 0) {
        if (!sdError) buildMenu(2, miniSumoMenu, true);
        else buildMenu(1, miniSumoMenu_NoSD, true);
        menuState = 1;
        printMenu();
      }
      // mBot Line is selected:
      else if (menuPos == 1) {
        if (!sdError) {
          bot = MBOT_DIR;
          // Load strategy list
          int8 r = loadStratList();
          if(r == 0) {
            if (stCount > 0) {
              buildMenu(stCount, strats, false);
              menuState = 4;
              printMenu();
            } else {
              showMsg("No Strategies");
              delay(1000);
              printMenu();
            }
          } else {
            throwError("SD ", r);
            printMenu();
          }
        }
      }
    }
  }

  // Mini Sumo menu
  else if (menuState == 1) {
    if (inputChange == 1) {
      lastChange = millis();
      printMenu();
    } else if (inputChange == 2) {
      if (menuPos == 0 && !sdError) {
        // Load strategy list
        int8 r = loadStratList();
        if(r == 0) {
          if (stCount > 0) {
            buildMenu(stCount, strats, false);
            menuState = 2;
            printMenu();
          } else {
            showMsg("No Strategies");
            delay(1000);
            printMenu();
          }
        } else {
          throwError("SD ", r);
          printMenu();
        }
      } else if (menuPos == 1 && !sdError) {
        // Dohyo Menu
        buildEditor(-1);
        printEditor();
      }  else if (menuPos == 0 && sdError) {
        // Dohyo Menu
        buildEditor(-1);
        printEditor();
      }
    } else if (inputChange == 4) {
      buildMenu(2, botMenu, true);
      menuState = 0;
      printMenu();
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
      } else {
        throwError("SD ", r);
        printMenu();
      }

    } else if (inputChange == 4) {
      buildMenu(2, miniSumoMenu, true);
      menuState = 1;
      printMenu();
    }
  }

  // Mini Sumo Strategy Screen
  else if (menuState == 3) {
    if (inputChange == 1) {
       printMenu();
    } else if (inputChange == 2) {
      if (menuPos == 0) {
        showMsg("Sent");
        sendStrat();
        delay(1000);
        printMenu();
      } else if (menuPos == 1) {
        showMsg("Running");
        sendStrat();

        delay(IR_DELAY);
        programMiniSumo();

        delay(IR_DELAY);
        startMiniSumo();

        delay(1000);
        printMenu();

      } else if (menuPos > 1) {
        if (!varNames[menuPos - 2].equals(STRAT_NUM)) {
          buildEditor(menuPos - 2);
          printEditor();
        }
      }
    } else if (inputChange == 4) {
      buildMenu(stCount, strats, false);
      menuState = 2;
      printMenu();
    }
  }


// mBot Strategies selection
  else if (menuState == 4) {
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
        menuState = 5;
        printMenu();
      } else {
        throwError("SD ", r);
        printMenu();
      }

    } else if (inputChange == 4) {
      buildMenu(2, botMenu, true);
      menuState = 0;
      printMenu();
    }
  }

  // mBot Strategy/Variable Screen
  else if (menuState == 5) {
    if (inputChange == 1) {
       printMenu();
    } else if (inputChange == 2) {
      if (menuPos == 0) {
        showMsg("Sent");
        sendStrat();
        delay(1000);
        printMenu();
      } else if (menuPos == 1) {
        showMsg("Running");
        sendStrat();

        delay(1000);
        printMenu();

      } else if (menuPos > 1) {
        if (!varNames[menuPos - 2].equals(STRAT_NUM)) {
          buildEditor(menuPos - 2);
          printEditor();
        }
      }
    } else if (inputChange == 4) {
      buildMenu(stCount, strats, false);
      menuState = 4;
      printMenu();
    }
  }



  // Variable editor
  else if (menuState == 10) {
    if (inputChange == 1) {
      if (updateValue()) printEditor();
    } else if (inputChange == 2) {
      if (type == 3) {
        floatSelected = !floatSelected;
        printEditor();
      } else {
        saveValue();
        writeToFile(selectedSt);

        resumeMenu();
        printMenu();
      }
    } else if (inputChange == 4) {
      resumeMenu();
      printMenu();
    }  else if (inputChange == 8) {
      saveValue();
      writeToFile(selectedSt);

      resumeMenu();
      printMenu();

      delay(OK_DELAY * 2);
    }
  }


}


/*
 * Inputs number values:
 * 0 - nothing
 * 1 - encoder change
 * 2 - OK Button pressed
 * 4 - Back Button pressed
 * 8 - Long Press OK
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
      while (digitalRead(okPin) && (millis() < okTime + 1000));
      if (millis() - okTime > OK_DELAY) r+= 8;
      else r += 2;
    }
  }


  // Back Button
  if (digitalRead(backPin)) r += 4;

  // Start Button
  if (digitalRead(startPin)) r += 16;

  // Stop Button
  if (digitalRead(stopPin)) r += 32;

  // Send Button
  if (digitalRead(programPin)) r += 64;

  return r;
}
