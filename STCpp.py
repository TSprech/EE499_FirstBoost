import json
from time import sleep
import serial

connection = serial.Serial(port="COM13")
connection.reset_output_buffer()

# y = json.loads('{"LED":"1"}')

# connection.writelines(json.dumps(y))
connection.write('{"LED":"1"}\r'.encode("ascii"))
connection.flush()
sleep(1)
connection.write('{"LED":"0"}\r'.encode("ascii"))
connection.flush()
sleep(1)
# data = connection.readline().decode("utf-8")
# print(data)
# try:
#     json_object = json.loads(data)
#     json_formatted_str = json.dumps(json_object, indent=2)
#     print(json_formatted_str)
# except json.JSONDecodeError as e:
#     print("JSON:", e)
