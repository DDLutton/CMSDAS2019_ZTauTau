////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Compiling the code:   ./Make.sh ZTT_XSection.cc
//   Running the code:     ./ZTT_XSection.exe OutPut.root   Input.root
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "TreeReader.h"
#include "WeightCalculator.h"
#include <string> //remove space for successful compilation
#include <ostream> //remove space for successful compilation
#include <ctime>

void registerBranches(TTree *tree);
void initBranch(TTree *tree, std::string name, void *add){
  const char *bname = name.c_str();
  tree->SetBranchStatus(bname, 1);
  tree->SetBranchAddress(bname, add);
}

int  GetPriMuon();
bool GetSecMuon(const int primu);
int  GetTauID();
bool GetBJets();

TLorentzVector LeptonP4;
TLorentzVector TauP4;

bool debug = false;

int main(int argc, char** argv) {
  using namespace std;

  std::string out = *(argv + 1);
    
  cout << "OUTPUT: " << out << endl;     //PRINTING THE OUTPUT FILE NAME
  TFile *fout = TFile::Open(out.c_str(), "RECREATE");
    
  std::string input = *(argv + 2);
  cout << " INPUT: " << input << endl;     //PRINTING THE INPUT FILE NAME
  TFile * myFile = TFile::Open(input.c_str());
  TH1F * HistoTot = (TH1F*) myFile->Get("hcount");

  cout<<"nargs: "<<argc<<endl;
  bool splitTauTau = false;
  bool selTauTau = false;
  if(argc>3){
    std::string splitTauTauString = *(argv + 3);
    std::string selTauTauString = *(argv + 4);
    splitTauTau = splitTauTauString == "1";
    selTauTau   = selTauTauString   == "1";
  }

  if(argc>5){
    std::string debugString = *(argv + 5);
    debug = debugString == "1";
  }

  std::cout<<"splitTauTau "<<splitTauTau<<std::endl;
  std::cout<<"selTauTau "<<selTauTau<<std::endl;
  std::cout<<"debug "<<debug<<std::endl;


  //add the histrograms of lepton and tau visible mass (both for opposite sign and same sign pair )
  TH1F *    visibleMassOS = new TH1F ("visibleMassOS","visibleMassOS", 30, 0, 300);
  TH1F *    visibleMassSS = new TH1F ("visibleMassSS","visibleMassSS", 30, 0, 300);
  visibleMassOS->SetDefaultSumw2();
  visibleMassSS->SetDefaultSumw2();
    
    
  TTree *Run_Tree = (TTree*) myFile->Get("EventTree");
  cout.setf(ios::fixed, ios::floatfield);

  registerBranches(Run_Tree);

  float MuMass= 0.10565837;
  float eleMass= 0.000511;
  
  //Luminosity
  float LumiWeight = 1;
  if (HistoTot) LumiWeight = weightCalc(HistoTot, input);
  cout << "LumiWeight is " << LumiWeight << "\n";

  //Pileup Reweighting
  TFile * PUData= new TFile("MyDataPileupHistogram2016.root");
  TH1F * HistoPUData= (TH1F *) PUData->Get("pileup");
  HistoPUData->Scale(1.0/HistoPUData->Integral());
  TFile * PUMC= new TFile("mcMoriondPU.root");
  TH1F * HistoPUMC= (TH1F *) PUMC->Get("pileup");
  HistoPUMC->Scale(1.0/HistoPUMC->Integral());

  //cutflow counts
  int nPassTrigger = 0;
  int nPassMuon    = 0;
  int nPassDYVeto  = 0;
  int nPassTau     = 0;
  int nPassMuMET   = 0;
  int nPassBVeto   = 0;

  //keep track of processing time
  std::clock_t start = std::clock();
  double duration;

  // Loop over all events in the TTree.
  auto nentries_wtn = Run_Tree->GetEntries();
  for(auto ievt = 0; ievt < nentries_wtn; ievt++){
    //Print Status
    if((ievt+1)%1000 == 0) {
      duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
      fprintf(stdout, "\r  Processed events: %8d of %8d (%6f events/s)", ievt+1, nentries_wtn, (ievt+1)/duration);
    }
    fflush(stdout);

    Run_Tree->GetEntry(ievt);  // read this event

    //Check if we want to only look at truth tau tau events
    if(splitTauTau){
      int numTau=0;
      for(int igen=0;igen < nMC; igen++){
	if(fabs(mcPID->at(igen)) == 15 && mcMomPID->at(igen)==23) numTau++;
      }
      if( selTauTau && (numTau < 1)) continue; // uncomment this line to get Ztautau contribution of DY
      if(!selTauTau && (numTau > 1)) continue; // uncomment this line to get Zll contribution of DY 
    }

    //
    //Trigger
    //
    bool PassTrigger = (HLTEleMuX >> 19 & 1) == 1; //       else if (name.find("HLT_IsoMu24_v") != string::npos) bitEleMuX = 19;
    if (!PassTrigger) continue;
    nPassTrigger += 1;

    //Compute Pileup Weight
    float PUWeight = 1;
    if (!isData){
      if(debug) std::cout<<"get PUWeight"<<std::endl;
      int puNUmmc=int(puTrue->at(0)*10);
      int puNUmdata=int(puTrue->at(0)*10);
      float PUMC_=HistoPUMC->GetBinContent(puNUmmc+1);
      float PUData_=HistoPUData->GetBinContent(puNUmdata+1);
      PUWeight= PUData_/PUMC_;
    }

    //
    //Loop over muons to find number passing quality cuts. 
    //
    const int imu = GetPriMuon();
    if( imu <  0 ) { continue; }
    nPassMuon += 1;
    LeptonP4.SetPtEtaPhiM(muPt->at(imu),muEta->at(imu),muPhi->at(imu),MuMass);

      
    //
    // Loose Muon selection for Z->mu mu veto
    //
    if( !GetSecMuon(imu) ) { continue; } 
    nPassDYVeto += 1;

    //
    //Now process Tau's
    //
    const int itau = GetTauID();
    if( itau <  0 ) { continue; }
    nPassTau += 1;
    TauP4.SetPtEtaPhiM(muPt->at(imu),muEta->at(imu),muPhi->at(imu),MuMass);

    //Reject W+Jets
    float LepMetTranverseMass = TMass_F(muPt->at(imu), muPt->at(imu)*cos(muPhi->at(imu)),muPt->at(imu)*sin(muPhi->at(imu)) ,  pfMET, pfMETPhi);      
    if(LepMetTranverseMass > 40) { continue; }
    nPassMuMET += 1;

    //ttbar veto with bjet veto
    if(GetBJets()) { continue; }
    nPassBVeto += 1;

    // Check charge of the lepton and Taus
    bool OS = muCharge->at(imu) * tauCharge->at(itau) < 0;
    bool SS = muCharge->at(imu) * tauCharge->at(itau) > 0;

    // Construct the visible mu+tau system
    TLorentzVector LepTauP4 = LeptonP4 + TauP4;
    float eventWeight = LumiWeight*PUWeight;

    //Check if there is an OS  muTau pair with dR > 0.5 and TMass(mu.MET) < 40 and then fill the weighted histogram as below:
    if(OS)
      visibleMassOS->Fill(LepTauP4.M(), eventWeight);

    //Check if there is a SS  muTau pair with dR > 0.5 and TMass(mu.MET) < 40 and then fill the weighted histogram as below:
    if(SS)
      visibleMassSS->Fill(LepTauP4.M(), eventWeight);

  } //End Event Loop

  //end of analysis code, close and write histograms/file
  fout->cd();
  visibleMassOS->Write();
  visibleMassSS->Write();
  //visibleMassOSRelaxedTauIso->Write();
  //visibleMassSSRelaxedTauIso->Write();
  fout->Close();

  std::cout << "Done" << std::endl;
  std::cout << "nPassTrigger: " << nPassTrigger << std::endl;
  std::cout << "nPassMuon: " << nPassMuon << std::endl;
  std::cout << "nPassDYVeto: " << nPassDYVeto << std::endl;
  std::cout << "nPassTau: " << nPassTau << std::endl;
  std::cout << "nPassMuMET: " << nPassMuMET << std::endl;
  std::cout << "nPassBVeto: " << nPassBVeto << std::endl;

}



