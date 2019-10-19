#define STRAT_NUM "STRAT"
#define END_CHAR '#'
#define SKIP_CHAR '*'

String strats[MAX_STRATS];
int8 stCount = 0;

int8 selectedSt = 0;
int8 stNumber = 0;

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
