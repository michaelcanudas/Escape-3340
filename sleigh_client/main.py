import socket
import threading
import time
import signal
import sys
from gpiozero import DigitalInputDevice, DigitalOutputDevice

HOST = '10.66.236.77'
PORT = 7531

HALL_PIN = 14
RELAY_PIN = 2

hall = DigitalInputDevice(HALL_PIN, pull_up=True)
relay = DigitalOutputDevice(RELAY_PIN)

running = True


def activate():
    relay.on()
    print('> Activated')


def deactivate():
    relay.off()
    print('> Deactivated')


def magnet_update(conn):
    hall_was_active = False

    while running:
        if hall.is_active and not hall_was_active:
            send_to_server(conn, 'event:magnetactive')
        elif not hall.is_active and hall_was_active:
            send_to_server(conn, 'event:magnetinactive')

        hall_was_active = hall.is_active
        time.sleep(0.1)


def send_to_server(conn, message):
    try:
        conn.send(message.encode())
        print(f'> Sent message to server: {message}')
    except Exception as e:
        print(f'> Failed to send message to server: {message} ({e})')


def receive_from_server(conn):
    try:
        while running:
            command = conn.recv(1024).decode()
            if not command:
                break

            if command == 'activate':
                activate()
            elif command == 'deactivate':
                deactivate()

    except Exception as e:
        print(f'> Server read error: {e}')

    finally:
        try:
            conn.close()
        except:
            pass


def connect():
    while running:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((HOST, PORT))
            print('> Connected')

            s.send('sleigh'.encode())
            return s

        except Exception as e:
            print(f'> Connection failed, retrying in 2s... ({e})')
            time.sleep(2)

    return None


def start_client():
    while running:
        conn = connect()
        if conn is None:
            break

        magnet_thread = threading.Thread(
            target=magnet_update,
            args=(conn,),
            daemon=True
        )
        magnet_thread.start()

        receive_from_server(conn)

        print('> Disconnected from server, reconnecting...')


def shutdown():
    global running
    running = False
    deactivate()
    print('> Shutting down...')
    sys.exit(0)


signal.signal(signal.SIGINT, lambda sig, frame: shutdown())


deactivate()
start_client()
