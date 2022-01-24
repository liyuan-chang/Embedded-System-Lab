/* Sockets Example
 * Copyright (c) 2016-2020 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "wifi_helper.h"
#include "mbed-trace/mbed_trace.h"
// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"
#include "buzzer.h"
#include "MFRC522.h"
#include "rfid.h"
#include "GPS.h"

#if MBED_CONF_APP_USE_TLS_SOCKET
#include "root_ca_cert.h"

#ifndef DEVICE_TRNG
#error "mbed-os-example-tls-socket requires a device which supports TRNG"
#endif
#endif // MBED_CONF_APP_USE_TLS_SOCKET

DigitalOut led(LED1);
mbed::Beep buzz(D5); // PA_15
MFRC522 *mfrc522 = new MFRC522(D11, D12, D13, D10, D8);  // rc522 module: (mosi, miso, sck, sda, rst)
PwmOut pwm_lock(D9);
// bool idSent = false;

static BufferedSerial serial_port(USBTX, USBRX);
FileHandle *mbed::mbed_override_console(int fd)
{
    return &serial_port;
}

class SocketDemo {
    static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 30;
    static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;

#if MBED_CONF_APP_USE_TLS_SOCKET
    static constexpr size_t REMOTE_PORT = 443; // tls port
#else
    static constexpr size_t REMOTE_PORT = 65431; // standard HTTP port 80
#endif // MBED_CONF_APP_USE_TLS_SOCKET

public:
    SocketDemo() : _net(NetworkInterface::get_default_instance())
    {
    }

    ~SocketDemo()
    {
        if (_net) {
            _net->disconnect();
        }
    }

    void run()
    {
        if (!_net) {
            printf("Error! No network interface found.\r\n");
            return;
        }

        /* if we're using a wifi interface run a quick scan */
        if (_net->wifiInterface()) {
            /* the scan is not required to connect and only serves to show visible access points */
            wifi_scan();

            /* in this example we use credentials configured at compile time which are used by
             * NetworkInterface::connect() but it's possible to do this at runtime by using the
             * WiFiInterface::connect() which takes these parameters as arguments */
        }

        /* connect will perform the action appropriate to the interface type to connect to the network */

        printf("Connecting to the network...\r\n");

        nsapi_size_or_error_t result = _net->connect();
        if (result != 0) {
            printf("Error! _net->connect() returned: %d\r\n", result);
            return;
        }

        print_network_info();

        /* opening the socket only allocates resources */
        result = _socket.open(_net);
        if (result != 0) {
            printf("Error! _socket.open() returned: %d\r\n", result);
            return;
        }

#if MBED_CONF_APP_USE_TLS_SOCKET
        result = _socket.set_root_ca_cert(root_ca_cert);
        if (result != NSAPI_ERROR_OK) {
            printf("Error: _socket.set_root_ca_cert() returned %d\n", result);
            return;
        }
        _socket.set_hostname(MBED_CONF_APP_HOSTNAME);
