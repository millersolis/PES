import tkinter as tk
import can
import time
import threading
import sys
import os
from tkinter import font

sys.path.append(os.path.join(os.path.dirname(__file__), '../DB'))
import create_sqlite_db

OUTPUTPATH = '../DB/sqlite_dbs'
SQLITE_FILENAME = 'echo_db.db'
# Assumme PDM public key is b06188d2-65d6-4ffb-b9b1-578b0e35e002
MOTORCYCLE_PDM = 'b06188d2-65d6-4ffb-b9b1-578b0e35e002'

sqlite_db_path = os.path.join(OUTPUTPATH, SQLITE_FILENAME)

class Database:
    def get_enrollment_table(self, sqlite_db_path, motorcycle_pdm):

        conn = create_sqlite_db.create_connection(sqlite_db_path)

        with conn:

            json_enrollment_table = create_sqlite_db.get_enrollment_table(conn, motorcycle_pdm)

            return json_enrollment_table


class SenderReceiver:

    # This function is binded to the press to start button rn
    def send_can_message(self):
        # Code to format can message data here
        msg = can.Message(arbitration_id=0xffff, data=[10, 1, 0, 1, 3, 1, 4, 1], is_extended_id=False)
        self.bus.send(msg)

    def send_enrollment_table_to_pdm(self, db):
        # Code to format JSON response from DB here
        json_enrollment_table = db.get_enrollment_table(sqlite_db_path, MOTORCYCLE_PDM)
        print(json_enrollment_table)


        msg = can.Message(arbitration_id=0xffff, data=[10, 1, 0, 1, 3, 1, 4, 1], is_extended_id=False)
        self.bus.send(msg)

    def recv_can_message(self, app):
        while True:
            message = self.bus.recv()

            # Code to interpret received message data and call proper function here
            if message.data[0] == 1:
                state = message.data[1]
                app.change_state_label(state)
            else:
                pass # TODO: Send enrollment table

    def __init__(self):
        # Note that channel will change depending on the port
        # bus = can.interface.Bus(bustype='seeedstudio', channel='com4', bitrate=500000)
        # bus = can.Bus(bustype='seeedstudio', channel='com4', bitrate=500000, operation_mode='normal')
        self.bus = can.Bus(bustype='seeedstudio', channel='com10', bitrate=500000, operation_mode='normal',frame_type='STD')


class App(tk.Tk):

    def __init__(self, sender):
        super().__init__()

        self.db = Database()

        self.geometry('900x600')
        self.columnconfigure([0,1], weight=1, minsize=75)
        self.rowconfigure([0,1,2], weight=1, minsize=50)

        self.create_widgets(sender)

    def create_widgets(self, sender):
        title_label = tk.Label(text='Echos Proximity Entrance System Simulated ECU')
        title_label.grid(row=0, column=0, columnspan=2)

        states_frame = tk.Frame()
        states_frame.grid(row=1, column=0, rowspan=2)

        states_title_label = tk.Label(master=states_frame, text='Bike state:')
        states_title_label.pack()

        self.wakeup_label = tk.Label(master=states_frame, text='WAKE-UP', fg='black', bg='#fcfccf', width=25, height=8)
        self.wakeup_label.pack()

        self.start_label = tk.Label(master=states_frame, text='START', fg='black', bg='#cffcd2', width=25, height=8)
        self.start_label.pack()

        self.lock_label = tk.Label(master=states_frame, text='STOP', fg='black', bg='#e2c3b8', width=25, height=8)
        self.lock_label.pack()

        self.start_button = tk.Button(text='Send enrollment table', fg='black', bg='green', width=40, height=3, command=lambda: sender.send_enrollment_table_to_pdm(self.db))
        self.start_button.grid(row=1, column=1)

        # Not sure if a text box should be used for the DB part but putting it here for now
        self.cmd_text_box = tk.Text()
        self.cmd_text_box.grid(row=2, column=1)


    def change_state_label(self, state):

        # state values can be changed later
        if state == 1:
            self.wakeup_label.configure(bg='yellow', font=font.Font(weight='bold'))
            self.start_label.configure(bg='#cffcd2', font=font.Font(weight='normal'))
            self.lock_label.configure(bg='#e2c3b8', font=font.Font(weight='normal'))

            self.start_button.configure(bg='green', font=font.Font(weight='bold'))
        elif state == 2:
            self.wakeup_label.configure(bg='#fcfccf', font=font.Font(weight='normal'))
            self.start_label.configure(bg='green', font=font.Font(weight='bold'))
            self.lock_label.configure(bg='#e2c3b8', font=font.Font(weight='normal'))

            self.start_button.configure(bg='#cffcd2', font=font.Font(weight='normal'))
        else:
            self.wakeup_label.configure(bg='#fcfccf', font=font.Font(weight='normal'))
            self.start_label.configure(bg='#cffcd2', font=font.Font(weight='normal'))
            self.lock_label.configure(bg='red', font=font.Font(weight='bold'))

            self.start_button.configure(bg='#cffcd2', font=font.Font(weight='normal'))


if __name__ == "__main__":
    sender = SenderReceiver()
    app = App(sender)
    x = threading.Thread(target=sender.recv_can_message, args=(app,), daemon=True)
    x.start()
    app.mainloop()


