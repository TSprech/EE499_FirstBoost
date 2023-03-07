import customtkinter as ctk
from serial.tools.list_ports import comports
import serial
import trio
import logging
from rich.logging import RichHandler
import event_bus as bus
import SerialJSON
import Headers as hfonts

ctk.set_appearance_mode("System")  # Modes: "System" (standard), "Dark", "Light"
ctk.set_default_color_theme("blue")  # Themes: "blue" (standard), "green", "dark-blue"

sjson = SerialJSON.SerialJSON()

v_in_offset = 0.0
v_in_multiplier = 2.0

v_out_offset = 0.0
v_out_multiplier = 1.0

i_in_offset = 1.65
i_in_gain = 1.0

i_out_offset = 1.65
i_out_gain = 1.0


class ValueWithUnits(ctk.CTkFrame):
    def __init__(self, root, label, value, units, label_unit_width, value_width, font):
        super().__init__(root, fg_color="transparent")
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


class RegulatorValues(ctk.CTkFrame):
    def __init__(self, root):
        super().__init__(root, fg_color='transparent')
        self.in_label = ctk.CTkLabel(self, text="In", font=hfonts.header_0_font).grid(row=0, column=0, columnspan=1, sticky="nwe")
        self.out_label = ctk.CTkLabel(self, text="Out", font=hfonts.header_0_font).grid(row=0, column=1, columnspan=1, sticky="nwe")
        self.out_label = ctk.CTkLabel(self, text="Other", font=hfonts.header_0_font).grid(row=0, column=2, columnspan=1, sticky="nwe")

        in_frame = ctk.CTkFrame(self)
        in_frame.grid(row=1, column=0, pady=(0, 5), sticky='nw')

        self.v_in_frame = ValueWithUnits(in_frame, label="V:  ", value="123.4", units="  V", label_unit_width=hfonts.header_1_font.cget('size') * 2, value_width=40, font=hfonts.header_1_font)
        self.v_in_frame.grid(row=1, column=0, pady=(10, 10), sticky="nw")
        self.i_in_frame = ValueWithUnits(in_frame, label="I:  ", value="123.4", units="  A", label_unit_width=hfonts.header_1_font.cget('size') * 2, value_width=40, font=hfonts.header_1_font)
        self.i_in_frame.grid(row=2, column=0, pady=(10, 10), sticky="nw")
        self.p_in_frame = ValueWithUnits(in_frame, label="P:  ", value="123.4", units="  W", label_unit_width=hfonts.header_1_font.cget('size') * 2, value_width=40, font=hfonts.header_1_font)
        self.p_in_frame.grid(row=3, column=0, pady=(10, 10), sticky="nw")

        out_frame = ctk.CTkFrame(self)
        out_frame.grid(row=1, column=1, padx=(10, 10), pady=(0, 5), sticky='nw')

        self.v_out_frame = ValueWithUnits(out_frame, label="V:  ", value="123.4", units="  V", label_unit_width=hfonts.header_1_font.cget('size') * 2, value_width=40, font=hfonts.header_1_font)
        self.v_out_frame.grid(row=1, column=1, pady=(10, 10), sticky="nw")
        self.i_out_frame = ValueWithUnits(out_frame, label="I:  ", value="123.4", units="  A", label_unit_width=hfonts.header_1_font.cget('size') * 2, value_width=40, font=hfonts.header_1_font)
        self.i_out_frame.grid(row=2, column=1, pady=(10, 10), sticky="nw")
        self.p_out_frame = ValueWithUnits(out_frame, label="P:  ", value="123.4", units="  W", label_unit_width=hfonts.header_1_font.cget('size') * 2, value_width=40, font=hfonts.header_1_font)
        self.p_out_frame.grid(row=3, column=1, pady=(10, 10), sticky="nw")

        other_frame = ctk.CTkFrame(self)
        other_frame.grid(row=1, column=2, pady=(0, 5), sticky='nw')

        self.e_frame = ValueWithUnits(other_frame, label="E:  ", value="123.4", units="  %", label_unit_width=hfonts.header_1_font.cget('size') * 2, value_width=40, font=hfonts.header_1_font)
        self.e_frame.grid(row=1, column=2, pady=(10, 10), sticky="nw")
        self.e_frame.set_data_format('{:2.2f}')
        self.m_frame = ValueWithUnits(other_frame, label="M:  ", value="123.4", units="  X", label_unit_width=hfonts.header_1_font.cget('size') * 2, value_width=40, font=hfonts.header_1_font)
        self.m_frame.grid(row=2, column=2, pady=(10, 10), sticky="nw")
        self.a_frame = ValueWithUnits(other_frame, label="A:  ", value="123.4", units="  %", label_unit_width=hfonts.header_1_font.cget('size') * 2, value_width=40, font=hfonts.header_1_font)
        self.a_frame.grid(row=3, column=2, pady=(10, 10), sticky="nw")
        self.a_frame.set_data_format('{:2.2f}')


