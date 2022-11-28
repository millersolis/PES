from asyncio import sleep
import tkinter as tk
import can
import time
import threading
import sys
import os
from tkinter import font

import json

sys.path.append(os.path.join(os.path.dirname(__file__), '../DB'))
import create_sqlite_db

OUTPUTPATH = '../DB/sqlite_dbs'
SQLITE_FILENAME = 'echo_db.db'
# Assumme PDM public key is b06188d2-65d6-4ffb-b9b1-578b0e35e002
MOTORCYCLE_PDM = 'b06188d2-65d6-4ffb-b9b1-578b0e35e002'

# States
WAKEUP = 1
START = 2
LOCK = 3
PANIC = 4


sqlite_db_path = os.path.join(OUTPUTPATH, SQLITE_FILENAME)

class Database:
    def get_enrollment_table(self, sqlite_db_path, motorcycle_pdm):

        conn = create_sqlite_db.create_connection(sqlite_db_path)

        with conn:

            json_enrollment_table = create_sqlite_db.get_enrollment_table(conn, motorcycle_pdm)

            return json_enrollment_table


class SenderReceiver:

    # This function was for testing - can be deleted
    def send_can_message(self):
        # Code to format can message data here
        # msg = can.Message(arbitration_id=0xffff, data=[10, 1, 0, 1, 3, 1, 4, 1], is_extended_id=False)

        testJson = {"name" : "GeeksforGeeks", "Topic" : "Json to String", "Method": 1}
        data1 = json.dumps(testJson).encode('utf-8')
        packet = [data1[i:i+8] for i in range (0, len(data1), 8)]
        
        num_of_msgs = len(packet)
        msg = can.Message(arbitration_id=0x101, data=[num_of_msgs], is_extended_id=False)
        self.bus.send(msg)

        arbitrationCounter = 0
        for i in range(0, len(packet)):
            print(f"Packet: {packet[i]}, arbitrationCounter: {arbitrationCounter}")
            msg = can.Message(arbitration_id=arbitrationCounter, data=packet[i], is_extended_id=False)
            self.bus.send(msg)
            arbitrationCounter += 1
            time.sleep(0.5)

        eof = ('eof' + str(len(data1)%8)).encode('utf-8')

        arbitrationCounter += 1
        msg = can.Message(arbitration_id=arbitrationCounter, data=eof, is_extended_id=False)
        self.bus.send(msg)

        # print(type(len(data)%8))

    def start_motorcycle(self, app):
        if self.current_state == WAKEUP:
            app.change_state_label(START)

    def send_enrollment_table_to_pdm(self, db):
        # Code to format JSON response from DB here
        json_enrollment_table = db.get_enrollment_table(sqlite_db_path, MOTORCYCLE_PDM)
        parsed_json_enrollment_table = json.loads(json_enrollment_table)

        # print(parsed_json_enrollment_table)
        
        byte_arr_enrollment_table = json.dumps(parsed_json_enrollment_table).encode('utf-8')
        packets = [byte_arr_enrollment_table[i:i+8] for i in range (0, len(byte_arr_enrollment_table), 8)]
        
        num_of_msgs = len(packets)
        msg = can.Message(arbitration_id=0x101, data=[num_of_msgs], is_extended_id=False)
        self.bus.send(msg)

        arbitrationCounter = 0
        for i in range(0, len(packets)):
            # print(f"Packet: {packets[i]}, arbitrationCounter: {arbitrationCounter}")
            msg = can.Message(arbitration_id=arbitrationCounter, data=packets[i], is_extended_id=False)
            self.bus.send(msg)
            arbitrationCounter += 1
            time.sleep(0.5)

    def recv_can_message(self, app):
        while True:
            message = self.bus.recv()
            print("CAN msg received")
            # Code to interpret received message data and call proper function here
            if message.data[0] == 1:
                state_received = message.data[1]
                self.current_state = state_received
                app.change_state_label(state_received)
            else:
                pass # TODO: Send enrollment table


    def __init__(self):
        # Note that channel will change depending on the port
        # bus = can.interface.Bus(bustype='seeedstudio', channel='com4', bitrate=500000)
        # bus = can.Bus(bustype='seeedstudio', channel='com4', bitrate=500000, operation_mode='normal')
        self.bus = can.Bus(bustype='seeedstudio', bitrate=500000, channel='com12', operation_mode='normal',frame_type='STD')
        self.current_state = LOCK   # bike is locked by default

