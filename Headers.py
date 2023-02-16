# TSprech 2023/02/16 15:46:08

import customtkinter as ctk

header_0_font = ctk.CTkFont
header_1_font = ctk.CTkFont
header_2_font = ctk.CTkFont
header_3_font = ctk.CTkFont
regular_font = ctk.CTkFont

def INIT():
    global header_0_font
    global header_1_font
    global header_2_font
    global header_3_font
    global regular_font

    header_0_font = ctk.CTkFont(size=48, weight='bold')
    header_1_font = ctk.CTkFont(size=36, weight='bold')
    header_2_font = ctk.CTkFont(size=24, weight='bold')
    header_3_font = ctk.CTkFont(size=16, weight='bold')
    regular_font = ctk.CTkFont(size=14)
