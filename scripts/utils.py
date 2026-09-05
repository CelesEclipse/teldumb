import socket
import struct

def run_command_sequence(host='127.0.0.1', port=8080, commands=None):
    if commands is None:
        return

    # Connect to the C server once
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect((host, port))
    except ConnectionRefusedError:
        print(f"X Error: Connection refused on {host}:{port}")
        return

    for payload in commands:
        print(f"\n> Sending: {payload.decode()}")
        
        # Pack the dynamic length into a 4-byte binary header (Network Order)
        header = struct.pack('!I', len(payload))

        # Send the composite [Header][Payload] package
        s.sendall(header + payload)

        # Read the length-prefixed response back from the server
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