class App(tk.Tk):

    def __init__(self, sender):
        super().__init__()

        self.db = Database()

        self.geometry('1080x900')
        # self.columnconfigure([0,1], weight=1, minsize=75)
        self.columnconfigure([0], weight=1, minsize=75)
        self.rowconfigure([0,1,2], weight=1, minsize=50)

        self.create_widgets(sender)
        self.panic_on = False

        self.change_state_label(sender.current_state)
        print ("starting Simulated ECU...")

    def create_widgets(self, sender):
        title_label = tk.Label(text='PES™ Simulated ECU', font=font.Font(weight='bold'))
        # title_label.grid(row=0, column=0, columnspan=2)
        title_label.grid(row=0, column=0, columnspan=1)

        states_frame = tk.Frame()
        states_frame.grid(row=1, column=0, rowspan=2)

        states_title_label = tk.Label(master=states_frame, text='Bike state:')
        states_title_label.pack()

        self.wakeup_label = tk.Label(master=states_frame, text='WAKE-UP', fg='black',bg='#C8DAFF', width=45, height=12)
        self.wakeup_label.pack()

        self.start_label = tk.Label(master=states_frame, text='START', fg='black', bg='#cffcd2', width=45, height=12)
        self.start_label.pack()

        self.lock_label = tk.Label(master=states_frame, text='LOCK', fg='black', bg='#e2c3b8', width=45, height=12)
        self.lock_label.pack()

        self.panic_label = tk.Label(text='PANIC', fg='white', bg='#FFF9E0', width=65, height=35)
        self.panic_label.grid(row=2, column=1)

        self.start_button = tk.Button(text='Start Switch', fg='black', bg='#909090', width=20, height=3, command=lambda: sender.start_motorcycle(self))
        self.start_button.grid(row=1, column=1)


    def change_state_label(self, state):

        # state values can be changed later
        if state == WAKEUP:
            print("got WAKEUP")
            self.wakeup_label.configure(bg='#1B5DF7', fg='white', font=font.Font(weight='bold'))
            self.start_label.configure(bg='#cffcd2', font=font.Font(weight='normal'))
            self.lock_label.configure(bg='#e2c3b8', font=font.Font(weight='normal'))

            self.start_button.configure(bg='black', fg='white', font=font.Font(weight='bold'))
        elif state == START:
            print("got START")
            self.wakeup_label.configure(bg='#C8DAFF', font=font.Font(weight='normal'))
            self.start_label.configure(bg='green', fg='white', font=font.Font(weight='bold'))
            self.lock_label.configure(bg='#e2c3b8', font=font.Font(weight='normal'))

            self.start_button.configure(bg='black', fg='white', font=font.Font(weight='bold'))
        elif state == LOCK:
            print("got LOCK")
            self.wakeup_label.configure(bg='#C8DAFF', font=font.Font(weight='normal'))
            self.start_label.configure(bg='#cffcd2', font=font.Font(weight='normal'))
            self.lock_label.configure(bg='red', fg='white', font=font.Font(weight='bold'))

            self.start_button.configure(bg='#909090', font=font.Font(weight='normal'))
        elif state == PANIC:
            print("got PANIC")
            if (not self.panic_on):
                self.panic_label.configure(bg='#FFC128', fg='black', font=font.Font(weight='bold'))
                self.panic_on = True
            else:
                self.panic_label.configure(fg='white', bg='#FFF9E0', font=font.Font(weight='normal'))
                self.panic_on = False
        else:
            print("got UNKNOWN message")


if __name__ == "__main__":
    sender = SenderReceiver()
    app = App(sender)
    x = threading.Thread(target=sender.recv_can_message, args=(app,), daemon=True)
    x.start()
    app.mainloop()


