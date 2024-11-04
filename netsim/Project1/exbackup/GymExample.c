#include "main.h"
#include <signal.h>
#include "Gym.h"
#include "../IP/IP.h"
#include "../Satellite/Satellite_PHY.h"
#include"../LTE_NR//LTE_NR.h"
#include"../LTE_NR/LTENR_GNBRRC.h"
#include"../LTE_NR/LTENR_PHY.h"
#include"../LTE_NR/LTENR_MAC.h"
#include"../LTE_NR/LTENR_HARQ.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define MAX_DEVICE_NAMES 1000  // max number of device_namefor saving 
#define MAX_STRING_LENGTH 2560 // max string length

#define SIZE_OF_MAP 1


void execute_sendHOM_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index )
{
	
	NETSIM_ID dest;

	dest = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);

	ptrLTENR_UERRC ueRRC;
	ptrLTENR_GNBRRC gnbRRC;
	NETSIM_ID interface_NEW;
	interface_NEW = fn_NetSim_Stack_GetInterfaceIdByName(dest, "5G_RAN");
	ueRRC = LTENR_UERRC_GET(dest, interface_NEW);
	gnbRRC = LTENR_GNBRRC_GET(ueRRC->SelectedCellID, ueRRC->SelectedCellIF);
	int HOM = gnbRRC->HandoverMargin;





	float values[SIZE_OF_MAP];
	for (uint32_t i = 0; i < SIZE_OF_MAP; i++) {
		values[i] = rand() % 100; // Random values between 0 and 99
	}


	contain_obs(info, "Throughput", values, SIZE_OF_MAP);

	return;

}

void execute_sendHOM1_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{

	NETSIM_ID dest;
	dest = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);

	ptrLTENR_UERRC ueRRC;
	ptrLTENR_GNBRRC gnbRRC;
	NETSIM_ID interface_NEW;
	interface_NEW = fn_NetSim_Stack_GetInterfaceIdByName(dest, "5G_RAN");
	ueRRC = LTENR_UERRC_GET(dest, interface_NEW);
	gnbRRC = LTENR_GNBRRC_GET(ueRRC->SelectedCellID, ueRRC->SelectedCellIF);
	int HOM = gnbRRC->HandoverMargin;


	float values[SIZE_OF_MAP];
	for (uint32_t i = 0; i < SIZE_OF_MAP; i++) {
		values[i] = rand() % 100;
	}


	contain_obs(info, "SINR", values, SIZE_OF_MAP);
	return;

}

void execute_sendHOM2_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
	NETSIM_ID dest;

	dest = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);

	ptrLTENR_UERRC ueRRC;
	ptrLTENR_GNBRRC gnbRRC;
	NETSIM_ID interface_NEW;
	interface_NEW = fn_NetSim_Stack_GetInterfaceIdByName(dest, "5G_RAN");
	ueRRC = LTENR_UERRC_GET(dest, interface_NEW);
	gnbRRC = LTENR_GNBRRC_GET(ueRRC->SelectedCellID, ueRRC->SelectedCellIF);
	int HOM = gnbRRC->HandoverMargin;


	float values[SIZE_OF_MAP];
	for (uint32_t i = 0; i < SIZE_OF_MAP; i++) {
		values[i] = rand() % 100; 
	}

	contain_reward(info, "Dummy", values, SIZE_OF_MAP);
	return;

}

void execute_GNBPRB_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{

	NETSIM_ID dest; //gNBname
	dest = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
	ptrLTENR_UERRC ueRRC;
	ptrLTENR_GNBRRC gnbRRC;

	NETSIM_ID interface_NEW;
	interface_NEW = fn_NetSim_Stack_GetInterfaceIdByName(dest, "5G_RAN");	


	gnbRRC = LTENR_GNBRRC_GET(dest, interface_NEW);
	ptrLTENR_GNBPHY phy;
	phy = LTENR_GNBPHY_GET(dest, interface_NEW);
	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(dest, interface_NEW);
	UINT CA_ID = phy->currentFrameInfo->Current_CA_ID;
	fprintf(stderr, "CA_ID : %d", CA_ID);
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];
	ptrLTENR_SCHEDULERINFO si = mac->schedulerInfo[CA_ID];
	ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
	fprintf(stderr, "ue id : %d", assocInfo->ueId);
	fprintf(stderr, "ue if : %d", assocInfo->ueIf);


	ptrLTENR_UESCHEDULERINFO sinfo = LTENR_MACScheduler_FindInfoForUE(si, assocInfo->ueId, assocInfo->ueIf, false);
	if (!sinfo) {
			fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

	}
	else {
		fprintf(stderr, "hihihihihi!");
		fprintf(stderr, " SINFO %d  ",sinfo->allocatedPRBCount);

	}

	for (NETSIM_ID r = 0; r < NETWORK->nDeviceCount; r++)
	{
		for (NETSIM_ID rin = 0; rin < DEVICE(r + 1)->nNumOfInterface; rin++)
		{
			if (!isLTE_NRInterface(r + 1, rin + 1))
				continue;

			ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(r + 1, rin + 1);
			switch (data->deviceType)
			{
			case LTENR_DEVICETYPE_UE:
				ueRRC = LTENR_UERRC_GET(r + 1, rin + 1);
				if (ueRRC->ueRRCState == UERRC_CONNECTED &&
					ueRRC->SelectedCellID == dest && ueRRC->SelectedCellIF == interface_NEW) {
					//fprintf(stderr, " name of ue : %d ", ueRRC->d);
					fprintf(stderr, " \n distance between UE %d and gNB %d : %f \n", ueRRC->d, dest , DEVICE_DISTANCE(ueRRC->d, dest));
					fprintf(stderr, " PRB utilization of gNB %d : %d ", dest, ca->PRBCount);
					//fprintf(stderr, " SINFO %d : %d ", ueRRC->d, si->PRBCount -(UINT)ceil((si->PRBCount)* (si->OH_DL)));
					fprintf(stderr, " SINFO %d : %d ", ueRRC->d, mac->schedulerInfo[CA_ID]->downlinkInfo->allocatedPRBCount);
					//fprintf(stderr, " SINFO %d : %d ", ueRRC->d, sinfo->allocatedPRBCount);


				}
				break;
			default:
				break;
			}
		}
	}

	float values[1];
	values[0] = 1;
	contain_obs(info, "Throughput", values, 1);

	return;

}