void registerBranches(TTree *tree) {
  tree->SetBranchStatus("*", 0);


  //########################################   General Info
  initBranch(tree, "isData", &isData);
  initBranch(tree,"run", &run);
  initBranch(tree,"lumis", &lumis);
  initBranch(tree,"event", &event);
  initBranch(tree,"genWeight",&genWeight);
  initBranch(tree,"HLTEleMuX", &HLTEleMuX);
  initBranch(tree,"puTrue", &puTrue);
  initBranch(tree,"nVtx",&nVtx);
        
  //########################################   MC Info
  initBranch(tree,"nMC", &nMC);
  initBranch(tree,"mcPID", &mcPID);
  initBranch(tree,"mcStatus", &mcStatus);
  initBranch(tree,"mcPt", &mcPt );
  initBranch(tree,"mcEta", &mcEta );
  initBranch(tree,"mcPhi", &mcPhi );
  initBranch(tree,"mcE", &mcE );
  initBranch(tree,"mcMass", &mcMass );
  initBranch(tree,"mcMomPID", &mcMomPID );
  initBranch(tree,"mcGMomPID", &mcGMomPID );
        
  //########################################   Tau Info
  initBranch(tree,"nTau", &nTau);
  initBranch(tree,"tauPt"  ,&tauPt);
  initBranch(tree,"tauEta"  ,&tauEta);
  initBranch(tree,"tauPhi"  ,&tauPhi);
  initBranch(tree,"tauMass"  ,&tauMass);
  initBranch(tree,"tauCharge"  ,&tauCharge);

  initBranch(tree,"taupfTausDiscriminationByDecayModeFinding", &taupfTausDiscriminationByDecayModeFinding);

  initBranch(tree,"tauByTightMuonRejection3", &tauByTightMuonRejection3);
  initBranch(tree,"tauByLooseMuonRejection3", &tauByLooseMuonRejection3);

  initBranch(tree,"tauByMVA6TightElectronRejection"  ,&tauByMVA6TightElectronRejection);
  initBranch(tree,"tauByMVA6MediumElectronRejection"  ,&tauByMVA6MediumElectronRejection);
  initBranch(tree,"tauByMVA6LooseElectronRejection", &tauByMVA6LooseElectronRejection);

  initBranch(tree,"tauDxy",&tauDxy);
  initBranch(tree,"tauDecayMode",&tauDecayMode);

  initBranch(tree,"tauByLooseIsolationMVArun2v1DBoldDMwLT",&tauByLooseIsolationMVArun2v1DBoldDMwLT);
  initBranch(tree,"tauByVLooseIsolationMVArun2v1DBoldDMwLT",&tauByVLooseIsolationMVArun2v1DBoldDMwLT);
  initBranch(tree,"tauByTightIsolationMVArun2v1DBoldDMwLT",&tauByTightIsolationMVArun2v1DBoldDMwLT);
        
  //########################################   Mu Info
  initBranch(tree,"nMu", &nMu);
  initBranch(tree,"muPt"  ,&muPt);
  initBranch(tree,"muEta"  ,&muEta);
  initBranch(tree,"muPhi"  ,&muPhi);
  initBranch(tree,"muIsoTrk", &muIsoTrk);
  initBranch(tree,"muCharge",&muCharge);
  initBranch(tree,"muIDbit",&muIDbit);//NEW
  initBranch(tree,"muPFChIso", &muPFChIso);
  initBranch(tree,"muPFPhoIso", &muPFPhoIso);
  initBranch(tree,"muPFNeuIso", &muPFNeuIso);
  initBranch(tree,"muPFPUIso", &muPFPUIso);
  initBranch(tree,"muD0",&muD0);
  initBranch(tree,"muDz",&muDz);
        
  //########################################   Ele Info
  initBranch(tree,"nEle", &nEle);
  initBranch(tree,"elePt"  ,&elePt);
  initBranch(tree,"eleEta"  ,&eleEta);
  initBranch(tree,"elePhi"  ,&elePhi);
  initBranch(tree,"elePFChIso", &elePFChIso);
  initBranch(tree,"eleIDMVA", &eleIDMVA);//NEW
  initBranch(tree,"eleCharge",&eleCharge);
  initBranch(tree,"eleSCEta",&eleSCEta);
  initBranch(tree,"elePFChIso", &elePFChIso);
  initBranch(tree,"elePFPhoIso", &elePFPhoIso);
  initBranch(tree,"elePFNeuIso", &elePFNeuIso);
  initBranch(tree,"elePFPUIso", &elePFPUIso);
  initBranch(tree,"eleD0",&eleD0);
  initBranch(tree,"eleDz",&eleDz);
  initBranch(tree,"eleMissHits", &eleMissHits);
  initBranch(tree,"eleConvVeto", &eleConvVeto);
  initBranch(tree,"eleSCEta", &eleSCEta );
        
  //########################################   Jet Info
  initBranch(tree,"nJet",&nJet);
  initBranch(tree,"jetPt",&jetPt);
  initBranch(tree,"jetEta",&jetEta);
  initBranch(tree,"jetPhi",&jetPhi);
  initBranch(tree,"jetCSV2BJetTags",&jetCSV2BJetTags);
  initBranch(tree,"jetPFLooseId",&jetPFLooseId);
  initBranch(tree,"jetPUID",&jetPUID);
  initBranch(tree,"jetRawPt",&jetRawPt);
  initBranch(tree,"jetJECUnc",&jetJECUnc);
  initBranch(tree,"jetRawEn",&jetRawEn);
  initBranch(tree,"jetHadFlvr",&jetHadFlvr);
        
  //########################################   MET Info
  initBranch(tree,"pfMET",&pfMET);
  initBranch(tree,"pfMETPhi",&pfMETPhi);
  initBranch(tree,"metFilters",&metFilters);
  initBranch(tree,"genHT",&genHT);

}

