// =======================================
//   Configurations file for RoboMote
// =======================================

// Address of your remote. Set this to a value no one else has.
static const uint16_t necAddr = 0xABC;

#define MBOT_START 1
#define MBOT_STOP 2

#define PRESS_DELAY 300 // Delay before another click
#define FLOAT_POINTS 6 // Floating point precision
#define ACCEL_TIME_OUT 100 // How long until input acceleration times out?
#define ACCEL_FACT_DIV 140 // The higher this, the higher acceleration.
#define OK_DELAY 600 // OK Long press time

#define MINI_SUMO_DIR "MINISUMO" // Name of Mini Sumo strategies directory
#define MBOT_DIR "MBOT" // Name of mBot strategies directory

// Number of maximum stratgies to have space for. Don't set too high or you'll have memory problems.
#define MAX_STRATS 10

// Number of maximum variables to have space for. Don't set too high or you'll have memory problems.
#define MAX_VARIABLES 10

// Number of maximum menu entries to have space for. Don't set too high or you'll have memory problems.
// Should be no less than MAX_VARIABLES + 2 or MAX_STRATS + 2, whichever is higher.
static const int8 MAX_MENU = 12;

// How many characters can you fit at 1X?
static const int8 OLED_WIDTH_SMALL = 21;
