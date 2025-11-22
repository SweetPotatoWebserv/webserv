import subprocess
import time
import requests
import pytest
import os
import signal
import socket

PORT = 8080
SERVER_CMD = ["./webserv"]

def wait_server(port, timeout=5):
    start = time.time()
    while time.time() - start < timeout:
        try:
            s = socket.socket()
            s.settimeout(0.1)
            s.connect(("127.0.0.1", port))
            s.close()
            return True
        except:
            time.sleep(0.1)
    return False

@pytest.fixture(scope="session")
def server():
    proc = subprocess.Popen(SERVER_CMD)

    assert wait_server(PORT), "Server did not start"
    yield f"http://127.0.0.1:{PORT}"

    os.kill(proc.pid, signal.SIGTERM)
    proc.wait()
