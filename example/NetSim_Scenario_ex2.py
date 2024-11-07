import threading
import os
import time
import pygetwindow as gw
import pyautogui
import subprocess

#--------------------------Client Code--------------------------

enable_auto_simulation = True # Set up automatic simulation

# Set the path for workspace and license
NETSIM_PATH = "C:\\Users\\Yerin\\Documents\\NetSim\\Workspaces\\NetSimGym_v1-0\\bin_x64"
IO_PATH = "C:\\Users\\Yerin\\Documents\\NetSim\\Workspaces\\NetSimGym_v1-0"
LICENSE_PATH = os.path.join(NETSIM_PATH, "netsim-cloud-license-etri-korea.lic")

if not os.path.exists(IO_PATH):
    os.makedirs(IO_PATH)

if not os.path.isfile(LICENSE_PATH):
    raise FileNotFoundError(f"License file not found: {LICENSE_PATH}")

CONFIG_FOLDER = os.path.join(IO_PATH, "NetSimGym_ex2-2_ran3_30s")

#----------------------------------------------------------------

pyautogui.FAILSAFE = False
lock = threading.Lock()

CurrentEpisodeNum = 0
MaxEpisodeNum = 1500

def set_cmd_title(title):
    os.system(f'title {title}')

def find_netsim_windows():
    netsim_windows = []
    windows = gw.getAllTitles()
    for window_title in windows:
        if window_title.startswith('NetSim_CMD'):
            netsim_windows.append(gw.getWindowsWithTitle(window_title)[0])
    return netsim_windows

def press_enter_on_windows(windows):
    for window in windows:
        try:
            window.activate()
            pyautogui.press('space')
            print(f"####Space key pressed successfully on the window '{window.title}'.####")
        except Exception as e:
            print(f"####Error occurred while pressing the Space key on window '{window.title}': {e}#####")

def main():
    CurrentEpisodeNum = 0
    MaxEpisodeNum = 1500
    
    set_cmd_title('NetSim_CMD')

    while CurrentEpisodeNum < MaxEpisodeNum:

        simulation_command = f"{NETSIM_PATH}\\NetSimcore.exe -apppath {NETSIM_PATH} -iopath {CONFIG_FOLDER} -license \"{LICENSE_PATH}\""
        process = subprocess.Popen(simulation_command, shell=True)

        if enable_auto_simulation:
            while True:
                netsim_windows = find_netsim_windows()
                
                if netsim_windows:
                    print("#### NetSim CMD window found. ####")
                    press_enter_on_windows(netsim_windows)
                    break
                else:
                    print("#### NetSim CMD window not found. Retrying... ####")
                    time.sleep(1)

        process.wait()

        CurrentEpisodeNum += 1

if __name__ == "__main__":
    main()
