import time
import pygetwindow as gw
import pyautogui
import subprocess
import sys

# # 에포크 시간으로 현재 시간 출력
# current_time_seconds = time.time()
# print("Current time (in seconds since epoch):", current_time_seconds)

# # 사람이 읽기 쉬운 형태로 현재 시간 출력
# readable_time = time.ctime()
# print("Current time (readable):", readable_time)

# # 현재 시간을 초 단위로 가져온 후 밀리초로 변환
# current_time_milliseconds = int(time.time() * 1000)
# print("Current time (in milliseconds since epoch):", current_time_milliseconds)

# import datetime

# # 현재 시간과 날짜를 datetime 객체로 가져오기
# now = datetime.datetime.now()

# # strftime을 사용하여 날짜와 시간을 원하는 형식으로 포맷팅
# # %f는 마이크로초를 나타내므로, 밀리초 단위로 출력하려면 앞 3자리만 사용
# formatted_time = now.strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]

# print("Current time (readable with milliseconds):", formatted_time)



# "9000" 포트 번호를 인자로 전달하면서 실행
# process = subprocess.Popen(["python", "NetSim_Scenario_RL_portnum.py", "9000"])

# # 만약 실행이 끝날 때까지 기다리려면:
# process.wait()


# from Auto_NetSim_RL_portnum_iternum import AutoNetSim
# import time

# auto_netsim = AutoNetSim()

# auto_netsim.NetSimAutoRun(9000,2)
# time.sleep(2)
# auto_netsim.stop_netsim(9000)

class AutoNetSim:
    def find_netsim_windows(self, portnum):
        netsim_windows = []
        windows = gw.getAllTitles()
        for window_title in windows:
            if window_title.startswith(f'NetSim{portnum}'): # here
                netsim_windows.append(gw.getWindowsWithTitle(window_title)[0])
        return netsim_windows

    def press_enter_on_windows(self, windows, iternum):
        num_press = 0
        for window in windows:
            try:
                window.activate()
                for _ in range(iternum):
                    pyautogui.press('enter')
                    #print(f"####Space key pressed successfully on the window '{window.title}'.####")
                    num_press += 1
                    #time.sleep(0.01)
            except Exception as e:
                print(f"####Error occurred while pressing the Space key on window '{window.title}': {e}#####")
        print(f"####Space key pressed successfully on the window '{window.title}' for {num_press} times.####")
    
    def press_ctrl_c_on_windows(self, windows):
        for window in windows:
            try:
                window.activate()
                pyautogui.hotkey('ctrl', 'c')
                pyautogui.hotkey('ctrl', 'c')
                time.sleep(0.05)
            except Exception as e:
                print(f"####Error pressing Ctrl+C on window '{window.title}': {e}####")

    def run_netsim(self, portnum, iternum):
        process = subprocess.Popen(
            ["python", "NetSim_Scenario_RL_portnum.py", str(portnum), str(iternum)],
            creationflags=subprocess.CREATE_NEW_CONSOLE
        )

    def NetSimAutoRun(self, portnum, iternum):
        self.run_netsim(portnum, iternum)
        time.sleep(5)
        window = self.find_netsim_windows(portnum)
        print (window)
        self.press_enter_on_windows(window, int(iternum))    

    def stop_netsim(self, portnum):
        window = self.find_netsim_windows(portnum)
        self.press_ctrl_c_on_windows(window)
   

def main():
    if len(sys.argv) < 3:
        print("Usage: python [script_name].py [NetSimPortNum] [IterationNum]")
        sys.exit(10)
    portnum = sys.argv[1]
    iternum = sys.argv[2]
    auto_netsim = AutoNetSim()
    auto_netsim.run_netsim(portnum, iternum)
    time.sleep(1)
    window = auto_netsim.find_netsim_windows(portnum)
    print (window)
    auto_netsim.press_enter_on_windows(window, int(iternum))    

# iternum = 2
# portnum = 9000

# process = subprocess.Popen(
#     ["python", "NetSim_Scenario_RL_portnum.py", str(portnum), str(iternum)],
#     creationflags=subprocess.CREATE_NEW_CONSOLE
# )

# time.sleep(1)
# window = find_netsim_windows(portnum)
# print (window)
# num_press = press_enter_on_windows(window, iternum)


# portnum = 9001
# iternum = 2

# process = subprocess.Popen(
#     ["python", "NetSim_Scenario_RL_portnum.py", str(portnum), str(iternum)],
#     creationflags=subprocess.CREATE_NEW_CONSOLE
# )

# time.sleep(1)
# window = find_netsim_windows(portnum)
# print (window)
# num_press = press_enter_on_windows(window, iternum)


if __name__ == "__main__":
    main()