import threading
from server import logs, devices, send_to_device
from prompt_toolkit import PromptSession
from prompt_toolkit.patch_stdout import patch_stdout

def display_logs():
    while True:
        log = logs.get()
        with patch_stdout():
            print(log)

def run_cli():
    session = PromptSession()
    threading.Thread(target=display_logs, daemon=True).start()
    
    logs.put('CLI running...')
    logs.put('Input format: [device]:[command]')

    while True:
        try:
            user_input = session.prompt('> ').strip()
            if not user_input:
                continue

            try:
                device, command = user_input.split(maxsplit=1)
            except ValueError:
                logs.put('Command has invalid format')
                continue

            if device == 'server':
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
