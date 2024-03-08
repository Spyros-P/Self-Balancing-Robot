"""
Create a program that sends to the controller the operating data
the program should communicate with the controller (an arduino nano)
using the serial port and send the data as a json string
"""

import json
import math
import serial
import time
import pygame
import os
import colorsys
import PySimpleGUI as sg
import matplotlib.pyplot as plt

class Controller:
    def __init__(self, port):
        self.port = port
        self.ser = serial.Serial(port, 57600, timeout=1)
        time.sleep(2)

    def send_data(self, data):
        """
        TODO: fix communication (the arduino does not receive successfully all the data)
        Sometimes the arduino misses some characters
        """
        data = json.dumps(data)
        self.ser.write(bytes(data+'\n', 'utf-8'))
        #self.ser.flush()  # Flush the buffer
    
    def read_data(self):
        if self.ser.in_waiting > 0:
            data = self.ser.readline()
            # TODO: flush the buffer
            #time.sleep(.01)
            #self.ser.reset_input_buffer()

            return json.loads(data)
        else:
            return None

    def close(self):
        self.ser.close()

def get_json_values(values_dict):
    names = [values_dict[i]["name"] for i in range(len(values_dict))]
    values = [values_dict[i]["value"] for i in range(len(values_dict))]
    return dict(zip(names, values))


key_mappings = {
    "Windows": {
        "A": 0,
        "B": 1,
        "X": 2,
        "Y": 3,
        "LB": 4,
        "RB": 5,
        "View": 6,
        "Menu": 7,
        "LJ": 8,
        "RJ": 9,
        "LT": 4,
        "RT": 5
    },
    "Linux": {
        "A": 0,
        "B": 1,
        "X": 3,
        "Y": 4,
        "LB": 6,
        "RB": 7,
        "View": 15,
        "Menu": 11,
        "LJ": 13,
        "RJ": 14,
        "LT": 5,
        "RT": 4
    }
}


