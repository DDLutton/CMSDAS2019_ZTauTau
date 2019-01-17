#ifndef  HISTOGRAMMGR
#define HISTOGRAMMGR

#include <string>
#include <TH1D.h>
#include "TreeReader.h"

std::string XaxisTitle(
    const std::string& title, const std::string& unit );

std::string YaxisTitle(
    const std::string& unit,
    const int nbins, const double xmin, const double xmax  );

class EventHisto {
public:
  EventHisto( const std::string& cutlevel, const bool );
  ~EventHisto();

  // Selected Tau, Muon and Electron index (leave -1 is not of this channel)
  void Fill(
      const int tau,
      const int iLep,
      const double evtweight );

  void Write();

private:
  const std::string cutlevel;
  const bool doMuon ;
  std::map<std::string, TH1D> histmap;

  TH1D& Hist( const std::string&, const bool, const bool );

  void defineHist(
    const std::string& xname,
    const std::string& xtitle,
    const std::string& xunit,
    const unsigned     nbins,
    const double       xmin,
    const double       xmax
  );

  std::string histname(
    const std::string& name,
    const bool tauiso,
    const bool os
    );
};

/** Begin Editing */

EventHisto::EventHisto( const std::string& x, const bool y ):
  cutlevel(x), doMuon(y)
{
  defineHist("visMass", "visible mass", "GeV", 30 , 0 , 300  );
  defineHist("visPt",   "visible p_{T}", "GeV", 30 , 0 , 300  );
  defineHist("visEta",  "visible #eta", "",  24 , -2.4 , 2.4  );

  defineHist("LepPt",    "Lepton p_{T}", "GeV", 20, 0 , 200 );
  defineHist("LepEta",   "Lepton #eta", "", 24, -2.4 , 2.4 );
  defineHist("TauPt",    "#tau p_{T}", "GeV", 20, 0 , 200 );
  defineHist("TauEta",   "#tau p_{T}", "GeV", 20, 0 , 200 );
  defineHist("MET",      "MET", "GeV", 20, 0 , 200 );
  defineHist("LepTMass", "Lepton Transverse Mass", "GeV", 25, 0, 50 );
}

void EventHisto::Fill( const int tau, const int iLep, const double evtweight )
{
  static const double muMass = 0.10565837;
  static const double elMass = 0.000511  ;

  TLorentzVector tauP4, lepP4;
  tauP4.SetPtEtaPhiM( tauPt->at(tau), tauEta->at(tau), tauPhi->at(tau), tauMass->at(tau) );
  if( doMuon )
    lepP4.SetPtEtaPhiM( muPt->at(iLep), muEta->at(iLep), muPhi->at(iLep), muMass );
  else
    lepP4.SetPtEtaPhiM( elePt->at(iLep), eleEta->at(iLep), elePhi->at(iLep), elMass );
  const TLorentzVector zP4 = tauP4 + lepP4;

  const bool tauiso = tauByTightIsolationMVArun2v1DBoldDMwLT ->at(tau) > 0.5 ;
  const bool os =
    doMuon ?  ( muCharge->at(iLep) * tauCharge->at(tau)  < 0 ) :
              ( eleCharge->at(iLep) * tauCharge->at(tau)  < 0 ) ;


  // Begin filling
  Hist("visMass",tauiso,os).Fill( zP4.M(), evtweight );
  Hist("visPt"  ,tauiso,os).Fill( zP4.Pt(), evtweight );
  Hist("visEta" ,tauiso,os).Fill( zP4.Eta(), evtweight );

  // TBC
}

/** Don't Edit After this! */

void EventHisto::defineHist(
    const std::string& xname,
    const std::string& xtitle,
    const std::string& xunit,
    const unsigned     nbins,
    const double       xmin,
    const double       xmax )

{
  for( const auto x : {true,false}){
    for( const auto y : {true,false} ){
      const std::string name = histname(xname,x,y);
      histmap[name] = TH1D( name.c_str(), name.c_str(), nbins, xmin, xmax );
      histmap[name].GetXaxis()->SetTitle( XaxisTitle(xtitle,xunit).c_str() );
      histmap[name].GetYaxis()->SetTitle( YaxisTitle(xunit,nbins,xmin,xmax).c_str() );
    }
  }
}

std::string EventHisto::histname(
  const std::string& x,
  const bool tauiso,
  const bool os
)
{
  const std::string tauisostr = tauiso ? "Iso" : "antiIso" ;
  const std::string osstr     = os ? "OS" : "SS";
  return cutlevel + "_" + x + "_" + tauisostr + "_" + osstr ;
}

