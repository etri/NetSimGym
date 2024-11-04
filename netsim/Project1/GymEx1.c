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




void execute_SINR_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    NETSIM_ID ueId;
    ueId = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);

    ptrLTENR_UERRC ueRRC;
    NETSIM_ID ueInterfaceId;

    //ueInterfaceId = fn_NetSim_Stack_GetInterfaceIdByName(ueId, "5G_RAN");
    ueInterfaceId = fn_NetSim_Stack_GetInterfaceIdByName(ueId, "LTE");
    ueRRC = LTENR_UERRC_GET(ueId, ueInterfaceId);

    if (ueRRC == NULL)
    {
        fprintf(stderr, "UE %d has no RRC data.\n", ueId);
        return;
    }

    ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(ueRRC->SelectedCellID, ueRRC->SelectedCellIF);
    if (phy == NULL)
    {
        fprintf(stderr, "No PHY data available for UE %d connected to eNB %d.\n", ueId, ueRRC->SelectedCellID);
        return;
    }

    ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;

    float sinr_value = 0.0;
    bool sinr_found = false;

    while (assocInfo)
    {
        if (assocInfo->ueId == ueId && assocInfo->isAssociated)
        {
            fprintf(stderr, "SINR values for UE %d connected to eNB %d:\n", ueId, ueRRC->SelectedCellID);

            if (phy->ca_count > 0)
            {
                UINT layerCount = assocInfo->propagationInfo[0]->downlink.layerCount;
                if (layerCount > 0)
                {
                    sinr_value = assocInfo->propagationInfo[0]->downlink.SINR_db[0];
                    fprintf(stderr, "SINR = %f dB\n", sinr_value);
                    sinr_found = true;
                    break;
                }
            }
        }
        assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
    }

    if (sinr_found)
    {
        float values[SIZE_OF_MAP];
        for (uint32_t i = 0; i < SIZE_OF_MAP; i++) {
            values[i] = sinr_value;
        }
        contain_obs(info, "SINR_ex1", values, 1);
    }
    else
    {
        fprintf(stderr, "No SINR values collected for UE %d.\n", ueId);
    }
}



void execute_Throughput_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    NETSIM_ID ueId;
    ueId = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
    ptrLTENR_UERRC ueRRC;
    NETSIM_ID ueInterfaceId;

    //ueInterfaceId = fn_NetSim_Stack_GetInterfaceIdByName(ueId, "5G_RAN");
    ueInterfaceId = fn_NetSim_Stack_GetInterfaceIdByName(ueId, "LTE");
    ueRRC = LTENR_UERRC_GET(ueId, ueInterfaceId);

    if (ueRRC == NULL)
    {
        fprintf(stderr, "UE %d has no RRC data.\n", ueId);
        return;
    }

    ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(ueRRC->SelectedCellID, ueRRC->SelectedCellIF);
    if (phy == NULL)
    {
        fprintf(stderr, "No PHY data available for UE %d connected to eNB %d.\n", ueId, ueRRC->SelectedCellID);
        return;
    }

    ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;

    float throughput_value = 0.0;
    bool throughput_found = false;

    while (assocInfo)
    {
        if (assocInfo->ueId == ueId && assocInfo->isAssociated)
        {
            fprintf(stderr, "Throughput values for UE %d connected to eNB %d:\n", ueId, ueRRC->SelectedCellID);

            if (phy->ca_count > 0)
            {
                UINT layerCount = assocInfo->propagationInfo[0]->downlink.layerCount;
                ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(ueId, ueInterfaceId);
                double frameDuration = phy->spectrumConfig->frameDuration;

                if (layerCount > 0)
                {
                    throughput_value = (pdcp->TotalPDSCHBytes * 8) / (timestep*1000000);
                    fprintf(stderr, "Throughput = %f Mbps\n", throughput_value);

                    throughput_found = true;
                    pdcp->TotalPDSCHBytes = 0;
                    break;
                }


            }

            break;
        }
        assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
    }

    if (throughput_found)
    {
        float values[SIZE_OF_MAP];
        for (uint32_t i = 0; i < SIZE_OF_MAP; i++) {
            values[i] = throughput_value; 
        }
        contain_obs(info, "Throughput_ex1", values, 1);
    }
    else
    {
        fprintf(stderr, "No Throughput values collected for UE %d.\n", ueId);
    }
}

