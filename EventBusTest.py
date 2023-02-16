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
import event_bus as bus
import SerialJSON

ctk.set_appearance_mode("System")  # Modes: "System" (standard), "Dark", "Light"
ctk.set_default_color_theme("blue")  # Themes: "blue" (standard), "green", "dark-blue"

sjson = SerialJSON.SerialJSON()


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

    def entry_lose_focus_callback(self):
        print(f'Emit: {self.string_var.get()} by {self.emit_message}')
        bus.emit(self.emit_message, self.string_var.get())
        return True

    def __init__(self, root, emit_message: str, font, format='{:3.1f}', num_type=float, default=0):
        super().__init__(root)
        self.emit_message = emit_message
        self.num_type = num_type
        self.grid_rowconfigure(0, weight=2)
        self.grid_columnconfigure(0, weight=2)
        self.grid_columnconfigure(1, weight=1)
        self.grid_columnconfigure(2, weight=1)

        self.string_var = ctk.StringVar()
        self.string_var.set(str(format.format(default)))

        # x, y, width, height = self.grid_bbox(2, 1)
        width = font.cget("size")
        self.dec_value_button = ctk.CTkButton(self, text='-', width=width * 1.5, command=self.dec_callback)
        self.dec_value_button.grid(row=2, column=1, pady=(0, 5), sticky="nsew")

        self.value = ctk.CTkEntry(self, width=width * 3, textvariable=self.string_var, validate="focusout", validatecommand=self.entry_lose_focus_callback)
        # self.value.insert(0, )
        self.value.grid(row=2, column=2, pady=(0, 5), sticky="nsew")

        self.inc_value_button = ctk.CTkButton(self, text='+', width=width * 1.5, command=self.inc_callback)
        self.inc_value_button.grid(row=2, column=3, padx=(0, 5), pady=(0, 5), sticky="nsew")

    def get_value(self):
        return self.num_type(self.value.get())

    def inc_callback(self):
        value = self.num_type(self.value.get())
        if value < 100:
            self.value.delete(0, 10)
            self.value.insert(0, value + 1)
            self.entry_lose_focus_callback()

    def dec_callback(self):
        value = self.num_type(self.value.get())
        if value > 0:
            self.value.delete(0, 10)
            self.value.insert(0, value - 1)
            self.entry_lose_focus_callback()


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
        # self.serial_port_connect_button = ctk.CTkButton(self.sidebar_frame, text='Connect', command=self.sidebar_button_event)
        self.serial_port_connect_button = ctk.CTkButton(self.sidebar_frame, text='Connect', command=lambda: bus.emit('gui:port_connect_button', self))
        self.serial_port_connect_button.grid(row=2, column=0, padx=20, pady=10)
        self.appearance_mode_label = ctk.CTkLabel(self.sidebar_frame, text="Appearance Mode:", anchor="w")
        self.appearance_mode_label.grid(row=5, column=0, padx=20, pady=(10, 0))
        self.appearance_mode_optionemenu = ctk.CTkOptionMenu(self.sidebar_frame, values=["Light", "Dark", "System"],
                                                             command=self.change_appearance_mode_event)
        self.appearance_mode_optionemenu.grid(row=6, column=0, padx=20, pady=(10, 10))
        self.scaling_label = ctk.CTkLabel(self.sidebar_frame, text="UI Scaling:", anchor="w")
        self.scaling_label.grid(row=7, column=0, padx=20, pady=(10, 0))
        self.scaling_optionemenu = ctk.CTkOptionMenu(self.sidebar_frame, values=["80%", "90%", "100%", "110%", "120%"], command=self.change_scaling_event)
        self.scaling_optionemenu.grid(row=8, column=0, padx=20, pady=(10, 20))

        # Create an area for input and output information to be displayed
        # create slider and progressbar frame
        # self.status_frame = ctk.CTkFrame(self, fg_color="black", corner_radius=5)
        self.status_frame = ctk.CTkFrame(self, corner_radius=5)
        self.status_frame.grid(row=0, column=1, columnspan=2, padx=(20, 20), pady=(20, 20), sticky="nsew")
        # self.status_frame.grid(row=1, column=1, sticky="nsew")
        self.status_frame.grid_columnconfigure(4, weight=1)
        self.status_frame.grid_rowconfigure(4, weight=1)
        self.in_label = ctk.CTkLabel(self.status_frame, text="IN", font=self.header_0_font).grid(row=0, column=0,
                                                                                                 columnspan=1,
                                                                                                 sticky="nw")
        self.out_label = ctk.CTkLabel(self.status_frame, text="OUT", font=self.header_0_font).grid(row=0, column=1,
                                                                                                   columnspan=1,
                                                                                                   sticky="nw")
        self.out_label = ctk.CTkLabel(self.status_frame, text="OTHER", font=self.header_0_font).grid(row=0, column=2,
                                                                                                     columnspan=1,
                                                                                                     sticky="nw")

        self.status_frame.grid_columnconfigure(0, pad=self.small_pad)
        self.status_frame.grid_columnconfigure(1, pad=self.small_pad)
        self.status_frame.grid_rowconfigure(1, pad=self.small_pad)
        self.status_frame.grid_rowconfigure(2, pad=self.small_pad)
        self.status_frame.grid_rowconfigure(3, pad=self.small_pad)
        self.v_in_frame = ValueWithUnits(self.status_frame, label="V:  ", value="...", units="  V",
                                         label_unit_width=self.header_1_font.cget('size') * 2, value_width=40,
                                         font=self.header_1_font)
        self.v_in_frame.grid(row=1, column=0, sticky="nw")
        self.get_v_in, self.set_v_in = self.v_in_frame.get_set_factory()
        self.i_in_frame = ValueWithUnits(self.status_frame, label="I:  ", value="...", units="  A",
                                         label_unit_width=self.header_1_font.cget('size') * 2, value_width=40,
                                         font=self.header_1_font)
        self.i_in_frame.grid(row=2, column=0, sticky="nw")
        self.get_i_in, self.set_i_in = self.i_in_frame.get_set_factory()
        self.p_in_frame = ValueWithUnits(self.status_frame, label="P:  ", value="...", units="  W",
                                         label_unit_width=self.header_1_font.cget('size') * 2, value_width=40,
                                         font=self.header_1_font)
        self.p_in_frame.grid(row=3, column=0, sticky="nw")
        self.get_p_in, self.set_p_in = self.p_in_frame.get_set_factory()

        self.v_out_frame = ValueWithUnits(self.status_frame, label="V:  ", value="...", units="  V",
                                          label_unit_width=self.header_1_font.cget('size') * 2, value_width=40,
                                          font=self.header_1_font)
        self.v_out_frame.grid(row=1, column=1, sticky="nw")
        self.get_v_out, self.set_v_out = self.v_out_frame.get_set_factory()
        self.i_out_frame = ValueWithUnits(self.status_frame, label="I:  ", value="...", units="  A",
                                          label_unit_width=self.header_1_font.cget('size') * 2, value_width=40,
                                          font=self.header_1_font)
        self.i_out_frame.grid(row=2, column=1, sticky="nw")
        self.get_i_out, self.set_i_out = self.i_out_frame.get_set_factory()
        self.p_out_frame = ValueWithUnits(self.status_frame, label="P:  ", value="...", units="  W",
                                          label_unit_width=self.header_1_font.cget('size') * 2, value_width=40,
                                          font=self.header_1_font)
        self.p_out_frame.grid(row=3, column=1, sticky="nw")
        self.get_p_out, self.set_p_out = self.p_out_frame.get_set_factory()

        self.e_frame = ValueWithUnits(self.status_frame, label="E:  ", value="...", units="  %",
                                      label_unit_width=self.header_1_font.cget('size') * 2, value_width=40,
                                      font=self.header_1_font)
        self.e_frame.grid(row=1, column=2, sticky="nw")
        self.e_frame.set_data_format('{:2.2f}')
        self.get_e, self.set_e = self.e_frame.get_set_factory()
        self.m_frame = ValueWithUnits(self.status_frame, label="M:  ", value="...", units="  X",
                                      label_unit_width=self.header_1_font.cget('size') * 2, value_width=40,
                                      font=self.header_1_font)
        self.m_frame.grid(row=2, column=2, sticky="nw")
        self.get_m, self.set_m = self.m_frame.get_set_factory()
        self.a_frame = ValueWithUnits(self.status_frame, label="A:  ", value="...", units="  %",
                                      label_unit_width=self.header_1_font.cget('size') * 2, value_width=40,
                                      font=self.header_1_font)
        self.a_frame.grid(row=3, column=2, sticky="nw")
        self.get_a, self.set_a = self.a_frame.get_set_factory()
        self.a_frame.set_data_format('{:2.2f}')

        self.duty_label = ctk.CTkLabel(self.status_frame, text='Duty Cycle: ', font=self.header_1_font)
        self.duty_label.grid(row=1, column=3, sticky="nw")
        self.duty_frame = EntryWithButtons(self.status_frame, emit_message='gui:duty_cycle_change', font=self.header_1_font, default=0.00)
        self.duty_frame.grid(row=1, column=4, sticky="nw")

        self.dead_band_frame = EntryWithButtons(self.status_frame, format='{}', num_type=int, emit_message='gui:dead_band_change', font=self.header_1_font, default=24)
        self.dead_band_frame.grid(row=2, column=4, sticky="nw")

        # self.estop_button = ctk.CTkButton(text="ESTOP",

        self.led_label = ctk.CTkLabel(self.status_frame, text='LED: ', font=self.header_1_font)
        self.led_label.grid(row=3, column=3, sticky="nw")
        self.led_switch = ctk.CTkSwitch(self.status_frame, onvalue=True, offvalue=False, command=lambda: bus.emit('gui:led_switch_change', self))
        self.led_switch.grid(row=3, column=4, sticky="nw")

    def open_input_dialog_event(self):
        dialog = ctk.CTkInputDialog(text="Type in a number:", title="CTkInputDialog")
        print("CTkInputDialog:", dialog.get_input())

    def change_appearance_mode_event(self, new_appearance_mode: str):
        ctk.set_appearance_mode(new_appearance_mode)

    def change_scaling_event(self, new_scaling: str):
        new_scaling_float = int(new_scaling.replace("%", "")) / 100
        ctk.set_widget_scaling(new_scaling_float)


