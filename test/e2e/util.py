import requests

def http_get(base, path="/", headers=None):
    return requests.get(base + path, headers=headers or {})

def http_post(base, path="/", data=None, files=None, headers=None):
    return requests.post(base + path, data=data, files=files, headers=headers or {})

def http_delete(base, path="/", headers=None):
    return requests.delete(base + path, headers=headers or {})

def http_custom_method(base, method, path="/", headers=None):
    return requests.request(method, base + path, headers=headers or {})
