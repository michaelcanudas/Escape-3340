import threading
import queue

devices = {}
devices_lock = threading.Lock()
logs = queue.Queue()
running = True

present_correct_ids = [
    "04C20D723E6180",
    "040C41CE2E6180",
    "045B98713E6180",
    "045616664E6180"
]

current_present_ids = [
    0,
    0,
    0,
    0
]

presents_lockbox_code_printed = False

sleigh_magnet_present = False
generator_magnet_present = False
generator_powered_on = False

def check_presents_correct():
    for i in range(len(present_correct_ids)):
        if present_correct_ids[i] != current_present_ids[i]:
            return False
    return True

def reset_state():
    current_present_ids = [0, 0,0, 0]
    presents_lockbox_code_printed = False
    sleigh_magnet_present = False
    generator_magnet_present = False
    generator_powered_on = False