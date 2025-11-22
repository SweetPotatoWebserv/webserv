from util import http_get

def test_route_mapping(server):
    res = http_get(server, "/foo/hello.txt", headers={"Host": "localhost"})
    assert res.status_code == 200
    assert "FOO HELLO" in res.text

