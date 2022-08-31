#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "des_mva.h"

//function prototypes
void *initial_state();
void *left_scan();
void *right_scan();
void *left_open();
void *right_open();
void *left_close();
void *right_close();
void *guard_left_unlock();
void *guard_left_lock();
void *guard_right_unlock();
void *guard_right_lock();
void *weight();
void *exit_function();

typedef void *(*StateFunction)();
StateFunction state = initial_state;
currentState current;
int coid;
response res;

int main(int argc, char* argv[]) {
	if(argc < 2){
		printf("Not enough arguments supplied.");
		return -1;
	}

	pid_t serverpid = atoi(argv[1]);
	int rcvid;
	int chid;
	chid = ChannelCreate(0);
	if(chid == -1){
		perror("Failed to create the channel.");
		return EXIT_FAILURE;
	}

	printf("The controller is running as process_id %d\n", getpid());
	coid = ConnectAttach(ND_LOCAL_NODE, serverpid, 1, _NTO_SIDE_CHANNEL, 0);
	if(coid == -1){
		fprintf(stderr, "Could not connect attach");
		return EXIT_FAILURE;
	}

	while(1){
		//server
		rcvid = MsgReceive (chid, &current, sizeof (current), NULL);
		state = (StateFunction)(*state)();

		//server
		MsgReply(rcvid, EOK, &current, sizeof(current));
		if(state == exit_function){
			(*exit_function)();
			break;
		}
	}

	// Disconnect from the channel
	ConnectDetach(coid);

	// Destroy the channel
	ChannelDestroy(chid);

	printf("Exiting the controller.\n");
	return EXIT_SUCCESS;
}


void *initial_state(){
	if (current.state == EXIT){
		state = exit_function;
	} else if(current.state == LEFT_SCAN){
		state = left_scan;
		current.direction = LEFT_SCAN;
	} else if (current.state == RIGHT_SCAN){
		state = right_scan;
		current.direction = RIGHT_SCAN;
	} else {
		state = initial_state;
		return state;
	}

	state = (StateFunction)(*state)();
	return state;
}

void *left_scan(){
	if(current.state == EXIT){
		state = exit_function;
	} else if (current.state != LEFT_SCAN){
		state = left_scan;
	} else {
		res.resCode = 1;
		res.currState = current;

		if(MsgSend(coid, &res, sizeof(res), &res, sizeof(res) == -1)){
			fprintf(stderr, "Unable to send message.");
			perror(NULL);
			exit(EXIT_FAILURE);
		}

		state = guard_left_unlock;
	}
	return state;
}

void *right_scan(){
	if(current.state == EXIT){
		state = exit_function;
	} else if (current.state != RIGHT_SCAN){
		state = right_scan;
	} else {
		res.resCode = 2;
		res.currState = current;

		if(MsgSend(coid, &res, sizeof(res), &res, sizeof(res) == -1)){
			fprintf(stderr, "Unable to send message.");
			exit( EXIT_FAILURE);
		}

		state = guard_right_unlock;
	}
	return state;
}

void *left_open(){
	if(current.state == EXIT){
		state = exit_function;
	} else if (current.state != LEFT_OPEN){
		state = left_open;
	} else {
		res.resCode = 3;
		res.currState = current;

		if(MsgSend(coid, &res, sizeof(res), &res, sizeof(res) == -1)){
			fprintf(stderr, "Unable to send message.");
			exit( EXIT_FAILURE);
		}

		if(current.direction == LEFT_SCAN){
			state = weight;
		} else if (current.direction == RIGHT_SCAN){
			state = left_close;
		} else{
			state = guard_right_unlock;
		}
	}
	return state;
}

void *right_open(){
	if(current.state == EXIT){
		state = exit_function;
	} else if (current.state != RIGHT_OPEN){
		state = right_open;
	} else {
		res.resCode = 4;
		res.currState = current;

		if (MsgSend (coid, &res, sizeof(res), &res, sizeof (res)) == -1) {
			fprintf (stderr, "Error during MsgSend\n");
			exit( EXIT_FAILURE);
		}
		if(current.direction == LEFT_SCAN){
			state = right_close;
		}else if(current.direction == RIGHT_SCAN){
			state = weight;
		}
	}
	return state;
}

