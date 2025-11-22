import socket

def test_invalid_header(server):
    s = socket.socket()
    s.connect(("127.0.0.1", 8080))
    s.send(b"GET / HTTP/1.1\r\nHost\r\n\r\n")  # 壊れたヘッダ
    data = s.recv(4096).decode()
    s.close()
    assert "400" in data

def test_disconnect_mid_request(server):
    s = socket.socket()
    s.connect(("127.0.0.1", 8080))
    s.send(b"POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10\r\n\r\n123")
    s.close()  # body 未完成で切断
    # クラッシュしないことが目的
