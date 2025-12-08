import threading
import queue

devices = {}
devices_lock = threading.Lock()
logs = queue.Queue()
running = True