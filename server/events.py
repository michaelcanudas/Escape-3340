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


@on('magnetactive')
def handle_magnet_active():
    from server import send_to_device
    send_to_device('lights', 'red')
    logs.put('Sleigh magnet is now active')


@on('magnetinactive')
def handle_magnet_inactive():
    from server import send_to_device
    send_to_device('lights', 'white')
    logs.put('Sleigh magnet is now inactive')

@on('presentupdate')
def handle_present_update(reader, uid):
    from server import send_to_device
    send_to_device('lights', 'red')
    logs.put(f'Reader {reader} updated with UID: {uid}')

@on('generator_code')
def handle_generator_code(code):
    logs.put(f'User entered code on electrical generator: {code}')

@on('generator_magnet')
def handle_generator_magnet(magnet_present):
    if(magnet_present == 'true'):
        logs.put('Magnet detected on electrical generator')
    else:
        logs.put('Magnet removed from electrical generator')
