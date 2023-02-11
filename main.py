import random
import time
import tkinter
import tkinter.messagebox
import customtkinter as ctk
from serial.tools.list_ports import comports
import serial
import trio
import logging
from rich.logging import RichHandler
import json

ctk.set_appearance_mode("System")  # Modes: "System" (standard), "Dark", "Light"
ctk.set_default_color_theme("blue")  # Themes: "blue" (standard), "green", "dark-blue"


class ValueWithUnits(ctk.CTkFrame):
    def __init__(self, root, label, value, units, label_unit_width, value_width, font):
        super().__init__(root)
        self.data_format = '{:1.3f}'
        self.label_label = ctk.CTkLabel(self, text=label, width=label_unit_width, font=font)
        self.label_label.grid(row=1, column=0, sticky="nw")
        self.value_label = ctk.CTkLabel(self, text=value, width=value_width, font=font)
        self.value_label.grid(row=1, column=1, sticky="ne")
        self.units_label = ctk.CTkLabel(self, text=units, width=label_unit_width, font=font)
        self.units_label.grid(row=1, column=2, sticky="nw")

    def set_data_format(self, fmt: str):
        self.data_format = fmt

    def set_value_factory(self):
        def set_value(value):
            self.value_label.configure(text=self.data_format.format(value))
        return set_value

    def get_value_factory(self):
        def get_value():
            return self.value_label.cget('text')
        return get_value

    def get_set_factory(self):
        return [self.get_value_factory(), self.set_value_factory()]


class EntryWithButtons(ctk.CTkFrame):
    # def __init__(self, root, label, value, units, label_unit_width, value_width, font):
    def __init__(self, root, font, format='{:3.1f}', default=0):
        super().__init__(root)
        self.grid_rowconfigure(0, weight=2)
        self.grid_columnconfigure(0, weight=2)
        self.grid_columnconfigure(1, weight=1)
        self.grid_columnconfigure(2, weight=1)

        # x, y, width, height = self.grid_bbox(2, 1)
        width = font.cget("size")
        self.dec_value_button = ctk.CTkButton(self, text='-', width=width*1.5, command=self.dec_callback)
        self.dec_value_button.grid(row=2, column=1, pady=(0,5), sticky="nsew")

        self.value = ctk.CTkEntry(self, width=width*3)
        self.value.insert(0, str(format.format(default)))
        self.value.grid(row=2, column=2, pady=(0,5), sticky="nsew")

        self.inc_value_button = ctk.CTkButton(self, text='+', width=width*1.5, command=self.inc_callback)
        self.inc_value_button.grid(row=2, column=3, padx=(0,5), pady=(0,5), sticky="nsew")


    def get_value(self) -> float:
        return float(self.value.get())

    def inc_callback(self):
        value = float(self.value.get())
        if value < 100:
            self.value.delete(0, 10)
            self.value.insert(0, value+1)

    def dec_callback(self):
        value = float(self.value.get())
        if value > 0:
            self.value.delete(0, 10)
            self.value.insert(0, value-1)

