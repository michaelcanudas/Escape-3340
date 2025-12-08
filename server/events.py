import state
from state import logs


events = {}


def on(name):
    def wrapper(func):
        events.setdefault(name, []).append(func)
        return func
    return wrapper


def trigger(name, *args, **kwargs):
    for handler in events.get(name, []):
        handler(*args, **kwargs)


@on('reset')
def handle_reset():
    state.reset_state()
    logs.put('System state has been reset')

@on('magnetinactive')
def handle_magnet_inactive():
    # legacy event
    handle_sleigh_magnet('false')

@on('magnetactive')
def handle_magnet_active():
    # legacy event
    handle_sleigh_magnet('true')

@on('sleigh_magnet')
def handle_sleigh_magnet(is_present):
    from server import send_to_device
    if is_present == 'true':
        logs.put('Magnet detected on sleigh')
        state.sleigh_magnet_present = True
        check_sleigh_powered_on()
    elif is_present == 'false':
        logs.put('Magnet removed from sleigh')
        state.sleigh_magnet_present = False
        check_sleigh_powered_on()

@on('generator_magnet')
def handle_generator_magnet(is_present):
    from server import send_to_device
    if is_present == 'true':
        logs.put('Magnet detected on electrical generator')
        state.generator_magnet_present = True
        check_sleigh_powered_on()
    elif is_present == 'false':
        logs.put('Magnet removed from electrical generator')
        state.generator_magnet_present = False
        check_sleigh_powered_on()

@on('generator_code')
def handle_generator_code(code):
    from server import send_to_device
    logs.put(f'User entered code on electrical generator: {code}')
    
    if(code == '4093'):
        state.generator_powered_on = True
        logs.put('Electrical generator powered on successfully')
        send_to_device('generator_pot_7seg', "spinner")
        check_sleigh_powered_on()

def check_sleigh_powered_on():
    from server import send_to_device
    sleigh_powered_on = state.generator_powered_on and state.sleigh_magnet_present and state.generator_magnet_present
    if sleigh_powered_on:
        logs.put('Sleigh is now powered on!!!!')
        send_to_device('sleigh', 'activate')
    else:
        logs.put('Sleigh is not powered on.')
        send_to_device('sleigh', 'deactivate')


@on('present_tapped')
def handle_present_update(reader, uid):
    from server import send_to_device
    state.current_present_ids[int(reader)] = uid
    logs.put(f'Present tapped to reader {reader} updated with UID: {uid}')
    
    # If all presents are correct, print lockbox code to receipt
    if state.check_presents_correct() and not state.presents_lockbox_code_printed:
        logs.put('All presents are correctly placed! Unlocking lockbox...')
        send_to_device('printer', 'print:Hohoho! The children will be so happy! A little birdie told me the number\n\n792')
        state.presents_lockbox_code_printed = True