#endif // MBED_CONF_APP_USE_TLS_SOCKET

        /* now we have to find where to connect */

        SocketAddress address;

        // if (!resolve_hostname(address)) {
        //     return;
        // }
        const char * IP_ADDRESS = "192.168.99.14"; // 192.168.184.14
        if(!address.set_ip_address(IP_ADDRESS)) {
            printf("Set IP address failed");
            return ;
        }
        address.set_port(REMOTE_PORT);

        /* we are connected to the network but since we're using a connection oriented
         * protocol we still need to open a connection on the socket */
        
        send_data_demo(address);

    }

    float average(float arr[], int n) {
        float sum = 0;
        for(int i = 0; i < n; i++) {
            sum += arr[i];
        }
        return sum / n;
    }

    // send senor data
    void send_data_demo(SocketAddress& address)
    {
        printf("Opening connection to remote port %d\r\n", REMOTE_PORT);

        nsapi_size_or_error_t result = _socket.connect(address);
        if (result != 0) {
            printf("Error here! _socket.connect() returned: %d\r\n", result);
            return;
        }

        // ---------- Sensor ----------
        float t = 0, h = 0, p = 0;
        int16_t pAccDataXYZ[3] = {0};
        int16_t pMagDataXYZ[3] = {0};
        float pGyroDataXYZ[3] = {0};

        printf("Start sensor init\n");

        BSP_TSENSOR_Init();
        BSP_HSENSOR_Init();
        BSP_PSENSOR_Init();

        BSP_MAGNETO_Init();
        BSP_GYRO_Init();
        BSP_ACCELERO_Init();

        std::printf("Calibrating...\n");
        int n = 50;
        float ax0[n], ay0[n], az0[n], gx0[n], gy0[n], gz0[n];
        for(int i = 0; i < n; i++) {
            BSP_MAGNETO_GetXYZ(pMagDataXYZ);
            BSP_GYRO_GetXYZ(pGyroDataXYZ);
            BSP_ACCELERO_AccGetXYZ(pAccDataXYZ);
            ax0[i] = pAccDataXYZ[0];
            ay0[i] = pAccDataXYZ[1];
            az0[i] = pAccDataXYZ[2];
            gx0[i] = pGyroDataXYZ[0];
            gy0[i] = pGyroDataXYZ[1];
            gz0[i] = pGyroDataXYZ[2];
            ThisThread::sleep_for(200);
        }
        
        float ax0bar = average(ax0, sizeof(ax0) / sizeof(ax0[0])),
        ay0bar = average(ay0, sizeof(ay0) / sizeof(ay0[0])),
        az0bar = average(az0, sizeof(az0) / sizeof(az0[0])),
        gx0bar = average(gx0, sizeof(gx0) / sizeof(gx0[0])),
        gy0bar = average(gy0, sizeof(gy0) / sizeof(gy0[0])),
        gz0bar = average(gz0, sizeof(gz0) / sizeof(gz0[0]));

        int sample_num = 0;
        char acc_json[1024] = {0};

        int alarm = 0;
        std::printf("Calibration finished.\nBeginning monitor...\n");
        buzz.tone(162.00, 300.0);
        ThisThread::sleep_for(50.0);
        buzz.tone(462.00, 300.0);
        ThisThread::sleep_for(50.0);
        buzz.tone(762.00, 300.0);
        ThisThread::sleep_for(50.0);
        buzz.tone(1062.00, 300.0);

        RFID_Reader rfidReader(mfrc522);
        printf("Scanning RFID...\n");

        GPS gps(PA_0, PA_1, 9600);  // GPS module: (RX, TX)

        bool lock = false;

        while(1) {
            

            led = 1;


            if (lock) {
                // read sensor data
                BSP_MAGNETO_GetXYZ(pMagDataXYZ);
                BSP_GYRO_GetXYZ(pGyroDataXYZ);
                BSP_ACCELERO_AccGetXYZ(pAccDataXYZ);
                t = BSP_TSENSOR_ReadTemp();
                h = BSP_HSENSOR_ReadHumidity();
                p = BSP_PSENSOR_ReadPressure();
                
                int16_t ax = pAccDataXYZ[0], ay = pAccDataXYZ[1], az = pAccDataXYZ[2];
                float gx = pGyroDataXYZ[0], gy = pGyroDataXYZ[1], gz = pGyroDataXYZ[2];

                //abs(gy - gy0bar) / gy0bar > 0.5
                if (abs(ax - ax0bar) / ax0bar > 0.8 || abs(ay - ay0bar) / ay0bar > 0.8 || abs(az - az0bar) / az0bar > 0.06 || abs(gx - gx0bar) / gx0bar > 0.5 || abs(gy - gy0bar) / gy0bar > 0.5 || abs(gz - gz0bar) / gz0bar > 0.5) {
                    alarm += 1;
                    led = 0;
                }
                else {
                    alarm = 0;
                }
                if (alarm >= 10) {
                    // buzzer beep on
                    buzz.beep(262.0, 2000.0);    // freq, time(ms)

                    // gps location
                    gps.sample();
                    printf("longitude: %.4f°%c, latitude: %.4f°%c\n", gps.longitude /100.0f, gps.ew, gps.latitude / 100.0f, gps.ns);
                    
                    int len = sprintf(acc_json,
                    "\nAlarm!!!\nYour Intelligent Safe Deposit Box is detecting abnormal movement!\n\nACCELERO_X:%d\nACCELERO_Y:%d\nACCELERO_Z:%d\nGYRO_X:%.2f\nGYRO_Y:%.2f\nGYRO_Z:%.2f\n\nTemperature:%.1f°C\nHumidity:%.4f%%\nPressure:%.4fhPa\n\nGPS coordinate:\n%.4f°%c, %.4f°%c\n",
                    ax, ay, az, gx, gy, gz, t, h, p, gps.longitude /100.0f, gps.ew, gps.latitude /100.0f, gps.ns);

                    result = _socket.send(acc_json, len); 
                    if (0 >= result){
                        printf("Error sending: %d\n", result);
                        return;
                    }
                    ++sample_num;
                    printf("\nSending sensor data (%d) to the server\n", sample_num);

                    alarm = 0;
                }
                // detect card
                if(rfidReader.read())
                {
                    printf("=============================\n");
                    printf("Unlock Safe Deposit Box\n");
                    printf("Card ID: ");
                    char id[2 * rfidReader.getSize()];
                    rfidReader.getCharID(id);
                    for(int i = 0; i < 8; ++i) printf("%c", id[i]);
                    printf("\n=============================\n");
                    lock = false;
                    buzz.beep(262.0, 200.0);
                    ThisThread::sleep_for(100.0);
                    buzz.beep(262.0, 200.0);
                    // unlock
                    pwm_lock.period(0.05);
                    pwm_lock.write(1.0);
                    ThisThread::sleep_for(4000.0);
                    pwm_lock.write(0.0);
                }
            }
            else {
                if(rfidReader.read())
                {
                    printf("=============================\n");
                    printf("Lock Safe Deposit Box\n");
                    printf("Card ID: ");
                    char id[2 * rfidReader.getSize()];
                    rfidReader.getCharID(id);
                    for(int i = 0; i < 8; ++i) printf("%c", id[i]);
                    printf("\n=============================\n");
                    lock = true;
                    buzz.beep(262.0, 500.0);
                    // unlock
                    pwm_lock.period(0.05);
                    pwm_lock.write(1.0);
                    ThisThread::sleep_for(4000.0);
                    pwm_lock.write(0.0);
                }
            }

            ThisThread::sleep_for(200);
        }

        printf("Demo concluded successfully \r\n");
    }

