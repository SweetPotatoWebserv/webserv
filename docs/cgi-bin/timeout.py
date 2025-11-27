#!/usr/bin/env python3
import time
# タイムアウトを引き起こすために長時間スリープ
time.sleep(10)

print("Content-Type: text/plain")
print("Status: 200 OK")
print("")
print("You should not see this.")
