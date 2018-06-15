#!/usr/bin/env python

import os
from subprocess import Popen
from time import sleep
from socket import create_connection, error
from pyester import PyEster
import atexit

ester_process = None

def setHost(host, port):
    global DEFAULT_ADDR
    DEFAULT_ADDR = (host, port)

# Set default values
setHost("localhost", 50004)

def get_ester(addr = DEFAULT_ADDR):
    s = PyEster()
    (s.settings.host, s.settings.port) = addr
    return s


def is_running(addr = DEFAULT_ADDR):
    try:
        s = create_connection(addr, 5)
        s.close()
        return True
    except error:
        return False


def start_ester(addr = DEFAULT_ADDR):
    global ester_process
    if not ester_process is None:
        stop_ester()
    cmd = ["ester", "--port", str(addr[1]) ]

    if is_running(addr):
        raise RuntimeError("Could ester server is already running")

    ester_process = Popen(cmd)
    # Check if server is running
    no_tries = 0
    while no_tries < 5:
        if is_running(addr):
            return
        else:
            sleep(1)
            no_tries += 1
    else:
        stop_ester()
        raise RuntimeError("Could not start ester server")

@atexit.register
def stop_ester():
    global ester_process
    if not ester_process is None:
        ester_process.terminate()
        sleep(1)
        if ester_process.poll() is None:
            ester_process.kill()
        ester_process.wait()
        ester_process = None
        if is_running():
            raise RuntimeError("Server still running")