class TuneTab:
    def __init__(self, root, label: str, emit: str, row: int, default):
        self.v_in_offset_label = ctk.CTkLabel(root, text=label, font=hfonts.header_3_font)
        self.v_in_offset_label.grid(row=row, column=0, sticky="nw")
        self.v_in_offset_frame = EntryWithButtons(root, num_type=float, emit_message=emit, font=hfonts.header_1_font, default=default)
        self.v_in_offset_frame.grid(row=row, column=1, sticky="ne")


# class MessageBox(ctk.CTkFrame):
#     def __init__(self, root):
#         super().__init__(root, fg_color="transparent")
#         self.label = ctk.CTkLabel(self, text="Messages", font=hfonts.header_0_font).grid(row=0, column=0, columnspan=1, pady=(0,5), sticky="nwe")
#
#         message_frame = ctk.CTkFrame(self)
#         message_frame.grid(row=1, column=0, pady=(0,5), sticky='nw')
#         self.message_box = ctk.CTkTextbox(message_frame, font=hfonts.regular_font)
#         self.message_box.grid(row=0, column=0, columnspan=1, sticky="we")


class SystemDetails(ctk.CTkFrame):
    def __init__(self, root):
        super().__init__(root, fg_color="transparent")
        self.label = ctk.CTkLabel(self, text="System", font=hfonts.header_0_font).grid(row=0, column=0, columnspan=1, pady=(0, 5), sticky="nwe")

        parameter_frame = ctk.CTkFrame(self)
        parameter_frame.grid(row=1, column=0, pady=(0, 5), sticky='nw')
        self.cpu_temp_label = ctk.CTkLabel(parameter_frame, text='CPU Temp: ', font=hfonts.header_3_font)
        self.cpu_temp_label.grid(row=0, column=0, sticky="nw")
        self.cpu_temp_value = ctk.CTkLabel(parameter_frame, text='XX°C', font=hfonts.header_3_font)
        self.cpu_temp_value.grid(row=0, column=1, sticky="nw")

        self.led_label = ctk.CTkLabel(parameter_frame, text='LED: ', font=hfonts.header_3_font)
        self.led_label.grid(row=1, column=0, sticky="nw")
        self.led_switch = ctk.CTkSwitch(parameter_frame, onvalue=True, text='', offvalue=False, command=lambda: bus.emit('gui:led_switch_change', self))
        self.led_switch.grid(row=1, column=1, sticky="nw")


