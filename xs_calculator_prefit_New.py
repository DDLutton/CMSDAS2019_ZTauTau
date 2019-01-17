# Code written by:  zaixing.mao@cern.ch && edward.laird@cern.ch from Brown U.
#!/usr/bin/env python
import ROOT as r
import numpy
from sys import argv, exit, stdout, stderr
import math

if len(argv) < 3:
   print 'Usage:python xs_calculator_prefit.py InputDirectory DYCrossSection[optional]'

dirName = argv[1]

if len(argv)>2:
   FoundXS= numpy.array([argv[2]],dtype=float)
else:
   FoundXS=1.00

r.gStyle.SetOptStat(0)
r.gROOT.SetBatch(True)  # to suppress canvas pop-outs

x_min = 50
x_max = 110

f = r.TFile("prefit.root","recreate")
################################################
# Sevreal Histograms are initiated/produced here
defaultOrder = [
 		('QCD', r.TColor.GetColor(250,202,255)),
                ('WJets',  r.TColor.GetColor(100,182,232)),
                ('TTJets', r.TColor.GetColor(155,152,204)),
		("ZLL", r.TColor.GetColor(222, 90,106)),
                ('DY', r.TColor.GetColor(248,206,104))]

def buildRatios(ratioHist, denom):
    for i in range(1, ratioHist.GetNbinsX()+1):
        de = denom.GetBinContent(i)
        nu = ratioHist.GetBinContent(i)
        nu_err = ratioHist.GetBinError(i)
        if de != 0:
            ratioHist.SetBinContent(i, nu/de)
            ratioHist.SetBinError(i, nu_err/de)
        else:
            ratioHist.SetBinContent(i, 1000)
            ratioHist.SetBinError(i, 0)

def buildHistDict(nbins, binsSetting):
    #binsSetting = [30, 0, 150]   

    histDict = {}
    for iSample, iColor in defaultOrder:
        histDict[iSample+'_OST'] = r.TH1F(iSample+'_OST', '', binsSetting[0], binsSetting[1], binsSetting[2])
        histDict[iSample+'_OST'].SetFillColor(iColor)
        histDict[iSample+'_OST'].SetMarkerColor(iColor)
        histDict[iSample+'_OST'].SetMarkerStyle(21)
        histDict[iSample+'_OST'].SetLineColor(r.kBlack)
    print 'Hello', iSample+'_OST', histDict[iSample+'_OST'].Integral()  
    histDict['bkg_OST'] = r.TH1F('bkg_OST', '', binsSetting[0], binsSetting[1], binsSetting[2])
    histDict['bkg_OST'].Sumw2()
    histDict['bkg_OST'].SetFillColor(r.kGray+2)
    histDict['bkg_OST'].SetLineColor(r.kGray+2)
    histDict['bkg_OST'].SetFillStyle(3344)
    histDict['data_OST'] = r.TH1F('data_OST', '', binsSetting[0], binsSetting[1], binsSetting[2])
    histDict['data_OST'].Sumw2()
    histDict['data_OST'].SetMarkerStyle(8)
    histDict['data_OST'].SetMarkerSize(0.9)
    histDict['data_OST'].SetMarkerColor(r.kBlack)


    histDict['DY_SST'] = r.TH1F('DY_SST', '', binsSetting[0], binsSetting[1], binsSetting[2])
    return histDict
################################################

def setMyLegend(lPosition, lHistList):
    l = r.TLegend(lPosition[0], lPosition[1], lPosition[2], lPosition[3])
    l.SetFillStyle(0)
    l.SetBorderSize(0)
    for i in range(len(lHistList)):
        l.AddEntry(lHistList[i][0], lHistList[i][1], lHistList[i][2])
    return l

def getBins(hist, mass_low, mass_high):
    bin_low = -1
    bin_high = -1
    for i in range(hist.GetNbinsX()):
        if hist.GetBinCenter(i+1) >= mass_low and bin_low == -1:
            bin_low = i+1
        if hist.GetBinCenter(i+1) >= mass_high and bin_high == -1:
            bin_high = i
        if bin_low != -1 and bin_high != -1:
            return bin_low, bin_high
    return bin_low, bin_high

