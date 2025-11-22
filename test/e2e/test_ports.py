import socket

def test_multiple_ports():
    # 8080 と 9090 の両方が設定されている想定
    for port in (8080, 9090):
        s = socket.socket()
        assert s.connect_ex(("127.0.0.1", port)) == 0
        s.close()