int GetTauID()
{
  int nSelTaus = 0;
  int tau;
  for  (int itau=0 ; itau < nTau; itau++){
    //check pt and eta
    if(tauPt->at(itau) < 30 || fabs(tauEta->at(itau)) > 2.3) continue;

    //Tau ID
    if(taupfTausDiscriminationByDecayModeFinding->at(itau) < 0.5) continue;
    if(tauByTightMuonRejection3                 ->at(itau) < 0.5) continue;
    if(tauByMVA6LooseElectronRejection          ->at(itau) < 0.5) continue;
    if(tauByTightIsolationMVArun2v1DBoldDMwLT   ->at(itau) < 0.5) continue;
    TauP4.SetPtEtaPhiM(tauPt->at(itau),tauEta->at(itau),tauPhi->at(itau),tauMass->at(itau));
    if(TauP4.DeltaR(LeptonP4) < 0.5) continue;
    nSelTaus += 1;
    tau = itau;
  } // End of tau loop

  // Only consider events with 1 tau candidate for now
  if(nSelTaus != 1) return -1;
  return tau;

}

int GetPriMuon()
{
  if(debug) std::cout<<"muon loop 1"<<std::endl;
  for  (int imu=0 ; imu < nMu; imu++){
    //check pt and eta
    if(muPt->at(imu) < 30) continue;
    if(fabs(muEta->at(imu)) > 2.1) continue;

    //compute isolation
    float IsoMu=muPFChIso->at(imu)/muPt->at(imu);
    if ( (muPFNeuIso->at(imu) + muPFPhoIso->at(imu) - 0.5* muPFPUIso->at(imu) )  > 0.0)
      IsoMu= ( muPFChIso->at(imu)/muPt->at(imu) + muPFNeuIso->at(imu) + muPFPhoIso->at(imu) - 0.5* muPFPUIso->at(imu))/muPt->at(imu);
    //check isolation
    if(IsoMu > 0.15) continue;

    //Check Muon ID
    if( !(muIDbit->at(imu)>>1 & 1)) continue; // 2 is tight, 1 is medium
    return imu; 
  }
  return -1;
}

