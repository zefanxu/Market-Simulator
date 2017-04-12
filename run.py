#!/usr/bin/env python
import subprocess, time
command = []
command.append('om send_order B 90000 SPY.ARCX 234.56 BATS')
command.append('om cancel_down 9100 30000')
command.append('om cancel 9100')
command.append('om send_order B 3000 SPY.ARCX 234.00 BATS')
command.append('om replace 9101 3000 233.00')

evtsim_proc = subprocess.Popen(
    ['./evtsim','-b', '11035'])

evts_proc = subprocess.Popen(
    ['../evts/evts', '-c', 'evts.config'])

time.sleep(3)

admin_proc = subprocess.Popen(
    ['/evt/apps/infra/prod/bin/admin', 'localhost:12014'],stdout=subprocess.PIPE,
    stdin=subprocess.PIPE)
time.sleep(2)

if (evtsim_proc.poll() != None):
    print "Fail to run evtsim"

if (admin_proc.poll() != None):
    print "Fail to run admin console"

if (evts_proc.poll() != None):
    print "Fail to run evts"

if ((evtsim_proc.poll() != None) or (admin_proc.poll()!= None) or (evts_proc.poll() != None)):
    try:
        admin_proc.kill()
        evtsim_proc.kill()
        evts_proc.kill()
    except:
        pass
    exit()

for each_command in command:
    admin_proc.stdin.write(each_command+'\n')
    time.sleep(2)


admin_proc.kill()
evtsim_proc.kill()
evts_proc.kill()

#print out admin console output
#print admin_proc.stdout.read()
