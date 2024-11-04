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




void execute_UEData_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    NETSIM_ID ueID;
    ueID = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
    float position_x = 0.0;
    float position_y = 0.0;
    float distance = 0.0;

    NETSIM_ID ueInterfaceId;
    ueInterfaceId = fn_NetSim_Stack_GetInterfaceIdByName(ueID, "5G_RAN");

    ptrLTENR_UERRC ueRRC;
    ueRRC = LTENR_UERRC_GET(ueID, ueInterfaceId);

    NETSIM_ID gnbID = 0;

    if (ueRRC != NULL)
    {
        gnbID = ueRRC->SelectedCellID;
    }
    else
    {
        fprintf(stderr, "UE %d has no RRC data.\n", ueID);
    }

    //position_x = DEVICE_POSITION(ueID)->X;
    //position_y = DEVICE_POSITION(ueID)->Y;
    distance = DEVICE_DISTANCE(ueID, gnbID);

    fprintf(stderr, "UE %d\n", ueID);
    //fprintf(stderr, "      x:   %f\n", DEVICE_POSITION(ueID)->X);
    //fprintf(stderr, "      y:   %f\n", DEVICE_POSITION(ueID)->Y);
    fprintf(stderr, "      gNB: %f\n", (float)gnbID);
    //fprintf(stderr, "      gNB x: %f\n", DEVICE_POSITION(gnbID)->X);
    //fprintf(stderr, "      gNB y: %f\n", DEVICE_POSITION(gnbID)->Y);
    fprintf(stderr, "      dist : %f\n\n", DEVICE_DISTANCE(ueID, gnbID));
    float values[2];
    values[0] = (float)gnbID;
    values[1] = distance;


    contain_obs(info, "UEData", values, 2);

}

void execute_CellLoad_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    NETSIM_ID gnbId = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
    NETSIM_ID gnbInterfaceId;
    gnbInterfaceId = fn_NetSim_Stack_GetInterfaceIdByName(gnbId, "5G_RAN");

    ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbInterfaceId);
    ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbId, gnbInterfaceId);

    if (!phy || !mac) {
        fprintf(stderr, "Error: Unable to retrieve gNB PHY or MAC for gNB ID %d, Interface %d\n", gnbId, gnbInterfaceId);
        return;
    }

    double total_required_prbs = 0.0;
    //double data_rate_per_ue = 0.584e6;  // change if application changed/ 584kbps
    double data_rate_per_ue = 0.256e6;  // change if application changed/ 3.51Mbps / 256kbps / 512kbps

    ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
    while (assocInfo) {
        if (assocInfo->isAssociated) {
            double spectral_efficiency = assocInfo->downlinkAMCInfo[0][0]->SpectralEfficiency;
            //fprintf(stderr, "\nsp   : %f\n", spectral_efficiency);
            if (spectral_efficiency < 0.1) {
                spectral_efficiency = 0.1;
            }
            double prb_bandwidth_khz = phy->spectrumConfig->CA[0]->prbBandwidth_kHz;
            //fprintf(stderr, "prbbw: %f\n", prb_bandwidth_khz);

            double required_prbs = data_rate_per_ue / (spectral_efficiency * (prb_bandwidth_khz * 1000));
            //fprintf(stderr, "req  : %f\n", required_prbs);
            total_required_prbs += required_prbs;
        }
        assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
    }

    double total_prbs = phy->spectrumConfig->CA[0]->PRBCount;
    //fprintf(stderr, "tot  : %f\n", total_prbs);

    double cell_load = total_required_prbs / total_prbs;

 
    fprintf(stderr, "Cell Load for gNB ID %d: %f\n", gnbId, cell_load);

    float values[1];
    values[0] = (float)cell_load;

    contain_obs(info, "CellLoad_ex2", values, 1);
 
}