void execute_SumThroughput_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    NETSIM_ID scenario1[] = { 4, 5, 6 };
    NETSIM_ID scenario2[] = { 13, 14, 15 };
    NETSIM_ID scenario3[] = { 22, 23, 24 };
    NETSIM_ID scenario4[] = { 31, 32, 33 };
    NETSIM_ID scenario5[] = { 40, 41, 42 };

    float scenario1_throughput = 0.0;
    float scenario2_throughput = 0.0;
    float scenario3_throughput = 0.0;
    float scenario4_throughput = 0.0;
    float scenario5_throughput = 0.0;

    bool throughput_found = false;

    //sum throughput for scenario[1] eNBs
    for (int i = 0; i < sizeof(scenario1) / sizeof(scenario1[0]); i++)
    {
        NETSIM_ID gnbId = scenario1[i];
        for (NETSIM_ID j = 0; j < NETWORK->ppstruDeviceList[gnbId - 1]->nNumOfInterface; j++)
        {
            if (!isLTE_NRInterface(gnbId, j + 1) || !isGNB(gnbId, j + 1))
                continue;

            ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, j + 1);
            if (phy == NULL)
                continue;

            ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
            while (assocInfo)
            {
                if (assocInfo->isAssociated)
                {
                    ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(gnbId, j + 1);
                    double frameDuration = phy->spectrumConfig->frameDuration;

                    if (phy->ca_count > 0 && assocInfo->propagationInfo[0]->downlink.layerCount > 0)
                    {
                        double throughput_value = (pdcp->TotalPDSCHBytes * 8) / (timestep*1000000);
                        scenario1_throughput += throughput_value;
                        throughput_found = true;
                        pdcp->TotalPDSCHBytes = 0;
                    }
                }
                assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
            }
        }
    }

    // sum throughput for scenario[2] if exist
    if (fn_NetSim_Stack_GetDeviceId_asName("Macro cell eNB Omni Ant_13") != NULL)
    {
        for (int i = 0; i < sizeof(scenario2) / sizeof(scenario2[0]); i++)
        {
            NETSIM_ID gnbId = scenario2[i];
            for (NETSIM_ID j = 0; j < NETWORK->ppstruDeviceList[gnbId - 1]->nNumOfInterface; j++)
            {
                if (!isLTE_NRInterface(gnbId, j + 1) || !isGNB(gnbId, j + 1))
                    continue;

                ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, j + 1);
                if (phy == NULL)
                    continue;

                ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
                while (assocInfo)
                {
                    if (assocInfo->isAssociated)
                    {
                        ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(gnbId, j + 1);
                        double frameDuration = phy->spectrumConfig->frameDuration;

                        if (phy->ca_count > 0 && assocInfo->propagationInfo[0]->downlink.layerCount > 0)
                        {
                            double throughput_value = (pdcp->TotalPDSCHBytes * 8) / (timestep*1000000);
                            scenario2_throughput += throughput_value;
                            throughput_found = true;
                            pdcp->TotalPDSCHBytes = 0;
                        }
                    }
                    assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
                }
            }
        }
    }

    // sum throughput for scenario[3] if exist
    if (fn_NetSim_Stack_GetDeviceId_asName("Macro cell eNB Omni Ant_22") != NULL)
    {
        for (int i = 0; i < sizeof(scenario3) / sizeof(scenario3[0]); i++)
        {
            NETSIM_ID gnbId = scenario3[i];
            for (NETSIM_ID j = 0; j < NETWORK->ppstruDeviceList[gnbId - 1]->nNumOfInterface; j++)
            {
                if (!isLTE_NRInterface(gnbId, j + 1) || !isGNB(gnbId, j + 1))
                    continue;

                ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, j + 1);
                if (phy == NULL)
                    continue;

                ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
                while (assocInfo)
                {
                    if (assocInfo->isAssociated)
                    {
                        ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(gnbId, j + 1);
                        double frameDuration = phy->spectrumConfig->frameDuration;

                        if (phy->ca_count > 0 && assocInfo->propagationInfo[0]->downlink.layerCount > 0)
                        {
                            double throughput_value = (pdcp->TotalPDSCHBytes * 8) / (timestep*1000000);
                            scenario3_throughput += throughput_value;
                            throughput_found = true;
                            pdcp->TotalPDSCHBytes = 0;
                        }
                    }
                    assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
                }
            }
        }
    }

    // sum throughput for scenario[4] if exist
    if (fn_NetSim_Stack_GetDeviceId_asName("Macro cell eNB Omni Ant_31") != NULL)
    {
        for (int i = 0; i < sizeof(scenario4) / sizeof(scenario4[0]); i++)
        {
            NETSIM_ID gnbId = scenario4[i];
            for (NETSIM_ID j = 0; j < NETWORK->ppstruDeviceList[gnbId - 1]->nNumOfInterface; j++)
            {
                if (!isLTE_NRInterface(gnbId, j + 1) || !isGNB(gnbId, j + 1))
                    continue;

                ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, j + 1);
                if (phy == NULL)
                    continue;

                ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
                while (assocInfo)
                {
                    if (assocInfo->isAssociated)
                    {
                        ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(gnbId, j + 1);
                        double frameDuration = phy->spectrumConfig->frameDuration;

                        if (phy->ca_count > 0 && assocInfo->propagationInfo[0]->downlink.layerCount > 0)
                        {
                            double throughput_value = (pdcp->TotalPDSCHBytes * 8) / (timestep*1000000);
                            scenario4_throughput += throughput_value;
                            throughput_found = true;
                            pdcp->TotalPDSCHBytes = 0;
                        }
                    }
                    assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
                }
            }
        }
    }

    // sum throughput for scenario[5] if exist
    if (fn_NetSim_Stack_GetDeviceId_asName("Macro cell eNB Omni Ant_40") != NULL)
    {
        for (int i = 0; i < sizeof(scenario5) / sizeof(scenario5[0]); i++)
        {
            NETSIM_ID gnbId = scenario5[i];
            for (NETSIM_ID j = 0; j < NETWORK->ppstruDeviceList[gnbId - 1]->nNumOfInterface; j++)
            {
                if (!isLTE_NRInterface(gnbId, j + 1) || !isGNB(gnbId, j + 1))
                    continue;

                ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, j + 1);
                if (phy == NULL)
                    continue;

                ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
                while (assocInfo)
                {
                    if (assocInfo->isAssociated)
                    {
                        ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(gnbId, j + 1);
                        double frameDuration = phy->spectrumConfig->frameDuration;

                        if (phy->ca_count > 0 && assocInfo->propagationInfo[0]->downlink.layerCount > 0)
                        {
                            double throughput_value = (pdcp->TotalPDSCHBytes * 8) / (timestep*1000000);
                            scenario5_throughput += throughput_value;
                            throughput_found = true;
                            pdcp->TotalPDSCHBytes = 0;
                        }
                    }
                    assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
                }
            }
        }
    }

    // packing values
    if (scenario5_throughput > 0)
    {
        fprintf(stderr, "Scenario 5: %f, %f, %f, %f, %f\n", scenario1_throughput, scenario2_throughput, scenario3_throughput, scenario4_throughput, scenario5_throughput);
        float values[5] = { scenario1_throughput, scenario2_throughput, scenario3_throughput, scenario4_throughput, scenario5_throughput };
        contain_reward(info, "Sum_ex1", values, 5);
    }
    else if (scenario4_throughput > 0)
    {
        fprintf(stderr, "Scenario 4: %f, %f, %f, %f\n", scenario1_throughput, scenario2_throughput, scenario3_throughput, scenario4_throughput);
        float values[4] = { scenario1_throughput, scenario2_throughput, scenario3_throughput, scenario4_throughput };
        contain_reward(info, "Sum_ex1", values, 4);
    }
    else if (scenario3_throughput > 0)
    {
        fprintf(stderr, "Scenario 3: %f, %f, %f\n", scenario1_throughput, scenario2_throughput, scenario3_throughput);
        float values[3] = { scenario1_throughput, scenario2_throughput, scenario3_throughput };
        contain_reward(info, "Sum_ex1", values, 3);
    }
    else if (scenario2_throughput > 0)
    {
        fprintf(stderr, "Scenario 2: %f, %f\n", scenario1_throughput, scenario2_throughput);
        float values[2] = { scenario1_throughput, scenario2_throughput };
        contain_reward(info, "Sum_ex1", values, 2);
    }
    else
    {
        fprintf(stderr, "Scenario 1: %f\n", scenario1_throughput);
        float values[1] = { scenario1_throughput };
        contain_reward(info, "Sum_ex1", values, 1);
    }
}

