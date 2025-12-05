import socket
import threading
import queue
import time

def wth(args):
    send_to_device('printer', 'print:what the helly!!!!' + ('\n' * 30))
    while True:
        send_to_device('relay', 'toggle')
        time.sleep(1) 

logs = queue.Queue()
devices = {}
commands = {
    'print': lambda args: send_to_device('printer', f'print:{args}\n\n\n'),
    'what the helly': wth
}

def receive_from_device(conn, addr):
    device_id = addr

    try:
        device_id = conn.recv(1024).decode().strip()
        devices[device_id] = conn

        logs.put(f'{addr} connected with id: {device_id}')

        while True:
            raw = conn.recv(1024).decode().strip()
            if not raw:
                break

            if ':' not in raw:
                logs.put(f'{device_id} sent command with invalid format: {raw}')
                continue

            device, command = raw.split(':', 1)
            if device not in devices:
                logs.put(f'{device_id} sent command to invalid device: {device}')
                continue
            
            send_to_device(device, command)
    finally:
        conn.close()

        devices.pop(device_id, None)
        logs.put(f'{device_id} disconnected')

def send_to_device(device, command):
    try:
        conn = devices[device]
        conn.send(command.encode())

        logs.put(f'Sent command to {device}: {command}')
    except:
        logs.put(f'Failed to send command to {device}: {command}')

def start_server(host='0.0.0.0', port=7531):
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((host, port))
    s.listen()

    logs.put('Server running...')

    try:
        while True:
            conn, addr = s.accept()
            threading.Thread(target=receive_from_device, args=(conn, addr), daemon=True).start()
    except KeyboardInterrupt:
        s.close()
