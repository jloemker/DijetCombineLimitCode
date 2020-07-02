#!/usr/local/bin/python2.7
import ROOT as rt
import time
from ROOT import *
import os
import glob
import math
import array
import sys
# sys.path.append('/nfs/dust/cms/user/zoiirene/CombineTutorial/CMSSW_8_1_0/src/DijetCombineLimitCode/Limits')
# import CMS_lumi
                                                                                        
sys.path.append(os.path.join(os.environ['CMSSW_BASE'],'src/DijetCombineLimitCode'))
import PointName as PN
import TGraphTools as TGT
import csv
import time
import random
import numpy as np

from optparse import OptionParser

# CMS_lumi.lumi_13TeV = "35.9 fb^{-1}"
# CMS_lumi.writeExtraText = 1
# CMS_lumi.extraText = ""
# CMS_lumi.lumi_sqrtS = "13 TeV" # used with iPeriod = 0, e.g. for simulation-only plots (default is an empty string)
# CMS_lumi.cmsText=""
# iPos = 11
# if( iPos==0 ): CMS_lumi.relPosX = 0.12
# iPeriod=4  


def extractLimit(grmean,gr1up,gr1down,gtheory,label,obs=False):
    #####Expected Limits:
    inter_mean=TGT.getIntersections(grmean,gtheory)
    inter_1up=TGT.getIntersections(gr1up,gtheory)
    inter_1down=TGT.getIntersections(gr1down,gtheory)


    if(len(inter_mean)<1):
        inter_mean.append((0,0))
    if(len(inter_mean)<2):
        inter_mean.append((0,0))

    if(len(inter_1up)<1):
        inter_1up.append((0,0))
    if(len(inter_1up)<2):
        inter_1up.append((0,0))
        
    if(len(inter_1down)<1):
        inter_1down.append((0,0))
    if(len(inter_1down)<2):
        inter_1down.append((0,0))

    
    #save to csv:
    # csvfile=open('Limits/%s_%s_limits.csv'%(label.split('_')[0],label.split('_')[1]),'wt')
    if(obs):
        expObs='observed'
    else:
        expObs='expected'
        
    csvfile=open('Limits/%s_%s_limits.csv'%(label,expObs),'wt')
    csvwriter=csv.DictWriter(csvfile,fieldnames=['parameter','lmean','l1down','l1up','umean','u1down','u1up'])
    csvwriter.writeheader()
    csvwriter.writerow({'parameter':label.split('_')[1],
                        'lmean':inter_mean[0][0],
                        'l1down':inter_1down[0][0],
                        'l1up':inter_1up[0][0],
                        # 'l1down':inter_1down[0][0]-inter_mean[0][0],
                        # 'l1up':inter_1up[0][0]-inter_mean[0][0],
                        'umean':inter_mean[1][0],
                        'u1down':inter_1down[1][0],
                        'u1up':inter_1up[1][0]
                        # 'u1down':inter_1down[1][0]-inter_mean[1][0],
                        # 'u1up':inter_1up[1][0]-inter_mean[1][0]
                        })
    csvfile.close()
    print 'mean:',inter_mean
    print '1up:',inter_1up
    print '1down:',inter_1down

    return [inter_mean,inter_1up,inter_1down]


