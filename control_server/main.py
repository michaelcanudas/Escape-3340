import threading
from server import start_server, shutdown_server
from cli import run_cli, shutdown_cli
import signal
import sys


def shutdown(sig, frame):
    shutdown_server()
    shutdown_cli()
    print('> Exiting...')
    sys.exit(0)


signal.signal(signal.SIGINT, shutdown)

threading.Thread(target=start_server, daemon=True).start()
run_cli()
