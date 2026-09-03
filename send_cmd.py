import socket
import struct

PORT = 8080
# Connect to the C server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1', PORT))

# raw text payload
payload = b"REGISTER user123"

# 2. Pack the length (16) into a 4-byte binary integer in Network Order
header = struct.pack('!I', len(payload))

# 3. Send the composite [Header][Payload] package
s.sendall(header + payload)

# 4. Read the echo response back from server
# It will read the 4-byte response header first!
response_header = s.recv(4)
if len(response_header) == 4:
    response_len = struct.unpack('!I', response_header)[0]
    response_payload = s.recv(response_len)
    print(f"Server Echo Response: {response_payload.decode()}")

s.close()
