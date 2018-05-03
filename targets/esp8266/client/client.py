#!/usr/bin/env python

import socket
import os
import sys
from time import sleep, gmtime, strftime
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("ip", help="ESP8266 ip:port")
args = parser.parse_args()

remote_ip = args.ip;

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    host, port = remote_ip.split(":")
    port = int(port)
    server_address = (host, port)
    s.bind(server_address)
    s.listen(1)
except socket.error:
    print 'Failed to create socket'
    sys.exit()
s.settimeout(5)
print 'Socket Created'

while True:
    print ("Waiting for connection:")
    while True:
        try:
            connection, client_address = s.accept()
            break;
        except Exception as e:
            sleep(1)

    received = ""
    curr_pos = 0
    file_opened = False
    output_file = None

    while True:
        try:
            data = connection.recv(4096)
            if not data:
                break
            received += data
        except socket.timeout:
            break

    content = ""
    content_length = 0

    while curr_pos < len(received):
        header_info_start = 0
        header_info_end = 0

        content_start = 0
        content_end = 0

        for i, c in enumerate(received[curr_pos:]):
            if c == '#' and header_info_start == 0 and content_start == 0:
                header_info_start = curr_pos + i + 1
            elif c == '#' and header_info_start != 0:
                header_info_end = curr_pos + i
            elif c == '&' and content_start == 0:
                content_start = curr_pos + i + 1

            if header_info_end != 0:
                content_length = int (received[header_info_start : header_info_end], 16)
                header_info_start = 0
                header_info_end = 0

            if content_start != 0 and content_length != 0:
                content = received[content_start : content_start + content_length]
                curr_pos += i + content_length + 2
                content_start = 0
                content_end = 0
                break

        if not file_opened and content.startswith('/') and content.split('.')[1] in ['txt', 'json', 'jpg']:
            file_mode =  "wb" if content.split('.')[1] == 'jpg' else "w"
            whole_path = "data" + content
            print ("Write to: ", whole_path)
            try:
                if not os.path.exists(os.path.dirname(whole_path)):
                    try:
                        os.makedirs(os.path.dirname(whole_path))
                    except OSError as exc:
                        if exc.errno != errno.EEXIST:
                            raise
                output_file = open(whole_path, file_mode)
                file_opened = True
            except IOError as e:
                print("Couldn't open file (%s)." % e)
        elif content != '':
            newFileByteArray = bytearray(content)
            try:
                output_file.write(newFileByteArray)
            except IOError as e:
                print("Couldn't write to file (%s)." % e)
    output_file.close();
