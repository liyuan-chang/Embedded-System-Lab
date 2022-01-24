#!/usr/bin/env python3

import socket
import numpy as np
import json
import time
import random
import requests

HOST = '192.168.110.200' # '192.168.184.14'
PORT = 65431 # Port to listen on (use ports > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        while True:
            data = conn.recv(1024).decode('utf-8')
            print('Received from socket server : ', data)
            # Line Notify
            line_url = "https://notify-api.line.me/api/notify"
            # token = "iUiglrg6Qnn61xDI7malK9Yoc35eicrUeBoecGL1wFE"
            token = "9I8GUpnFza2EiTthUAuyFq2ZuXmhTLTaz92lsbQf63M"
            headers = {
                "Authorization": "Bearer " + token, 
                "Content-Type" : "application/x-www-form-urlencoded"
                }

            payload = {'message': data}
            r = requests.post(line_url, headers = headers, params = payload)
            #time.sleep(10)