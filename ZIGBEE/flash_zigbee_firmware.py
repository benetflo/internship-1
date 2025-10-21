import subprocess
import sys
from pathlib import PureWindowsPath
import os
import zipfile

# fixa ssh nycklar? 

LOCAL_GBL_DIR = PureWindowsPath('c:/users/benfl/downloads/firmware-build-hemla_zigbee_ncp_7.5.0.0') # THIS SHOULD BE A FOLDER, IT CAN BE A ZIPPED FOLDER, PROGRAM WILL UNZIP IF NEEDED
UNZIPPED_DIR = "c:/users/benfl/downloads/unzipped_firmware" # created if user enters ZIP-file as LOCAL_GBL_DIR. LOCAL_GBL_DIR will be set to this after unzipping
REMOTE_GBL_PATH = "~/ZIGBEE"
SSH_HOST = "benflo@192.168.1.198"
DEVICE_PATH = "/dev/ttyAMA3"                # device port of the Zigbee chip
VIRTUAL_ENV_NAME = ".benjamin-zigbee"       # name of python venv on CM5

def get_last_modified(gbls: dict):
    
    last_modified_date = 0
    last_modified_file = ""
    date_updated_flag = True

    for key, value in gbls.items():
        
        if key == "file_name" and date_updated_flag is True:
            last_modified_file = value
            date_updated_flag = False

        if key == "last_modified":
            if value > last_modified_date:
                last_modified_date = value
                date_updated_flag = True

    return last_modified_file
            
def find_latest_gbl():
            
    global LOCAL_GBL_DIR

    # UNZIP FOLDER IF NEEDED
    if os.path.isfile(str(LOCAL_GBL_DIR)) and str(LOCAL_GBL_DIR).lower().endswith(".zip"):
        
        if not os.path.exists(UNZIPPED_DIR):
            os.makedirs(UNZIPPED_DIR)
        with zipfile.ZipFile(str(LOCAL_GBL_DIR), 'r') as zip_ref:
            zip_ref.extractall(UNZIPPED_DIR)
        print(f"ZIP extracted to: {UNZIPPED_DIR}")
        LOCAL_GBL_DIR = UNZIPPED_DIR 

    # IF LOCAL_GBL_DIR is not a folder or not found
    elif not os.path.isdir(str(LOCAL_GBL_DIR)):
        sys.exit(f"{LOCAL_GBL_DIR} - DOES NOT EXIST OR IS NOT A DIRECTORY")

    dir_list = os.listdir(LOCAL_GBL_DIR)
    latest_gbl_file = ""
    list_of_gbls = []
    
    # add all present .gbl files to list
    for file in dir_list:
        if file.endswith(".gbl"):
            modified_time = os.path.getmtime(f"{LOCAL_GBL_DIR}/{file}")    
            list_of_gbls.append({
                "file_name": file,
                "last_modified": modified_time
            })
    
    # FILE NOT FOUND --> Clean exit
    if list_of_gbls == []:
        sys.exit(f"FILE NOT FOUND IN --> {LOCAL_GBL_DIR}")
    
    # get latest modified firmware file
    latest_gbl_file = max(list_of_gbls, key=lambda x: x["last_modified"])["file_name"]

    return latest_gbl_file

def upload_to_cm5(local_file: str):

    print() # this is just for clean print in terminal

    # Copy file to CM5 --> User must enter SSH password:
    result = subprocess.run(["scp", f"{LOCAL_GBL_DIR}/{local_file}", f"{SSH_HOST}:{REMOTE_GBL_PATH}"], check=False)    

    if result.returncode == 0:
        print("\nUpload to CM5 was successful! :)")
    else:
        print("\nUpload to CM5 was unsuccessful! :(")

def flash_on_cm5(local_file: str):
    
    command = f"cd ZIGBEE && source {VIRTUAL_ENV_NAME}/bin/activate && universal-silabs-flasher --device {DEVICE_PATH} flash --firmware {REMOTE_GBL_PATH}/{local_file} && universal-silabs-flasher --device {DEVICE_PATH} probe"
    # Has to enter password again :(
    result = subprocess.run(["ssh", "-t", f"{SSH_HOST}", command], check=False, stderr=subprocess.PIPE, text=True)

    # if successful
    if result.returncode == 0:
        return True 
    
    return False

def run_reset_chip():
    
    print("Running 'reset_chip.py'...")
    command = f"cd ZIGBEE && sudo python reset_chip.py" 
    # Has to enter password again :)
    result = subprocess.run(["ssh", "-t", f"{SSH_HOST}", command], check=False, stderr=subprocess.PIPE, text=True)

    # if successful
    if result.returncode == 0:
        return True
    
    print(result.stderr)
    return False

def main():

    gbl_file = find_latest_gbl()
    upload_to_cm5(gbl_file)
    if flash_on_cm5(gbl_file):
        print("Flash was successful! :)")
    else:
        print("Flash was unsuccessful! :(")
        if run_reset_chip():
            print("Chip reset was successful! :)")
        else:
            print("Chip reset was not successful! :(")

#END PROGRAM
if __name__ == "__main__":
    main()
