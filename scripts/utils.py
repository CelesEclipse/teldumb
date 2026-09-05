import socket
import struct
import psutil

def clear_port_if_blocked(port):

    """Finds and kills any active local process using the specified port."""
    for proc in psutil.process_iter(['pid', 'name']):
        try:
            for conn in proc.net_connections(kind='inet'):
                if conn.laddr.port == port:
                    print(f"Port {port} is occupied. Killing {proc.info['name']} (PID: {proc.info['pid']})...")
                    proc.kill()
                    # Wait slightly for the OS to release the socket completely
                    proc.wait(timeout=2)
                    return True
        except (psutil.AccessDenied, psutil.NoSuchProcess, psutil.ZombieProcess):
            pass
    return False

def run_command_sequence(host='127.0.0.1', port=8080, commands=None):
    if commands is None:
        return

    # Check and clear the port before starting
    clear_port_if_blocked(port)

    # Connect to the C server once
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect((host, port))
    except ConnectionRefusedError:
        print(f"Error: Connection refused on {host}:{port}. ")
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
            print("Error: Failed to read 4-byte response header from server.")
            break

    # Clean up socket closure after the sequence completes
    s.close()
    print("\nSequence completed. Socket closed.")
