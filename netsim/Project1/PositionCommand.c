#include "main.h"
#include <signal.h>
#include "Gym.h"
#include "../IP/IP.h"
#include "../Mobility/Mobility.h"
#include "../Satellite/Satellite_PHY.h"
#include"../LTE_NR//LTE_NR.h"
#include"../LTE_NR/LTENR_GNBRRC.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"../Include/CLIInterface.h"






static bool validate_position_device_x_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    if (command->length - index < 2)
    {
        send_message(info, "USAGE: device_x deviceName\n");
        return false;
    }

    if (_stricmp(command->commands[index], "device_x"))
        return false;

    if (!isCommandAsDeviceName(command->commands[index + 1]))
    {

        send_message(info, "%s is not a valid device name.\n",
            command->commands[index + 1]);
        return false;

    }
    return true;
}
static bool validate_position_device_y_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    if (command->length - index < 2)
    {
        send_message(info, "USAGE: device_y deviceName\n");
        return false;
    }

    if (_stricmp(command->commands[index], "device_y"))
        return false;

    if (!isCommandAsDeviceName(command->commands[index + 1]))
    {

        send_message(info, "%s is not a valid device name.\n",
            command->commands[index + 1]);
        return false;

    }
    return true;
}
static bool validate_position_device_z_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    if (command->length - index < 2)
    {
        send_message(info, "USAGE: device_z deviceName\n");
        return false;
    }

    if (_stricmp(command->commands[index], "device_z"))
        return false;

    if (!isCommandAsDeviceName(command->commands[index + 1]))
    {

        send_message(info, "%s is not a valid device name.\n",
            command->commands[index + 1]);
        return false;

    }
    return true;
}

static bool validate_position_device_d_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index) //kihoon
{
    if (command->length - index < 2)
    {
        send_message(info, "USAGE: device_k deviceName\n");
        return false;
    }

    if (_stricmp(command->commands[index], "device_d"))
        return false;

    if (!isCommandAsDeviceName(command->commands[index + 1]))
    {

        send_message(info, "%s is not a valid device name.\n",
            command->commands[index + 1]);
        return false;

    }
    return true;
}
static bool validate_position_device_NR_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index) //kihoon
{
    if (command->length - index < 2)
    {
        send_message(info, "USAGE: device_NR deviceName\n");
        return false;
    }

    if (_stricmp(command->commands[index], "device_NR"))
        return false;

    if (!isCommandAsDeviceName(command->commands[index + 1]))
    {

        send_message(info, "%s is not a valid device name.\n",
            command->commands[index + 1]);
        return false;

    }
    return true;
}

static bool validate_position_device_xx_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index) //kihoon
{
    if (command->length - index < 2)
    {
        send_message(info, "USAGE: device_xx deviceName\n");
        return false;
    }

    if (_stricmp(command->commands[index], "device_xx"))
        return false;

    if (!isCommandAsDeviceName(command->commands[index + 1]))
    {

        send_message(info, "%s is not a valid device name.\n",
            command->commands[index + 1]);
        return false;

    }
    return true;
}

static bool validate_GNBPRB_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index) //kihoon
{
    if (command->length - index < 2)
    {
        send_message(info, "USAGE: device_xx deviceName\n");
        return false;
    }

    if (_stricmp(command->commands[index], "GNBPRB"))
        return false;

    if (!isCommandAsDeviceName(command->commands[index + 1]))
    {

        send_message(info, "%s is not a valid device name.\n",
            command->commands[index + 1]);
        return false;

    }
    return true;
}

