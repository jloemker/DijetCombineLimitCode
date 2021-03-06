import array, math
import os, sys

channels=["graviton","radion"]
masses=[int(sys.argv[1])]
fullToys=False

for chan in channels:
    print "chan =",chan
    
    bins=["_invMass","_invMass_afterVBFsel"]

    if fullToys:
      points=[]
      for p in range(1,10):
          points+=[float(p/10.)]
          points+=[float(p/10.+0.05)]
          points+=[float(p/1.)]
          points+=[float(p/1.+0.5)]
          points+=[float(p*10.)]
          points+=[float(p*10.+5.)]
    else:
      points=[0.1]
    for bin in bins:

        for mass in masses:
          print "mass =",mass

          for point in points:

            outputname = "CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_limit"+str(int(point*10))+"_submit.src"
            logname = "CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_limit"+str(int(point*10))+"_submit.out"
            outputfile = open(outputname,'w')
            outputfile.write('#!/bin/bash\n')
            outputfile.write("cd ${CMSSW_BASE}/src/DijetCombineLimitCode; eval `scramv1 run -sh`\n")
            if fullToys:
              outputfile.write("combine datacards/CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+".txt -M HybridNew --frequentist --clsAcc 0 -T 100 -i 30 --singlePoint "+str(point)+" -s 10000"+str(int(point*100))+" --saveHybridResult --saveToys -m "+str(mass) + " -n "+chan+str(bin)+" &>CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_toy"+str(int(point*10))+"_fullCLs.out\n")
              outputfile.write("hadd -f grid_mX"+str(mass)+"_" + chan + "_13TeV_"+bin+".root higgsCombine" + chan + str(bin)+".HybridNew.mH"+str(int(mass))+".10000*.root\n")
              outputfile.write("combine datacards/CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+".txt -M HybridNew --frequentist --grid grid_mX"+str(mass)+"_" + chan + "_13TeV_"+bin+".root -m "+str(mass) + " -n "+chan+str(bin)+" &>CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_obs_fullCLs.out\n")
              outputfile.write("combine datacards/CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+".txt -M HybridNew --frequentist --grid grid_mX"+str(mass)+"_" + chan + "_13TeV_"+bin+".root -m "+str(mass) + " -n "+chan+str(bin)+" --expectedFromGrid 0.5 &>CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_50_fullCLs.out\n")
              outputfile.write("combine datacards/CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+".txt -M HybridNew --frequentist --grid grid_mX"+str(mass)+"_" + chan + "_13TeV_"+bin+".root -m "+str(mass) + " -n "+chan+str(bin)+" --expectedFromGrid 0.16 &>CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_16_fullCLs.out\n")
              outputfile.write("combine datacards/CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+".txt -M HybridNew --frequentist --grid grid_mX"+str(mass)+"_" + chan + "_13TeV_"+bin+".root -m "+str(mass) + " -n "+chan+str(bin)+" --expectedFromGrid 0.84 &>CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_84_fullCLs.out\n")
              outputfile.write("combine datacards/CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+".txt -M HybridNew --frequentist --grid grid_mX"+str(mass)+"_" + chan + "_13TeV_"+bin+".root -m "+str(mass) + " -n "+chan+str(bin)+" --expectedFromGrid 0.025 &>CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_025_fullCLs.out\n")
              outputfile.write("combine datacards/CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+".txt -M HybridNew --frequentist --grid grid_mX"+str(mass)+"_" + chan + "_13TeV_"+bin+".root -m "+str(mass) + " -n "+chan+str(bin)+" --expectedFromGrid 0.975 &>CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_975_fullCLs.out\n")
            else:
                outputfile.write("combine datacards/CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+".txt -M Asymptotic -v2 -m "+str(mass) + " -n "+chan+str(bin)+" --rMax 100 --rMin 0.000001 &>CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_asymptoticCLs.out\n")
#                outputfile.write("combine datacards/CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+".txt -M MaxLikelihoodFit -v2 -m "+str(mass) + " -n "+chan+str(bin)+" --rMax 100 --rMin 0.000001 &>CMS_jj_"+chan+"_"+str(mass)+"_13TeV_"+bin+"_MaxLikelihoodFit.out --plots --out /nfs/dust/cms/user/zoiirene/CombineTutorial/CMSSW_8_1_0/src/DijetCombineLimitCode/plots\n")
                outputfile.write("mv higgsCombine"+chan+str(bin)+".Asymptotic.mH"+str(int(mass))+".root Limits/CMS_jj_"+str(mass)+"_"+chan+"_13TeV_"+bin+"_asymptoticCLs_new.root")
            outputfile.close()
  
            command="rm "+logname
            print command
            os.system(command)
            if fullToys:
              command="""bsub -q 8nh -o """+logname+" source "+outputname
              # command="""qsub -q all.q """+outputname+" source "+outputname
               # submitJobsOnT3batch.sh
            else:
              command="chmod 755 ./"+outputname+";./"+outputname
              print command
              os.system(command)
              os.system("chmod 755 ./"+outputname+";./"+outputname)
