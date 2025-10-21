import subprocess
import sys
from pathlib import PureWindowsPath
import os
import zipfile

# fixa ssh nycklar? för att slippa skriva in lösenord hela tin' :)

LOCAL_GBL_DIR = PureWindowsPath('c:/users/username/downloads/firmware-build-zigbee_ncp_7.5.0.0') # THIS SHOULD BE A FOLDER, IT CAN BE A ZIPPED FOLDER, PROGRAM WILL UNZIP IF NEEDED
UNZIPPED_DIR = "c:/users/benfl/downloads/unzipped_firmware" # created if user enters ZIP-file as LOCAL_GBL_DIR. LOCAL_GBL_DIR will be set to this after unzipping
REMOTE_GBL_PATH = "~/ZIGBEE"
SSH_HOST = "example@127.0.0.1"
DEVICE_PATH = "/dev/ttyAMA3"                # device port of the Zigbee chip
VIRTUAL_ENV_NAME = ".example-venv-zigbee"       # name of python venv on CM5
            
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
    
    # NO FIRMWARE FILES --> Clean exit
    if list_of_gbls == []:
        sys.exit(f"NO FIRMWARE FILES IN --> {LOCAL_GBL_DIR}")
    
    # get latest modified firmware file
    latest_gbl_file = max(list_of_gbls, key=lambda x: x["last_modified"])["file_name"]

    return latest_gbl_file

def upload_to_cm5(local_file: str):

    print() # this is just for clean print in terminal

    # Copy file to CM5 --> User must enter SSH password:
    try:
        result = subprocess.run(["scp", f"{LOCAL_GBL_DIR}/{local_file}", f"{SSH_HOST}:{REMOTE_GBL_PATH}"], check=False, stderr=subprocess.PIPE, text=True)    

        if result.returncode == 0:
            print("\nUpload to CM5 was successful! :)")
        else:
            print("\nUpload to CM5 was unsuccessful! :(")
            print(result.stderr)
    
    except Exception as e:
            print(f"Error during upload stage: {e}")
            sys.exit(1)

def flash_on_cm5(local_file: str):
    
    command = f"cd ZIGBEE && source {VIRTUAL_ENV_NAME}/bin/activate && universal-silabs-flasher --device {DEVICE_PATH} flash --firmware {REMOTE_GBL_PATH}/{local_file} && universal-silabs-flasher --device {DEVICE_PATH} probe"
    # Has to enter password again :(
    try:
        result = subprocess.run(["ssh", "-t", f"{SSH_HOST}", command], check=False, stderr=subprocess.PIPE, text=True)

        # if successful
        if result.returncode == 0:
            return True 
        else:
            return False
    
    except Exception as e:
        print(f"Error during flash/probe stage: {e}")
        sys.exit(1)

def run_reset_chip():
    
    print("Running 'reset_chip.py'...")
    command = f"cd ZIGBEE && sudo python reset_chip.py" 
    # Has to enter password again :)
    try:
        result = subprocess.run(["ssh", "-t", f"{SSH_HOST}", command], check=False, stderr=subprocess.PIPE, text=True)

        # if successful
        if result.returncode == 0:
            return True
        else:
            print(result.stderr)
            return False
    
    except Exception as e:
        print(f"Error resetting the chip: {e}")
        sys.exit(1)

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
