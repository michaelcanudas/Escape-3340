import state
from state import logs
import time


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
    from server import send_to_device
    state.reset_state()
    send_to_device('generator_pot_7seg', 'display')
    send_to_device('sleigh', 'deactivate')
    send_to_device('lights', 'white')
    logs.put('System state has been reset')

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
        state.ready_for_finish = True
    else:
        logs.put('Sleigh is not powered on.')
        send_to_device('sleigh', 'deactivate')




@on('present_tapped')
def handle_present_update(reader, uid):
    from server import send_to_device
    state.current_present_ids[int(reader)] = uid

    if not uid in state.present_id_to_name:
        logs.put(f'Present tapped to reader {reader} has unknown UID: {uid}')
        return
    logs.put(f'Present tapped to reader {state.present_reader_names[int(reader)]}: {state.present_id_to_name[uid]}')
    
    # If all presents are correct, print lockbox code to receipt
    if state.check_presents_correct() and not state.presents_lockbox_code_printed:
        logs.put('All presents are correctly placed! Unlocking lockbox...')
        send_to_device('printer', 'print:Hohoho! The children will be so happy! A little birdie told me the number\n\n792')
        state.presents_lockbox_code_printed = True

@on('power_generator')
def power_generator():
    from server import send_to_device
    
    state.generator_powered_on = True
    send_to_device('generator_pot_7seg', "spinner")

@on('map_submit')
def handle_map_submit(data):
    from server import send_to_device
    if not state.ready_for_finish:
        logs.put(f'Attempted to submit map without completing other puzzles')
        send_to_device('printer', 'print:Santa recommends focusing on other puzzles before the map...')
        return
    
    logs.put(f'Map data submitted: {data}')

    if state.is_voting_period:
        logs.put('Map submission ignored: voting period already active')
        return

    if state.last_map_check_time != -1 and time.time() - state.last_map_check_time < 20:
        logs.put('Map submission ignored: submitted too soon after last submission')
        send_to_device('printer', 'print:ERROR! Naughty/nice map overloaded. Please wait 20 seconds before resubmitting.')
        return

    state.last_map_check_time = time.time()

    if data.strip() == '11001':
        state.is_voting_period = True
        send_to_device('lights', 'red')
        logs.put('!!!!! Map completed successfully !!!!! Time to voice act! Your script:\n\n')
        logs.put('Sunday, December 7th. Today, I broke into Santa’s Workshop. He had a lot of cool gadgets, including a present tracker, a map, and even a whole freaking sleigh! Ugh, so disgusting. Why would anyone go to such lengths to deliver presents to a bunch of ungrateful little brats? Anyways, I messed everything up. Shuffled around the kids’ letters, mismatched presents to kids, changed the colors on the map, unplugged Santa’s sleigh… you know, typical Grinch stuff. That fat man wearing the ugly-ass red costume won’t see what’s coming.')
        logs.put('Monday, December 8th. Ugh, it looks like Santa’s sending a bunch of people into his Workshop to repair it today. I spent all that time messing everything up… and he’s just gonna get back to delivering presents, despite all that?\n\n...\n\nwait… I have an idea. I can disguise myself as a human and pretend I’m one of the people sent to repair the workshop. While they try to put everything back in order, I’ll be sabotaging them the whole time. I’ll create chaos amongst friends, give out false information, and even switch things around when they’re not looking. Now, I just need to choose a fake name to tell them… hopefully they don’t discuss amongst themselves, decide who The Grinch is, and vote me out!')
    else:
        logs.put('Player submitted incorrect map solution: ' + data)
        logs.put('(Order: Mr. Seville, Patricia, Rohan, Andy, Bart)')