class SMPSParameters(ctk.CTkFrame):
    def __init__(self, root):
        super().__init__(root, fg_color="transparent")
        self.label = ctk.CTkLabel(self, text="SMPS", font=hfonts.header_0_font).grid(row=0, column=0, columnspan=1, pady=(0, 5), sticky="nwe")

        parameter_frame = ctk.CTkFrame(self)
        parameter_frame.grid(row=1, column=0, pady=(0, 0), sticky='nw')

        xpad = (10, 10)
        duty_frame = ctk.CTkFrame(parameter_frame, fg_color='transparent')
        duty_frame.grid(row=1, column=0, padx=xpad, pady=(10, 10), sticky='nwe')
        self.duty_label = ctk.CTkLabel(duty_frame, text='Duty Cycle: ', font=hfonts.header_3_font)
        self.duty_label.grid(row=0, column=0, sticky="w")
        self.duty_frame = EntryWithButtons(duty_frame, emit_message='gui:duty_cycle_change', font=hfonts.header_1_font, default=0.00)
        self.duty_frame.grid(row=0, column=1, sticky="e")

        self.duty_label = ctk.CTkLabel(duty_frame, text='Dead Band: ', font=hfonts.header_3_font)
        self.duty_label.grid(row=1, column=0, sticky="w")
        self.duty_frame = EntryWithButtons(duty_frame, format='{}', num_type=int, emit_message='gui:dead_band_change', font=hfonts.header_1_font, default=24)
        self.duty_frame.grid(row=1, column=1, sticky="e")

        # dead_band_frame = ctk.CTkFrame(parameter_frame, fg_color='transparent')
        # dead_band_frame.grid(row=2, column=0, padx=xpad, pady=(0, 0), sticky='nwe')
        # self.duty_label = ctk.CTkLabel(dead_band_frame, text='Dead Band: ', font=hfonts.header_3_font)
        # self.duty_label.grid(row=0, column=0, sticky="w")
        # self.duty_frame = EntryWithButtons(dead_band_frame, format='{}', num_type=int, emit_message='gui:dead_band_change', font=hfonts.header_1_font, default=24)
        # self.duty_frame.grid(row=0, column=1, sticky="e")

        # enable_frame = ctk.CTkFrame(parameter_frame, fg_color='transparent')
        # enable_frame.grid(row=1, column=1, padx=(0, 10), pady=(10, 0), sticky='nwe')
        # duty_frame.grid_columnconfigure(0, pad=10)

        self.enable_label = ctk.CTkLabel(duty_frame, text='Enable: ', font=hfonts.header_3_font)
        self.enable_label.grid(row=0, column=2, sticky="w")
        self.enable_switch = ctk.CTkSwitch(duty_frame, onvalue=True, text='', offvalue=False, command=lambda: bus.emit('gui:enable_switch_change', self.enable_switch.get()))
        self.enable_switch.grid(row=0, column=3, sticky="e")

        self.pi_enable_label = ctk.CTkLabel(duty_frame, text='PI Loop: ', font=hfonts.header_3_font)
        self.pi_enable_label.grid(row=1, column=2, sticky="w")
        self.pi_enable_switch = ctk.CTkSwitch(duty_frame, onvalue=True, text='', offvalue=False, command=lambda: bus.emit('gui:pi_enable_switch_change', self.pi_enable_switch.get()))
        self.pi_enable_switch.grid(row=1, column=3, sticky="e")

        # pi_enable_frame = ctk.CTkFrame(parameter_frame, fg_color='transparent')
        # pi_enable_frame.grid(row=2, column=1, padx=(0, 10), pady=(0, 0), sticky='nwe')
        # self.pi_enable_label = ctk.CTkLabel(pi_enable_frame, text='PI Loop: ', font=hfonts.header_3_font)
        # self.pi_enable_label.grid(row=1, column=0, sticky="w")
        # self.pi_enable_switch = ctk.CTkSwitch(pi_enable_frame, onvalue=True, text='', offvalue=False, command=lambda: bus.emit('gui:pi_enable_switch_change', self))
        # self.pi_enable_switch.grid(row=1, column=1, sticky="e")


