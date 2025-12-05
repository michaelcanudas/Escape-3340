import socket
import threading
import queue

logs = queue.Queue()

devices = {}

def receive_from_device(conn, addr):
    device_id = addr

    try:
        device_id = conn.recv(1024).decode().strip()
        devices[device_id] = conn

        logs.put(f'{addr} connected with id: {device_id}')

        while True:
            raw = conn.recv(1024).decode()
            if not raw:
                break

            if ':' not in raw:
                logs.put(f'{device_id} sent command with invalid format: {raw}')
                continue

            device, command = raw.split(':', 1)
            if device not in devices:
                logs.put(f'{device_id} sent command to invalid device: {device}')
                continue
            
            if send_to_device(device, command):
                logs.put(f'{device_id} sent command to {device}: {command}')
            else:
                logs.put(f'{device_id} failed to send command to {device}: {command}')
    finally:
        conn.close()

        devices.pop(device_id, None)
        logs.put(f'{device_id} disconnected')

def send_to_device(device, command):
    try:
        conn = devices[device]
        conn.send(command.encode())
        return True
    except:
        return False

def start_server(host='0.0.0.0', port=7531):
    s = socket.socket()
    s.bind((host, port))
    s.listen()

    logs.put('Server running...')

    while True:
        conn, addr = s.accept()
        threading.Thread(target=receive_from_device, args=(conn, addr), daemon=True).start()

