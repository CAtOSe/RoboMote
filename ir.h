#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>

IRsend irsend;

static const byte progMsg = 0b001011;
static const byte driveMsg = 0b000111;

static const byte necStartT = 0x1;
static const byte necStrategyT = 0x2;
static const byte necVarT = 0x3;
static const byte necEndT = 0x6;

static const int IR_DELAY = 50;


uint32_t message(uint16_t addr, uint8_t type, uint8_t cmd) {
  return ((uint32_t) addr << 20) | ((uint32_t) type << 16) | ((uint32_t) cmd << 8) | ((uint8_t) ~cmd);
}

void startMiniSumo() {
  irsend.sendRC5((driveMsg << 6) | (ring << 1) | 1 , 12);
  delay(IR_DELAY);

  irsend.sendRC5((driveMsg << 6) | (ring << 1) | 1 , 12);
  delay(IR_DELAY);

  irsend.sendRC5((driveMsg << 6) | (ring << 1) | 1 , 12);
}

void stopMiniSumo() {
  irsend.sendRC5((driveMsg << 6) | (ring << 1) | 0 , 12);
  delay(IR_DELAY);

  irsend.sendRC5((driveMsg << 6) | (ring << 1) | 0 , 12);
  delay(IR_DELAY);

  irsend.sendRC5((driveMsg << 6) | (ring << 1) | 0 , 12);
}

void programMiniSumo() {
  irsend.sendRC5((progMsg << 6) | (ring << 1) | 1 , 12);
  delay(IR_DELAY);

  irsend.sendRC5((progMsg << 6) | (ring << 1) | 1 , 12);
  delay(IR_DELAY);

  irsend.sendRC5((progMsg << 6) | (ring << 1) | 1 , 12);
}


bool sendStrat() {
  // Stretegy variable check
  if (!varNames[0].equals(STRAT_NUM) && varVals[0] < 256 && varVals[0] >= 0) return false;

  int8 msgNum = 3 + (vars - 1) * 2;

  // Start of transmission message
  irsend.sendNEC(message(necAddr, necStartT, msgNum), 32);
  delay(IR_DELAY);

  // Strategy message
  irsend.sendNEC(message(necAddr, necStrategyT, varVals[0]), 32);
  delay(IR_DELAY);

  // Variables
  for (int x = 1; x < vars; x++) {
    irsend.sendNEC(message(necAddr, necVarT, x - 1), 32);
    delay(IR_DELAY);

    uint32_t sendVar;
    memcpy(&sendVar, &varVals[x],  sizeof(uint32_t));

    irsend.sendNEC(sendVar, 32);
    delay(IR_DELAY);
  }

  // End of transmission
  irsend.sendNEC(message(necAddr, necEndT, msgNum), 32);

  return true;
}