TH1D& EventHisto::Hist(
  const std::string& x , const bool tauiso, const bool os )
{
  return histmap.at( histname(x,tauiso,os) );
}


void EventHisto::Write()
{
  for( const auto& hist : histmap ){
    hist.second.Write();
  }
}

EventHisto::~EventHisto() {}


///////////////////////////////////////////////////////////////////

class ObjectHisto{
public:
  ObjectHisto( const std::string& cutlevel, const bool domuon );
  ~ObjectHisto();

  void Fill( const double eventweight, const int tau = -1 , const int lep=-1 );
  void Write();
private:
  const std::string cutlevel;
  const bool doMuon;
  std::map<std::string, TH1D> histmap;

  TH1D& Hist( const std::string& );

  void defineHist(
    const std::string& xname,
    const std::string& xtitle,
    const std::string& xunit,
    const unsigned     nbins,
    const double       xmin,
    const double       xmax
  );

  std::string histname( const std::string& );
};

/*** Begin Histogram definition */

ObjectHisto::ObjectHisto( const std::string& cutLevel, const bool domuon ):
  cutlevel( cutLevel ), doMuon(doMuon)
{
  defineHist("JetPt", "Jet p_{T}",          "GeV", 50,   0, 500 );
  defineHist("Jet1Pt", "Leading Jet p_{T}", "GeV", 60,   0, 600 );
  defineHist("JetEta", "Jet #eta",          "GeV", 48,-2.4, 2.4 );

  // TBC
}

void ObjectHisto::Fill( const double eventweight, const int tau, const int iLep )
{
  // Setting up variables
  static const double muMass = 0.10565837;
  static const double elMass = 0.000511  ;
  TLorentzVector tauP4, lepP4;
  if( tau >= 0 )
    tauP4.SetPtEtaPhiM( tauPt->at(tau), tauEta->at(tau), tauPhi->at(tau), tauMass->at(tau) );
  if( doMuon && iLep >= 0 )
    lepP4.SetPtEtaPhiM( muPt->at(iLep), muEta->at(iLep), muPhi->at(iLep), muMass );
  else if ( iLep >= 0 )
    lepP4.SetPtEtaPhiM( elePt->at(iLep), eleEta->at(iLep), elePhi->at(iLep), elMass );

  // Filling jet histograms
  for( int i = 0 ; i < nJet ; ++i ){
    Hist("JetPt").Fill( jetPt->at(i), eventweight );
    Hist("JetEta").Fill( jetEta->at(i), eventweight );

    if( i == 0 ){
      Hist("Jet1Pt").Fill( jetPt->at(i), eventweight );
    }
  }

  // TBC
}

/** END Histogram definition */
ObjectHisto::~ObjectHisto() {}

void ObjectHisto::Write()
{
  for( const auto& p : histmap ){
    p.second.Write();
  }
}

TH1D& ObjectHisto::Hist( const std::string& x )
{
  return histmap.at(histname(x));
}

std::string ObjectHisto::histname( const std::string& xname )
{
  return cutlevel + "_" + xname;
}

void ObjectHisto::defineHist(
    const std::string& xname,
    const std::string& xtitle,
    const std::string& xunit,
    const unsigned     nbins,
    const double       xmin,
    const double       xmax )

{
  const std::string name = histname(xname);
  histmap[name] = TH1D( name.c_str(), name.c_str(), nbins, xmin, xmax );
  histmap[name].GetXaxis()->SetTitle( XaxisTitle(xtitle,xunit).c_str() );
  histmap[name].GetYaxis()->SetTitle( YaxisTitle(xunit,nbins,xmax,xmin).c_str() );
}

/////////////////////////////////////////////////////////////

std::string XaxisTitle(
    const std::string& title, const std::string& unit )
{
  std::string ans = title ;
  if( unit != "" ){
    ans += "[" + unit + "]";
  }
  return ans;
}

std::string YaxisTitle(
    const std::string& unit,
    const int nbins, const double xmin, const double xmax  )
{
  char ans[1024];
  const double binwidth = (xmax - xmin)/nbins;

  if( binwidth == 1 ){
    if( unit == "" ){
      return "Events";
    } else {
      sprintf(ans, "Events / %s", unit.c_str() );
    }
  } else {
    sprintf( ans, "Events / %2.2lf %s", binwidth, unit.c_str()  );
  }
  return ans;
}

#endif