bool GetSecMuon( const int primu )
{
  bool passDYVeto = true;
  if(debug) std::cout<<"muon loop 2"<<std::endl;
  for(int imu=0 ; imu < nMu; imu++){
    if(imu == primu) continue; //skip the primary muon
    if(muPt->at(imu) < 15) continue;
    if(fabs(muEta->at(imu)) > 2.4) continue;//this one doesnt need to satisfy the muon trigger, so have extended eta range and lower pt threshold

    //compute isolation
    float IsoMu2=muPFChIso->at(imu)/muPt->at(imu);
    if ( (muPFNeuIso->at(imu) + muPFPhoIso->at(imu) - 0.5* muPFPUIso->at(imu) )  > 0.0)
      IsoMu2= ( muPFChIso->at(imu)/muPt->at(imu) + muPFNeuIso->at(imu) + muPFPhoIso->at(imu) - 0.5* muPFPUIso->at(imu))/muPt->at(imu);
    bool MuIdIso2=((muIDbit->at(imu) >> 0 & 1)  && IsoMu2 < 0.30 && fabs(muD0->at(imu)) < 0.045 && fabs(muDz->at(imu)) < 0.2);
    if(!MuIdIso2) continue;

    //Only worry about the second muon if it is opposite sign of the main muon
    bool OS = muCharge->at(imu) * muCharge->at(primu) < 0;
    if(OS) return false;
  }
  return true;
}

//veto events with bjets passing our quality/overlap cuts
bool GetBJets()
{
  TLorentzVector Jet4Momentum;
  for (int ijet= 0 ; ijet < nJet ; ijet++){
    Jet4Momentum.SetPtEtaPhiE(jetPt->at(ijet),jetEta->at(ijet),jetPhi->at(ijet),jetRawEn->at(ijet));
    if(     jetPt ->at(ijet)  < 20) continue;//jet Pt
    if(fabs(jetEta->at(ijet)) > 2.5) continue;//jet Eta
    if(Jet4Momentum.DeltaR(TauP4)    < 0.5) continue;//Tau Overlap
    if(Jet4Momentum.DeltaR(LeptonP4) < 0.5) continue;//Lepton Overlap
    if(jetCSV2BJetTags->at(ijet) < 0.8484) continue;//btag
    return true;
  }
  return false;
}