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

#define MAX_DEVICE_NAMES 1000  // max number of device_namefor saving 
#define MAX_STRING_LENGTH 2560 // max string length

DictContainer* Container = NULL;
#define SIZE_OF_MAP 1

DictContainer* Container_init()
{
	DictContainer* mapManager = (DictContainer*)malloc(sizeof(DictContainer));
	if (mapManager == NULL) {
		fprintf(stderr, "Memory allocation failed for mapManager\n");
		return NULL;
	}

	mapManager->maxSize = 100; // 최대 크기 설정
	mapManager->currentSize = 0;

	mapManager->maps = (BoxContainer*)malloc(mapManager->maxSize * sizeof(BoxContainer));
	if (mapManager->maps == NULL) {
		fprintf(stderr, "Memory allocation failed for maps\n");
		free(mapManager);
		return NULL;
	}

	return mapManager;
}

void Box_insert_obs(DictContainer* mapManager, const char* key, float value_array[], uint32_t SizeofMap)
{
	if (mapManager == NULL || mapManager->maps == NULL || key == NULL || value_array == NULL) {
		fprintf(stderr, "Error: Invalid argument(s)\n");
		return;
	}

	if (mapManager->currentSize >= mapManager->maxSize) {
		fprintf(stderr, "Error: Map is full.\n");
		return;
	}

	if (strlen(key) >= MAP_MAX_KEY_LEN) {
		fprintf(stderr, "Error: Key length exceeds maximum allowed length.\n");
		return;
	}

	if (SizeofMap > MAP_MAX_DATA_LEN) {
		fprintf(stderr, "Error: SizeofMap exceeds maximum allowed length.\n");
		return;
	}

	// Check for duplicate key and append values if found
	for (uint32_t i = 0; i < mapManager->currentSize; i++) {
		if (strncmp(mapManager->maps[i].key, key, MAP_MAX_KEY_LEN) == 0) {
			// Duplicate key found, append new values to existing values
			uint32_t existingSize = mapManager->maps[i].SizeofMap;

			// Ensure we do not exceed the maximum data length
			if (existingSize + SizeofMap > MAP_MAX_DATA_LEN) {
				fprintf(stderr, "Error: Appending values exceeds maximum allowed length.\n");
				return;
			}

			// Append new values to the existing array
			memcpy(mapManager->maps[i].value + existingSize, value_array, sizeof(float) * SizeofMap);
			mapManager->maps[i].SizeofMap += SizeofMap;
			return;
		}
	}

	// If key is not found, insert new entry
	strncpy(mapManager->maps[mapManager->currentSize].key, key, MAP_MAX_KEY_LEN);
	mapManager->maps[mapManager->currentSize].key[MAP_MAX_KEY_LEN] = '\0';
	memcpy(mapManager->maps[mapManager->currentSize].value, value_array, sizeof(float) * SizeofMap);
	mapManager->maps[mapManager->currentSize].SizeofMap = SizeofMap;
	mapManager->maps[mapManager->currentSize].is_reward = false;  // false as obs

	mapManager->currentSize++;
}

void Box_insert_reward(DictContainer* mapManager, const char* key, float value_array[], uint32_t SizeofMap)
{
	if (mapManager == NULL || mapManager->maps == NULL || key == NULL || value_array == NULL) {
		fprintf(stderr, "Error: Invalid argument(s)\n");
		return;
	}

	if (mapManager->currentSize >= mapManager->maxSize) {
		fprintf(stderr, "Error: Map is full.\n");
		return;
	}

	if (strlen(key) >= MAP_MAX_KEY_LEN) {
		fprintf(stderr, "Error: Key length exceeds maximum allowed length.\n");
		return;
	}

	if (SizeofMap > MAP_MAX_DATA_LEN) {
		fprintf(stderr, "Error: SizeofMap exceeds maximum allowed length.\n");
		return;
	}

	// Check for duplicate key and append values if found
	for (uint32_t i = 0; i < mapManager->currentSize; i++) {
		if (strncmp(mapManager->maps[i].key, key, MAP_MAX_KEY_LEN) == 0) {
			// Duplicate key found, append new values to existing values
			uint32_t existingSize = mapManager->maps[i].SizeofMap;

			// Ensure we do not exceed the maximum data length
			if (existingSize + SizeofMap > MAP_MAX_DATA_LEN) {
				fprintf(stderr, "Error: Appending values exceeds maximum allowed length.\n");
				return;
			}

			// Append new values to the existing array
			memcpy(mapManager->maps[i].value + existingSize, value_array, sizeof(float) * SizeofMap);
			mapManager->maps[i].SizeofMap += SizeofMap;
			return;
		}
	}

	// If key is not found, insert new entry
	strncpy(mapManager->maps[mapManager->currentSize].key, key, MAP_MAX_KEY_LEN);
	mapManager->maps[mapManager->currentSize].key[MAP_MAX_KEY_LEN] = '\0';
	memcpy(mapManager->maps[mapManager->currentSize].value, value_array, sizeof(float) * SizeofMap);
	mapManager->maps[mapManager->currentSize].SizeofMap = SizeofMap;
	mapManager->maps[mapManager->currentSize].is_reward = true;  // true as reward

	mapManager->currentSize++;
}

void contain_obs(ptrCLIENTINFO info, const char* rewardName, float* values, size_t size)
{
	CommandCounter += 1;

	if (CommandCounter <= CommandNum) {
		Box_insert_obs(Container, rewardName, values, size);
	}

	if (CommandCounter == CommandNum) {
		SendtoPython(Container, info);
		CommandCounter = 0;
	}
}

void contain_reward(ptrCLIENTINFO info, const char* rewardName, float* values, size_t size)
{
	CommandCounter += 1;

	if (CommandCounter <= CommandNum) {
		Box_insert_reward(Container, rewardName, values, size);
	}
	if (CommandCounter == CommandNum) {
		SendtoPython(Container, info);
		CommandCounter = 0;
	}
}

void PrintOuterMessage(const NetSimMsg* msg) {
	for (size_t i = 0; i < msg->n_details; i++) {
		Google__Protobuf__Any* any_msg = msg->details[i];
		if (strcmp(any_msg->type_url, "type.googleapis.com/ObsMsg") == 0) {
			ObsMsg* netsim_send_msg = obs_msg__unpack(NULL, any_msg->value.len, any_msg->value.data);
			if (netsim_send_msg) {
				fprintf(stderr, "  Value Name %zu: %s\n", i + 1, netsim_send_msg->key_name);
				fprintf(stderr, "  Values %zu: ", i + 1);
				for (size_t j = 0; j < netsim_send_msg->n_values; j++) {
					fprintf(stderr, "%f ", netsim_send_msg->values[j]);
				}
				fprintf(stderr, "\n");
				obs_msg__free_unpacked(netsim_send_msg, NULL);
			}
			else {
				fprintf(stderr, "Detail %zu Failed to unpack NetSimSendMSG\n", i);
			}
		}
		else {
			fprintf(stderr, "Detail %zu Unknown Type URL\n", i);
		}
	}
}