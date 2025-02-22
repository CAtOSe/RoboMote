// State var setup
int8 menuState = 0;
int8 prevMenuState = 0;
int menuPos = 0;
int8 menuPosMax = 0;

int8 menuSize;
String menu[MAX_MENU] = {"Entry 1", "Entry 2", "Entry 3", "Entry 4", "Entry 5", "Entry 6", "Entry 7", "Entry 8"};
bool isBig;
bool hasVariables;
int8 varStart = 0;
int8 varEnd = 0;

bool editorOpen = true;
unsigned long lastChange = 0;
bool useAcceleration = false;
int accelerationFactor = 1;
long offset = 0;

bool floatSelected = 0;
int floatSelPos = 0;
int8 ring = 0;

int vPos = 0;
int8 type = 0;
float tempVarF = 0;
boolean tempVarB = 0;
long tempVarI = 0;
// =========== FUNCTIONS =============
int8 countDigit(int n)
{
    if (n == 0) return 1;
    int8 count = 0;
    while (n != 0) {
        n = n / 10;
        ++count;
    }
    return count;
}


void buildMenu(int8 menuEntryCount, String menuEntries[MAX_MENU], bool isMenuBig) {
  for (int8 x = 0; x < menuEntryCount; x++) {
    menu[x] = menuEntries[x];
  }
  menuSize = menuEntryCount;
  menuPos = 0;
  oldPos = menuPos;
  enc.write(0);
  menuPosMax = menuEntryCount - 1;
  isBig = isMenuBig;
  hasVariables = false;
  useAcceleration = false;
  editorOpen = false;
}

void appendMenu(int8 menuEntryCount, String menuEntries[MAX_MENU]) {
  for (int8 x = 0; x < menuEntryCount; x++) {
    menu[menuSize + x] = menuEntries[x];
  }

  menuSize += menuEntryCount;
  menuPosMax = menuSize - 1;
}

void addVariables(int8 from, int8 to) {
  hasVariables = true;
  varStart = from;
  varEnd = to;
}

void addSpace(int8 w1, int8 w2) {
  int8 spaces = OLED_WIDTH_SMALL - w1 - w2;
  if (spaces > 0) {
    for (int x = 0; x < spaces; x++) {
      oled.print(" ");
    }
  }
}

void printMenu() {
  int8 subdiv;
  if (isBig) {
    oled.set2X();
    subdiv = 2;
  }
  else {
    oled.set1X();
    subdiv = 4;
  }

  int8 menuPage = menuPos / subdiv;  // 0

  oled.clear();
  for (int y = menuPage * subdiv; y < min((menuPage + 1) * subdiv, menuSize); y++) {
    if (y == menuPos) oled.setInvertMode(1);
    else oled.setInvertMode(0);

    oled.print(menu[y]);

    if (hasVariables && (y >= varStart && y <= varEnd)) {
      String val;
      if (varType[y - varStart] == 1) {
        if (varVals[y - varStart] == 1) val = "True";
        else val = "False";
      } else if (varType[y - varStart] == 3) {
        float temp;
        memcpy(&temp, &varVals[y - varStart], sizeof(temp));
        val = String(temp, FLOAT_POINTS);
      }
      else val = String(varVals[y - varStart]);

      addSpace(menu[y].length(), val.length());
      oled.print(val);
    }

    oled.println();

  }
}

void resumeMenu() {
  menuState = prevMenuState;
  menuPos = 0;
  oldPos = menuPos;
  enc.write(0);
  menuPosMax = menuSize - 1;
  useAcceleration = false;
  editorOpen = false;
}

void throwError(char *message, int code) {
  oled.clear();
  oled.set1X();
  oled.setInvertMode(0);

  oled.println("ERROR:");
  oled.println(message);

  if(code != -1) {
    oled.print("Code: ");
    oled.println(code);
  }

  oled.print("Press OK");
  delay(OK_DELAY);
  while(!digitalRead(okPin));
  delay(OK_DELAY);
}

String formatBool(bool val) {
  if (val) return String("True");
  else return String("False");
}

void buildEditor(int pos) {
  prevMenuState = menuState;
  menuState = 10;
  editorOpen = true;

  if (pos == -1) {
    // Dohyo
    type = 2;
    tempVarI = ring;
    vPos = -1;
    return;
  }
  vPos = pos;
  type = varType[pos];

  if (type == 1) {
    tempVarB = varVals[pos];
  } else if (type == 2) {
    tempVarI = varVals[pos];
  }
  else if (type == 3) memcpy(&tempVarF, &varVals[pos], sizeof(tempVarF));
}

bool updateValue() {
  if (type == 1) {
    bool lastV = tempVarB;
    if (offset > 0) tempVarB = true;
    else tempVarB = false;
    if (lastV == tempVarB) return false;
  } else if (type == 2) tempVarI += offset;
  else if (type == 3) {
    if (floatSelected) {
      tempVarF += offset / pow(10, floatSelPos + 1);
    } else {

      floatSelPos += offset;
      if (floatSelPos < 0) floatSelPos = 0;
      else if (floatSelPos > FLOAT_POINTS - 1 ) floatSelPos = FLOAT_POINTS - 1;
    }
  }

  if (vPos == -1) {
    tempVarI = max(tempVarI, 0);
    tempVarI = min(tempVarI, 31);
  }

  return true;
}

void saveValue() {
  if (vPos == -1) {
    ring = tempVarI;
    return;
  }

  if (type == 1) varVals[vPos] = tempVarB;
  else if (type == 2) varVals[vPos] = tempVarI;
  else if (type == 3) memcpy(&varVals[vPos], &tempVarF, sizeof(tempVarF));
}



void printEditor() {
  oled.clear();
  oled.set1X();
  oled.setInvertMode(0);

  if (vPos == -1) {
    oled.set2X();
    oled.println("Dohyo");
    oled.setCursor(0, 2);
    oled.println(tempVarI);
  } else {
    oled.println(varNames[vPos]);
    if (type == 1) oled.println(formatBool(varVals[vPos]));
    else if (type == 2) oled.println(varVals[vPos]);
  }

  if (type == 1) {
    oled.set2X();
    oled.setCursor(0, 2);
    if (!tempVarB) oled.setInvertMode(1);
    oled.print("False");

    oled.setInvertMode(0);
    oled.print(" ");

    if (tempVarB) oled.setInvertMode(1);
    oled.print("True");
    oled.setInvertMode(0);
  } else if (type == 2) {
    oled.setInvertMode(0);
    oled.set2X();
    oled.setCursor(0, 2);
    oled.println(tempVarI);
  } else if (type == 3) {
    oled.setInvertMode(0);

    int8 cursor = countDigit(abs(tempVarF)) + 1;
    String num = String(tempVarF, FLOAT_POINTS);
    if (tempVarF < 0 || tempVarF == -0.0) cursor++;
    cursor += floatSelPos;

    if (!floatSelected) {
      oled.setCursor(0, 1);
      for (int x = 0; x < num.length(); x++) {
        if (x == cursor) oled.print("\\/");
        else oled.print("  ");
      }
    }

    oled.set2X();
    oled.setCursor(0, 2);

    for (int x = 0; x < num.length(); x++) {
      if (floatSelected) {
        if (x == cursor) oled.setInvertMode(1);
        else oled.setInvertMode(0);
      }

      oled.print(num[x]);
    }
  }
}

void showMsg(String msg) {
  oled.clear();
  oled.set2X();
  oled.setCursor(0, 1);
  oled.println(msg);
}