def Plot(files, label, obs,CompareLimits=False,plotExpLimitRatio=""):
    radmasses = []
    for f in files:
      if not postfix:
        radmasses.append(float(f.replace("CMS_jj_","").split("_")[0])/1000.)
      else:
        stmp = f.replace("CMS_jj_","").split("_")[3].replace("p",".")
        stmp = stmp.replace("m","-")
        # print 'stmp', stmp
        radmasses.append(float(stmp)) 
    #print radmasses

    efficiencies={}
    for mass in radmasses:
        efficiencies[mass]=1.0 # assume 1/pb signal cross section
        
    rad = []
    for onefile in files:
        print "using file " + onefile
        file = rt.TFile(onefile)
        tree = file.Get("limit")
        print tree
        limits = []
        for quantile in tree:
            limits.append(tree.limit)
            print ">>>   %.2f" % limits[-1]

        rad.append(limits[:6])
        file.Close()
    print 'rad:',rad
    print 'limits:',limits
    mg = rt.TMultiGraph()
    mg.SetTitle("")
    x = []
    yobs = []
    y2up = []
    y1up = []
    y1down = []
    y2down = []
    ymean = []

    for i in range(0,len(efficiencies)):
        print "rad i 2 " + str(rad[i][2])        
        print "radmasses i " + str(radmasses[i])        
        print "efficiency  " + str(efficiencies[radmasses[i]])        

        y2up.append(rad[i][0]*efficiencies[radmasses[i]])
        y1up.append(rad[i][1]*efficiencies[radmasses[i]])
        ymean.append(rad[i][2]*efficiencies[radmasses[i]])
        y1down.append(rad[i][3]*efficiencies[radmasses[i]])
        y2down.append(rad[i][4]*efficiencies[radmasses[i]])
        yobs.append(rad[i][5]*efficiencies[radmasses[i]])
     
    grobs = rt.TGraphErrors(1)
    grobs.SetMarkerStyle(8)
    grobs.SetMarkerSize(0.8)
    grobs.SetLineColor(rt.kBlack)
    grobs.SetLineWidth(2)
    gr2up = rt.TGraphErrors(1)
    gr2up.SetMarkerColor(0)
    gr1up = rt.TGraphErrors(1)
    gr1up.SetMarkerColor(0)
    grmean = rt.TGraphErrors(1)
    grmean.SetLineColor(1)
    grmean.SetLineWidth(4)
    grmean.SetLineStyle(2) #irene
    grmean.SetMarkerStyle(20) #irene
    gr1down = rt.TGraphErrors(1)
    gr1down.SetMarkerColor(0)
    gr2down = rt.TGraphErrors(1)
    gr2down.SetMarkerColor(0)
    
    for j in range(0,len(radmasses)):
        grobs.SetPoint(j, radmasses[j], yobs[j])
        gr2up.SetPoint(j, radmasses[j], y2up[j])
        gr1up.SetPoint(j, radmasses[j], y1up[j])
        grmean.SetPoint(j, radmasses[j], ymean[j])
        gr1down.SetPoint(j, radmasses[j], y1down[j])    
        gr2down.SetPoint(j, radmasses[j], y2down[j])
       
    
    mg.Add(gr2up)#.Draw("same")
    mg.Add(gr1up)#.Draw("same")
    mg.Add(grmean,"L")#.Draw("same,AC*")
    mg.Add(gr1down)#.Draw("same,AC*")
    mg.Add(gr2down)#.Draw("same,AC*")
    if obs: mg.Add(grobs,"L")#.Draw("AC*")
    
    
    H_ref = 600; 
    W_ref = 800; 
    W = W_ref
    H  = H_ref
  
    T = 0.08*H_ref
    B = 0.12*H_ref 
    L = 0.12*W_ref
    R = 0.04*W_ref

    c1 = rt.TCanvas("c1","c1",50,50,W,H)
    c1.SetFillColor(0)
    c1.SetBorderMode(0)
    c1.SetFrameFillStyle(0)
    c1.SetFrameBorderMode(0)
    c1.SetLeftMargin( L/W )
    c1.SetRightMargin( R/W )
    c1.SetTopMargin( T/H )
    c1.SetBottomMargin( B/H )
    c1.SetTickx(0)
    c1.SetTicky(0)
    c1.GetWindowHeight()
    c1.GetWindowWidth()
    c1.SetLogy()
    #c1.SetGrid()
    c1.SetLogy()
    c1.cd()
    
    frame = c1.DrawFrame(1.1,0.001, 4.2, 10)
    if "qZ" in label.split("_")[0] or label.find("qW")!=-1: frame = c1.DrawFrame(1.1,0.001, 6.2, 800.)
    #frame.GetYaxis().CenterTitle()
    frame.GetYaxis().SetTitleSize(0.05)
    frame.GetXaxis().SetTitleSize(0.05)
    frame.GetXaxis().SetLabelSize(0.04)
    frame.GetYaxis().SetLabelSize(0.04)
    frame.GetYaxis().SetTitleOffset(1.15)
    frame.GetXaxis().SetTitleOffset(1.05)
    #frame.GetXaxis().CenterTitle()
    frame.SetMinimum(0.01)
    frame.SetMaximum(100)
    frame.GetXaxis().SetNdivisions(508)
    #frame.GetYaxis().CenterTitle(True)
    
    
    frame.GetXaxis().SetTitle("F_{%s}/#Lambda^{-4} (TeV^{-4})"%label.split('_')[1])
    frame.GetYaxis().SetTitle("Signal strength")

    

    mg.GetXaxis().SetLimits(radmasses[0],radmasses[-1])
    # mg.GetXaxis().SetLimits(radmasses[(len(radmasses)/2)-5],radmasses[(len(radmasses)/2)+5])
        

    # histo to shade
    n=len(radmasses)

    grgreen = rt.TGraph(2*n)
    for i in range(0,n):
        grgreen.SetPoint(i,radmasses[i],y2up[i])
        grgreen.SetPoint(n+i,radmasses[n-i-1],y2down[n-i-1])

    grgreen.SetFillColor(rt.kOrange)
    grgreen.SetLineColor(rt.kOrange)
    grgreen.SetFillStyle(1001)
    grgreen.Draw("F") 


    gryellow = rt.TGraph(2*n)
    for i in range(0,n):
        gryellow.SetPoint(i,radmasses[i],y1up[i])
        gryellow.SetPoint(n+i,radmasses[n-i-1],y1down[n-i-1])

    gryellow.SetFillColor(rt.kGreen+1)
    gryellow.SetLineColor(rt.kGreen+1)
    gryellow.SetFillStyle(1001)
    gryellow.Draw("Fsame") 

    grmean.Draw("LP")
    if obs: grobs.Draw("LPsame")
    

    gtheory = rt.TGraphErrors(1)
    gtheory.SetLineColor(rt.kRed)
    gtheory.SetLineWidth(3)
    gtheoryUP = rt.TGraphErrors(1)
    gtheoryUP.SetLineColor(rt.kRed-2)
    gtheoryUP.SetLineWidth(3)
    gtheoryDOWN = rt.TGraphErrors(1)
    gtheoryDOWN.SetLineColor(rt.kRed-2)
    gtheoryDOWN.SetLineWidth(3)
    gtheorySHADE = rt.TGraphErrors(1)
    gtheorySHADE.SetLineColor(rt.kRed-2)
    gtheorySHADE.SetLineWidth(3)
    
    n=len(radmasses)

    gtheory = rt.TGraph(n)
    for i in range(0,n):
        gtheory.SetPoint(i,radmasses[i],1)
    
    ###Steffen: calculate intersection of theory with mean & +- 1 sigma
    
    expectedLimits=extractLimit(grmean,gr1up,gr1down,gtheory,label)    
    inter_mean=expectedLimits[0]
    inter_1up=expectedLimits[1]
    inter_1down=expectedLimits[2] 
    if(obs):
        observedLimits=extractLimit(grobs,gr1up,gr1down,gtheory,label,True)         
        inter_mean_obs=observedLimits[0]
    
    # frame.GetXaxis().SetRangeUser()
    frame.GetXaxis().SetRangeUser(inter_mean[0][0]*2.,inter_mean[1][0]*2.)

    
    #lower limit:
    l1down=TGraph()
    l1down.SetPoint(0,inter_1down[0][0],0)
    l1down.SetPoint(1,inter_1down[0][0],1)
    l1down.SetLineStyle(2)
    l1down.SetLineColor(1)
    l1down.SetMarkerStyle(26)
    l1down.SetMarkerSize(1.5)
    l1down.Draw("LPSAME")

    l1up=TGraph()
    l1up.SetPoint(0,inter_1up[0][0],0)
    l1up.SetPoint(1,inter_1up[0][0],1)
    l1up.SetLineStyle(2)
    l1up.SetLineColor(1)
    l1up.SetMarkerStyle(26)
    l1up.SetMarkerSize(1.5)
    l1up.Draw("LPSAME")

    lmean=TGraph()
    lmean.SetPoint(0,inter_mean[0][0],0)
    lmean.SetPoint(1,inter_mean[0][0],1)
    lmean.SetLineStyle(1)
    lmean.SetLineColor(1)
    lmean.SetMarkerStyle(22)
    lmean.SetMarkerColor(1)
    lmean.SetMarkerSize(1.5)
    lmean.Draw("LPSAME")

    if(obs):
        lmean_obs=TGraph()
        lmean_obs.SetPoint(0,inter_mean_obs[0][0],0)
        lmean_obs.SetPoint(1,inter_mean_obs[0][0],1)
        lmean_obs.SetLineStyle(3)
        lmean_obs.SetLineWidth(2)
        lmean_obs.SetLineColor(12)
        lmean_obs.SetMarkerStyle(23)
        lmean_obs.SetMarkerColor(12)
        lmean_obs.SetMarkerSize(1.5)
        lmean_obs.Draw("LPSAME")
    
    #upper limit:
    u1down=TGraph()
    u1down.SetPoint(0,inter_1down[1][0],0)
    u1down.SetPoint(1,inter_1down[1][0],1)
    u1down.SetLineStyle(2)
    u1down.SetLineColor(1)
    u1down.SetMarkerStyle(26)
    u1down.SetMarkerSize(1.5)
    u1down.Draw("LPSAME")

    u1up=TGraph()
    u1up.SetPoint(0,inter_1up[1][0],0)
    u1up.SetPoint(1,inter_1up[1][0],1)
    u1up.SetLineStyle(2)
    u1up.SetLineColor(1)
    u1up.SetMarkerStyle(26)
    u1up.SetMarkerSize(1.5)
    u1up.Draw("LPSAME")

    umean=TGraph()
    umean.SetPoint(0,inter_mean[1][0],0)
    umean.SetPoint(1,inter_mean[1][0],1)
    umean.SetLineStyle(1)
    umean.SetLineColor(1)
    umean.SetMarkerStyle(22)
    umean.SetMarkerColor(1)
    umean.SetMarkerSize(1.5)
    umean.Draw("LPSAME")

    if(obs):
        umean_obs=TGraph()
        umean_obs.SetPoint(0,inter_mean_obs[1][0],0)
        umean_obs.SetPoint(1,inter_mean_obs[1][0],1)
        umean_obs.SetLineStyle(3)
        umean_obs.SetLineWidth(2)
        umean_obs.SetLineColor(12)
        umean_obs.SetMarkerStyle(23)
        umean_obs.SetMarkerColor(12)
        umean_obs.SetMarkerSize(1.5)
        umean_obs.Draw("LPSAME")

   
    print "max cross section (observed limit ) : " +str(round(rt.TMath.MaxElement(n,grobs.GetY()),5))+ " pb" 
    print "min cross section (observed limit ) : " +str(round(rt.TMath.MinElement(n,grobs.GetY()),5))+ " pb"
    
    mg.Add(gtheory,"L")
    gtheory.Draw("L") 
    
    ltheory="Signal strength = 1"

    # leg = rt.TLegend(0.4,0.6002591,0.9446734,0.9011917)
    # leg2 = rt.TLegend(0.4,0.6002591,0.9446734,0.9011917)
    leg = rt.TLegend(0.6,0.7002591,0.9446734,0.9011917)
    leg2 = rt.TLegend(0.6,0.7002591,0.9446734,0.9011917)
    #leg.SetTextFont(42)
    #leg2.SetTextFont(42)
    leg.SetTextSize(0.028)
    leg.SetLineColor(1)
    leg.SetShadowColor(0)
    leg.SetLineStyle(1)
    leg.SetLineWidth(1)
    leg.SetFillColor(kWhite)
    # leg.SetFillStyle(0)
    leg.SetMargin(0.35)
    leg2.SetTextSize(0.028)
    leg2.SetLineColor(1)
    leg2.SetShadowColor(0)
    leg2.SetLineStyle(1)
    leg2.SetLineWidth(1)
    leg2.SetFillColor(0)
    leg2.SetFillStyle(0)
    leg2.SetMargin(0.35)
    leg.SetBorderSize(1)

    if obs: leg.AddEntry(grobs, "Observed", "Lp")
    leg.AddEntry(gryellow, "Expected #pm 1 std. deviation", "f")
    leg.AddEntry(grgreen , "Expected #pm 2 std. deviation", "f")
    leg.AddEntry(gtheory, ltheory, "L")

    if obs: leg2.AddEntry(grobs, " ", "")
    leg2.AddEntry(grmean, " ", "L")
    leg2.AddEntry(grmean, " ", "L")
    leg2.AddEntry(gtheorySHADE, " ", "")

    
    # addInfo = rt.TPaveText(0.548995,0.1830769,0.9346734,0.2897203,"NDC")
    addInfo = rt.TPaveText(0.6946309,0.5437063,0.795302,0.6363636,"NDC")
    addNarrow = rt.TPaveText(0.9,0.02,0.64,0.3,"NDC")
    if label.find("qW")!=-1 or label.find("qZ")!=-1 or label.find("Zprime")!=-1 or label.find("WZ_")!=-1:
        addNarrow = rt.TPaveText(0.15,0.02,0.64,0.3,"NDC")
    if label.find("BulkWW")!=-1:
        addNarrow = rt.TPaveText(0.4,0.02,0.64,0.3,"NDC")
    if (label.find("new")!=-1) and label.find("qW")!=-1 or label.find("qZ")!=-1:addInfo = rt.TPaveText(0.7846309,0.5437063,0.825302,0.6363636,"NDC")

    addInfo.SetFillColor(0)
    addInfo.SetLineColor(0)
    addInfo.SetFillStyle(0)
    addInfo.SetBorderSize(0)
    addInfo.SetTextFont(42)
    addInfo.SetTextSize(0.040)
    addInfo.SetTextAlign(12)
    
    addNarrow.SetFillColor(0)
    addNarrow.SetLineColor(0)
    #addNarrow.SetTextColor(kRed)
    addNarrow.SetFillStyle(0)
    addNarrow.SetBorderSize(0)
    addNarrow.SetTextFont(42)
    addNarrow.SetTextSize(0.035)
    addNarrow.SetTextAlign(12)
    addNarrow.AddText("narrow width approximation")
  
    # addInfo.AddText("Pruned mass sideband")
    if(label.find("HP")!=-1):
      if(label.find("_WW")!=-1):addInfo.AddText("WW enriched")
      elif(label.find("_WZ")!=-1):addInfo.AddText("WZ enriched")
      elif(label.find("_ZZ")!=-1):addInfo.AddText("ZZ enriched")
      elif(label.find("_VV_new")!=-1):addInfo.AddText("WW+WZ+ZZ")
      elif(label.find("_VVHP_new")!=-1):addInfo.AddText("WW+WZ+ZZ")
      elif(label.find("_VV_old")!=-1):addInfo.AddText("VV category")
      elif(label.find("_qW")!=-1):addInfo.AddText("qW enriched")
      elif(label.find("_qZ")!=-1):addInfo.AddText("qZ enriched")
      elif(label.find("_qV")!=-1):addInfo.AddText("qW+qZ")
      addInfo.AddText("High-purity")
    elif(label.find("LP")!=-1):
      if(label.find("_WW")!=-1):addInfo.AddText("WW enriched")
      elif(label.find("_WZ")!=-1):addInfo.AddText("WZ enriched")
      elif(label.find("_ZZ")!=-1):addInfo.AddText("ZZ enriched")
      elif(label.find("_VV_new")!=-1):addInfo.AddText("WW+WZ+ZZ")
      elif(label.find("_VVLP_new")!=-1):addInfo.AddText("WW+WZ+ZZ")
      elif(label.find("_VV_old")!=-1):addInfo.AddText("VV category")
      
      elif(label.find("_qW")!=-1):addInfo.AddText("qW enriched")
      elif(label.find("_qZ")!=-1):addInfo.AddText("qZ enriched")
      elif(label.find("_qV")!=-1):addInfo.AddText("qW+qZ")
      addInfo.AddText("Low-purity")
    else:
      if label.find("old")!=-1:
        addInfo.AddText("VV category")
        addInfo.AddText("HP+LP")
      elif (label.find("new")!=-1) and label.find("qW")!=-1 or label.find("qZ")!=-1:
        addInfo.AddText("qW+qZ")
        addInfo.AddText("HP+LP")
        
      elif label.find("new_combined")!=-1:
        addInfo.AddText("WW+WZ+ZZ")
        addInfo.AddText("HP+LP")
      
    #addInfo.Draw()
    #addNarrow.Draw()
    c1.Update() 
    frame = c1.GetFrame()
    frame.Draw()
    # CMS_lumi.CMS_lumi(c1, iPeriod, iPos)
    c1.cd()
    c1.Update()
    c1.RedrawAxis()
    c1.RedrawAxis("g")
    c1.cd()
    c1.Update()
    c1.cd()
    c1.Update()
    
    leg.Draw()
    leg2.Draw("same")
        
    fname = postfix+"brazilianFlag_%s_13TeV.pdf" %label
    c1.SaveAs(fname)
    c1.SaveAs(fname.replace(".pdf" ,".C"  ))
    
    if plotExpLimitRatio != '':     
        yexp1 = grmean.GetY()
        if obs == True:
           yexp2 = cgraphs[1].GetY()
        else:
           yexp2 = cgraphs[0].GetY()
        
        if obs:
            yexp2 = cgraphs[1].GetY()
        x = grmean.GetX()
        print x
        print yexp1
        print yexp2
        print yexp1[0]
        y = []
        n = grmean.GetN()
        gratio = TGraph()
        for i in range(0,n):
            #y.append((yexp2[i]/yexp1[i]))
            y.append( (1-yexp2[i]/yexp1[i]))
            print y[i]
            gratio.SetPoint(i,x[i],y[i])
        print y
        gratio.SetMaximum(0.1)
        gratio.SetMinimum(-0.1)
        gratio.GetXaxis().SetTitle("m_{jj} (TeV)")
        gratio.GetYaxis().SetTitle("(1- exp. limit alt. func./ exp. limit)")
        gratio.SetMarkerStyle(24)
        gratio.SetMarkerColor(kBlue)
        cratio = TCanvas("cratio","cratio",400,400)
        gratio.Draw("AP")
        addInfo2 = addText(label)
        addInfo2.Draw()
        time.sleep(5)
        cratio.SaveAs("testratio_"+label+".pdf")
        
    #prevents memory leak in Canvas Creation/Deletion
    #see: https://root.cern.ch/root/roottalk/roottalk04/2484.html
    gSystem.ProcessEvents()

    del c1
    # del leg
    # del leg2
  #   del addInfo
  #   del mg
  #   del gtheory
  #   del gtheoryUP
  #   del gtheoryDOWN
    #del gtheorySHADE
    #thFile.Close()

    
    
    time.sleep(5)


    
