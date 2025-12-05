import socket
from escpos.printer import Usb

HOST = '10.66.236.77'
PORT = 7531

def print_message(printer, message):
    printer.text(message.replace('\\n', '\n'))
    print(f"Printed message {message}")

def receive_from_server(conn):
    printer = Usb(0x0416, 0x5011, in_ep=0x81, out_ep=0x03, profile="POS-5890")
    try:
        while True:
            raw = conn.recv(1024).decode()
            if not raw:
                break

            action, param = raw.split(':', 1)
            if action == "print":
                print_message(printer, param)            
            else:
                conn.send(b'Action does not exist')
                continue

            conn.send(b'Action handled')
    finally:
        conn.close()

def start_client():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        print("Listening...")

        s.send('printer'.encode())
        receive_from_server(s)


start_client()