private:
    bool resolve_hostname(SocketAddress &address)
    {
        const char hostname[] = MBED_CONF_APP_HOSTNAME;

        /* get the host address */
        printf("\nResolve hostname %s\r\n", hostname);
        nsapi_size_or_error_t result = _net->gethostbyname(hostname, &address);
        if (result != 0) {
            printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
            return false;
        }

        printf("%s address is %s\r\n", hostname, (address.get_ip_address() ? address.get_ip_address() : "None") );

        return true;
    }

    bool send_http_request()
    {
        /* loop until whole request sent */
        const char buffer[] = "GET / HTTP/1.1\r\n"
                              "Host: ifconfig.io\r\n"
                              "Connection: close\r\n"
                              "\r\n";

        nsapi_size_t bytes_to_send = strlen(buffer);
        nsapi_size_or_error_t bytes_sent = 0;

        printf("\r\nSending message: \r\n%s", buffer);

        while (bytes_to_send) {
            bytes_sent = _socket.send(buffer + bytes_sent, bytes_to_send);
            if (bytes_sent < 0) {
                printf("Error! _socket.send() returned: %d\r\n", bytes_sent);
                return false;
            } else {
                printf("sent %d bytes\r\n", bytes_sent);
            }

            bytes_to_send -= bytes_sent;
        }

        printf("Complete message sent\r\n");

        return true;
    }

    bool receive_http_response()
    {
        char buffer[MAX_MESSAGE_RECEIVED_LENGTH];
        int remaining_bytes = MAX_MESSAGE_RECEIVED_LENGTH;
        int received_bytes = 0;

        /* loop until there is nothing received or we've ran out of buffer space */
        nsapi_size_or_error_t result = remaining_bytes;
        while (result > 0 && remaining_bytes > 0) {
            result = _socket.recv(buffer + received_bytes, remaining_bytes);
            if (result < 0) {
                printf("Error! _socket.recv() returned: %d\r\n", result);
                return false;
            }

            received_bytes += result;
            remaining_bytes -= result;
        }

        /* the message is likely larger but we only want the HTTP response code */

        printf("received %d bytes:\r\n%.*s\r\n\r\n", received_bytes, strstr(buffer, "\n") - buffer, buffer);

        return true;
    }

    void wifi_scan()
    {
        WiFiInterface *wifi = _net->wifiInterface();

        WiFiAccessPoint ap[MAX_NUMBER_OF_ACCESS_POINTS];

        /* scan call returns number of access points found */
        int result = wifi->scan(ap, MAX_NUMBER_OF_ACCESS_POINTS);

        if (result <= 0) {
            printf("WiFiInterface::scan() failed with return value: %d\r\n", result);
            return;
        }

        printf("%d networks available:\r\n", result);

        for (int i = 0; i < result; i++) {
            printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\r\n",
                   ap[i].get_ssid(), get_security_string(ap[i].get_security()),
                   ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
                   ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5],
                   ap[i].get_rssi(), ap[i].get_channel());
        }
        printf("\r\n");
    }

    void print_network_info()
    {
        /* print the network info */
        SocketAddress a;
        _net->get_ip_address(&a);
        printf("IP address: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        _net->get_netmask(&a);
        printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        _net->get_gateway(&a);
        printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
    }

private:
    NetworkInterface *_net;

#if MBED_CONF_APP_USE_TLS_SOCKET
    TLSSocket _socket;
#else
    TCPSocket _socket;
#endif // MBED_CONF_APP_USE_TLS_SOCKET
};

int main() {
    printf("\r\nStarting security monitor program...\r\n\r\n");

#ifdef MBED_CONF_MBED_TRACE_ENABLE
    mbed_trace_init();
#endif

    SocketDemo *example = new SocketDemo();
    MBED_ASSERT(example);
    example->run();

    return 0;
}