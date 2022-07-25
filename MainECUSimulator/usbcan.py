import can


def send():


    bus = can.interface.Bus(bustype='seeedstudio',
                            channel='/dev/ttyUSB0',
                            bitrate=500000)

    msg = can.Message(arbitration_id=0xC0FFEE, 
                        data=[0, 25, 0, 1, 3, 1, 4, 1],
                        is_extended_id=True)

    try:
        bus.send(msg)
        print(f"Message sent on {bus.channel_info}")
    except can.CanError:
        print("Message Not sent")

if __name__ == "__main__": 
    send()