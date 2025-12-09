import threading
import queue

devices = {}
devices_lock = threading.Lock()
logs = queue.Queue()
running = True

present_correct_ids = [
    "04c20d723e6180",
    "040c41ce2e6180",
    "045b98713e6180",
    "045616664e6180"
]

present_id_to_name = {
    "043362cc2e6180": "cello",
    "048b25664e6180": "saxophone",
    "049d8fcd2e6180": "flute",
    "045b98713e6180": "nutcracker",
    "04c20d723e6180": "hula hoop",
    "042c566f3e6180": "golden ring",
    "04065c703e6180": "toy rocket",
    "040c41ce2e6180": "labubu",
    "0442ab723e6180": "needlenose pliers",
    "04f6556f3e6180": "marshmallow",
    "04f6a7713e6180": "nintendo switch 2",
    "0416c96e3e6180": "cheeseburger",
    "040304723e6180": "violin",
    "045616664e6180": "oreo"

}

present_reader_names = [
    "Mr. Seville",
    "Andy",
    "Patricia",
    "Bart"
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
is_voting_period = False
last_map_check_time = -1

ready_for_finish = False

def check_presents_correct():
    for i in range(len(present_correct_ids)):
        if present_correct_ids[i] != current_present_ids[i]:
            return False
    return True

def reset_state():
    global ready_for_finish, current_present_ids, presents_lockbox_code_printed, sleigh_magnet_present, generator_magnet_present, generator_powered_on, is_voting_period, last_map_check_time
    current_present_ids = [0, 0,0, 0]
    presents_lockbox_code_printed = False
    sleigh_magnet_present = False
    generator_magnet_present = False
    generator_powered_on = False
    is_voting_period = False
    last_map_check_time = -1
    ready_for_finish = False