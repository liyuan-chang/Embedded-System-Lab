# Final Project: Intelligent Safe Deposit Box

# Slide

https://docs.google.com/presentation/d/13vahoGzPyO3vHJgrohE_sUdvlec83cgk/edit?usp=sharing&ouid=105147041817391087239&rtpof=true&sd=true

# Demo

## Security Monitor & GPS

https://drive.google.com/file/d/1PI4LYKfEet5ypRfvju3PmYhrZ2tBp7xk/view?usp=sharing

## Lock and Unlock

https://drive.google.com/file/d/1_nd07cvj_bU4PdmbwFV6lwyo-1Fuc-z0/view?usp=sharing

# Usage

1. Modify `mbed_app.json`

   * nsapi.default-wifi-security

   * nsapi.default-wifi-ssid

   * nsapi.default-wifi-password

2. Set **IP_ADDRESS** and **REMOTE_PORT** in `main.cpp`

3. Set **HOST** and **PORT** in `Server.py`

4. Run `Server.py`

5. Compile and run the program on a STM32 board