app = App()


@bus.on('gui:port_connect_button')
def port_connect_button_handler(gui: App):
    if not hasattr(port_connect_button_handler, "connected"):  # Used like static function variables in C++, just maintains state between function calls
        port_connect_button_handler.connected = False

    @bus.on('serial:connected')
    def _change_connected_button():
        port_connect_button_handler.connected = True
        app.serial_port_connect_button.configure(text='Disconnect')

    @bus.on('serial:disconnected')
    def _change_disconnected_button():
        port_connect_button_handler.connected = False
        app.serial_port_connect_button.configure(text='Connect')

    option = gui.serial_port_optionmenu.get()
    if not option == 'Port':
        if not port_connect_button_handler.connected:
            port_name = option.split(' ')[0]
            bus.emit('serial:connect', port_name)
        else:
            bus.emit('serial:disconnect')


# logging.basicConfig(level=logging.DEBUG)
FORMAT = "%(message)s"
logging.basicConfig(
    level="NOTSET", format=FORMAT, datefmt="[%X]", handlers=[RichHandler()]
)

log = logging.getLogger("rich")


@bus.on('gui:led_switch_change')
def led_switch_handler(gui: App):
    doc = {"Control": {"LED": gui.led_switch.get()}}
    bus.emit('serial:transmit', doc)


