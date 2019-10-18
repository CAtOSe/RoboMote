#include "PF.h"
FATFS fs;
DIR dir;
FILINFO fInfo;

int8 lineLen;

int8 initSD() {
  pinMode(10, OUTPUT);
  digitalWrite(10, true); // Disable Ethernet

  return PF.begin(&fs);
}

char readChar() {
  char line[1];
  unsigned int bytesRead = 0;
  int res = PF.readFile(line, 1, &bytesRead);
  return line[0];
}


int8 loadStratList() {
  int8 res = PF.openDirectory(&dir, bot.c_str());
  if (res > 0) return res;

  fInfo.fname[0] = 1;

  stCount = 0;
  while(fInfo.fname[0] != 0 && res == 0) {

    res = PF.readDirectory(&dir, &fInfo);

    if (fInfo.fname[0] > 1) {
      strats[stCount] = fInfo.fname;
    } else break;

    stCount++;

    if (stCount == MAX_STRATS) break;
  }

  if (res > 0) {
    // Read error
    return res;
  }

  return res;
}

int8 loadStrat(int8 index) {
  String path = String(bot + "/" + strats[index]);
  int8 res = PF.open(path.c_str());
  if (res > 0) return res;

  char ch = 0;
  String num = "";
  String varName = "";
  String varValue = "";
  int8 varTyp = 2;

  int8 state = 0;
  int8 varCount = 0;
  int lineLn = 0;
  while(ch != END_CHAR) {
    ch = readChar();

    if (ch != '\n') lineLn++;

    if (ch == SKIP_CHAR) continue;
    else if (ch == ' ') state++;
    else if (ch == '\n') state = 2;

    if (state == 0) varName += ch;
    else if (state == 1) {
      varValue += ch;
      if (ch == 'T' || ch == 'F' || ch == 'f' || ch == 't') varTyp = 1;
      if (ch == '.') varTyp = 3;
    }
    else if (state == 2) {
      lineLen = lineLn;
      lineLn = 0;
      varNames[varCount] = varName;
      varType[varCount] = varTyp;

      if (varTyp == 1) {
        if (varValue.c_str()[1] == 't' || varValue.c_str()[1] == 'T') varVals[varCount] = 1;
        else varVals[varCount] = 0;
      } else if (varTyp == 2) {
        varVals[varCount] = (long) varValue.toInt();
      } else if (varTyp == 3) {
        float temp = varValue.toFloat();
        memcpy(&varVals[varCount], &temp, sizeof(temp));
      }

      varCount++;
      state = 0;
      num = "";
      varName = "";
      varValue = "";
      varTyp = 2;
    }
  }

  vars = varCount;
  return res;
}

int8 writeToFile(int8 index) {
  String path = String(bot + "/" + strats[index]);
  int8 res = PF.open(path.c_str());
  if (res > 0) return res;

  // Write every line
  String nl = String('\n');
  String sp = String(' ');
  String end = String(END_CHAR);

  unsigned int n = 0;
  for (int x = 0; x < vars; x++) {
    res = PF.writeFile(varNames[x].c_str(), strlen(varNames[x].c_str()), &n); // Write var name
    if (res > 0) return res;

    res = PF.writeFile(sp.c_str(), 1, &n); // Write space
    if (res > 0) return res;

    String value;
    if (varType[x] == 1) {
      if (varVals[x] == 1) value = "True";
      else value = "False";
    } else if (varType[x] == 2) value = String(varVals[x]);
    else if (varType[x] == 3) {
      float temp;
      memcpy(&temp, &varVals[x], sizeof(temp));
      value = String(temp, FLOAT_POINTS);
    }

    res = PF.writeFile(value.c_str(), strlen(value.c_str()), &n); // Write value
    if (res > 0) return res;

    int skips = lineLen - n - varNames[x].length() - 1;

    String skipsStr = "";
    for (int y = 0; y < skips; y++) skipsStr += SKIP_CHAR;
    res = PF.writeFile(skipsStr.c_str(), strlen(skipsStr.c_str()), &n); // Write value
    if (res > 0) return res;


    res = PF.writeFile(nl.c_str(), 1, &n); // Write new line
    if (res > 0) return res;
  }

  res = PF.writeFile(end.c_str(), 1, &n); // Write end char
  if (res > 0) return res;

  res = PF.writeFile(0,0,&n);
  return res;
}