void execute_EdgeUE_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    NETSIM_ID gnbId = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
    NETSIM_ID gnbInterfaceId = fn_NetSim_Stack_GetInterfaceIdByName(gnbId, "5G_RAN");

    fprintf(stderr, "gNB id: %d\n", gnbId);
    fprintf(stderr, "Connected UE id: ");

    ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbInterfaceId);
    if (!phy) {
        fprintf(stderr, "Error: Unable to retrieve gNB PHY for gNB ID %d, Interface %d\n", gnbId, gnbInterfaceId);
        return;
    }

    ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
    int totalUECount = 0;
    int edgeUECount = 0;
    float distance;

    while (assocInfo) {
        if (assocInfo->isAssociated) {
            totalUECount++;
            distance = DEVICE_DISTANCE(gnbId, assocInfo->ueId);
            fprintf(stderr, "%d, ", assocInfo->ueId);
            if (distance >= 175.0) { //edge ue = 70% upper
                edgeUECount++;
            }
        }
        assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
    }

    float edgeUEPercentage = 0.0;
    if (totalUECount > 0) {
        edgeUEPercentage = ((float)edgeUECount / (float)totalUECount) * 100.0f;
    }

    fprintf(stderr, "\nTotal connected UEs: %d\n", totalUECount);
    fprintf(stderr, "Edge UEs (>175 m): %d\n", edgeUECount);
    fprintf(stderr, "Percentage of Edge UEs (>175 m): %f%%\n", edgeUEPercentage);

    float values[1];
    values[0] = (float)edgeUEPercentage;

    contain_obs(info, "EdgeUE_ex2", values, 1);

}
/* 
void apply_HOM_to_all_gNBs(ActMsg* actmsg) {

    if (actmsg->command_name == 2) {
        int actionCount = actmsg->n_values;

        if (actionCount >= 1) {
            int actionIndex = 0;

            // 네트워크에 있는 모든 gNB 장치에 대해 루프 실행
            for (NETSIM_ID i = 0; i < NETWORK->nDeviceCount; i++) {
                for (NETSIM_ID j = 0; j < NETWORK->ppstruDeviceList[i]->nNumOfInterface; j++) {
                    // LTE_NR 인터페이스가 아니거나 gNB가 아닌 장치는 건너뜀
                    if (!isLTE_NRInterface(i + 1, j + 1) || !isGNB(i + 1, j + 1)) {
                        continue;
                    }

                    // action 값이 더 이상 남아있지 않다면 break
                    if (actionIndex >= actionCount) {
                        fprintf(stderr, "No more action values available for gNB Device %d Interface %d.\n", i + 1, j + 1);
                        break;
                    }

                    // gNB ID와 인터페이스 ID 설정
                    NETSIM_ID gnbId = i + 1;
                    NETSIM_ID gnbInterfaceId = j + 1;

                    // gNB RRC 정보 가져오기
                    ptrLTENR_GNBRRC gnbRRC = LTENR_GNBRRC_GET(gnbId, gnbInterfaceId);
                    if (gnbRRC == NULL) {
                        fprintf(stderr, "No RRC data available for gNB Device %d Interface %d.\n", gnbId, gnbInterfaceId);
                        continue;
                    }

                    // 받은 action 값을 적용하여 HOM 업데이트
                    double actionValue = actmsg->values[actionIndex];

                    // 이전 HOM 값을 저장
                    int previousHOM = gnbRRC->HandoverMargin;
                    // 새로운 HOM 값 설정
                    gnbRRC->HandoverMargin = (int)actionValue;

                    // 변경된 HOM 값 출력
                    fprintf(stderr, "Set Handover Margin for gNB %d, Interface %d from %d to %d\n", gnbId, gnbInterfaceId, previousHOM, (int)gnbRRC->HandoverMargin);
                    fprintf(stderr, "Action: %f \n", actmsg->values[actionIndex]);

                    // 다음 action 값을 처리하기 위해 인덱스 증가
                    actionIndex++;
                }
            }
        }
        else {
            fprintf(stderr, "Error: Not enough values in ActMsg to perform handover margin action. Expected at least 1, got %d\n", actionCount);
        }
    }
    else {
        fprintf(stderr, "Error: apply_handover_margin_to_all_gNBs called with non-handover margin command (command_name: %d)\n", actmsg->command_name);
    }
}*/
void apply_HOM_to_all_gNBs(ActMsg* actmsg) {

    int actionCount = actmsg->n_values;

    if (actionCount >= 1) {
        int actionIndex = 0;

        for (NETSIM_ID i = 0; i < NETWORK->nDeviceCount; i++) {
            for (NETSIM_ID j = 0; j < NETWORK->ppstruDeviceList[i]->nNumOfInterface; j++) {
                if (!isLTE_NRInterface(i + 1, j + 1) || !isGNB(i + 1, j + 1)) {
                    continue;
                }
                if (actionIndex >= actionCount) {
                    fprintf(stderr, "No more action values available for gNB Device %d Interface %d.\n", i + 1, j + 1);
                    break;
                }
                NETSIM_ID gnbId = i + 1;
                NETSIM_ID gnbInterfaceId = j + 1;

                ptrLTENR_GNBRRC gnbRRC = LTENR_GNBRRC_GET(gnbId, gnbInterfaceId);
                if (gnbRRC == NULL) {
                    fprintf(stderr, "No RRC data available for gNB Device %d Interface %d.\n", gnbId, gnbInterfaceId);
                    continue;
                }

                float actionValue = (float)actmsg->values[actionIndex];

                float previousHOM = gnbRRC->HandoverMargin;

                gnbRRC->HandoverMargin = actionValue;

                fprintf(stderr, "Set Handover Margin for gNB %d, Interface %d from %f to %f\n", gnbId, gnbInterfaceId, previousHOM, gnbRRC->HandoverMargin);

                actionIndex++;
            }
        }
    }
    else {
        fprintf(stderr, "Error: Not enough values in ActMsg to perform handover margin action. Expected at least 1, got %d\n", actionCount);
    }
}
