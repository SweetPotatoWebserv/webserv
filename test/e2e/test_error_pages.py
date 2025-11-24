from util import http_get, http_post, http_delete

def test_error_override(server):
    res = http_post(server, "/error", headers={"Host": "localhost"})
    assert res.status_code == 200

def test_error_page(server):
    res = http_delete(server, "/error", headers={"Host": "localhost"})
    assert res.status_code == 403
    assert "<p>403</p>" in res.text
