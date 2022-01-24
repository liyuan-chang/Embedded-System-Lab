#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Oct 18 02:19:37 2021

@author: Chang
"""
import socket
import numpy as np
import json
import time
import matplotlib.pyplot as plt
import sys, os

HOST = '192.168.1.106'   # Standard loopback interface address (192.168.1.107)
PORT = 65431         # Port to listen on (use ports > 1023)

t = []; h = []; p = []
a = [[] for _ in range(3)]
g = [[] for _ in range(3)]
m = [[] for _ in range(3)]
buf = 30

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print('Server at: %s:%s' % (HOST,PORT))
    conn, addr = s.accept()
    try:
        with conn:
            print('Connected by', str(addr))
            fig, axes = plt.subplots(nrows=2, ncols=3, figsize=(24, 8))
            for cnt in range(30):
                data = conn.recv(1024).decode('utf-8')
                print('Received from socket server : ', data)
                recv_data = json.loads(data)
                if len(t) >= buf:
                    t.pop(0)
                    h.pop(0)
                    p.pop(0)
                    for i in range(3):
                        a[i].pop(0)
                        g[i].pop(0)
                        m[i].pop(0)
                t.append(recv_data['t'])
                h.append(recv_data['h'])
                p.append(recv_data['p'])
                acc_data = [recv_data['ax'], recv_data['ay'], recv_data['az']]
                gyro_data = [recv_data['gx'], recv_data['gy'], recv_data['gz']]
                mag_data = [recv_data['mx'], recv_data['my'], recv_data['mz']]
                for i in range(3):
                    a[i].append(acc_data[i])
                    g[i].append(gyro_data[i])
                    m[i].append(mag_data[i])
        
                print('Received data: ',recv_data)
        
                x = list(range(len(t)))
                label = ['x', 'y', 'z']
                # 1
                axes[0][0].plot(x, t)
                axes[0][0].set_xlabel('Time')
                axes[0][0].set_ylabel('Temperature ('+chr(176)+'C)')
                # 2
                axes[0][1].plot(x, h)
                axes[0][1].set_xlabel('Time')
                axes[0][1].set_ylabel('Humidity (%)')
                # 3
                axes[0][2].plot(x, p)
                axes[0][2].set_xlabel('Time')
                axes[0][2].set_ylabel('Pressure (mBar)')
                # 4
                ax = axes[1][0]
                for i in range(len(label)):
                    ax.plot(x, a[i], label=label[i])
                    ax.set_xlabel('Time')
                    ax.set_ylabel('Accelerometer')
                    # avoid duplicating legend
                    handles, labels = plt.gca().get_legend_handles_labels()
                    by_label = dict(zip(labels, handles))
                    ax.legend(by_label.values(), by_label.keys())
                # 5
                ax = axes[1][1]
                for i in range(len(label)):
                    ax.plot(x, g[i], label=label[i])
                    ax.set_xlabel('Time')
                    ax.set_ylabel('Gyroscope')
                    # avoid duplicating legend
                    handles, labels = plt.gca().get_legend_handles_labels()
                    by_label = dict(zip(labels, handles))
                    ax.legend(by_label.values(), by_label.keys())
                # 6
                ax = axes[1][2]
                for i in range(len(label)):
                    ax.plot(x, m[i], label=label[i])
                    ax.set_xlabel('Time')
                    ax.set_ylabel('Magnetometer')
                    # avoid duplicating legend
                    handles, labels = plt.gca().get_legend_handles_labels()
                    by_label = dict(zip(labels, handles))
                    ax.legend(by_label.values(), by_label.keys())
                plt.pause(0.001)
                time.sleep(1)
            plt.show()
            plt.savefig('image.png')
    except KeyboardInterrupt:
       print('Interrupted')
       plt.savefig('image.png')
       try:
           sys.exit(1)
       except SystemExit:
           os._exit(1)