import threading
from server import start_server
from cli import run_cli

threading.Thread(target=start_server, daemon=True).start()

run_cli()
