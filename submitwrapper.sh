#!/bin/bash
source /cvmfs/cms.cern.ch/cmsset_default.sh

DIJETDIR=/nfs/dust/cms/user/loemkerj/bachelor/CMSSW_10_2_13/src/DijetCombineLimitCode

cd $DIJETDIR
eval `scram runtime -sh`

python submitSteps.py $1 $2 $3 $4
