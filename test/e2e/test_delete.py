from util import http_delete, http_get
import os

def test_delete_file(server):
    path = "docs/delete/to_delete.txt"
    with open(path, "w") as f:
        f.write("delete me")

    res = http_delete(server, "/delete/to_delete.txt", headers={"Host": "localhost"})
    assert res.status_code == 200 or res.status_code == 204

def test_delete_not_found(server):
    res = http_delete(server, "/delete/nope.txt", headers={"Host": "localhost"})
    assert res.status_code == 404

def test_delete_not_allowed(server):
    res = http_delete(server, "/", headers={"Host": "localhost"})
    assert res.status_code == 405