@bus.on('gui:duty_cycle_change')
def duty_cycle_change_handler(value: str):
    doc = {"SMPS": {"Duty": float(value)}}
    bus.emit('serial:transmit', doc)


@bus.on('gui:dead_band_change')
def daed_band_change_handler(value: str):
    doc = {"SMPS": {"DeadBand": int(value)}}
    bus.emit('serial:transmit', doc)


@bus.on('serial:receive')
def updated_from_json(json_data: dict):
    # json_formatted_str = json.dumps(json_data, indent=2)
    # log.debug(json_formatted_str)
    app.set_v_in(json_data["SMPS"]['In']['Volts'] * 2)
    app.set_i_in(json_data["SMPS"]['In']['Amps'])
    # app.set_p_in(float(app.get_v_in()) * float(app.get_i_in()))

    app.set_v_out(json_data["SMPS"]['Out']['Volts'] * 9.8)
    app.set_i_out(json_data["SMPS"]['Out']['Amps'])
    # app.set_p_out(float(app.get_v_out()) * float(app.get_i_out()))
    #
    # app.set_m(float(app.get_v_out()) / float(app.get_v_in()))
    # app.set_e(float(app.get_p_out()) / float(app.get_p_in()) * 100)

    # print(f"A Level: {json_data['SMPS']['ALevel']}")
    # print(f"B Level: {json_data['SMPS']['BLevel']}")
    # print(f"Duty: {json_data['SMPS']['InDuty']}")
    # print(f"Deadband: {json_data['SMPS']['InDeadBand']}")
    # print(f"Top: {json_data['SMPS']['Top']}")


async def get_com_ports():
    while True:
        bus.emit('serial:port_list', comports())  # Send out the newest list of serial ports
        await trio.sleep(0.5)


@bus.on('serial:port_list')
def update_com_ports(com_port_list: list):
    if not hasattr(update_com_ports, "last_com_ports"):  # Used like static function variables in C++, just maintains state between function calls
        update_com_ports.last_com_ports = []  # Used to keep track of if the port list has changed

    ports = [str(port_name) for port_name in com_port_list]  # Get a list of the port names as strings
    if not ports == update_com_ports.last_com_ports:  # If the ports have changed
        log.debug(f'Serial port list changed, new ports: {ports}')
        app.serial_port_optionmenu.configure(values=ports)  # Change the option menu to reflect the new ports
        update_com_ports.last_com_ports = ports  # Update the last ports for checking in the future


async def tkloop():
    app.update()
    while True:
        # if 'normal' == app.state():
        try:
            app.update()  # Update all the TKinter widgets
        except:
            app.quit()
            exit(0)
        await trio.sleep(1 / 60)  # Run at 60 FPS


async def main():
    async with trio.open_nursery() as nursery:
        nursery.start_soon(get_com_ports)
        nursery.start_soon(tkloop)
        nursery.start_soon(sjson.async_update(0.005))
        # Duty cycle adjustment, dead band adjustment, VIO/IIO, Protection Control, LED heartbeat, ESTOP (set duty cycle to limits)


if __name__ == "__main__":
    trio.run(main)
