# Import the function from your client.py file
from utils import run_command_sequence

HOST = '127.0.0.1'
PORT = 8080

commands = [
    b"REGISTER UE001",
    b"AUTH UE001",
    b"PING",
    b"GET_STATUS",
    b"SEND_DATA hello",
    b"DISCONNECT"
]

# Execute the sequence
run_command_sequence(host=HOST, port=PORT, commands=commands)