def buildStackDict(histDict, xs_T):
    stackDict = {}
    stackDict['OST'] = r.THStack()

    for iSample, iColor in defaultOrder:
        scale = 1.0
        if iSample != 'DY':
            stackDict['OST'].Add(histDict[iSample+'_OST'])
            histDict['bkg_OST'].Add(histDict[iSample+'_OST'])
        else:
            tmpOST = histDict['DY_OST'].Clone()
            tmpOST.Scale(xs_T/5765.4)
            stackDict['OST'].Add(tmpOST)
            histDict['bkg_OST'].Add(tmpOST)
            
    return stackDict

def FillHisto(input, output, weight = 1.0, mass_low = 0, mass_high = 1000):
    for i in range(input.GetNbinsX()):
        currentValue = output.GetBinContent(i+1)
        currentError = output.GetBinError(i+1)
        output.SetBinContent(i+1, currentValue+input.GetBinContent(i+1)*weight)
        output.SetBinError(i+1, math.sqrt((input.GetBinError(i+1))**2 + currentError**2))
	#output.Scale(2)

def buildLegendDict(histDict, position, XS_OST):
    legendDict = {}
    histList = {'T': []}
    histList['T'].append((histDict['data_OST'], 'Observed', 'lep'))
    for iSample, iColor in reversed(defaultOrder):
        if iSample == 'DY':        
            histList['T'].append((histDict[iSample+'_OST'], "%s (xs = %.1f pb)" %(iSample, 5765.4), 'f'))
        else:
            histList['T'].append((histDict[iSample+'_OST'], iSample, 'f'))

    legendDict['T'] = setMyLegend(position, histList['T'])
    return legendDict


