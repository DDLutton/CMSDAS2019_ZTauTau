#add path to PlotTools if your plot script is not in PlotTools/python/
import sys
sys.path.insert(0, '../PlotTools/python/')

#import plotting library
import PlotTools

#Use ordered dictionaries (collections library) because they allow you to specify the order in which to draw hists naturally
import collections

import optparse
parser = optparse.OptionParser()

parser.add_option('-f', '--flavor',
                  dest='flavor',
                  default="muon"
                  )

o, a = parser.parse_args()

#First define the list of samples. Is a dictionary where the key is the file path/name and the 
#value is a dictionary where the keys are TObjects in th file and the values are dictionaris with parameters for the corresponding TObject
flavor=o.flavor
files={"data": flavor+"/data.root",
       "QCD": flavor+"/QCD.root",
       "WJets": flavor+"/WJetsToLNu.root",
       "TTJets": flavor+"/TTJets.root",
       "ZLL": flavor+"/DYJetsToLL.root",
       "DY": flavor+"/DYJetsToTauTau.root"}

colors={'QCD': "ROOT.TColor.GetColor(250,202,255)",
        'WJets':  "ROOT.TColor.GetColor(100,182,232)",
        'TTJets': "ROOT.TColor.GetColor(155,152,204)",
	"ZLL": "ROOT.TColor.GetColor(222, 90,106)",
        'DY': "ROOT.TColor.GetColor(248,206,104)"}

varLabels = {"LepEta": flavor+" #eta",
             "LepPt": flavor+" p_{T} [GeV]",
             "LepTMass": flavor+" Transverse Mass [GeV]",
             "MET": "MET [GeV]",
             "TauEta": "#tau_{h} #eta",
             "TauPt": "#tau_{h} p_{T} [GeV]",
             "visEta": "visEta",
             "visMass": "Visible Mass [GeV]",
             "visPt": "Visible p_{T} [GeV]",
             }

QCD_SS_to_OS_SF = 1.0

for sel in ["BasicSelection"]:
    for iso in ["Iso","antiIso"]:
        for var, label in varLabels.items():
            samples=collections.OrderedDict()
            samples[files["data"]] = collections.OrderedDict()
            samples[files["data"]][sel+"_"+var+"_"+iso+"_OS"] = {"label"    : "Data",
                                                                 "ratio"    : "numer A",
                                                                 "isData"   : True,
                                                                 "color"    : "ROOT.kBlack"}

            samples[files["DY"]] = collections.OrderedDict()
            samples[files["DY"]][sel+"_"+var+"_"+iso+"_OS"] = {"label"    : "Z/#gamma^{*} #rightarrow #tau^{+}#tau^{-}",
                                                               "ratio"    : "denom A",
                                                               "stack"    : 5,
                                                               "color"    : colors["DY"]}
            samples[files["ZLL"]] = collections.OrderedDict()
            samples[files["ZLL"]][sel+"_"+var+"_"+iso+"_OS"] = {"label"    : "Z/#gamma^{*} #rightarrow l^{+}l^{-}",
                                                                "ratio"    : "denom A",
                                                                "stack"    : 4,
                                                                "color"    : colors["ZLL"]}
            samples[files["TTJets"]] = collections.OrderedDict()
            samples[files["TTJets"]][sel+"_"+var+"_"+iso+"_OS"] = {"label"    : "W+Jets",
                                                                   "ratio"    : "denom A",
                                                                   "stack"    : 3,
                                                                   "color"    : colors["TTJets"]}
            samples[files["WJets"]] = collections.OrderedDict()
            samples[files["WJets"]][sel+"_"+var+"_"+iso+"_OS"] = {"label"    : "W+Jets",
                                                                  "ratio"    : "denom A",
                                                                  "stack"    : 2,
                                                                  "color"    : colors["WJets"]}
            samples[files["QCD"]] = collections.OrderedDict()
            samples[files["QCD"]][sel+"_"+var+"_"+iso+"_SS"] = {"label"    : "Multijet",
                                                                "ratio"    : "denom A",
                                                                "stack"    : 1,
                                                                "weight"   :QCD_SS_to_OS_SF,
                                                                "color"    : colors["QCD"]}

            #define global plot parameters which are not specific to a given input histogram to the final pdf
            parameters = {"ratio"     : True,
                          "rTitle"    : "Data / Model",
                          "xTitle"    : label,
                          "yTitle"    : "Events / Bin",
                          "outputDir" : flavor+"/",
                          "outputName": sel+"_"+var+"_"+iso,
                          }

            PlotTools.plot(samples, parameters)

