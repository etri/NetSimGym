#include "main.h"
#include <signal.h>
#include "Gym.h"
#include "../IP/IP.h"
#include "../Satellite/Satellite_PHY.h"
#include"../LTE_NR//LTE_NR.h"
#include"../LTE_NR/LTENR_GNBRRC.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../CLIInterpretor/CLI.h"


/*NetSimGym modified start*/
#define MAX_DEVICE_NAMES 1000  // max number of device_namefor saving 
#define MAX_STRING_LENGTH 2560 // max string length
/*NetSimGym modified fin*/

#define SIZE_OF_MAP 1


void process_obs_command(int commandname, char* source){

	// obs_command for example ///////////////////////////////////////////////////////////////////////////////////
	
	switch (commandname) {
	case 1:
		strcpy(source, "RandomState");
		break;
	case 2:
		strcpy(source, "IntState");
		break;
	case 3:
		strcpy(source, "Dummy");
		break;
	case 4:
		strcpy(source, "SINR");
		break;
	case 5:
		strcpy(source, "Throughput");
		break;
	case 6:
		strcpy(source, "SumThroughput");
		break;
	case 7:
		strcpy(source, "UEData");
		break;
	case 8:
		strcpy(source, "CellLoad");
		break;
	case 9:
		strcpy(source, "EdgeUE");
		break;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	default:
		source[0] = '\0';
		break;
	}

}

void process_act_command(ActMsg* actmsg, ptrCLIENTINFO info, int j) {
	


	switch (actmsg->command_name) {

		// act_command for example ///////////////////////////////////////////////////////////////////////////

	case 1: 
		apply_random_action(actmsg);
		break;
	case 2: 
		apply_int_action(actmsg);
		break;
	case 3:
		apply_power_action_to_all_gNBs(actmsg);
		//fprintf(stderr, "                                        HOM %d : %f \n", j, actmsg->values[0]);
		break;
	case 4:
		apply_HOM_to_all_gNBs(actmsg);
		break;
		/////////////////////////////////////////////////////////////////////////////////////////////////////

	default:
		break;
	}

	// for processing extra doneinfo
	
	if (actmsg != NULL && actmsg->doneinfo == TRUE) {
		cli_stop_simulation(info);
	}
	act_msg__free_unpacked(actmsg, NULL);
}

