#import serial
import os
import numpy as np
import time

MAX_ENTRIES = 1000

#ser = serial.Serial('/dev/ttyUSB0', 115200)

while True:
    distance = 123 #ser.readline()
    data = np.array([distance,time.time()])
    print(data)