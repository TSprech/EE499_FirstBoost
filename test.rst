Gets a list of all async functions, should be added to the nursery.

.. code-block:: Python

    async with trio.open_nursery() as nursery:
        for func in sjson.update_functions(0.5):
            nursery.start_soon(func)

:param callback_time_s: The sleep time for each coroutine, lower = faster serial port response rate
:returns: A list of all async function handlers which handle the serial port