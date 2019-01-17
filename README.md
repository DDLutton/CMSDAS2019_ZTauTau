# CMSDAS2019_ZTauTau

#compile

>./Make.sh ZTT_XSection.cc

#run

Usage: 
>./ZTT_XSection.exe <outFile> <inputFile> <electron/muon, default: muon> <split by truth Z decay 0/1, default: 0> <if spitting by truth Z decay, 1 for tau tau, 0 for ell ell, default: 0> <debug 0/1, default: 0>

To run electron analysis:

>mkdir electron

>./ZTT_XSection.exe electron/TTJets.root     root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/TTbar.root electron

>./ZTT_XSection.exe electron/WJetsToLNu.root root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/WJetsToLNu_Inc.root electron

>./ZTT_XSection.exe electron/data.root   root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/SingleElectron.root electron

>./ZTT_XSection.exe electron/DYJetsToTauTau.root root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/DYJetsToLL_M-50_Inc.root electron 1 1

>./ZTT_XSection.exe electron/DYJetsToLL.root root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/DYJetsToLL_M-50_Inc.root electron 1 0

Now make the QCD histograms

>python makeQCD.py -f electron

And make a lot of plots (requires git clone git@github.com:patrickbryant/PlotTools.git in directory above this one)

>python plotDump.py -f muon


To run muon analysis:

>mkdir muon

>./ZTT_XSection.exe muon/TTJets.root     root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/TTbar.root muon

>./ZTT_XSection.exe muon/WJetsToLNu.root root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/WJetsToLNu_Inc.root muon

>./ZTT_XSection.exe muon/data.root   root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/SingleMuon.root muon

>./ZTT_XSection.exe muon/DYJetsToTauTau.root root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/DYJetsToLL_M-50_Inc.root muon 1 1

>./ZTT_XSection.exe muon/DYJetsToLL.root root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/DYJetsToLL_M-50_Inc.root muon 1 0

Now make the QCD histograms

>python makeQCD.py -f muon

And make a lot of plots (requires git clone git@github.com:patrickbryant/PlotTools.git in directory above this one)

>python plotDump.py -f muon





# Plot visible mass spectrum

Usage: >python xs_calculator_prefit.py InputDirectory DYCrossSection[optional]

> python xs_calculator_prefit_New.py electron

> python xs_calculator_prefit_New.py muon


# Input Files

root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/DYJetsToLL_M-50_Inc.root

root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/SingleElectron.root

root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/SingleMuon.root

root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/TTbar.root

root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/WJetsToLNu_Inc.root

root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/WW.root

root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/WZ.root

root://131.225.204.161:1094//store/user/cmsdas/2019/long_exercises/ZTauTau/ZZ.root