/*
void execute_SumThroughput_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    float sum_throughput = 0.0;
    bool throughput_found = false;

    for (NETSIM_ID i = 0; i < NETWORK->nDeviceCount; i++)
    {
        for (NETSIM_ID j = 0; j < NETWORK->ppstruDeviceList[i]->nNumOfInterface; j++)
        {
            if (!isLTE_NRInterface(i + 1, j + 1) || !isGNB(i + 1, j + 1))
                continue;

            NETSIM_ID gnbId = i + 1;
            NETSIM_ID gnbInterfaceId = j + 1;

            ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbInterfaceId);
            if (phy == NULL)
                continue;

            ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;

            while (assocInfo)
            {
                if (assocInfo->isAssociated)
                {
                    ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(gnbId, gnbInterfaceId);
                    double frameDuration = phy->spectrumConfig->frameDuration;

                    if (phy->ca_count > 0 && assocInfo->propagationInfo[0]->downlink.layerCount > 0)
                    {
                        double throughput_value = (pdcp->TotalPDSCHBytes * 8) / (timestep*1000000);
                        sum_throughput += throughput_value;

                        throughput_found = true;
                        pdcp->TotalPDSCHBytes = 0;
                    }
                }

                assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
            }
        }
    }

    if (throughput_found)
    {
        fprintf(stderr, "Total Sum Throughput for all gNBs: %f Mbps\n", sum_throughput);
        float values[SIZE_OF_MAP];
        for (uint32_t i = 0; i < SIZE_OF_MAP; i++) {
            values[i] = sum_throughput; // Random values between 0 and 99
        }
        contain_reward(info, "SumThroughput", values, 1);
    }
    else
    {
        fprintf(stderr, "No Throughput values found for any gNB.\n");
    }
}*/


