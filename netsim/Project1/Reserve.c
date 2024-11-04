#include <signal.h>
#include "Gym.h"
#include "../CLIInterpretor/CLI.h"
#include "../IP/IP.h"
#include "../Satellite/Satellite_PHY.h"
#include"../LTE_NR//LTE_NR.h"
#include"../LTE_NR/LTENR_GNBRRC.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




bool validate_reserve_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    if (command->length - index < 2)
    {
        send_message(info, "USAGE: reserve deviceName\n");
        return false;
    }

    if (_stricmp(command->commands[index], "reserve"))
        return false;

    return true;
}



void execute_reserve_command(ptrCLIENTINFO info, InitMsg* initmsg, const char* devicename, float totaltime, float timestep)
{
    for (int i = 0; i < initmsg->n_command_name; i++) {
        int commandname = initmsg->command_name[i];
    
        char source[100];

        process_obs_command(commandname, source);

        ptrCOMMANDARRAY command = create_command_array(source, devicename); 

  
        schedule_events(command, info, totaltime, timestep); 
        commandname = 0;
    }
}


void scheduling_pause(ptrCLIENTINFO info, float totaltime, float timestep)
{

	double timeschedule = 1;
	while (1) {
		timeschedule += timestep * 1000000; // time step(s) * 1000000
		timeschedule = timeschedule / 1000000;
		cli_pause_simulation_at(info, timeschedule);
		timeschedule = timeschedule * 1000000;
		if (timeschedule > totaltime* 1000000)
			break;
	}
}

ptrCOMMANDARRAY create_command_array(const char* source, const char* devicename)
{
    size_t source_length = strlen(source);
    size_t devicename_length = strlen(devicename);
    size_t total_length = source_length + 1 + devicename_length; 

    // Allocate memory for the command string
    char* commandstring = NULL;
    commandstring = (char*)malloc((total_length + 1) * sizeof(char)); 
    if (commandstring == NULL) {
        fprintf(stderr, "Memory allocation failed for commandstring\n");
        return;
    }
    snprintf(commandstring, total_length + 1, "%s %s", source, devicename); 


    fprintf(stderr, "stirng: %s ", commandstring);
    // Allocate memory for the COMMANDARRAY structure
    ptrCOMMANDARRAY command = (ptrCOMMANDARRAY)malloc(sizeof(COMMANDARRAY));
    command->originalCommand = commandstring;
    command->commands = (char**)malloc(2 * sizeof(char*));

    // Allocate memory for each command part and copy the strings
    command->commands[0] = (char*)malloc((source_length + 1) * sizeof(char));
    command->commands[1] = (char*)malloc((devicename_length + 1) * sizeof(char));
    strncpy(command->commands[0], source, source_length);
    command->commands[0][source_length] = '\0';
    strncpy(command->commands[1], devicename, devicename_length);
    command->commands[1][devicename_length] = '\0';
    command->length = 2; 

    return command;
}

void schedule_events(ptrCOMMANDARRAY command, ptrCLIENTINFO info, float totaltime, float timestep)
{   
    for (double time = timestep; time < totaltime; time += timestep) {
        NetSim_EVENTDETAILS pevent;
        memset(&pevent, 0, sizeof pevent);
        pevent.dEventTime = time * SECOND;
        pevent.nEventType = TIMER_EVENT;
        pevent.nProtocolId = PROTOCOL_CLI;
        pevent.nSubEventType = SUBEVENT_EXECUTECOMMAND;
        pevent.szOtherDetails = FORM_CLI_HANDLE(command, info);
        fnpAddEvent(&pevent);
    }
}
