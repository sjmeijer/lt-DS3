#!/bin/csh
echo Beginning sjmeijer ROOT script

set r1=$1

echo Running on List $r1
# get the MJD things
echo Getting MJD setup
source /common/majorana/scripts/setupMajorana.csh

# move into a "safe" directory
# (https://www.nersc.gov/users/computational-systems/pdsf/using-the-sge-batch-system/submitting-jobs/)
cd $TMPDIR

set pHomeDir = '/global/project/projectdirs/majorana/users/sjmeijer'
set homeDir = '/global/u2/s/sjmeijer'

set finalDir = $pHomeDir/livetime/DS3/m1_out

#cd $pHomeDir

#echo root -b -q "$homeDir/mjd/analysis/livetime/lt_single.cxx($r1)"
#root -b -q "$homeDir/mjd/analysis/livetime/lt_single.cxx($r1)"
#root -b -q "$homeDir/mjd/analysis/livetime/lt_single_hit.cxx($r1)"
root -b -q -L+ "/global/u2/s/sjmeijer/mjd/analysis/livetime/DS3/livetime_checks.cxx($r1)"

echo Moving files out to $finalDir ...
#mv *.pdf $finalDir
#mv *.png $finalDir
mv *.root $finalDir

echo Shell script completed