void apply_power_action_to_all_gNBs(ActMsg* actmsg) {

    int actionCount = actmsg->n_values;

    if (actionCount >= 1) {
        int actionIndex = 0;

        for (NETSIM_ID i = 0; i < NETWORK->nDeviceCount; i++) {
            for (NETSIM_ID j = 0; j < NETWORK->ppstruDeviceList[i]->nNumOfInterface; j++) {
                if (!isLTE_NRInterface(i + 1, j + 1) || !isGNB(i + 1, j + 1)) {
                    continue;
                }

                if (actionIndex >= actionCount) {
                    fprintf(stderr, "No more action values available for eNB Device %d Interface %d.\n", i + 1, j + 1);
                    break;
                }

                NETSIM_ID gnbId = i + 1;
                NETSIM_ID gnbInterfaceId = j + 1;

                ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbInterfaceId);
                if (phy == NULL) {
                    //fprintf(stderr, "No PHY data available for eNB Device %d Interface %d.\n", gnbId, gnbInterfaceId);
                    continue;
                }

                double actionValue_dB = actmsg->values[actionIndex];

                fprintf(stderr, "Action value for eNB %d (Index %d) is %f dB\n", gnbId, actionIndex, actionValue_dB);

                ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;

                while (assocInfo) {
                    if (assocInfo->isAssociated) {
                        for (UINT CAid = 0; CAid < phy->ca_count; CAid++) {
                            double previousPower = assocInfo->propagationInfo[CAid]->downlink.txPower_dbm;
                            double newPower = actionValue_dB;
                            assocInfo->propagationInfo[CAid]->downlink.txPower_dbm = newPower;
                            fprintf(stderr, "Set power for eNB %d, CA %d, UE %d from %f dBm to %f dBm\n", gnbId, CAid, assocInfo->ueId, previousPower, newPower);
                        }
                    }

                    assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
                }

                actionIndex++;
            }
        }
    }
    else {
        fprintf(stderr, "Error: Not enough values in ActMsg to perform power action. Expected at least 1, got %d\n", actionCount);
    }

}


/* //collect sinr for all layer
void execute_SINR_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    NETSIM_ID ueId;
    ueId = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);

    ptrLTENR_UERRC ueRRC;
    NETSIM_ID ueInterfaceId;

    ueInterfaceId = fn_NetSim_Stack_GetInterfaceIdByName(ueId, "5G_RAN");
    ueRRC = LTENR_UERRC_GET(ueId, ueInterfaceId);

    if (ueRRC == NULL)
    {
        fprintf(stderr, "UE %d has no RRC data.\n", ueId);
        return;
    }

    ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(ueRRC->SelectedCellID, ueRRC->SelectedCellIF);
    if (phy == NULL)
    {
        fprintf(stderr, "No PHY data available for UE %d connected to gNB %d.\n", ueId, ueRRC->SelectedCellID);
        return;
    }

    ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;

    double sinr_values[10];
    int sinr_count = 0;

    while (assocInfo)
    {
        if (assocInfo->ueId == ueId && assocInfo->isAssociated)
        {
            fprintf(stderr, "SINR values for UE %d connected to gNB %d:\n", ueId, ueRRC->SelectedCellID);
            for (UINT CAid = 0; CAid < phy->ca_count; CAid++)
            {
                UINT layerCount = assocInfo->propagationInfo[CAid]->downlink.layerCount;
                for (UINT i = 0; i < layerCount; i++)
                {
                    double sinr = assocInfo->propagationInfo[CAid]->downlink.SINR_db[i];
                    fprintf(stderr, "Layer %d: SINR = %f dB\n", i, sinr);

                    if (sinr_count < 10)
                    {
                        sinr_values[sinr_count++] = sinr;
                    }
                }
            }
            break;
        }
        assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
    }

    if (sinr_count > 0)
    {
        contain_obs(info, "SINR", sinr_values, sinr_count);
    }
    else
    {
        fprintf(stderr, "No SINR values collected for UE %d.\n", ueId);
    }
}*/

