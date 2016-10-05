#!/usr/bin/python

from sys import argv
import time
import os

# runlistDir = '/global/u2/s/sjmeijer/P3KJR_silver_runlist/'
# runlistDir = '/global/u2/s/sjmeijer/DS0_silver_clint/'
runlistDir = '/global/project/projectdirs/majorana/users/sjmeijer/livetime/DS3/runlist_m1/'
# runlistDir = '/global/u2/s/sjmeijer/mjd/analysis/livetime/DS3/runlist_m1/'
f = os.listdir(runlistDir)

toRun = [];

for entry in f:
    # tmp = int(entry.split('_')[1])  # for P3KJR_silver_runlist
    # tmp = int(entry.split('silver_')[1])    # for DS0_silver_clint
    tmp = int(entry.split('_m1_')[1])    # for DS0_runlist_split and DS1_runlist_split
    toRun.append( tmp )

exp = len(toRun)

print toRun
print "Expecting ", exp, " jobs to be submitted."
print "Starting in 5 seconds..."
time.sleep(1)
print "4..."
time.sleep(1)
print "3..."
time.sleep(1)
print "2..."
time.sleep(1)
print "1..."
time.sleep(1)
print "Beginning submission..."
time.sleep(1) # I lied and gave you 6 seconds


count = 0;
for aList in toRun:
    # run,channel = selection.split('_')
    cmd = "qsub -P majorana -l projectio=1 -j y /global/u2/s/sjmeijer/lt_run.py %d" % (aList)
    print "\n"
    print cmd
    os.system(cmd)
    count = count + 1
    print count, " / ", exp

    # if(count > 5000):
    #     print "Reached my limit of 5000 jobs submitted, quitting submission"
    #     quit()
    # subprocess.call(cmd)
    # call(["qsub -P majorana -l projectio=1 /global/u2/s/sjmeijer/runL.csh",str(run)])

print "\n\n *****\n"
print "Submitted ", count," jobs."
