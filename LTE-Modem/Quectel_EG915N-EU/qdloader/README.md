Quectel EG915N-EU

The latest firmware version can be downloaded, when available from this link https://www.quectel.com/download-zone/. At the time of writing, the latest version is **EG915UEUABR03A01M08 01.301.01.301**. **Note:** A Quectel account login is required to download the firmware.

**Step 1 - Moving necessary folders/files into Hemlas module:**

- Unzip qdloader.zip and EG915UEUABR03A01M08_01.301.01.301.zip to your computer.
- Now copy both files to the module via SSH with the following commands:

```bash
scp -r “path_to_qdloader_folder” user@ip-adress:~
```

```bash
scp “path_to_(.pac)-file_within_EG915UEUABR03A01M08 01.301.01.301_folder” user@ip-adress:~
```

**NOTES:**

- There is no need to copy over the whole **EG915UEUABR03A01M08 01.301.01.301-folder** since the only file needed for flashing is the **“.pac” file**.
- After the colon : you can enter whatever path you want as the destination.

**Step 2 - Building the QDloader:**

- Connect to the module via SSH and navigate to `/home/${YOUR_USER}/qdloader/qdloader`, which is the same directory as the Makefile.
- Run the command:
- Check the `out` directory, a new file called `QDloader` should be created.
- If `QDloader` is not executable, make it so with:

```bash
chmod +x <file_name> # sudo pre-fix might be needed depending on user permissions
```

**Step 3 - Flashing the firmware:**

**!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!**

**VERY IMPORTANT NOTE!! DO NOT INTERRUPT THE FLASH IT CAN MAKE THE MODEM UNUSABLE**

**!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!**

- Command to flash firmware looks like this if you are in this directory -> “~/qdloader/qdloader”:

```bash
sudo ./out/QDloader -f ~/8915DM_cat1_EG915UEUABR03A01M08.pac -s /dev/ttyUSB0
```

The standard syntax without my personal paths looks like this:

```bash
sudo ./QDloader -f <firmware_file> -s /dev/ttyUSB0
```

The flash was succesful if the following text is printed in the terminal:

```bash
[028:262] Upgrade module successfully

```

Lägga till eget script i python med subprocess som gör det enklare för just Hemla?? Ha en config.txt där tex Firmwarefile, device_port står.

Kommandon för att kolla version: 

ATI
AT+CGMR

**EG915N-SERIES MANUAL**

https://www.manualslib.com/manual/3551518/Quectel-Eg915n-Series.html?page=52#manual

**AT-COMMANDS MANUAL:**

https://quectel.com/content/uploads/2024/02/Quectel_EG800Q-EUEG915Q-NA_AT_Commands_Manual_V1.0.pdf

**EG915N-SERIES SPEC:**

https://quectel.com/content/uploads/2024/03/Quectel_EG915N_Series_LTE_Standard_Specification_V1.3-1.pdf?wpId=114168
