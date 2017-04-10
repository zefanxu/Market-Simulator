#!/usr/bin/env python
import subprocess, time
text1 = 'om send_order B 100 SPY.ARCX 234.56 BATS'
text2 = 'om cancel 9100'
evtsim_proc = subprocess.Popen(
    './evtsim -boeport 33333')
admin_proc = subprocess.Popen(
    '/evt/apps/infra/prod/bin/admin localhost:12011',stdout=subprocess.PIPE,
    stdin=subprocess.PIPE)
time.sleep(3)
admin_proc.stdin.write(text1)
time.sleep(5)
admin_proc.stdin.write(text2)
time.sleep(2)
admin_proc.stdin.close()
result = admin_proc.stdout.read()
#print result
admin_proc.terminate()
evtsim_proc.terminate()