if __name__ == "__main__":

    values_dict = {
        0 : {"name": "Kp", "value": 7, "MIN": 0, "MAX": 50, "increase_rate": 0.5, "decrease_rate": 0.5},
        1 : {"name": "Kd", "value": 3, "MIN": 0, "MAX": 10, "increase_rate": 0.1, "decrease_rate": 0.1},
        2 : {"name": "Ki", "value": 3, "MIN": 0, "MAX": 10, "increase_rate": 0.1, "decrease_rate": 0.1},
        3 : {"name": "setpoint", "value": 1, "MIN": -9, "MAX": 11, "increase_rate": 0.01, "decrease_rate": 0.01},
        4 : {"name": "sum_limit", "value": 120, "MIN": 0, "MAX": 100, "increase_rate": 0.1, "decrease_rate": 0.1},
        5 : {"name": "sum_weight", "value": 0.5, "MIN": 0, "MAX": 1, "increase_rate": 0.1, "decrease_rate": 0.1},
        6 : {"name": "exp_dec_sum", "value": 0.6, "MIN": 0, "MAX": 1, "increase_rate": 0.1, "decrease_rate": 0.1},
        7 : {"name": "exp_inc_sum", "value": 1.03, "MIN": 0, "MAX": 1, "increase_rate": 0.1, "decrease_rate": 0.1}
    }
    
    robot_info = {
        "theta_deg": [],
        "gyr": [],
        "acc_x": [],
        "acc_z": [],
        "sum": [],
        "output": []
    }


    changing_value = 0

    num_values = 3 #len(values_dict)

    # Initialize pygame and the joystick
    pygame.init()  # Initialize pygame module
    pygame.joystick.init()  # Initialize joystick module
    try:
        joystick = pygame.joystick.Joystick(0)  # Attempt to get the first joystick
        joystick.init()  # Initialize the first joystick
    except pygame.error:
        print("No joystick found.")
        exit(1)

    # Check the operating system
    if os.name == "nt":
        os_name = "Windows"
    elif os.name == "posix":
        os_name = "Linux"
    else:
        print("Unknown operating system")
        exit(1)

    key_mapping = key_mappings[os_name]    

    controller = Controller("/dev/ttyUSB0")
    #controller = Controller("COM3")

    prev_theta_deg = 0

    # Define the layout of the GUI
    layout = [
        [sg.Text('Sum Limit'), sg.Input(default_text=values_dict[4]['value'], key='sum_limit')],
        [sg.Text('Sum Weight'), sg.Input(default_text=values_dict[5]['value'], key='sum_weight')],
        [sg.Text('Exp Dec Sum'), sg.Input(default_text=values_dict[6]['value'], key='exp_dec_sum')],
        [sg.Text('Exp Inc Sum'), sg.Input(default_text=values_dict[7]['value'], key='exp_inc_sum')],
        # add output text box
        [sg.Text('theta_deg'), sg.Text('0', key='theta_deg')],
        [sg.Text('last_connection'), sg.Text('0', key='last_connection')],
        [sg.Graph(canvas_size=(1000, 1000), graph_bottom_left=(0,0), graph_top_right=(10, 10), background_color='white', key='graph')]
    ]

    window = sg.Window('PID Controller', layout)

    while True:
        event, values = window.read(timeout=10)  # Wait for up to 10 milliseconds for an event
        if event == sg.WINDOW_CLOSED:
            break
        else:
            try:
                values_dict[4]["value"] = float(values['sum_limit'])
            except:
                pass
            try:
                values_dict[5]["value"] = float(values['sum_weight'])
            except:
                pass
            try:
                values_dict[6]["value"] = float(values['exp_dec_sum'])
            except:
                pass
            try:
                values_dict[7]["value"] = float(values['exp_inc_sum'])
            except:
                pass


            for event in pygame.event.get():
                if event.type == pygame.JOYBUTTONDOWN:
                    # If LB button is pressed decrease the changing value
                    if event.button == key_mapping["LB"]:
                        changing_value = (changing_value - 1) % num_values
                    # If RB button is pressed increase the changing value
                    if event.button == key_mapping["RB"]:
                        changing_value = (changing_value + 1) % num_values
                
                    # If A button is pressed
                    if event.button == key_mapping["A"]:
                        # Prompt the user whether they want to save the values
                        save = sg.popup_yes_no("Do you want to save the values?")
                        if save == "Yes":
                            # TODO: Fix issues like pressing the X button
                            while True:
                                # Prompt the user for the file name
                                file_name = sg.popup_get_text("Enter the profile name:")

                                # If the file name is empty, prompt the user again
                                if file_name == "":
                                    continue
                                # If the file name exits, prompt the user whether they want to overwrite it
                                elif file_name+".json" in os.listdir():
                                    overwrite = sg.popup_yes_no("The file already exists. Do you want to overwrite it?")
                                    if overwrite == "Yes":
                                        break
                                # If the file name does not exist, break the loop
                                else:
                                    break

                            # Save the values to a file
                            with open(file_name+'.json', "w") as file:
                                save_dict = get_json_values(values_dict)
                                json.dump(save_dict, file)
                        
            
            val_dict = values_dict[changing_value]

            # Get the RT and LT values
            LT = joystick.get_axis(key_mapping["LT"])
            RT = joystick.get_axis(key_mapping["RT"])

            val_dict["value"] += (RT - LT) * val_dict["increase_rate"]

            if val_dict["value"] > val_dict["MAX"]:
                val_dict["value"] = val_dict["MAX"]
            elif val_dict["value"] < val_dict["MIN"]:
                val_dict["value"] = val_dict["MIN"]

            # use joystick
            values_dict[3]["value"] = (joystick.get_axis(1) + 1) / 2 * (values_dict[3]["MAX"] - values_dict[3]["MIN"]) + values_dict[3]["MIN"]
            # round at 3 decimal places
            values_dict[3]["value"] = round(values_dict[3]["value"], 3)

            # Convert the HSV colors to RGB
            rgb_blue = colorsys.hsv_to_rgb(0.67, 0.5+0.5*int(changing_value==0), 1)  # Hue for blue is 0.67
            rgb_red = colorsys.hsv_to_rgb(0, 0.5+0.5*int(changing_value==1), 1)  # Hue for red is 0
            rgb_green = colorsys.hsv_to_rgb(0.33, 0.5+0.5*int(changing_value==2), 1)  # Hue for green is 0.33
            rgb_4 = colorsys.hsv_to_rgb(0.5, 0.5+0.5*int(changing_value==3), 0.8)

            # Scale the RGB values to 0-255 and convert to integers
            rgb_blue = tuple(int(255 * x) for x in rgb_blue)
            rgb_red = tuple(int(255 * x) for x in rgb_red)
            rgb_green = tuple(int(255 * x) for x in rgb_green)
            rgb_4 = tuple(int(255 * x) for x in rgb_4)

            # Convert the RGB values to hexadecimal color codes
            color_blue = '#%02x%02x%02x' % rgb_blue
            color_red = '#%02x%02x%02x' % rgb_red
            color_green = '#%02x%02x%02x' % rgb_green
            color_4 = '#%02x%02x%02x' % rgb_4


            # Clear the graph
            window['graph'].erase()

            # Draw the bar graph
            window['graph'].draw_rectangle((0.5, 0), (1.5, 10*values_dict[0]["value"]/values_dict[0]["MAX"]), fill_color=color_blue)
            window['graph'].draw_rectangle((2, 0), (3, 10*values_dict[1]["value"]/values_dict[1]["MAX"]), fill_color=color_red)
            window['graph'].draw_rectangle((3.5, 0), (4.5, 10*values_dict[2]["value"]/values_dict[2]["MAX"]), fill_color=color_green)
            window['graph'].draw_rectangle((5, 0), (6, 10*(values_dict[3]["MAX"]-values_dict[3]["value"])/(values_dict[3]["MAX"]-values_dict[3]["MIN"])), fill_color=color_4)
            # Draw the protractor as a semi-circle
            for angle in range(0, 180, 5):
                x = 1.5 * math.cos(math.radians(angle))
                y = 1.5 * math.sin(math.radians(angle))
                window['graph'].draw_point((8+x, y+0.5), size=0.05, color='black')

            # Draw theta_deg as a point on the protractor
            setpoint = values_dict[3]["value"] + 90
            x = 1.5 * math.cos(math.radians(setpoint))
            y = 1.5 * math.sin(math.radians(setpoint))
            window['graph'].draw_point((8+x, y+0.5), size=0.25, color='red')


            controller.send_data(get_json_values(values_dict))

            #pygame.time.wait(10)

            # TODO: fix it
            try:
                controller_data = controller.read_data()                
                # update output text boxes
                angle = round(controller_data['theta_deg'], 2) # keep 2 decimal places
                window['theta_deg'].update(angle)
                
                theta_deg = controller_data['theta_deg'] + 90
                prev_theta_deg = theta_deg
                x = 1.5 * math.cos(math.radians(theta_deg))
                y = 1.5 * math.sin(math.radians(theta_deg))
                window['graph'].draw_point((8+x, y+0.5), size=0.2, color='blue')

            except:
                x = 1.5 * math.cos(math.radians(prev_theta_deg))
                y = 1.5 * math.sin(math.radians(prev_theta_deg))
                window['graph'].draw_point((8+x, y+0.5), size=0.2, color='blue')
            try:
                window['last_connection'].update(controller_data['last_connection'])
            except:
                pass

            #try:
            #    controller_data = controller.read_data()
            #    print(controller_data)
            #except Exception as e:
            #    print(e)

    window.close()
    controller.close()