def addText(label):
    bla = rt.TPaveText(0.9,0.8,0.6,0.7,"NDC")
    bla.SetFillColor(0)
    bla.SetLineColor(0)
    bla.SetFillStyle(0)
    bla.SetBorderSize(0)
    bla.SetTextFont(42)
    bla.SetTextSize(0.040)
    bla.SetTextAlign(12)
    if label.find("BulkWW")!=-1:
        bla.AddText("Bulk Graviton to WW")
    if label.find("BulkZZ")!=-1:
        bla.AddText("Bulk Graviton to ZZ") 
    if label.find("WZ_")!=-1:
        bla.AddText("W' to WZ")
    if label.find("Zprime")!=-1:
        bla.AddText("Z' to WW")
    if label.find("qW")!=-1:
        bla.AddText("q* to qW")
    if label.find("qZ")!=-1:
        bla.AddText("q* to qZ") 
    if label.find("WWLP")!=-1:
        bla.AddText("WW LP")
    if label.find("ZZ LP")!=-1:
        bla.AddText("ZZ LP") 
    if label.find("WZLP")!=-1:
        bla.AddText("WZ LP")
    if label.find("WWHP")!=-1:
        bla.AddText("WW HP")
    if label.find("ZZ HP")!=-1:
        bla.AddText("ZZ HP") 
    if label.find("WZHP")!=-1:
        bla.AddText("WZ HP")
    if label.find("VVnew")!=-1:
        bla.AddText("WW + WZ + ZZ")
        bla.AddText("HP + LP")
    if label.find("qWLP")!=-1:
        bla.AddText("qW LP")
    if label.find("qZ LP")!=-1:
        bla.AddText("qZ LP") 
    if label.find("qWHP")!=-1:
        bla.AddText("qW HP")
    if label.find("qZ HP")!=-1:
        bla.AddText("qZ HP") 
    if label.find("qVnew")!=-1:
        bla.AddText("qW + qZ")
        bla.AddText("HP + LP")
    if label.find("graviton")!=-1:
        bla.AddText("G* to WW")
    if label.find("radion")!=-1:
        bla.AddText("R to WW")
    return bla
    
        
    