def xs_calculator(fileList = [], mass_low = 25, mass_high = 125, nbins = 50, variableName = "visibleMass", binsSetting = [30, 0, 150]):

    #print 'Estimating Z->ll xs in visible mass region (%.1f, %.1f)' %(mass_low, mass_high)
    IsoOrAnti = ["Iso_","antiIso_"]
    ZTT_OST_IA = [0.0,0.0] #data - all other bkg in opposite sign tight tau isolation region
    QCD_SST_IA = [0.0,0.0] #data - all other bkg in same sign tight tau isolation region
    DY_OST = 0.0
    DY_SST = 0.0


    QCD_SS_to_OS_SF = 1.0

    histDict = buildHistDict(nbins, binsSetting)
    #loop over all the samples
    for iFileName, iFileLocation in fileList:
        isData = False
        if iFileName == 'data':
            isData = True
        isDY = False
        if iFileName == 'DY':
            isDY = True

        print iFileName

        ifile = r.TFile(iFileLocation)

        weight = -1.0
        tauWeight = 0.9
        if isData:
                weight = 1.0
                tauWeight = 1.0
	for i,iA in enumerate(IsoOrAnti):
        	osName = "%sOS" %(variableName+iA)
       		ssName = "%sSS" %(variableName+iA)
		#print(osName)
		#print(ssName)
        	lowBin, highBin = getBins(ifile.Get(osName), mass_low, mass_high)
		print(mass_low)
		print(mass_high)
		print(lowBin)
		print(highBin)
		if (i == 0):
			FillHisto(ifile.Get(osName), histDict[iFileName+'_OST'], tauWeight, mass_low, mass_high)
			if not isDY:
				FillHisto(ifile.Get(ssName), histDict['QCD_OST'], weight*tauWeight, mass_low, mass_high)
        		else:
				FillHisto(ifile.Get(ssName), histDict['DY_SST'], tauWeight, mass_low, mass_high)
                        	DY_OST += ifile.Get(osName).Integral(lowBin, highBin)
            	else:
			ZTT_OST_IA[i] += weight*ifile.Get(osName).Integral(lowBin, highBin)
            		QCD_SST_IA[i] += weight*ifile.Get(ssName).Integral(lowBin, highBin)
			
    QCD_SS_to_OS_SF = ZTT_OST_IA[1] / QCD_SST_IA[1]
    print("Ratio: "+str(QCD_SS_to_OS_SF))
    lowBin, highBin = getBins(histDict['DY_SST'], mass_low, mass_high)
    XS_OST = FoundXS*5765.4
    histDict['QCD_OST'].Add(histDict['DY_SST'], -1.0)
    #histDict['QCD_OST'].Add(histDict['DY_SST'], -1.0*XS_OST/6025.2)
    histDict['QCD_OST'].Scale(QCD_SS_to_OS_SF)
    stackDict = buildStackDict(histDict, XS_OST)
    legendDict = buildLegendDict(histDict, (0.6, 0.8 - 0.06*4, 0.85, 0.8), XS_OST)
    #histDict['ZLL_OST'].Scale(2)
    #print 'Hello 2222', histDict['ZLL_OST'].Integral()
    #plot
    pdf = dirName+'/xs.pdf'
    c = r.TCanvas("c","Test", 800, 800)
    p_coords = [0., 1, 1., 0.3]
    p_r_coords = [0.,0.3,1.,0.06]
    p = r.TPad("stack", "", p_coords[0], p_coords[1], p_coords[2], p_coords[3])
    p_r = r.TPad("ratio", "", p_r_coords[0], p_r_coords[1], p_r_coords[2], p_r_coords[3])
    p.SetMargin(1, 1, 0, 0.1)
    p_r.SetMargin(1, 1, 0.2, 0)

    p.Draw()
    p_r.Draw()
    p.cd()
    r.gPad.SetTicky()
    r.gPad.SetTickx()

    max_t = 1.2*max(stackDict['OST'].GetMaximum(), histDict['data_OST'].GetMaximum())
    stackDict['OST'].SetMinimum(0.001)
    stackDict['OST'].Draw('hist H')
    stackDict['OST'].SetTitle('OS Tight Tau Iso; visibleMass; events')
    stackDict['OST'].SetMaximum(max_t)
    stackDict['OST'].GetYaxis().SetTitleOffset(1.2)
    histDict['data_OST'].Draw('same PE')
    histDict['bkg_OST'].Draw('E2 same')

    print 'Observation: %0.2f' %(histDict['data_OST'].Integral(lowBin,highBin))
    print 'ZTT (unscaled) Expected: %0.2f' %(histDict['DY_OST'].Integral(lowBin,highBin))
    print 'TT Expected: %0.2f' %(histDict['TTJets_OST'].Integral(lowBin,highBin))
    print 'W Expected: %0.2f' %(histDict['WJets_OST'].Integral(lowBin,highBin))
    print 'QCD Expected: %0.2f' %(histDict['QCD_OST'].Integral(lowBin,highBin))
    legendDict['T'].Draw('same')

    c.Update()
    p_r.cd()
    r.gPad.SetTicky()
    r.gPad.SetTickx()
    ratio = histDict['data_OST'].Clone()
    ratio_unc = histDict['bkg_OST'].Clone()
    buildRatios(ratio, histDict['bkg_OST'])
    buildRatios(ratio_unc, histDict['bkg_OST'])
    ratio.GetYaxis().SetNdivisions(5,5,0)
    ratio.GetXaxis().SetLabelSize(0.1)
    ratio.GetXaxis().SetTitleSize(0.1)
    ratio.GetYaxis().SetLabelSize(0.1)
    ratio.GetYaxis().SetNdivisions(5,5,0)
    ratio.GetYaxis().SetTitleSize(0.1)
    ratio.GetYaxis().SetTitleOffset(0.43)
    ratio.GetYaxis().CenterTitle()
    ratio.SetMaximum(2.0)
    ratio.SetMinimum(0.0)
    ratio.SetTitle("; %s; ratio" %variableName)

    ratio.Draw("PE")
    ratio_unc.Draw('E2 same')
    p_r.SetGridy(1)
    r.gPad.Update()
    r.gPad.RedrawAxis()
    c.Update()

    c.SaveAs('%s' %pdf)
    f.Write()
    f.Close()


    print 'DY->ll xs used: %.1f pb' %XS_OST

fileList = [('DY', '%s/DYJetsToTauTau.root' %dirName),
            ('ZLL', '%s/DYJetsToLL.root ' %dirName),
            ('TTJets', '%s/TTJets.root' %dirName),
            ('WJets', '%s/WJetsToLNu.root' %dirName),
           # ('ZLL', '%s/DYJetsToLL.root ' %dirName),
#            ('Diboson', '%s/WZ.root' %dirName),
#            ('Diboson', '%s/ZZ.root' %dirName),
            ('data', '%s/data.root' %dirName),
            ]

variableName = "BasicSelection_visMass_"
bining = []
xs_calculator(fileList, 40, 120, 30, variableName, [30, 0, 300])
