#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>

#include "../../des_controller/src/des_mva.h"

int main(void) {
	response s_response;
	int rcvid;
	int chid;
//	int serverLoop = 1;

	chid = ChannelCreate(0);
	if (chid == -1)
	{
		perror("failed to create the channel.");
		exit(EXIT_FAILURE);
	}

	printf("The display is running as process_id %d\n", getpid());

	while(1){
		rcvid = MsgReceive(chid, &s_response, sizeof(s_response),NULL);
		MsgReply(rcvid, EOK, &s_response, sizeof(s_response));

		if(s_response.currState.state == LEFT_SCAN || s_response.currState.state == RIGHT_SCAN){
			printf("%s %d \n", outMsg[s_response.resCode], s_response.currState.userId);
		}
		else if(s_response.currState.state == WEIGHT_SCALE){
			printf("%s %d \n", outMsg[s_response.resCode], s_response.currState.userWeight);
		}
		else{
   			printf("%s\n",outMsg[s_response.resCode]);
		}

		if(s_response.currState.state == EXIT){
			break;
		}
	}

    ChannelDestroy(chid);
    printf("Exiting the display. \n");
	return EXIT_SUCCESS;
}
