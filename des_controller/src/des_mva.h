#ifndef DES_MVA_H_
#define DES_MVA_H_

#define NAME_ATTACH_DISPLAY "des_display"
#define NAME_ATTACH_CONTROLLER "des_controller"

//State transition
#define NUM_STATES 13
typedef enum {
	READY = 0,
	LEFT_SCAN = 1,
	RIGHT_SCAN = 2,
	LEFT_OPEN = 3,
	RIGHT_OPEN = 4,
	LEFT_CLOSE = 5,
	RIGHT_CLOSE = 6,
	GUARD_RIGHT_UNLOCK = 7,
	GUARD_LEFT_UNLOCK = 8,
	GUARD_RIGHT_LOCK = 9,
	GUARD_LEFT_LOCK = 10,
	WEIGHT_SCALE = 11,
	EXIT = 12,
} State;

#define NUM_INPUTS 13
const char *inMessage[NUM_INPUTS] = {
	"",
	"ls",
	"rs",
	"lo",
	"ro",
	"lc",
	"rc",
	"gru",
	"glu",
	"grl",
	"gll",
	"ws",
	"exit"
};

#define NUM_OUTPUTS 13
const char *outMsg[NUM_OUTPUTS] = {
	"",
	"Left scan ID, ID = ",
	"Right scan ID, ID = ",
	"Left door open",
	"Right door open",
	"Left door close (automatically)",
	"Right door close (automatically)",
	"Right door unlocked by Guard",
	"Left door unlocked by Guard",
	"Right door locked by Guard",
	"Left door locked by Guard",
	"Person weight, Weight: ",
	"Exiting..."
};

struct currentState {
	int state;
	int direction;
	int userId;
	int userWeight;
} typedef currentState;

struct response {
	currentState currState;
	int resCode;
} typedef response;

#endif
