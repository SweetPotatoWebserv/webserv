from util import http_post

def test_post_basic(server):
    res = http_post(server, "/upload", data="hello", headers={"Host": "localhost"})
    assert res.status_code == 201

def test_post_body_limit(server):
    big = "A" * (1024 * 1024 * 2)  # 1MB body など
    res = http_post(server, "/upload", data=big, headers={"Host": "localhost"})
    assert res.status_code in (413, 400)

def test_file_upload(server):
    files = {"file": ("test.txt", b"UPLOAD")}
    res = http_post(server, "/upload", files=files, headers={"Host": "localhost"})
    assert res.status_code == 201