class EntryWithButtons(ctk.CTkFrame):
    # def __init__(self, root, label, value, units, label_unit_width, value_width, font):

    def entry_lose_focus_callback(self):
        print(f'Emit: {self.string_var.get()} by {self.emit_message}')
        bus.emit(self.emit_message, self.num_type(self.string_var.get()))
        return True

    def __init__(self, root, emit_message: str, font, format='{:3.1f}', num_type=float, default=0):
        super().__init__(root, fg_color="transparent")
        self.emit_message = emit_message
        self.num_type = num_type
        self.grid_rowconfigure(0, weight=2)
        self.grid_columnconfigure(0, weight=2)
        self.grid_columnconfigure(1, weight=1)
        self.grid_columnconfigure(2, weight=1)

        self.string_var = ctk.StringVar()
        self.string_var.set(str(format.format(default)))

        # x, y, width, height = self.grid_bbox(2, 1)
        width = font.cget("size") * 0.8
        self.dec_value_button = ctk.CTkButton(self, text='-', width=width, command=self.dec_callback)
        self.dec_value_button.grid(row=2, column=1, pady=(0, 5), sticky="nsew")

        self.value = ctk.CTkEntry(self, width=width * 2, textvariable=self.string_var, validate="focusout", validatecommand=self.entry_lose_focus_callback)
        # self.value.insert(0, )
        self.value.grid(row=2, column=2, pady=(0, 5), sticky="nsew")

        self.inc_value_button = ctk.CTkButton(self, text='+', width=width, command=self.inc_callback)
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

        hfonts.INIT()

        self.link = serial.Serial
        self.connected = False

        self.small_pad = 10
        self.large_pad = 20

        # configure window
        self.title("CustomTkinter complex_example.py")
        self.geometry(f"{1240}x{700}")

        # configure grid layout (4x4)
        self.grid_columnconfigure(1, weight=1)
        self.grid_columnconfigure((2, 3), weight=0)
        self.grid_rowconfigure((0, 1, 2), weight=1)

        # create sidebar frame with widgets
        self.sidebar_frame = ctk.CTkFrame(self, width=140, corner_radius=0)
        self.sidebar_frame.grid(row=0, column=0, rowspan=4, sticky="nsew")
        self.sidebar_frame.grid_rowconfigure(4, weight=1)
        self.serial_label = ctk.CTkLabel(self.sidebar_frame, text="Serial Port", font=hfonts.header_2_font)
        self.serial_label.grid(row=0, column=0, padx=20, pady=(20, 10))
        self.serial_port_optionmenu = ctk.CTkOptionMenu(self.sidebar_frame, dynamic_resizing=False, values=["Port"])
        self.serial_port_optionmenu.grid(row=1, column=0, padx=20, pady=10)
        # self.serial_port_connect_button = ctk.CTkButton(self.sidebar_frame, text='Connect', command=self.sidebar_button_event)
        self.serial_port_connect_button = ctk.CTkButton(self.sidebar_frame, text='Connect', command=lambda: bus.emit('gui:port_connect_button', self))
        self.serial_port_connect_button.grid(row=2, column=0, padx=20, pady=10)
        self.appearance_mode_label = ctk.CTkLabel(self.sidebar_frame, text="Appearance Mode:", anchor="w")
        self.appearance_mode_label.grid(row=5, column=0, padx=20, pady=(10, 0))
        self.appearance_mode_optionemenu = ctk.CTkOptionMenu(self.sidebar_frame, values=["Light", "Dark", "System"], command=self.change_appearance_mode_event)
        self.appearance_mode_optionemenu.grid(row=6, column=0, padx=20, pady=(10, 10))
        self.scaling_label = ctk.CTkLabel(self.sidebar_frame, text="UI Scaling:", anchor="w")
        self.scaling_label.grid(row=7, column=0, padx=20, pady=(10, 0))
        self.scaling_optionemenu = ctk.CTkOptionMenu(self.sidebar_frame, values=["80%", "90%", "100%", "110%", "120%"], command=self.change_scaling_event)
        self.scaling_optionemenu.grid(row=8, column=0, padx=20, pady=(10, 20))

        #######################################################################################################################

        self.right_frame = ctk.CTkFrame(self, fg_color="transparent").grid(row=0, column=1, sticky='nsew')

        self.regulator_values = RegulatorValues(self.right_frame)
        self.regulator_values.grid(row=0, column=1, columnspan=2, padx=(10, 10), pady=(10, 0), sticky='nw')

        sys_details = SystemDetails(self.right_frame)
        sys_details.grid(row=1, column=2, pady=(10, 0), padx=(10, 10), sticky='nwe')

        smps_parameters = SMPSParameters(self.right_frame)
        smps_parameters.grid(row=1, column=1, pady=(10, 0), padx=(10, 10), sticky='nwe')

        self.message_box_label = ctk.CTkLabel(self.right_frame, text="Messages", font=hfonts.header_0_font).grid(row=2, column=1, columnspan=3, pady=(0, 5), sticky="swe")
        self.message_box = ctk.CTkTextbox(self.right_frame, font=hfonts.regular_font)
        self.message_box.grid(row=3, column=1, padx=(10, 10), pady=(0, 10), columnspan=3, sticky="we")

        self.tune_tab_frame = ctk.CTkFrame(self.right_frame, fg_color='transparent')
        self.tune_tab_frame.grid(row=0, column=3, pady=(10, 0), sticky='nw')
        self.tune_tab_label = ctk.CTkLabel(self.tune_tab_frame, text="Tune", font=hfonts.header_0_font)
        self.tune_tab_label.grid(row=0, column=0, sticky='nwe')

        self.tune_tabs = ctk.CTkTabview(self.tune_tab_frame, width=255, height=180)
        self.tune_tabs.grid(row=1, column=0, padx=(20, 20), pady=(0, 0), sticky="nw")
        button_height = hfonts.header_2_font.cget('size') * 2
        button_width = self.tune_tabs.cget('width')

        self.tune_tabs.add('V In')
        self.v_in_tune_offset_row = TuneTab(self.tune_tabs.tab('V In'), label='V In Offset: ', emit='gui:v_in_offset_change', row=0, default=v_in_offset)
        self.v_in_tune_multiplier_row = TuneTab(self.tune_tabs.tab('V In'), label='V In Multiplier: ', emit='gui:v_in_multiplier_change', row=1, default=v_in_multiplier)
        self.v_in_tune_apply_button = ctk.CTkButton(self.tune_tabs.tab('V In'), text='Apply', font=hfonts.header_2_font, height=button_height, width=button_width)
        self.v_in_tune_apply_button.grid(row=2, column=0, columnspan=2, pady=(10, 0), sticky="nwe")

        self.tune_tabs.add("I In")
        self.i_in_tune_offset_row = TuneTab(self.tune_tabs.tab('I In'), label='I In Offset: ', emit='gui:i_in_offset_change', row=0, default=i_in_offset)
        self.i_in_tune_gain_row = TuneTab(self.tune_tabs.tab('I In'), label='I In Gain: ', emit='gui:i_in_gain_change', row=1, default=i_in_gain)
        self.i_in_tune_apply_button = ctk.CTkButton(self.tune_tabs.tab('I In'), text='Apply', font=hfonts.header_2_font, height=button_height, width=button_width)
        self.i_in_tune_apply_button.grid(row=2, column=0, columnspan=2, pady=(10, 0), sticky="nwe")

        self.tune_tabs.add("V Out")
        self.v_out_tune_offset_row = TuneTab(self.tune_tabs.tab('V Out'), label='V Out Offset: ', emit='gui:v_out_offset_change', row=0, default=v_out_offset)
        self.v_out_tune_multiplier_row = TuneTab(self.tune_tabs.tab('V Out'), label='V Out Multiplier: ', emit='gui:v_out_multiplier_change', row=1, default=v_out_multiplier)
        self.v_out_tune_apply_button = ctk.CTkButton(self.tune_tabs.tab('V Out'), text='Apply', font=hfonts.header_2_font, height=button_height, width=button_width)
        self.v_out_tune_apply_button.grid(row=2, column=0, columnspan=2, pady=(10, 0), sticky="nwe")

        self.tune_tabs.add("I Out")
        self.i_out_tune_offset_row = TuneTab(self.tune_tabs.tab('I Out'), label='I Out Offset: ', emit='gui:i_out_offset_change', row=0, default=i_out_offset)
        self.i_out_tune_gain_row = TuneTab(self.tune_tabs.tab('I Out'), label='I Out Gain: ', emit='gui:i_out_gain_change', row=1, default=i_out_gain)
        self.i_out_tune_apply_button = ctk.CTkButton(self.tune_tabs.tab('I Out'), text='Apply', font=hfonts.header_2_font, height=button_height, width=button_width)
        self.i_out_tune_apply_button.grid(row=2, column=0, columnspan=2, pady=(10, 0), sticky="nwe")

        self.get_v_in, self.set_v_in = self.regulator_values.v_in_frame.get_set_factory()
        self.get_i_in, self.set_i_in = self.regulator_values.i_in_frame.get_set_factory()
        self.get_p_in, self.set_p_in = self.regulator_values.p_in_frame.get_set_factory()
        self.get_v_out, self.set_v_out = self.regulator_values.v_out_frame.get_set_factory()
        self.get_i_out, self.set_i_out = self.regulator_values.i_out_frame.get_set_factory()
        self.get_p_out, self.set_p_out = self.regulator_values.p_out_frame.get_set_factory()
        self.get_e, self.set_e = self.regulator_values.e_frame.get_set_factory()
        self.get_m, self.set_m = self.regulator_values.m_frame.get_set_factory()
        self.get_a, self.set_a = self.regulator_values.a_frame.get_set_factory()

    def open_input_dialog_event(self):
        dialog = ctk.CTkInputDialog(text="Type in a number:", title="CTkInputDialog")
        print("CTkInputDialog:", dialog.get_input())

    def change_appearance_mode_event(self, new_appearance_mode: str):
        ctk.set_appearance_mode(new_appearance_mode)

    def change_scaling_event(self, new_scaling: str):
        new_scaling_float = int(new_scaling.replace("%", "")) / 100
        ctk.set_widget_scaling(new_scaling_float)


