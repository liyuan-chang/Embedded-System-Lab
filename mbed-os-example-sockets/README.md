# HW2 Socket Programming and Data Visualization

Send the sensor data of STM board to a PC host and visualize the sensor data

# Usage

1. Modify `mbed_app.json`

   * nsapi.default-wifi-security

   * nsapi.default-wifi-ssid

   * nsapi.default-wifi-password

2. Set **IP_ADDRESS** and **REMOTE_PORT** in `source/main.cpp`

3. Set **HOST** and **PORT** in `server.py`

4. Run `server.py`

5. Compile and run the program on a STM board
