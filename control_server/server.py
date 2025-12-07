import socket
import threading
from cli import logs
from events import trigger


devices = {}
devices_lock = threading.Lock()
running = True


def receive_from_device(conn, addr):
    global running
    device_id = None
    
    try:
        conn.settimeout(0.5)
        device_id = conn.recv(1024).decode().strip()
        if not device_id:
            logs.put(f'{addr} failed to send device id, closing')
            return

        with devices_lock:
            devices[device_id] = conn

        logs.put(f'{addr} connected with id: {device_id}')

        while running:
            try:
                raw = conn.recv(1024).decode().strip()
                if not raw:
                    break

                if ':' not in raw:
                    logs.put(f'{device_id} sent message with invalid format: {raw}')
                    continue

                kind, data = raw.split(':', 1)

                if kind == 'event':
                    trigger(data)
                    continue

                with devices_lock:
                    if kind in devices:
                        send_to_device(kind, data)
                        continue

                logs.put(f'{device_id} sent message with invalid kind: {raw}')

            except socket.timeout:
                continue
            except Exception as e:
                logs.put(f'{device_id} recv error: {e}')
                break

    finally:
        conn.close()
        if device_id:
            with devices_lock:
                devices.pop(device_id, None)
            logs.put(f'{device_id} disconnected')


def send_to_device(device, command):
    with devices_lock:
        conn = devices.get(device)
    if not conn:
        logs.put(f'Failed to send command to {device}: {command} (not connected)')
        return
    
    try:
        conn.sendall(command.encode())
        logs.put(f'Sent command to {device}: {command}')
    except Exception as e:
        logs.put(f'Failed to send command to {device}: {command} ({e})')


def start_server(host='0.0.0.0', port=7531):
    global running

    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((host, port))
    s.listen()

    logs.put('Server running...')

    try:
        while running:
            try:
                conn, addr = s.accept()
                threading.Thread(target=receive_from_device, args=(conn, addr), daemon=True).start()
            except Exception:
                continue
    finally:
        s.close()
        logs.put('Server stopped')


def shutdown_server():
    global running
    running = False