app = App()


@bus.on('gui:v_in_offset_change')
def v_in_offset_change(value):
    global v_in_offset
    v_in_offset = value


@bus.on('gui:v_in_multiplier_change')
def v_in_multiplier_change(value):
    global v_in_multiplier
    v_in_multiplier = value


@bus.on('gui:v_out_offset_change')
def v_out_offset_change(value):
    global v_out_offset
    v_out_offset = value


@bus.on('gui:v_out_multiplier_change')
def v_out_multiplier_change(value):
    global v_out_multiplier
    v_out_multiplier = value


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


@bus.on('gui:enable_switch_change')
def enable_switch_handler(state: bool):
    doc = {"SMPS": {"Enable": state}}
    bus.emit('serial:transmit', doc)


@bus.on('gui:pi_enable_switch_change')
def pi_enable_switch_handler(state: bool):
    doc = {"SMPS": {"PIEnable": state}}
    bus.emit('serial:transmit', doc)


@bus.on('gui:duty_cycle_change')
def duty_cycle_change_handler(value: str):
    doc = {"SMPS": {"Duty": float(value)}}
    bus.emit('serial:transmit', doc)


@bus.on('gui:dead_band_change')
def dead_band_change_handler(value: str):
    doc = {"SMPS": {"DeadBand": int(value)}}
    bus.emit('serial:transmit', doc)


