from util import http_get

def test_get_basic(server):
    res = http_get(server, "/", headers={"Host": "localhost"})
    assert res.status_code == 200
    assert "text/html" in res.headers["Content-Type"]

def test_get_not_found(server):
    res = http_get(server, "/no-such-file", headers={"Host": "localhost"})
    assert res.status_code == 404

def test_get_host_required(server):
    import socket
    s = socket.socket()
    s.connect(("127.0.0.1", 8080))
    s.send(b"GET / HTTP/1.1\r\n\r\n")  # Host 無し
    data = s.recv(4096).decode()
    s.close()
    assert "400" in data

