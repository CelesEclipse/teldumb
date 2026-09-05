import socket
import struct

PORT = 8080

commands = [
    b"REGISTER UE001",
    b"AUTH UE001",
    b"PING",
    b"GET_STATUS",
    b"SEND_DATA hello",
    b"DISCONNECT"
]

# Connect to the C server once
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1', PORT))

for payload in commands:
    print(f"\n> Sending: {payload.decode()}")
    
    # 2. Pack the dynamic length into a 4-byte binary header (Network Order)
    header = struct.pack('!I', len(payload))

    # 3. Send the composite [Header][Payload] package
    s.sendall(header + payload)

    # 4. Read the length-prefixed response back from the server
    response_header = s.recv(4)
    if len(response_header) == 4:
        response_len = struct.unpack('!I', response_header)[0]
        response_payload = s.recv(response_len)
        print(f"< Received: {response_payload.decode()}")
    else:
        print("X Error: Failed to read 4-byte response header from server.")
        break

# Clean up socket closure after the sequence completes
s.close()
print("\nSequence completed. Socket closed.")
