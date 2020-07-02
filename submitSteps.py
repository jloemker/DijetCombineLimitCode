#!/usr/bin/env python
import os,glob,sys

if (__name__=='__main__'):
    mode=sys.argv[1]
    if(mode=='fit'):
        steps=[1,2,3]
        coupling=sys.argv[3]
    elif(mode=='plot'):
        steps=[4]
        coupling=''
    signal=sys.argv[2]
    parameter=signal[-2:]
    print signal, coupling , steps

    variable = sys.argv[4] if len(sys.argv)>4 else ""
    name=signal+"_"+coupling+variable
    print('name:',name)
    # exit(0)
    if 1 in steps:
        os.system('root -b -q "MiniTreeProducerDataUHH_cut.C(\\"\\",\\"\\",\\"'+str(name)+'\\")"')
        os.system('root -b -q "MiniTreeSignalProducerUHH_cuts.C(10,11,0,\\"'+str(name)+'\\")"')
    #Fit and create datacards
    if 2 in steps:
        os.system('root -b -q "UHHFitter_cuts_newStrategy.cc(\\"'+str(name)+'\\",0,10,0,\\\"\\\")"') # VV-notVBF
        os.system('root -b -q "UHHFitter_cuts_newStrategy.cc(\\"'+str(name)+'\\",0,10,1,\\\"\\\")"') # VBF
        os.system('python Limits/CombineDatacardsUHH_cuts.py 0 '+str(name))
    #run combine
    if 3 in steps:
        os.system('python Limits/CalcAsympLimitsUHH_cuts.py 0 '+str(name))
    if 4 in steps:
        os.system('python Limits/brazilianFlag_aQTGC.py '+str(signal)+' '+variable)
