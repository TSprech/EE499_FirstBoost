import serial
import trio
import json
import event_bus as bus


class SerialJSON(serial.Serial):
    """
    Trio compatible wrapper for dealing with a serial port. Handles all port I/O operations and interfaces with other programs using two memory channels.
    """

    def __init__(self, eol_delimiter: str = '\r', **kwargs):
        """
        :param eol_delimiter: The character (or string) that signifies the end of the JSON string being received.
        :param channel_buffer_size: Number of element that can sit in the channel communication buffers, 0 means no buffering (typical).
        :param kwargs: Same as the arguments that can be passed to a pyserial.SerialBase object.
        """
        super().__init__(**kwargs)
        self.eol_delimiter = eol_delimiter
        self.connected = False

        bus.add_event(lambda data: self.tx_json(data), 'serial:transmit')
        bus.add_event(lambda port_name: self.connect(port_name), 'serial:connect')
        bus.add_event(lambda: self.disconnect(), 'serial:disconnect')

    def connect(self, com_port=None):
        if com_port is not None:
            self.setPort(com_port)  # Make sure that were actually trying to connect to a port
            self.open()  # Handles the actual acquisition of the serial port
            # Clear out any existing data
            self.reset_input_buffer()
            self.reset_output_buffer()
            self.connected = True
            bus.emit('serial:connected')

    def disconnect(self):
        if self.connected:
            self.close()  # Handles releasing the port back to the system to make it available again
            self.connected = False
            bus.emit('serial:disconnected')

    def async_update(self, callback_time_s: float):
        """
        Gets a list of all async functions, should be added to the nursery.

        .. code-block:: Python

            async with trio.open_nursery() as nursery:
                nursery.start_soon(sjson.async_update(0.5))

        :param callback_time_s: The sleep time for each coroutine, lower = lower serial port latency.
        :returns: A list of all async function handlers which handle the serial port.
        """

        async def rx_callback():
            while True:
                self.rx_json()
                await trio.sleep(callback_time_s)

        return rx_callback

    def tx_json(self, data):
        try:
            if self.connected:  # Don't try to send data if there is no device connected
                # async for data in self._tx_receive_channel:  # Go through any (maybe no) data that has been queued to be sent out to the serial device
                #     self.write((json.dumps(data) + self.eol_delimiter).encode("ascii"))  # For each, make sure it is encoded correctly
                self.write((json.dumps(data) + self.eol_delimiter).encode("ascii"))  # For each, make sure it is encoded correctly
                print(data)
        except serial.PortNotOpenError:
            # log.debug("Port no longer open, abandoning send")
            pass

    def rx_json(self):
        try:
            if self.connected and self.in_waiting > 0:  # Don't try to read data if there is no device connected, or if nothing has arrived from the device
                data = self.readline().decode("utf-8")  # Since there is data, read the full line as the JSON is sent as a single continuous line
                try:
                    json_object = json.loads(data)  # Convert from string to dictionary
                    bus.emit('serial:receive', json_object)  # Take the dictionary and send it out the rx memory channel to be consumed by application
                except json.JSONDecodeError as e:  # If any parsing error happens, report it (can happen if connecting to the device in the middle of a transmission)
                    # log.error(f"JSON Decode error: {e}")
                    pass
        except serial.SerialException:
            # log.debug("Port no longer open, abandoning receive")
            pass