bool validate_position_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index)
{
    if (command->length - index < 2)
    {
        send_message(info, "Too less argument for position command\n");
        return false;
    }

    if (!_stricmp(command->commands[index + 1], "device_x"))
        return validate_position_device_x_command(info, command, index + 1);

    else if (!_stricmp(command->commands[index + 1], "device_y"))
        return validate_position_device_y_command(info, command, index + 1);

    else if (!_stricmp(command->commands[index + 1], "device_z"))
        return validate_position_device_z_command(info, command, index + 1);

    else if (!_stricmp(command->commands[index + 1], "device_d"))
        return validate_position_device_d_command(info, command, index + 1);

    else if (!_stricmp(command->commands[index + 1], "device_NR"))
        return validate_position_device_NR_command(info, command, index + 1);

    else if (!_stricmp(command->commands[index + 1], "device_xx"))
        return validate_position_device_xx_command(info, command, index + 1);
    else if (!_stricmp(command->commands[index + 1], "GNBPRB"))
        return validate_GNBPRB_command(info, command, index + 1);

    send_message(info, "%s is not valid argument for route command\n",
        command->commands[index + 1]);
    return false;
}

void execute_position_device_x_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index, NETSIM_ID d)
{

    fprintf(stderr, "command: %s\n", command->originalCommand);
    fprintf(stderr, "command: %s\n", command->commands[0]);



    NETSIM_ID dest;
    if (isCommandAsDeviceName(command->commands[index + 1]))
    {
        dest = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
        send_message(info, "%f\n", DEVICE_POSITION(dest)->X);
        return;
    }

}

static void execute_position_device_y_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index, NETSIM_ID d)
{
    NETSIM_ID dest;
    if (isCommandAsDeviceName(command->commands[index + 1]))
    {
        dest = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
        send_message(info, "%f\n", DEVICE_POSITION(dest)->Y);
        return;
    }

}

static void execute_position_device_z_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index, NETSIM_ID d)
{
    NETSIM_ID dest;
    if (isCommandAsDeviceName(command->commands[index + 1]))
    {
        dest = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
        send_message(info, "%f\n", DEVICE_POSITION(dest)->Z);
        return;
    }

}


static void execute_position_device_d_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index, NETSIM_ID d) // distance betwenn devices
{


    NETSIM_ID dest1;
    NETSIM_ID dest2;

    if (isCommandAsDeviceName(command->commands[index + 1]))
    {
        dest1 = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
        dest2 = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 2]);
        send_message(info, "%f\n", DEVICE_DISTANCE(dest1, dest2));
        send_message(info, "%f\n", DEVICE_POSITION(dest1)->X);
        send_message(info, "%f\n", DEVICE_POSITION(dest2)->X);
        send_message(info, "%f\n", DEVICE_POSITION(dest1)->Y);
        send_message(info, "%f\n", DEVICE_POSITION(dest2)->Y);
        return;
    }
}

static void execute_position_device_NR_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index, NETSIM_ID d){
    NETSIM_ID dest1;
    NETSIM_ID dest2;
    if (isCommandAsDeviceName(command->commands[index + 1]))
    {
        dest1 = fn_NetSim_Stack_GetDeviceId_asName(command->commands[index + 1]);
        dest2 = DEVICE_NAME(dest1);


        send_message(info, "%d\n", fn_NetSim_Stack_GetInterfaceIdFromIP(dest1, fn_NetSim_Stack_GetFirstIPAddressByName(dest2)));
        return;
    }

}


static void execute_position_device_xx_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index, NETSIM_ID d) {



    if (isCommandAsDeviceName(command->commands[index + 1]))
    {



        send_message(info, "%f\n", pstruEventDetails->dEventTime);


    }
    return;
}

void execute_position_command(ptrCLIENTINFO info, ptrCOMMANDARRAY command, int index, NETSIM_ID d)
{
    if (!_stricmp(command->commands[index + 1], "device_x"))
        execute_position_device_x_command(info, command, index + 1, d);

    else if (!_stricmp(command->commands[index + 1], "device_y"))
        execute_position_device_y_command(info, command, index + 1, d);

    else if (!_stricmp(command->commands[index + 1], "device_z"))
        execute_position_device_z_command(info, command, index + 1, d);

    else if (!_stricmp(command->commands[index + 1], "device_d"))
        execute_position_device_d_command(info, command, index + 1, d);

    else if (!_stricmp(command->commands[index + 1], "device_NR"))
        execute_position_device_NR_command(info, command, index + 1, d);

    else if (!_stricmp(command->commands[index + 1], "device_xx"))
        execute_position_device_xx_command(info, command, index + 1, d);
}