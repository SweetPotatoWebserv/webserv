from util import http_get, http_post

def test_cgi_get(server):
    res = http_get(server, "/cgi-bin/test.py?x=1", headers={"Host": "localhost"})
    assert res.status_code == 200
    assert "x=1" in res.text

def test_cgi_post(server):
    res = http_post(server, "/cgi-bin/test.py", data="abc=123", headers={"Host": "localhost"})
    assert res.status_code == 200
    assert "abc=123" in res.text

def test_cgi_error(server):
    res = http_get(server, "/cgi-bin/error.py", headers={"Host": "localhost"})
    assert res.status_code == 500

