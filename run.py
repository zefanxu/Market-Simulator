#!/usr/bin/env python
import subprocess, time
text1 = 'om send_order B 90000 SPY.ARCX 234.56 BATS\n'
text2 = 'om cancel 9100\n'

evtsim_proc = subprocess.Popen(
    ['./evtsim','-b', '11035'])

evts_proc = subprocess.Popen(
    ['../evts/evts', '-c', 'evts.config'])

time.sleep(2)

admin_proc = subprocess.Popen(
    ['/evt/apps/infra/prod/bin/admin', 'localhost:12014'],stdout=subprocess.PIPE,
    stdin=subprocess.PIPE)
time.sleep(3)

if (evtsim_proc.poll() == None):
    print "Fail to run evtsim"
    exit()
if (admin_proc.poll() == None):
    print "Fail to run admin console"
    exit()
if (evts_proc.poll() == None):
    print "Fail to run evts"
    exit()

admin_proc.stdin.write(text1)
time.sleep(3)
admin_proc.stdin.write(text2)
time.sleep(1)



admin_proc.kill()
evtsim_proc.kill()
evts_proc.kill()

#print admin_proc.stdout.read()
