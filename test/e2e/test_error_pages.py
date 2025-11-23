from util import http_get

def test_error_override(server):
    res = http_get(server, "/hoge", headers={"Host": "localhost"})
    assert res.status_code == 200

def test_error_page(server):
    res = http_get(server, "/hoge", headers={"Host": "localhost"})
    assert res.status_code == 404
    assert "<p>404</p>" == res.text