@bus.on('serial:receive')
def updated_from_json(json_data: dict):
    # json_formatted_str = json.dumps(json_data, indent=2)
    # log.debug(json_formatted_str)
    try:
        app.set_v_in((json_data["SMPS"]['In']['VVolts']-v_in_offset) * v_in_multiplier)
        app.set_i_in((json_data["SMPS"]['In']['IVolts'] - i_in_offset) * i_in_gain / 0.1)
        app.set_p_in(float(app.get_v_in()) * float(app.get_i_in()))

        app.set_v_out((json_data["SMPS"]['Out']['VVolts']-v_out_offset) * v_out_multiplier)
        app.set_i_out((json_data["SMPS"]['Out']['IVolts'] - i_out_offset) * i_out_gain / 0.1)
        app.set_p_out(float(app.get_v_out()) * float(app.get_i_out()))

        if 'System' in json_data and 'Message' in json_data['System']:
            app.message_box.insert(ctk.END, json_data['System']['Message'])
            app.message_box.see(ctk.END)
        #
        # app.set_m(float(app.get_v_out()) / float(app.get_v_in()))
        # app.set_e(float(app.get_p_out()) / float(app.get_p_in()) * 100)

        # print(f"A Level: {json_data['SMPS']['ALevel']}")
        # print(f"B Level: {json_data['SMPS']['BLevel']}")
        # print(f"Duty: {json_data['SMPS']['InDuty']}")
        # print(f"Deadband: {json_data['SMPS']['InDeadBand']}")
        # print(f"Top: {json_data['SMPS']['Top']}")
    except:
        pass


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
        nursery.start_soon(sjson.async_update(0.001))
        # Duty cycle adjustment, dead band adjustment, VIO/IIO, Protection Control, LED heartbeat, ESTOP (set duty cycle to limits)


if __name__ == "__main__":
    trio.run(main)
