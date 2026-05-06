import threading
import os
import time
import pygetwindow as gw
import pyautogui
import subprocess
import sys


import psutil

#--------------------------Client Code--------------------------

# enable_auto_simulation = False # Set up automatic simulation
allocate_cpu = False
cpu_selection = [0, 1, 2, 3, 4]

# Set the path for workspace and license

NETSIM_PATH = "C:\\Users\\leegh\\Desktop\\rllib_netsim\\bin_x64"
IO_PATH = "C:\\Users\\leegh\\Desktop\\rllib_netsim"
LICENSE_PATH = os.path.join(NETSIM_PATH, "netsim-cloud-license-etri-korea-univ.lic")

if not os.path.exists(IO_PATH):
    os.makedirs(IO_PATH)

if not os.path.isfile(LICENSE_PATH):
    raise FileNotFoundError(f"License file not found: {LICENSE_PATH}")


CONFIG_FOLDER = os.path.join(IO_PATH, "NetSimGym_ex2-2_ran3_30s")


#----------------------------------------------------------------

pyautogui.FAILSAFE = False
#lock = threading.Lock()

def set_port():
    if len(sys.argv) < 3:
        print("Usage: python [script_name].py [NetSimPortNum] [MaxEpisode]")
        sys.exit(10)
    portnum = sys.argv[1]
    max_episode = sys.argv[2]
    return portnum, max_episode

def set_cmd_title(portnum):
    os.system(f'title NetSim{portnum}')

# def find_netsim_windows(portnum):
#     netsim_windows = []
#     windows = gw.getAllTitles()
#     for window_title in windows:
#         if window_title.startswith(f'NetSim{portnum}'): # here
#             netsim_windows.append(gw.getWindowsWithTitle(window_title)[0])
#     return netsim_windows

# def press_enter_on_windows(windows):
#     for window in windows:
#         try:
#             window.activate()
#             pyautogui.press('space')
#             print(f"####Space key pressed successfully on the window '{window.title}'.####")
#         except Exception as e:
#             print(f"####Error occurred while pressing the Space key on window '{window.title}': {e}#####")

def main():
    CurrentEpisodeNum = 0
    portnum, MaxEpisodeNum = set_port()
    set_cmd_title(portnum) # here

    while CurrentEpisodeNum < int(MaxEpisodeNum):

        simulation_command = f"{NETSIM_PATH}\\NetSimcore.exe -apppath {NETSIM_PATH} -iopath {CONFIG_FOLDER} -license \"{LICENSE_PATH}\""
        process = subprocess.Popen(simulation_command, shell=True)

        if allocate_cpu:
            p = psutil.Process(process.pid)
            p.cpu_affinity(cpu_selection)

        # if enable_auto_simulation:
        #     while True:
        #         netsim_windows = find_netsim_windows(portnum)
                
        #         if netsim_windows:
        #             print("#### NetSim CMD window found. ####")
        #             press_enter_on_windows(netsim_windows)
        #             break
        #         else:
        #             print("#### NetSim CMD window not found. Retrying... ####")
        #             time.sleep(1)

        process.wait()

        CurrentEpisodeNum += 1

if __name__ == "__main__":
    main()
