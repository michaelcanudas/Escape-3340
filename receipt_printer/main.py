import socket
import time
from escpos.printer import Usb


HOST = '193.122.147.106'
PORT = 7531


def print_message(printer, args):
    try:
        printer.text(args.replace('\\n', '\n').strip())
        print(f"Printed: {args}")
    except Exception as e:
        print(f"Printer error: {e}")


def receive_from_server(conn):
    printer = Usb(0x0416, 0x5011, in_ep=0x81, out_ep=0x03, profile="POS-5890")
    try:
        while True:
            raw = conn.recv(1024).decode()
            if not raw:
                break
            try:
                command, args = raw.split(':', 1)
            except ValueError:
                print(f"Invalid message: {raw}")
                continue

            if command == "print":
                print_message(printer, args)
    finally:
        conn.close()


def start_client():
    while True:
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((HOST, PORT))
                print("Listening...")
                s.send('printer'.encode())

                receive_from_server(s)
        except Exception as e:
            print(f"Connection failed, retrying in 2s... ({e})")
            time.sleep(2)


start_client()
