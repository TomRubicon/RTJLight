import asyncio
import pyaudio
import numpy as np
import websockets
import socket
from rich.console import Console

default_fps = 24;

CHUNK = 4096
RATE = 44100

port = 5678

p=pyaudio.PyAudio()
stream = p.open(format=pyaudio.paInt16, channels=1, rate=RATE, input=True,
                frames_per_buffer=CHUNK)

console = Console()

def fps(targetfps):
    return 1 / targetfps

async def send_bars(websocket, path):
    while True:
        data = np.fromstring(stream.read(CHUNK), dtype=np.int16)
        peak = np.average(np.abs(data))*2
        console.print(int(peak))
        await websocket.send(str(int(peak)))
        await asyncio.sleep(fps(default_fps))

host_name =  socket.gethostname()
ip_address = socket.gethostbyname(host_name)

console.print('[bold red] IP: ' + ip_address + '[/bold red]')

start_server = websockets.serve(send_bars, ip_address, port, 
                                ping_interval=None)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()