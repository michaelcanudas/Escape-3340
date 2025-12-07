from cli import logs


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
    logs.put('Sleigh magnet is now active')


@on('magnetinactive')
def handle_magnet_inactive():
    logs.put('Sleigh magnet is now inactive')
