import ROOT
import copy, sys
import optparse

import math
from array import array
import os
ROOT.gROOT.SetBatch(True)

#sys.path.insert(0, 'XhhResolved/plotting/')
#from plotTools import read_mu_qcd_file

parser = optparse.OptionParser()

parser.add_option('-f', '--flavor',
                  dest='flavor',
                  default="muon"
                  )
parser.add_option('-q', '--qcdFile',
                  dest="qcdFile",
                  default="QCD.root",
                  )

o, a = parser.parse_args()
# if not os.path.isdir(o.outputDir):
#     os.mkdir(o.outputDir)

files={"data": o.flavor+"/data.root",
       "WJets": o.flavor+"/WJetsToLNu.root",
       "TTJets": o.flavor+"/TTJets.root",
       "ZLL": o.flavor+"/DYJetsToLL.root",
       "DY": o.flavor+"/DYJetsToTauTau.root"}

f_data  = ROOT.TFile(files["data"],  "READ")
f_wjets = ROOT.TFile(files["WJets"], "READ")
f_ttbar = ROOT.TFile(files["TTJets"],"READ")
f_zll   = ROOT.TFile(files["ZLL"],   "READ")
f_ztt   = ROOT.TFile(files["DY"],    "READ")

f_qcd   = ROOT.TFile(o.flavor+"/"+o.qcdFile,"RECREATE")

def subtract():
    # for dName in inFile.GetListOfKeys():
    #     #if "TwoTag" not in dName.GetName(): continue
    #     print dName,dName.GetClassName()
    #     thisDirName = dName.GetName()
    #     dataDir  = inFile.Get(thisDirName)
    #     f_qcd.mkdir(thisDirName)
    #     f_qcd.cd(thisDirName)
    for histKey in f_data.GetListOfKeys():
        histName = histKey.GetName()
        print histName
        h_data   = f_data .Get(histName)
        h_data.SetName("data_"+histName)
        h_wjets  = f_wjets.Get(histName)
        h_wjets.SetName("wjets_"+histName)
        h_ttbar  = f_ttbar.Get(histName)
        h_ttbar.SetName("ttbar_"+histName)
        h_zll  = f_zll.Get(histName)
        h_zll.SetName("zll_"+histName)
        h_ztt  = f_ztt.Get(histName)
        h_ztt.SetName("ztt_"+histName)
        

        h_qcd   = ROOT.TH1D(h_data)
        h_qcd.SetName(histName)
        h_qcd.Add(h_wjets,-1)
        h_qcd.Add(h_ttbar,-1)
        h_qcd.Add(h_zll,-1)
        h_qcd.Add(h_ztt,-1)
        h_qcd.Write()

print "Make QCD hists by subtracting SS MC from SS Data"
print " data:",f_data
print "  qcd:",f_qcd
subtract()
f_qcd.Close()
