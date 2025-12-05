import threading
from server import logs, devices, send_to_device

def display_logs():
    while True:
        try:
            log = logs.get(timeout=0.5)
            print(f'> {log}')
        except:
            pass

def run_cli():
    threading.Thread(target=display_logs, daemon=True).start()

    logs.put('CLI running...')
    logs.put('Input format: [device]:[command]')
    logs.put('')

    while True:
        try:
            user_input = input('> ').strip()
            if not user_input:
                continue

            try:
                device, command = user_input.split(maxsplit=1)
            except ValueError:
                logs.put('Command has invalid format')
                continue

            if device not in devices:
                logs.put('Device is invalid')
                continue

            if send_to_device(device, command):
                logs.put(f'Sent command to {device}: {command}')
            else:
                logs.put(f'Failed to send command to {device}: {command}')
        except KeyboardInterrupt:
            logs.put('Exiting CLI...')
            break
