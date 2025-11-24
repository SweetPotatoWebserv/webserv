import subprocess
import json

def test_siege_stress(server):
    cmd = ["siege", "-b", "-t5S", server + "/"]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    j = json.loads(proc.stdout)
    assert j["availability"] == 100.00