/* //collect throughput for all layer v1
void execute_Throughput_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    NETSIM_ID ueId;
    ueId = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
    //fprintf(stderr, "UE name is %d\n", ueId);
    ptrLTENR_UERRC ueRRC;
    NETSIM_ID ueInterfaceId;

    ueInterfaceId = fn_NetSim_Stack_GetInterfaceIdByName(ueId, "5G_RAN");
    ueRRC = LTENR_UERRC_GET(ueId, ueInterfaceId);

    if (ueRRC == NULL)
    {
        fprintf(stderr, "UE %d has no RRC data.\n", ueId);
        return;
    }

    ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(ueRRC->SelectedCellID, ueRRC->SelectedCellIF);
    if (phy == NULL)
    {
        fprintf(stderr, "No PHY data available for UE %d connected to gNB %d.\n", ueId, ueRRC->SelectedCellID);
        return;
    }

    ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;

    double throughput_values[10];
    int throughput_count = 0;

    while (assocInfo)
    {
        if (assocInfo->ueId == ueId && assocInfo->isAssociated)
        {
            fprintf(stderr, "Throughput values for UE %d connected to gNB %d:\n", ueId, ueRRC->SelectedCellID);
            for (UINT CAid = 0; CAid < phy->ca_count; CAid++)
            {
                UINT layerCount = assocInfo->propagationInfo[CAid]->downlink.layerCount;
                ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(ueId, ueInterfaceId);
                double frameDuration = phy->spectrumConfig->frameDuration;

                for (UINT i = 0; i < layerCount; i++)
                {
                    double throughput = (pdcp->TotalPDSCHBytes * 8) / (timestep*1000000);
                    fprintf(stderr, "Layer %d: Throughput = %f Mbps\n", i, throughput);

                    if (throughput_count < 10)
                    {
                        throughput_values[throughput_count++] = throughput;
                    }
                }

                pdcp->TotalPDSCHBytes = 0;
            }
            break;
        }
        assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
    }

    if (throughput_count > 0)
    {

        contain_obs(info, "Throughput", throughput_values, throughput_count);
    }
    else
    {
        fprintf(stderr, "No Throughput values collected for UE %d.\n", ueId);
    }
}*/

/*  //collect throughput for all layer v2
void execute_Throughput_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    NETSIM_ID ueId;
    ueId = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
    //fprintf(stderr, "UE name is %d\n", ueId);
    ptrLTENR_UERRC ueRRC;
    NETSIM_ID ueInterfaceId;

    ueInterfaceId = fn_NetSim_Stack_GetInterfaceIdByName(ueId, "5G_RAN");
    ueRRC = LTENR_UERRC_GET(ueId, ueInterfaceId);

    if (ueRRC == NULL)
    {
        fprintf(stderr, "UE %d has no RRC data.\n", ueId);
        return;
    }

    ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(ueRRC->SelectedCellID, ueRRC->SelectedCellIF);
    if (phy == NULL)
    {
        fprintf(stderr, "No PHY data available for UE %d connected to gNB %d.\n", ueId, ueRRC->SelectedCellID);
        return;
    }

    ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;

    double throughput_value = 0.0;
    bool throughput_found = false;

    while (assocInfo)
    {
        if (assocInfo->ueId == ueId && assocInfo->isAssociated)
        {
            fprintf(stderr, "Throughput values for UE %d connected to gNB %d:\n", ueId, ueRRC->SelectedCellID);

            if (phy->ca_count > 0)
            {
                UINT layerCount = assocInfo->propagationInfo[0]->downlink.layerCount;
                ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(ueId, ueInterfaceId);
                double frameDuration = phy->spectrumConfig->frameDuration;

                if (layerCount > 0)
                {
                    throughput_value = (pdcp->TotalPDSCHBytes * 8) / (timestep*1000000);
                    fprintf(stderr, "Layer 0: Throughput = %f Mbps\n", throughput_value);

                    throughput_found = true;
                    break;
                }

                pdcp->TotalPDSCHBytes = 0;
            }

            break;
        }
        assocInfo = LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
    }

    if (throughput_found)
    {
        contain_obs(info, "Throughput", &throughput_value, 1);
    }
    else
    {
        fprintf(stderr, "No Throughput values collected for UE %d.\n", ueId);
    }
}
*/