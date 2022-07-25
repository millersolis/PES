# Test for seed studio USB-CAN analyzer using python-can[seeedstudio]
# for Windows

# https://python-can.readthedocs.io/en/master/interfaces/seeedstudio.html
# https://python-can.readthedocs.io/en/master/api.html
# Different interface with more examples: https://python-can.readthedocs.io/en/master/interfaces/socketcan.html
import can
import time
import threading

# Note that channel will change depending on the port
# bus = can.interface.Bus(bustype='seeedstudio', channel='com4', bitrate=500000)
# bus = can.Bus(bustype='seeedstudio', channel='com4', bitrate=500000, operation_mode='normal')
bus = can.Bus(bustype='seeedstudio', channel='com4', bitrate=500000, operation_mode='loopback_and_silent')

def send_test(id):
    """:param id: Spam the bus by sending messages including the data id."""


    for i in range(10):
        msg = can.Message(arbitration_id=0xffff, data=[id, i, 0, 1, 3, 1, 4, 1], is_extended_id=False)
        bus.send(msg)

        time.sleep(1.5)

def recv_test():
    """listener"""

    while True:
        message = bus.recv()
        print(message)


if __name__ == "__main__":
    x = threading.Thread(target=recv_test, daemon=True)
    x.start()
    send_test(10)
