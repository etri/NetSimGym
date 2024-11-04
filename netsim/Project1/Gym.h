#pragma once

#include "NetSimProto.pb-c.h"
#include "main.h"
#include "../CLIInterpretor/CLI.h"



struct CLIENTINFO;
//typedef struct CLIENTINFO* ptrCLIENTINFO;
struct COMMANDARRAY;
//typedef struct COMMANDARRAY* ptrCOMMANDARRAY;
typedef unsigned int NETSIM_ID;

#pragma comment(lib,"libCLI.lib")
#pragma comment(lib,"CLIInterpretor.lib")


#ifndef NETSIM_ID
#define NETSIM_ID UINT
#endif


// Reserve.c
__declspec(dllexport) bool validate_reserve_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index); //kihoon
__declspec(dllexport) void execute_reserve_command(ptrCLIENTINFO info, InitMsg* initmsg, const char* devicename, float totaltime, float timestep);
void scheduling_pause(ptrCLIENTINFO info, float totaltime, float timestep);
ptrCOMMANDARRAY create_command_array(const char* source, const char* devicename);
void schedule_events(ptrCOMMANDARRAY command, ptrCLIENTINFO info, float totaltime, float timestep);

#define MAP_MAX_KEY_LEN		1000
#define MAP_MAX_DATA_LEN	10000
#define AgentNum 4

// Container.c

__declspec(dllimport) CLIHANDLE FORM_CLI_HANDLE(ptrCOMMANDARRAY cmd, ptrCLIENTINFO info);
__declspec(dllimport) void cli_pause_simulation_at(ptrCLIENTINFO info, double time);
__declspec(dllimport) CLIHANDLE FORM_CLI_HANDLE(ptrCOMMANDARRAY cmd, ptrCLIENTINFO info);



//__declspec(dllexport) void execute_GNBPRB_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
//__declspec(dllexport) void execute_sendHOM2_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
//__declspec(dllexport) void execute_sendHOM1_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
//__declspec(dllexport) void execute_sendHOM_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);

// GymExBasic.c
__declspec(dllexport) void execute_RandomState_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
__declspec(dllexport) void execute_IntState_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
__declspec(dllexport) void execute_Dummy_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
__declspec(dllexport) void apply_random_action(ActMsg* actmsg);
__declspec(dllexport) void apply_int_action(ActMsg* actmsg);

// GymEx1.c
__declspec(dllexport) void execute_SINR_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
__declspec(dllexport) void execute_Throughput_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
__declspec(dllexport) void execute_SumThroughput_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
__declspec(dllexport) void apply_power_action_to_all_gNBs(ActMsg* actmsg);

// GymEx2.c
__declspec(dllexport) void execute_UEData_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
__declspec(dllexport) void execute_CellLoad_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
__declspec(dllexport) void execute_EdgeUE_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);
__declspec(dllexport) void apply_HOM_to_all_gNBs(ActMsg* actmsg);


typedef struct
{
	char key[MAP_MAX_KEY_LEN + 1];
	float value[MAP_MAX_DATA_LEN]; 
	uint32_t SizeofMap; 
	bool is_reward; 
} BoxContainer;

typedef struct
{
	BoxContainer* maps;
	uint32_t currentSize;
	uint32_t maxSize;
} DictContainer;

extern DictContainer* Container;
uint32_t CommandCounter;
uint32_t CommandNum;
DictContainer* Container_init();
void Box_insert_obs(DictContainer* mapManager, const char* key, float value_array[], uint32_t SizeofMap);
void Box_insert_reward(DictContainer* mapManager, const char* key, float value_array[], uint32_t SizeofMap);
void contain_reward(ptrCLIENTINFO info, const char* rewardName, float* values, size_t size);
void contain_obs(ptrCLIENTINFO info, const char* rewardName, float* values, size_t size);
extern const char* DeviceName;

// PythonInteraction.c
void PrintOuterMessage(const NetSimMsg* msg);
void SendtoPython(DictContainer* mapManager, ptrCLIENTINFO info);
__declspec(dllexport ) void process_gym(ptrCLIENTINFO clientInfo, const char* recvbuf, int iResult);
void process_act_command(ActMsg* actmsg, ptrCLIENTINFO info , int j);
void process_obs_command(int commandname, char* source);
__declspec(dllexport) int isCommand(const char* data, int length);


int Agentcount;
float timestep;

__declspec(dllexport) int Get_port_num (char* title);
//static bool validate_GNBPRB_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index);//kihoon

/*NetSimGym modified fin*/
int agent_port_num;