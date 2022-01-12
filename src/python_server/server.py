import asyncio
import random
import json
import websockets
import socket
import gzip
import datetime
from rich.console import Console

console = Console()
default_fps = 60;

def fps(targetfps):
    return 1 / targetfps

async def send_json(websocket, path):
    x = 0

    while True:
        data = {}

        for i in range(0, 98):
            if i <= x:
                r = 255
            else:
                r = 0
            g = 0
            b = 0
            data[i] = {'r':r, 'g':g, 'b':b}

        json_str = json.dumps(data) + '\n'
        json_bytes = json_str.encode('utf-8')

        #console.print(json_bytes)

        x += 1
        if x >= 98: x = 0

        #await websocket.send(json_bytes)
        await websocket.send(json_str)
        ts = datetime.datetime.now()
        console.print(str(ts) + ">[bold green] Send packet [/bold green]")
        #await websocket.send("Hello!")
        await asyncio.sleep(fps(default_fps))

host_name = socket.gethostname()
ip_address = socket.gethostbyname(host_name)
console.print('[bold red] IP: ' + ip_address + '[/bold red]')

start_server = websockets.serve(send_json, ip_address, 5678, ping_interval=None)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
