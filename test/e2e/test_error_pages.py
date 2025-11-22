from util import http_get

def test_index(server):
    res = http_get(server, "/dir/", headers={"Host": "localhost"})
    assert res.status_code == 200

def test_autoindex(server):
    res = http_get(server, "/list/", headers={"Host": "localhost"})
    assert res.status_code == 200
    assert "<a href=" in res.text
