from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate
import time

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print("Discovered device", dev.addr)
        elif isNewData:
            print("Received new data from", dev.addr)
    def handleNotification(self, cHandle, data):
        print(str(data))

def setup_notify(dev, target_uuid):
    setup_data = b"\x01\x00"
    notify = dev.getCharacteristics(uuid=target_uuid)[0]
    # write to the CCCD, not the characteristic itself
    notify_handle = notify.getHandle() + 1
    dev.writeCharacteristic(notify_handle, setup_data, withResponse=True)

scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(10.0)
n = 0
for dev in devices:
    print("%d: Device %s (%s), RSSI=%d dB" % (n, dev.addr, dev.addrType, dev.rssi))
    n += 1
    for(adtype, desc, value) in dev.getScanData():
        print(" %s = %s" % (desc, value))

number = input('Enter your device number: ')
print('Device', number)
print(devices[number].addr)

print("Connecting...")
dev = Peripheral(devices[number].addr, 'random')

print("Services...")
for svc in dev.services:
    print(str(svc))

try:
    # testService = dev.getServiceByUUID(UUID(0xfff0))
    # for ch in testService.getCharacteristics():
    #    print(str(ch))
    # Student ID service
    ch = dev.getCharacteristics(uuid=UUID(0xA005))[0]
    if (ch.supportsRead()):
        print("Student ID: %s" % ch.read())
    
    # Button Service
    for i in range(5):
        setup_notify(dev, 0xA001)
        if dev.waitForNotifications(5.0):
            # handleNotification() will be called here
            print("Notification: button pressed")
            continue

        print("Waiting...")

    # LED Service
    ch = dev.getCharacteristics(uuid=UUID(0xA003))[0]
    for i in range(5):
        ch.write(b"\x01", withResponse=True)
        time.sleep(2)
        ch.write(b"\x00", withResponse=True)
        time.sleep(2)

finally:
    dev.disconnect()