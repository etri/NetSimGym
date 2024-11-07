#include "main.h"
#include <signal.h>
#include "Gym.h"
#include "../IP/IP.h"
#include "../Satellite/Satellite_PHY.h"
#include"../LTE_NR//LTE_NR.h"
#include"../LTE_NR/LTENR_GNBRRC.h"
#include <stdio.h>
#include <stdlib.h>
#include "../CLIInterpretor/CLI.h"
#include <string.h>

#define MAX_DEVICE_NAMES 1000  // max number of device_namefor saving 
#define MAX_STRING_LENGTH 2560 // max string lengt

#define SIZE_OF_MAP 1

int isCommand(const char* data, int length) {
    for (int i = 0; i < length; i++) {
        if (!isprint(data[i]) && data[i] != '\0') {
            return 0;
        }
    }
    return 1;
}

void process_gym(ptrCLIENTINFO clientInfo, const char* recvbuf, int iResult) {

	PythonMsg* pythonmsg = python_msg__unpack(NULL, iResult, (const uint8_t*)recvbuf);
	if (pythonmsg == NULL) {
		fprintf(stderr, "Error unpacking OuterMessage\n");
		return;
	}

	if (pythonmsg->is_init == 0) {
		CommandNum = 0;
		Container = Container_init();
		float totaltime = 0.0;
		//float timestep;
		for (size_t i = 0; i < pythonmsg->n_details; ++i) {
			if (strcmp(pythonmsg->details[i]->type_url, "type.googleapis.com/InitMsg") == 0) {
				InitMsg* init_msg = init_msg__unpack(NULL, pythonmsg->details[i]->value.len, pythonmsg->details[i]->value.data);
				if (init_msg) {
					totaltime = init_msg->total_time;
					timestep = init_msg->time_step;
					char** device_names = NULL;
					device_names = (char**)malloc(MAX_DEVICE_NAMES * sizeof(char*));
					if (device_names == NULL) {
						fprintf(stderr, "Memory allocation failed for device_names\n");
						return;
					}
					for (size_t i = 0; i < MAX_DEVICE_NAMES; i++) {
						device_names[i] = (char*)malloc(MAX_STRING_LENGTH * sizeof(char));

						if (device_names[i] == NULL) {
							fprintf(stderr, "Memory allocation failed for device_names[%zu]\n", i);
							for (size_t j = 0; j < i; j++) {
								free(device_names[j]);
							}
							free(device_names);
							return;
						}
					}
					uint32_t totalcommandnum = init_msg->n_command_name;			
					CommandNum += init_msg->n_device_name;
					
					for (size_t i = 0; i < init_msg->n_device_name && i < MAX_DEVICE_NAMES; i++) {
						strncpy(device_names[i], init_msg->device_name[i], MAX_STRING_LENGTH - 1);
					}

					for (size_t i = 0; i < init_msg->n_device_name && i < MAX_DEVICE_NAMES; i++) {
						execute_reserve_command(clientInfo, init_msg, device_names[i],totaltime,timestep); 
					}
					init_msg__free_unpacked(init_msg, NULL);
										
					for (size_t i = 0; i < MAX_DEVICE_NAMES; i++) {
						free(device_names[i]);
					}
					free(device_names);
				}
				else {
					fprintf(stderr, "Failed to unpack InitMSG\n");
				}
			}
		}

		scheduling_pause(clientInfo, totaltime,timestep);

	}

	else {
		const char* border = "\n -------------------------------------------------------------------------------------------";
		fprintf(stderr, "%s%s\n", border, border);

		Container = Container_init();

		for (size_t i = 0; i < pythonmsg->n_details; ++i) {
			if (strcmp(pythonmsg->details[i]->type_url, "type.googleapis.com/ActMsg") == 0) {
				ActMsg* actmsg = act_msg__unpack(NULL, pythonmsg->details[i]->value.len, pythonmsg->details[i]->value.data);
				if (actmsg) {
					process_act_command(actmsg, clientInfo, i + 1);
				}
				else {
					fprintf(stderr, "Failed to unpack PythonSendMSG\n");
				}
			}
		}
		fprintf(stderr, "%s%s\n", border, border);
		cli_continue_simulation(clientInfo);
	}
	python_msg__free_unpacked(pythonmsg, NULL);

}

void SendtoPython(DictContainer* mapManager, ptrCLIENTINFO info) {
	NetSimMsg outer_netsim_send_msg = NET_SIM_MSG__INIT;
	outer_netsim_send_msg.key_num = mapManager->currentSize;

	outer_netsim_send_msg.details = malloc(mapManager->currentSize * sizeof(Google__Protobuf__Any*));

	for (uint32_t i = 0; i < mapManager->currentSize; i++) {
		BoxContainer* currentBox = &mapManager->maps[i];

		ObsMsg netsim_send_msg = OBS_MSG__INIT;

		netsim_send_msg.key_name = strdup(currentBox->key);

		netsim_send_msg.values = malloc(currentBox->SizeofMap * sizeof(float));
		for (uint32_t j = 0; j < currentBox->SizeofMap; j++) {
			netsim_send_msg.values[j] = currentBox->value[j];
		}
		netsim_send_msg.n_values = currentBox->SizeofMap;

		netsim_send_msg.is_reward = currentBox->is_reward;

		size_t msg_len = obs_msg__get_packed_size(&netsim_send_msg);
		uint8_t* msg_buf = NULL;
		msg_buf = malloc(msg_len);
		if (msg_buf == NULL) {
			fprintf(stderr, "Memory allocation failed for msg_buf\n");
			return;
		}
		obs_msg__pack(&netsim_send_msg, msg_buf);

		Google__Protobuf__Any* any_msg = NULL;
		any_msg = malloc(sizeof(Google__Protobuf__Any));
		if (any_msg == NULL) {
			fprintf(stderr, "Memory allocation failed for any_msg\n");
			free(msg_buf);
			return;
		}
		google__protobuf__any__init(any_msg);
		any_msg->type_url = strdup("type.googleapis.com/ObsMsg");
		any_msg->value.len = msg_len;
		any_msg->value.data = msg_buf;

		outer_netsim_send_msg.details[i] = any_msg;

		free(netsim_send_msg.key_name);
		free(netsim_send_msg.values);
	}
	outer_netsim_send_msg.n_details = mapManager->currentSize;

	size_t outer_msg_len = net_sim_msg__get_packed_size(&outer_netsim_send_msg);

	uint8_t* outer_msg_buf = NULL;
	outer_msg_buf = malloc(outer_msg_len);
	if (outer_msg_buf == NULL) {
		fprintf(stderr, "Memory allocation failed for outer_msg_buf\n");
		return;
	}

	net_sim_msg__pack(&outer_netsim_send_msg, outer_msg_buf);
	send_to_socket(info, outer_msg_buf, outer_msg_len);
	

	//PrintOuterMessage(&outer_netsim_send_msg);

	for (uint32_t i = 0; i < mapManager->currentSize; i++) {
		free(outer_netsim_send_msg.details[i]->value.data);
		free(outer_netsim_send_msg.details[i]->type_url);
		free(outer_netsim_send_msg.details[i]);
	}
	free(outer_netsim_send_msg.details);
	free(outer_msg_buf);

}

int Get_port_num(char* title) {

	char* newtitle = title + 6;

	int port_num = atoi(newtitle);

	return port_num;

}
