#define MINI_SUMO_DIR "MINISUMO"
#define MBOT_DIR "MBOT"
#define END_CHAR '#'
#define SKIP_CHAR '*'
#define MAX_STRATS 10
#define MAX_VARIABLES 10

String strats[MAX_STRATS];
int8 stCount = 0;

int8 selectedSt = 0;

String varNames[MAX_VARIABLES];
long varVals[MAX_VARIABLES];
int8 varType[MAX_VARIABLES];
int8 vars = 0;
/*
 * Types:
 * 0 - undefined
 * 1 - boolean
 * 2 - int
 * 3 - float
 */
