#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <ctype.h>

#include "../../des_controller/src/des_mva.h"

int main(int argc, char* argv[]) {
	int coid;
	char state[5];
	currentState currState;
	int userId = 0;
	int userWeight = 0;
	pid_t serverpid;
	int isLoop = 1;

	if(argc < 2){
		printf("Not enough arguments supplied.");
		return EXIT_FAILURE;
	}

	serverpid = atoi(argv[1]);

	if ((coid = ConnectAttach(ND_LOCAL_NODE, serverpid, 1, _NTO_SIDE_CHANNEL, 0)) == -1) {
		printf("ERROR: Could not connect to controller\n");
		return EXIT_FAILURE;
	}

	while(isLoop == 1){
		printf("Enter the event type (ls = left scan, rs = right scan, ws = weight scale, lo = left open, ro = right open, lc = left closed, "
				"rc = right closed, gru = guard right unlock, grl = guard right lock, gll = guard left lock, glu = guard left unlock)\n");

		scanf(" %s", &state);

		if(strcmp(state, inMessage[0]) == 0){
			continue;
		}else if (strcmp(state, "ls") == 0 || strcmp(state, "rs") == 0) {
			printf("Enter your ID: \n");
			scanf("%d", &userId);
			currState.userId = userId;

			if (strcmp(state, "ls") == 0) {
				currState.state = LEFT_SCAN;
			} else {
				currState.state = RIGHT_SCAN;
			}
		}else if(strcmp(state, inMessage[3]) == 0){
			currState.state = LEFT_OPEN;
		}else if(strcmp(state, inMessage[4]) == 0){
			currState.state = RIGHT_OPEN;
		}else if(strcmp(state, inMessage[5]) == 0){
			currState.state = LEFT_CLOSE;
		}else if(strcmp(state, inMessage[6]) == 0){
			currState.state = RIGHT_CLOSE;
		}else if(strcmp(state, inMessage[7]) == 0){
			currState.state = GUARD_RIGHT_UNLOCK;
		}else if(strcmp(state, inMessage[8]) == 0){
			currState.state = GUARD_LEFT_UNLOCK;
		}else if(strcmp(state, inMessage[9]) == 0){
			currState.state = GUARD_RIGHT_LOCK;
		}else if(strcmp(state, inMessage[10]) == 0){
			currState.state = GUARD_LEFT_LOCK;
		}else if(strcmp(state, inMessage[11]) == 0){
			printf("Please enter your weight\n");
			scanf("%d",&userWeight);

			currState.userWeight = userWeight;
			currState.state = WEIGHT_SCALE;
		}else if(strcmp(state,inMessage[12]) == 0){
			currState.state = EXIT;
			isLoop = 0;
		}

		if (MsgSend (coid, &currState, sizeof(currState), &currState, sizeof (currState)) == -1) {
			fprintf(stderr, "Error during sending message");
			return EXIT_FAILURE;
		}
	}

	ConnectDetach(coid);
	printf("Exiting the inputs.\n");

	return EXIT_SUCCESS;
}
