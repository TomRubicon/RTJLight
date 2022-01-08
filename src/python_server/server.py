import asyncio
import random
import json
import websockets
import gzip
from rich.console import Console

console = Console()

async def hello(websocket, path):
    x = 1

    while True:
        data = {}

        for i in range(1, 98):
            if i <= x:
                r = 255
            else:
                r = 0
            g = 0
            b = 0
            data[i] = {'r':r, 'g':g, 'b':b}

        json_str = json.dumps(data) + '\n'
        json_bytes = json_str.encode('utf-8')

        console.print(json_bytes)

        x += 1
        if x > 98: x = 1

        await websocket.send(json_bytes)
        await asyncio.sleep(.03)

start_server = websockets.serve(hello, 'localhost', 5678, ping_interval=None)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
