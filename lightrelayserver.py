import socket

HOST = '0.0.0.0'  # Listen on all interfaces
PORT = 5000

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"Server listening on {PORT}...")
    conn, addr = s.accept()
    with conn:
        print(f"Connected by {addr}")
        while True:
            cmd = input("Command to send (ON/OFF): ").strip()
            if cmd not in ["ON", "OFF"]:
                print("Invalid command")
                continue
            conn.sendall((cmd + '\n').encode())
            response = conn.recv(1024).decode().strip()
            print("ESP32 replied:", response)