void *left_close(){
	if(current.state == EXIT){
		state = exit_function;
	}else if(current.state != LEFT_CLOSE){
		state = left_close;
	}else{
		res.resCode = 5;
		res.currState = current;

		if (MsgSend (coid, &res, sizeof(res), &res, sizeof (res)) == -1) {
			fprintf (stderr, "Error during MsgSend\n");
			exit( EXIT_FAILURE);
		}
		state = guard_left_lock;
	}
	return state;
}


void *right_close(){
	if(current.state == EXIT){
		state = exit_function;
	}else if(current.state != RIGHT_CLOSE){
		state = right_close;
	}else{
		res.resCode = 6;
		res.currState = current;

		if (MsgSend (coid, &res, sizeof(res), &res, sizeof (res)) == -1) {
			fprintf (stderr, "Error during MsgSend\n");
			exit( EXIT_FAILURE);
		}
		state = guard_right_lock;
	}
	return state;
}


void *guard_left_unlock(){
	if(current.state == EXIT){
		state = exit_function;
	}else if(current.state != GUARD_LEFT_UNLOCK){
		state = guard_left_unlock;
	}else{
		res.resCode = 8;
		res.currState = current;

		if (MsgSend (coid, &res, sizeof(res), &res, sizeof (res)) == -1) {
			fprintf (stderr, "Error during MsgSend\n");
			exit( EXIT_FAILURE);
		}
		state = left_open;
	}
	return state;
}


void *guard_left_lock(){
	if(current.state == EXIT){
		state = exit_function;
	}else if(current.state != GUARD_LEFT_LOCK){
		state = guard_left_lock;
	}else{
		res.resCode = 10;
		res.currState = current;

		if (MsgSend (coid, &res, sizeof(res), &res, sizeof (res)) == -1) {
			fprintf (stderr, "Error during MsgSend\n");
			exit( EXIT_FAILURE);
		}

		if(current.direction == LEFT_SCAN){
			state = guard_right_unlock;
		}else if(current.direction == RIGHT_SCAN){
			state = initial_state;
			printf("Waiting for person....\n");
		}
	}
	return state;
}


void *guard_right_unlock(){
	if(current.state == EXIT){
		state = exit_function;
	}else if(current.state != GUARD_RIGHT_UNLOCK){
		state = guard_right_unlock;
	}else{
		res.resCode = 7;
		res.currState = current;

		if (MsgSend (coid, &res, sizeof(res), &res, sizeof (res)) == -1) {
			fprintf (stderr, "Error during MsgSend\n");
			exit( EXIT_FAILURE);
		}
		state = right_open;
	}
	return state;
}

void *guard_right_lock(){
	if(current.state == EXIT){
		state = exit_function;
	}else if(current.state != GUARD_RIGHT_LOCK){
		state = guard_right_lock;
	}else{
		res.resCode = 9;
		res.currState = current;

		if (MsgSend (coid, &res, sizeof(res), &res, sizeof (res)) == -1) {
			fprintf (stderr, "Error during MsgSend\n");
			exit( EXIT_FAILURE);
		}
		if(current.direction == LEFT_SCAN){
			state = initial_state;
			printf("Waiting for person....\n");
		}else if(current.direction == RIGHT_SCAN){
			state = guard_left_unlock;
		}
	}
	return state;
}

void *weight(){
	if(current.state == EXIT){
		state = exit_function;
	} else if (current.state != WEIGHT_SCALE){
		state = weight;
	}else{
		res.resCode = 11;
		res.currState = current;

		if (MsgSend (coid, &res, sizeof(res), &res, sizeof (res)) == -1) {
			fprintf (stderr, "Error during MsgSend\n");
			exit( EXIT_FAILURE);
		}
		if(current.direction == LEFT_SCAN){
			state = left_close;
		}else if(current.direction == RIGHT_SCAN){
			state = right_close;
		}
	}
	return state;
}


void *exit_function(){
	res.resCode = 12;
	res.currState = current;

	if (MsgSend (coid, &res, sizeof(res), &res, sizeof (res)) == -1) {
		fprintf (stderr, "Error during MsgSend\n");
		exit( EXIT_FAILURE);
	}

	return exit_function;
}

