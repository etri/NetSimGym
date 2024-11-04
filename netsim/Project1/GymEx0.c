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

#define SIZE_OF_MAP 1

void execute_RandomState_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
	float values[SIZE_OF_MAP];
	for (uint32_t i = 0; i < SIZE_OF_MAP; i++) {
		values[i] = rand() % 5; // Random values between 0 and 4
		fprintf(stderr, "Random State %d: %f \n", i, values[i]);
	}

	contain_obs(info, "RandomState", values, SIZE_OF_MAP);

	return;

}

void execute_IntState_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
	float values[SIZE_OF_MAP];
	for (uint32_t i = 0; i < SIZE_OF_MAP; i++) {
		values[i] = rand() % 5; // Random values between 0 and 4
		fprintf(stderr, "Int State %d: %f \n", i, values[i]);
	}

	contain_obs(info, "IntState", values, SIZE_OF_MAP);

	return;
}

void execute_Dummy_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
	float values[SIZE_OF_MAP];
	for (uint32_t i = 0; i < SIZE_OF_MAP; i++) {
		values[i] = rand() % 5; // Random values between 0 and 4
		fprintf(stderr, "Dummy Reward %d: %f \n", i, values[i]);
	}

	contain_reward(info, "Dummy", values, SIZE_OF_MAP);

	return;
}

void apply_random_action(ActMsg* actmsg) {

	int actionCount = actmsg->n_values;

	if (actionCount >= 1) {
		fprintf(stderr, "Random Action \n");
		for (int actionIndex = 0; actionIndex < actionCount; actionIndex++) {
			double actionValue = actmsg->values[actionIndex];
			fprintf(stderr, "Action %d: %f\n", actionIndex, actionValue);
		}
	}

}

void apply_int_action(ActMsg* actmsg) {

	int actionCount = actmsg->n_values;

	if (actionCount >= 1) {
		fprintf(stderr, "Int Action \n");
		for (int actionIndex = 0; actionIndex < actionCount; actionIndex++) {
			double actionValue = actmsg->values[actionIndex];
			fprintf(stderr, "Action %d: %f \n", actionIndex, actionValue);
		}
	}

}