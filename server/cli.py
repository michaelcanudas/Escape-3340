import threading
from server import send_to_device
from events import trigger
from state import devices, logs, running
from prompt_toolkit import PromptSession
from prompt_toolkit.patch_stdout import patch_stdout


def display_logs():
    while running:
        try:
            log = logs.get(timeout=0.5)
            with patch_stdout():
                print(log)
        except:
            continue


def run_cli():
    global running

    session = PromptSession()
    threading.Thread(target=display_logs, daemon=True).start()
    
    logs.put('CLI running...')
    logs.put('Input format: [device]:[command] or event:[name]')

    while running:
        try:
            user_input = session.prompt('> ').strip()
            if not user_input:
                continue

            try:
                kind, data = user_input.split(':', 1)
                if kind == 'event':
                    trigger(data)
                    continue

                with threading.Lock():
                    if kind in devices:
                        send_to_device(kind, data)
                        continue

                logs.put(f'Command has invalid kind: {user_input}')
            except Exception as e:
                logs.put(f'Failed to process command: {e}')
        except KeyboardInterrupt:
            break


def shutdown_cli():
    global running
    running = False
