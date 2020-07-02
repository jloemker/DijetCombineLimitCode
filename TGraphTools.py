from ROOT import *
import ROOT as rt
#def getIntersection(TGraphErrors)
def getIntersections(a,b,logY=True):
    intersections=[]
    a.Print()
    b.Print()
    for i in range(a.GetN()-1):
        for j in range(b.GetN()-1):
            ax1=rt.Double(0)
            ay1=rt.Double(0)
            ax2=rt.Double(0)
            ay2=rt.Double(0)
            bx1=rt.Double(0)
            by1=rt.Double(0)
            bx2=rt.Double(0)
            by2=rt.Double(0)

            a.GetPoint(i,ax1,ay1)
            a.GetPoint(i+1,ax2,ay2)
            b.GetPoint(j,bx1,by1)
            b.GetPoint(j+1,bx2,by2)          

            if(logY):
                ay1=TMath.Log10(ay1)
                ay2=TMath.Log10(ay2)
                by1=TMath.Log10(by1)
                by2=TMath.Log10(by2)
            am=(ay2-ay1)/(ax2-ax1)
            ab=(ay1*ax2-ay2*ax1)/(ax2-ax1)
            bm=(by2-by1)/(bx2-bx1)
            bb=(by1*bx2-by2*bx1)/(bx2-bx1)

            xden=am-bm
            if(xden==0):
                continue
            x=(bb-ab)/xden
            y=a.Eval(x)

            xrange_min = max(min(ax1, ax2), min(bx1, bx2))
            xrange_max = min(max(ax1, ax2), max(bx1, bx2))

            if ((ax1 == bx1 and ay1 == by1) or (ax2 == bx2 and ay2 == by2)):
                if( ax1==bx1 and ay1==by1):
                    intersections.append((ax1,ay1))
                else:
                    intersections.append((ax1,ay1))
            elif(x > xrange_min and x < xrange_max):
                intersections.append((x,y))
    return intersections
    

if(__name__=='__main__'):
    canv=TCanvas('plot','plot',600,600)
    canv.DrawFrame(-10,-10,10,10)
    N_Points=100
    a=TGraph(N_Points)
    a.SetLineColor(1)
    b=TGraph(N_Points)
    b.SetLineColor(2)
    for i in range(0,N_Points):
        xcurrent=-N_Points/(N_Points/10)+i
        a.SetPoint(i,xcurrent,xcurrent**2-3)
        b.SetPoint(i,xcurrent,-xcurrent**2+3)

    a.Draw("LP")
    b.Draw("LPSAME")
    a.GetYaxis().SetRangeUser(-10,10)
    a.GetXaxis().SetRangeUser(-10,10)
    a.GetYaxis().SetTitle('y')
    a.GetXaxis().SetTitle('x')
    # a.Draw()
    # inter=TGraph()
    inter=getIntersections(a,b)
    print(inter)
    # inter.Print()
    # inter=TGraph()
    # inter.SetPoint(0,5,5)
    # inter.SetMarkerColor(4)
    # inter.SetMarkerStyle(21)
    # inter.Draw('P')
    canv.Update()

    del inter

    hold=raw_input("press enter to quit...")
