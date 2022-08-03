import tkinter as tk
import can
import time
import threading


class SenderReceiver:

    # This function is binded to the press to start button rn
    def send_can_message(self):
        # Code to format can message data here
        msg = can.Message(arbitration_id=0xffff, data=[10, 1, 0, 1, 3, 1, 4, 1], is_extended_id=False)
        self.bus.send(msg)

    def send_enrollment_table_to_pdm(self):
        # Code to format JSON response from DB here
        msg = can.Message(arbitration_id=0xffff, data=[10, 1, 0, 1, 3, 1, 4, 1], is_extended_id=False)
        self.bus.send(msg)
       
    def recv_can_message(self, app):
        while True:
            message = self.bus.recv()

            # Code to interpret received message data and call proper function here
            app.change_state_label()
            
    def __init__(self):
        # Note that channel will change depending on the port
        # bus = can.interface.Bus(bustype='seeedstudio', channel='com4', bitrate=500000)
        # bus = can.Bus(bustype='seeedstudio', channel='com4', bitrate=500000, operation_mode='normal')
        self.bus = can.Bus(bustype='seeedstudio', channel='com10', bitrate=500000, operation_mode='normal',frame_type='STD')
       
  
class App(tk.Tk):
  
    def __init__(self, sender):   
        super().__init__()  
        
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

        self.start_button = tk.Button(text='Send enrollment table', fg='black', bg='green', width=40, height=3, command=sender.send_can_message)      
        self.start_button.grid(row=1, column=1)
        
        # Not sure if a text box should be used for the DB part but putting it here for now
        self.cmd_text_box = tk.Text()
        self.cmd_text_box.grid(row=2, column=1)
        

    def change_state_label(self):
        # Change bike state labels here
        self.wakeup_label.configure(bg='yellow')

  
if __name__ == "__main__":
    sender = SenderReceiver()
    app = App(sender)
    x = threading.Thread(target=sender.recv_can_message, args=(app,), daemon=True)
    x.start()
    app.mainloop()
        

