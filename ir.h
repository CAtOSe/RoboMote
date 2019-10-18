#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>

IRsend irsend;

byte progMsg = 0b001011;
byte driveMsg = 0b000111;

void startMiniSumo() {
  irsend.sendRC5((driveMsg << 6) | (ring << 1) | 1 , 12);
}

void stopMiniSumo() {
  irsend.sendRC5((driveMsg << 6) | (ring << 1) | 0 , 12);
}

void programMiniSumo() {
  irsend.sendRC5((progMsg << 6) | (ring << 1) | 1 , 12);
}