def testFileForLimits(filename):
    if(not os.path.isfile(filename)):
        print filename, 'does not exists!'
        return False
    file=TFile(filename)
    tree=file.Get('limit')
    if(not tree):
        print filename,'does not have any trees!'
        return False
    N_limits=tree.GetEntries()
    # file.Close()
    if(not (N_limits==6)):
        print filename, 'is not complete!'
        return False
    else:
        return True

if __name__ == '__main__':
    postfix = "Limits/"
    gROOT.SetBatch(True)

    chan=sys.argv[1]
    variable=sys.argv[2] if len(sys.argv)>2 else ""
    parameter=chan.split('_')[1]
    print('chan:',chan)
    print('praramter:',parameter)
    print('variable:',variable)
    # regions=["_invMass","_invMass_afterVBFsel","_invMass_combined"]
    
    regions=["_invMass_combined"]
    
    CompareLimits = False #True
    plotExpLimitRatio = ""  

    for region in regions:
        couplings=PN.OpList(parameter)
        combinedplots=[]    
        failed=[]
        for coupling in couplings:
            if(testFileForLimits(postfix+"CMS_jj_0_"+chan+"_"+str(coupling)+variable+"_13TeV_"+region+"_asymptoticCLs_new.root")):
               combinedplots+=[postfix+"CMS_jj_0_"+chan+"_"+str(coupling)+variable+"_13TeV_"+region+"_asymptoticCLs_new.root"]
            else:
               print 'skipping', coupling
               failed.append(postfix+"CMS_jj_0_"+chan+"_"+str(coupling)+variable+"_13TeV_"+region+"_asymptoticCLs_new.root")
        print '('+str(len(failed))+'/'+str(len(couplings))+') failed!!'
        if region == "_invMass":
            Plot(combinedplots,chan+"_"+region+"_new_combined", obs=False,CompareLimits=False,plotExpLimitRatio="")  
            gSystem.ProcessEvents()
        else:
            Plot(combinedplots,chan+"_"+region+"_new_combined", obs=True,CompareLimits=False,plotExpLimitRatio="")  
            gSystem.ProcessEvents()
        print 'Failed (N=%i):'%len(failed)
            