class App(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.link = serial.Serial
        self.connected = False

        self.header_0_font = ctk.CTkFont(size=48, weight='bold')
        self.header_1_font = ctk.CTkFont(size=24, weight='bold')
        self.header_2_font = ctk.CTkFont(size=16, weight='bold')

        self.small_pad = 10
        self.large_pad = 20

        # configure window
        self.title("CustomTkinter complex_example.py")
        self.geometry(f"{1100}x{580}")

        # configure grid layout (4x4)
        self.grid_columnconfigure(1, weight=1)
        self.grid_columnconfigure((2, 3), weight=0)
        self.grid_rowconfigure((0, 1, 2), weight=1)

        # create sidebar frame with widgets
        self.sidebar_frame = ctk.CTkFrame(self, width=140, corner_radius=0)
        self.sidebar_frame.grid(row=0, column=0, rowspan=4, sticky="nsew")
        self.sidebar_frame.grid_rowconfigure(4, weight=1)
        self.serial_label = ctk.CTkLabel(self.sidebar_frame, text="Serial Port", font=self.header_1_font)
        self.serial_label.grid(row=0, column=0, padx=20, pady=(20, 10))
        self.serial_port_optionmenu = ctk.CTkOptionMenu(self.sidebar_frame, dynamic_resizing=False, values=["Port"])
        self.serial_port_optionmenu.grid(row=1, column=0, padx=20, pady=10)
        self.serial_port_connect_button = ctk.CTkButton(self.sidebar_frame, text='Connect', command=self.sidebar_button_event)
        self.serial_port_connect_button.grid(row=2, column=0, padx=20, pady=10)
        self.appearance_mode_label = ctk.CTkLabel(self.sidebar_frame, text="Appearance Mode:", anchor="w")
        self.appearance_mode_label.grid(row=5, column=0, padx=20, pady=(10, 0))
        self.appearance_mode_optionemenu = ctk.CTkOptionMenu(self.sidebar_frame, values=["Light", "Dark", "System"], command=self.change_appearance_mode_event)
        self.appearance_mode_optionemenu.grid(row=6, column=0, padx=20, pady=(10, 10))
        self.scaling_label = ctk.CTkLabel(self.sidebar_frame, text="UI Scaling:", anchor="w")
        self.scaling_label.grid(row=7, column=0, padx=20, pady=(10, 0))
        self.scaling_optionemenu = ctk.CTkOptionMenu(self.sidebar_frame, values=["80%", "90%", "100%", "110%", "120%"], command=self.change_scaling_event)
        self.scaling_optionemenu.grid(row=8, column=0, padx=20, pady=(10, 20))

        # Create an area for input and output information to be displayed
        # create slider and progressbar frame
        self.status_frame = ctk.CTkFrame(self, fg_color="black", corner_radius=5)
        self.status_frame.grid(row=0, column=1, columnspan=2, padx=(20, 20), pady=(20, 20), sticky="nsew")
        # self.status_frame.grid(row=1, column=1, sticky="nsew")
        self.status_frame.grid_columnconfigure(4, weight=1)
        self.status_frame.grid_rowconfigure(4, weight=1)
        self.in_label = ctk.CTkLabel(self.status_frame, text="IN", font=self.header_0_font).grid(row=0, column=0, columnspan=1, sticky="nw")
        self.out_label = ctk.CTkLabel(self.status_frame, text="OUT", font=self.header_0_font).grid(row=0, column=1, columnspan=1, sticky="nw")
        self.out_label = ctk.CTkLabel(self.status_frame, text="OTHER", font=self.header_0_font).grid(row=0, column=2, columnspan=1, sticky="nw")

        self.status_frame.grid_columnconfigure(0, pad=self.small_pad)
        self.status_frame.grid_columnconfigure(1, pad=self.small_pad)
        self.status_frame.grid_rowconfigure(1, pad=self.small_pad)
        self.status_frame.grid_rowconfigure(2, pad=self.small_pad)
        self.status_frame.grid_rowconfigure(3, pad=self.small_pad)
        self.v_in_frame = ValueWithUnits(self.status_frame, label="V:  ", value="...", units="  V", label_unit_width=self.header_1_font.cget('size')*2, value_width=40, font=self.header_1_font)
        self.v_in_frame.grid(row=1, column=0, sticky="nw")
        self.get_v_in, self.set_v_in = self.v_in_frame.get_set_factory()
        self.i_in_frame = ValueWithUnits(self.status_frame, label="I:  ", value="...", units="  A", label_unit_width=self.header_1_font.cget('size')*2, value_width=40, font=self.header_1_font)
        self.i_in_frame.grid(row=2, column=0, sticky="nw")
        self.get_i_in, self.set_i_in = self.i_in_frame.get_set_factory()
        self.p_in_frame = ValueWithUnits(self.status_frame, label="P:  ", value="...", units="  W", label_unit_width=self.header_1_font.cget('size')*2, value_width=40, font=self.header_1_font)
        self.p_in_frame.grid(row=3, column=0, sticky="nw")
        self.get_p_in, self.set_p_in = self.p_in_frame.get_set_factory()

        self.v_out_frame = ValueWithUnits(self.status_frame, label="V:  ", value="...", units="  V", label_unit_width=self.header_1_font.cget('size')*2, value_width=40, font=self.header_1_font)
        self.v_out_frame.grid(row=1, column=1, sticky="nw")
        self.get_v_out, self.set_v_out = self.v_out_frame.get_set_factory()
        self.i_out_frame = ValueWithUnits(self.status_frame, label="I:  ", value="...", units="  A", label_unit_width=self.header_1_font.cget('size')*2, value_width=40, font=self.header_1_font)
        self.i_out_frame.grid(row=2, column=1, sticky="nw")
        self.get_i_out, self.set_i_out = self.i_out_frame.get_set_factory()
        self.p_out_frame = ValueWithUnits(self.status_frame, label="P:  ", value="...", units="  W", label_unit_width=self.header_1_font.cget('size')*2, value_width=40, font=self.header_1_font)
        self.p_out_frame.grid(row=3, column=1, sticky="nw")
        self.get_p_out, self.set_p_out = self.p_out_frame.get_set_factory()

        self.e_frame = ValueWithUnits(self.status_frame, label="E:  ", value="...", units="  %", label_unit_width=self.header_1_font.cget('size')*2, value_width=40, font=self.header_1_font)
        self.e_frame.grid(row=1, column=2, sticky="nw")
        self.e_frame.set_data_format('{:2.2f}')
        self.get_e, self.set_e = self.e_frame.get_set_factory()
        self.m_frame = ValueWithUnits(self.status_frame, label="M:  ", value="...", units="  X", label_unit_width=self.header_1_font.cget('size')*2, value_width=40, font=self.header_1_font)
        self.m_frame.grid(row=2, column=2, sticky="nw")
        self.get_m, self.set_m = self.m_frame.get_set_factory()
        self.a_frame = ValueWithUnits(self.status_frame, label="A:  ", value="...", units="  %", label_unit_width=self.header_1_font.cget('size')*2, value_width=40, font=self.header_1_font)
        self.a_frame.grid(row=3, column=2, sticky="nw")
        self.get_a, self.set_a = self.a_frame.get_set_factory()
        self.a_frame.set_data_format('{:2.2f}')

        self.duty_frame = EntryWithButtons(self.status_frame, font=self.header_1_font, default=100.00)
        self.duty_frame.grid(row = 1, column=3, sticky="nw")


    def open_input_dialog_event(self):
        dialog = ctk.CTkInputDialog(text="Type in a number:", title="CTkInputDialog")
        print("CTkInputDialog:", dialog.get_input())

    def change_appearance_mode_event(self, new_appearance_mode: str):
        ctk.set_appearance_mode(new_appearance_mode)

    def change_scaling_event(self, new_scaling: str):
        new_scaling_float = int(new_scaling.replace("%", "")) / 100
        ctk.set_widget_scaling(new_scaling_float)

    def sidebar_button_event(self):
        option = self.serial_port_optionmenu.get()
        if not option == 'Port':
            # if not self.connected:
            port_name = option.split(' ')[0]
            # self.link = serial.Serial(port="COM13")
            self.link = serial.Serial(port=port_name)
            self.link.reset_input_buffer()
            self.connected = True
                # self.serial_port_connect_button.configure(text='Disconnect')
            # else:
            #     self.link.close()
            #     self.connected = False
            #     self.serial_port_connect_button.configure(text='Connect')


app = App()
# logging.basicConfig(level=logging.DEBUG)
FORMAT = "%(message)s"
logging.basicConfig(
    level="NOTSET", format=FORMAT, datefmt="[%X]", handlers=[RichHandler()]
)

log = logging.getLogger("rich")


# class SerialJSON:
#     def __init__(self):
#         self.com
#
#     def open(self):
#         self.open()
#
#     # def update(self):
#
#     # def rx_json(self):
#
#
# sjson = SerialJSON(port="COM5")
# # sjson.open()


async def rx_json(send_channel: trio.MemorySendChannel):
    while True:
        if app.connected and app.link.in_waiting > 0:
            data = app.link.readline().decode("utf-8")
            try:
                json_object = json.loads(data)
                await send_channel.send(json_object)
            except json.JSONDecodeError as e:
                log.error(f"JSON Decode error: {e}")
        await trio.sleep(1/60)


async def tx_json(rec_channel: trio.MemoryReceiveChannel):
    while True:
        if app.connected:
            async for json_data in rec_channel:
                app.link.write((json.dumps(json_data) + '\r').encode("ascii"))
        await trio.sleep(1/60)


async def generate_transmit_json(send_channel: trio.MemorySendChannel):
    state = True
    while True:
        # await rec_channel.receive()
        # async for json_data in rec_channel:
        doc = {"LED": state}
        state = not state
        await send_channel.send(doc)
        # doc["SMPS"]["Duty"] = app.get_duty()
        # doc["SMPS"]["DeadBand"] = app.get_dead_band()
        # doc["SMPS"]["Frequency"] = app.get_frequency()
        # doc_str = json.dumps(doc)
        await trio.sleep(1)


async def updated_from_json(rec_channel: trio.MemoryReceiveChannel):
    while True:
        async for json_data in rec_channel:
            print(json_data)
            # json_formatted_str = json.dumps(json_data, indent=2)
            # log.error(json_formatted_str)
            # app.set_v_in(json_data['in']['volts'])
            # app.set_i_in(json_data['in']['amps'])
            # app.set_p_in(float(app.get_v_in()) * float(app.get_i_in()))
            #
            # v_out = random.randrange(1, 6) + random.random()
            # i_out = random.random()
            # # app.set_v_out(data['out']['volts'])
            # app.set_v_out(v_out)
            # # app.set_i_out(data['out']['amps'])
            # app.set_i_out(i_out)
            # app.set_p_out(float(app.get_v_out()) * float(app.get_i_out()))
            #
            # app.set_m(float(app.get_v_out()) / float(app.get_v_in()))
            # app.set_e(float(app.get_p_out()) / float(app.get_p_in()) * 100)
            await trio.sleep(1)


async def get_com_ports(send_channel: trio.MemorySendChannel):
    while True:
        await send_channel.send(comports())
        await trio.sleep(0.5)


async def update_com_ports(rec_channel: trio.MemoryReceiveChannel):
    last_com_ports = []  # Used to keep track of if the port list has changed
    while True:
        async for com_port in rec_channel:
            ports = [str(port_name) for port_name in com_port]  # Get a list of the port names as strings
            if not ports == last_com_ports:  # If the ports have changed
                log.debug(f'Serial port list changed, new ports: {ports}')
                app.serial_port_optionmenu.configure(values=ports)  # Change the option menu to reflect the new ports
                last_com_ports = ports  # Update the last ports for checking in the future
        await trio.sleep(0.25)  # Only check for new ports every second


async def tkloop():
    app.update()
    while True:
        # if 'normal' == app.state():
        try:
            app.update()  # Update all the TKinter widgets
        except:
            exit(0)
        await trio.sleep(1 / 60)  # Run at 60 FPS


async def main():
    async with trio.open_nursery() as nursery:
        send_channel, receive_channel = trio.open_memory_channel(0)
        json_send_channel, json_receive_channel = trio.open_memory_channel(0)
        json_tx_send_channel, json_tx_receive_channel = trio.open_memory_channel(0)
        nursery.start_soon(update_com_ports, receive_channel)
        nursery.start_soon(get_com_ports, send_channel)
        nursery.start_soon(tkloop)
        nursery.start_soon(rx_json, json_send_channel)
        nursery.start_soon(generate_transmit_json, json_tx_send_channel)
        nursery.start_soon(tx_json, json_tx_receive_channel)
        nursery.start_soon(updated_from_json, json_receive_channel)
        # Duty cycle adjustment, dead band adjustment, VIO/IIO, Protection Control, LED heartbeat, ESTOP (set duty cycle to limits)


if __name__ == "__main__":
    trio.run(main)

# self.seg_button_1 = ctk.CTkSegmentedButton(self.slider_progressbar_frame)
# self.seg_button_1.grid(row=0, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
# self.progressbar_1 = ctk.CTkProgressBar(self.slider_progressbar_frame)
# self.progressbar_1.grid(row=1, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
# self.progressbar_2 = ctk.CTkProgressBar(self.slider_progressbar_frame)
# self.progressbar_2.grid(row=2, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
# self.slider_1 = ctk.CTkSlider(self.slider_progressbar_frame, from_=0, to=1, number_of_steps=4)
# self.slider_1.grid(row=3, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
# self.slider_2 = ctk.CTkSlider(self.slider_progressbar_frame, orientation="vertical")
# self.slider_2.grid(row=0, column=1, rowspan=5, padx=(10, 10), pady=(10, 10), sticky="ns")
# self.progressbar_3 = ctk.CTkProgressBar(self.slider_progressbar_frame, orientation="vertical")
# self.progressbar_3.grid(row=0, column=2, rowspan=5, padx=(10, 20), pady=(10, 10), sticky="ns")

# # create main entry and button
# self.entry = ctk.CTkEntry(self, placeholder_text="CTkEntry")
# self.entry.grid(row=3, column=1, columnspan=2, padx=(20, 0), pady=(20, 20), sticky="nsew")
#
# self.main_button_1 = ctk.CTkButton(master=self, fg_color="transparent", border_width=2, text_color=("gray10", "#DCE4EE"))
# self.main_button_1.grid(row=3, column=3, padx=(20, 20), pady=(20, 20), sticky="nsew")

# # create textbox
# self.textbox = ctk.CTkTextbox(self, width=250)
# self.textbox.grid(row=0, column=1, padx=(20, 0), pady=(20, 0), sticky="nsew")

# # create tabview
# self.tabview = ctk.CTkTabview(self, width=250)
# self.tabview.grid(row=0, column=2, padx=(20, 0), pady=(20, 0), sticky="nsew")
# self.tabview.add("CTkTabview")
# self.tabview.add("Tab 2")
# self.tabview.add("Tab 3")
# self.tabview.tab("CTkTabview").grid_columnconfigure(0, weight=1)  # configure grid of individual tabs
# self.tabview.tab("Tab 2").grid_columnconfigure(0, weight=1)
#
# self.optionmenu_1 = ctk.CTkOptionMenu(self.tabview.tab("CTkTabview"), dynamic_resizing=False,
#                                                 values=["Value 1", "Value 2", "Value Long Long Long"])
# self.optionmenu_1.grid(row=0, column=0, padx=20, pady=(20, 10))
# self.combobox_1 = ctk.CTkComboBox(self.tabview.tab("CTkTabview"),
#                                             values=["Value 1", "Value 2", "Value Long....."])
# self.combobox_1.grid(row=1, column=0, padx=20, pady=(10, 10))
# self.string_input_button = ctk.CTkButton(self.tabview.tab("CTkTabview"), text="Open CTkInputDialog",
#                                                    command=self.open_input_dialog_event)
# self.string_input_button.grid(row=2, column=0, padx=20, pady=(10, 10))
# self.label_tab_2 = ctk.CTkLabel(self.tabview.tab("Tab 2"), text="CTkLabel on Tab 2")
# self.label_tab_2.grid(row=0, column=0, padx=20, pady=20)
#
# # create radiobutton frame
# self.radiobutton_frame = ctk.CTkFrame(self)
# self.radiobutton_frame.grid(row=0, column=3, padx=(20, 20), pady=(20, 0), sticky="nsew")
# self.radio_var = tkinter.IntVar(value=0)
# self.label_radio_group = ctk.CTkLabel(master=self.radiobutton_frame, text="CTkRadioButton Group:")
# self.label_radio_group.grid(row=0, column=2, columnspan=1, padx=10, pady=10, sticky="")
# self.radio_button_1 = ctk.CTkRadioButton(master=self.radiobutton_frame, variable=self.radio_var, value=0)
# self.radio_button_1.grid(row=1, column=2, pady=10, padx=20, sticky="n")
# self.radio_button_2 = ctk.CTkRadioButton(master=self.radiobutton_frame, variable=self.radio_var, value=1)
# self.radio_button_2.grid(row=2, column=2, pady=10, padx=20, sticky="n")
# self.radio_button_3 = ctk.CTkRadioButton(master=self.radiobutton_frame, variable=self.radio_var, value=2)
# self.radio_button_3.grid(row=3, column=2, pady=10, padx=20, sticky="n")
#
# # create checkbox and switch frame
# self.checkbox_slider_frame = ctk.CTkFrame(self)
# self.checkbox_slider_frame.grid(row=1, column=3, padx=(20, 20), pady=(20, 0), sticky="nsew")
# self.checkbox_1 = ctk.CTkCheckBox(master=self.checkbox_slider_frame)
# self.checkbox_1.grid(row=1, column=0, pady=(20, 10), padx=20, sticky="n")
# self.checkbox_2 = ctk.CTkCheckBox(master=self.checkbox_slider_frame)
# self.checkbox_2.grid(row=2, column=0, pady=10, padx=20, sticky="n")
# self.switch_1 = ctk.CTkSwitch(master=self.checkbox_slider_frame, command=lambda: print("switch 1 toggle"))
# self.switch_1.grid(row=3, column=0, pady=10, padx=20, sticky="n")
# self.switch_2 = ctk.CTkSwitch(master=self.checkbox_slider_frame)
# self.switch_2.grid(row=4, column=0, pady=(10, 20), padx=20, sticky="n")
#
# # create slider and progressbar frame
# self.slider_progressbar_frame = ctk.CTkFrame(self, fg_color="transparent")
# self.slider_progressbar_frame.grid(row=1, column=1, columnspan=2, padx=(20, 0), pady=(20, 0), sticky="nsew")
# self.slider_progressbar_frame.grid_columnconfigure(0, weight=1)
# self.slider_progressbar_frame.grid_rowconfigure(4, weight=1)
# self.seg_button_1 = ctk.CTkSegmentedButton(self.slider_progressbar_frame)
# self.seg_button_1.grid(row=0, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
# self.progressbar_1 = ctk.CTkProgressBar(self.slider_progressbar_frame)
# self.progressbar_1.grid(row=1, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
# self.progressbar_2 = ctk.CTkProgressBar(self.slider_progressbar_frame)
# self.progressbar_2.grid(row=2, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
# self.slider_1 = ctk.CTkSlider(self.slider_progressbar_frame, from_=0, to=1, number_of_steps=4)
# self.slider_1.grid(row=3, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
# self.slider_2 = ctk.CTkSlider(self.slider_progressbar_frame, orientation="vertical")
# self.slider_2.grid(row=0, column=1, rowspan=5, padx=(10, 10), pady=(10, 10), sticky="ns")
# self.progressbar_3 = ctk.CTkProgressBar(self.slider_progressbar_frame, orientation="vertical")
# self.progressbar_3.grid(row=0, column=2, rowspan=5, padx=(10, 20), pady=(10, 10), sticky="ns")
#
# # set default values
# # self.sidebar_button_3.configure(state="disabled", text="Disabled CTkButton")
# self.checkbox_2.configure(state="disabled")
# self.switch_2.configure(state="disabled")
# self.checkbox_1.select()
# self.switch_1.select()
# self.radio_button_3.configure(state="disabled")
# self.appearance_mode_optionemenu.set("Dark")
# self.scaling_optionemenu.set("100%")
# self.optionmenu_1.set("CTkOptionmenu")
# self.combobox_1.set("CTkComboBox")
# self.slider_1.configure(command=self.progressbar_2.set)
# self.slider_2.configure(command=self.progressbar_3.set)
# self.progressbar_1.configure(mode="indeterminnate")
# self.progressbar_1.start()
# self.textbox.insert("0.0", "CTkTextbox\n\n" + "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\n\n" * 20)
# self.seg_button_1.configure(values=["CTkSegmentedButton", "Value 2", "Value 3"])
# self.seg_button_1.set("Value 2")