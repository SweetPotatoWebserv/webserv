import subprocess

def test_siege_stress(server):
    cmd = ["siege", "-b", "-t5S", server + "/"]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    assert "Availability: 100.00 %" in proc.stdout or "Availability: 99" in proc.stdout
