import json
from time import sleep
import serial

connection = serial.Serial(port="COM13")
connection.reset_output_buffer()

y = json.loads('{"LED":true}')

connection.write((json.dumps(y) + '\r').encode("ascii"))
sleep(1)
connection.write('{"LED":false}\r'.encode("ascii"))
sleep(1)
# try:
#     json_object = json.loads(data)
#     json_formatted_str = json.dumps(json_object, indent=2)
#     print(json_formatted_str)
# except json.JSONDecodeError as e:
#     print("JSON:", e)
