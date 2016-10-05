#include "TFile.h"
#include "TTree.h"
#include "TList.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TChain.h"
#include "TCut.h"
// #include "TTreeReader.h"
// #include "TTreeReaderValue.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "TEntryList.h"
#include "TROOT.h"
#include "TFile.h"
#include "TStopwatch.h"

// #include "GATDataSet.hh"
// #include "MJTChannelMap.hh"
// #include "TClonesArray.h"
// #include "MGTEvent.hh"
// #include "GATUtils.hh"


int livetime_checks(int runNumber)
{
  std::cout << "Beginning 'livetime_checks.cxx' analysis" << std::endl;
  TStopwatch timer;
	timer.Start();

  // gROOT->ProcessLine(".x $MGDODIR/Majorana/LoadMGDOMJClasses.C");
  // gROOT->ProcessLine(".x $GATDIR/LoadGATClasses.C");

  // load run
  std::cout << "Loading GATDataSet for run " << runNumber << std::endl;
  GATDataSet ds(runNumber);
	TChain* chain = ds.GetChains();
	TChain* gChain = ds.GetGatifiedChain();	// gat chain
	double runTime = ds.GetRunTime()/CLHEP::second;

  std::cout << "Defining TTrees..." << std::endl;
  TChain* ch = new TChain("mjdTree","mjdTree");
  TTree* skimTree = new TTree("skimTree","skimTree"); // my own "skim tree"

  std::cout << "Touching status file..." << std::endl;
  char *cmd = new char[100];
  sprintf(cmd,"touch /global/u2/s/sjmeijer/status-hiLo/%d",runNumber);
  gSystem->Exec(cmd);

  std::cout << "Creating output file..." << std::endl;
  char *outFileTitle = new char[30];
	sprintf(outFileTitle,"lt_skim_%d.root",runNumber);
	TFile f(outFileTitle,"recreate");

  std::cout << "Defining branch variables..." << std::endl;
  Double_t        run_p = 0;
  vector<double>* timestamp = 0;
  vector<int>*    channel = 0;
  vector<double>* energy = 0;
  vector<double>* trapECal = 0;
  vector<double>* trapENFCal = 0;
  vector<double>* trapENFDBSGCal = 0;
  vector<string>* detName = 0;
  Double_t        startTime = 0;
  Double_t        stopTime = 0;
  vector<double>* energyCal = 0;

  // additionally, I'll use some of the same variables on the output, as well
  // as calculating some of my own
  vector<int>* hiLoHit = 0;
  vector<int>* hitCount = 0;
  vector<int>* hg = 0;
  vector<int>* lg = 0;
  vector<double>* dT = 0;


  MJTChannelMap *chmap = ds.GetChannelMap();
  vector<float> IDs;
  int c = 1; // for Module 1
  for(int p = 1; p <= 7; p++)
  {
    for(int d = 1; d <= 5; d++)
    {
      if(chmap->GetDetectorName(c,p,d)!="") // avoid nonexistent detectors
      {
        IDs.push_back(chmap->GetInt(chmap->GetDetectorName(c,p,d),"kIDHi")); // returns HG channel #
        IDs.push_back(chmap->GetInt(chmap->GetDetectorName(c,p,d),"kIDLo")); // returns LG channel #
      }
    }
  }

  uint32_t numChannels = IDs.size(); //58; // 51;//enabledIDs.size();
  std::map<int,double> lastTimestamp;   // lastTimestamp[anID]

  for(int iCh=0; iCh<numChannels; iCh++)
  {
    int ch = IDs[iCh];
    lastTimestamp[ch] = 0;
  }

  std::cout << "Automatically filled the ID list to include all enabled channels for dataset" << std::endl;



  std::cout << "Setting GAT branch addresses..." << std::endl;
  // variables I'll pull from the GAT-ified data
  gChain->SetBranchAddress("run", &run_p);
  gChain->SetBranchAddress("timestamp", &timestamp);
  gChain->SetBranchAddress("channel", &channel);
  gChain->SetBranchAddress("energy", &energy);
  gChain->SetBranchAddress("trapECal", &trapECal);
  gChain->SetBranchAddress("trapENFCal", &trapENFCal);
  gChain->SetBranchAddress("trapENFDBSGCal",&trapENFDBSGCal);
  gChain->SetBranchAddress("detName",&detName);
  gChain->SetBranchAddress("startTime", &startTime);
  gChain->SetBranchAddress("stopTime", &stopTime);

  // variables I'll want to put back into my skim files
  std::cout << "Branching the output skim tree..." << std::endl;


  skimTree->Branch("run",&run_p);
  skimTree->Branch("timestamp",&timestamp);
  skimTree->Branch("channel",&channel);
  skimTree->Branch("energy", &energy);
  skimTree->Branch("trapECal",&trapECal);
  skimTree->Branch("trapENFCal",&trapENFCal);
  skimTree->Branch("trapENFDBSGCal",&trapENFDBSGCal);
  skimTree->Branch("detName",&detName);
  skimTree->Branch("startTime", &startTime);
  skimTree->Branch("stopTime", &stopTime);
  // calculated vars
  skimTree->Branch("runtime",&runTime);
  skimTree->Branch("hiLoHit",&hiLoHit);
  skimTree->Branch("hitCount",&hitCount);
  skimTree->Branch("hg",&hg);
  skimTree->Branch("lg",&lg);
  skimTree->Branch("dT",&dT);

  std::cout << "Branch addresses defined" << std::endl;

  // loop over each event in the run
  int nEvents = gChain->GetEntries();
  std::cout << "total number of entries = " << nEvents << std::endl;

  for(Long64_t iEvent=0; iEvent < nEvents; iEvent++)
  {

    if(iEvent%10000==0)
      {std::cout << "  iEvent: " << iEvent << std::endl;}

    gChain->GetEntry(iEvent);

    if(iEvent%10000==0)
      {std::cout << "  got event: " << iEvent << std::endl;}

    std::map<std::string,int> eventChans; // eventChans[hiChan] gives the number of events in hi/lo for detector
    std::map<std::string,int> eventCount;
    std::map<std::string,int> eventHi;  // eventHi[hiChan]
    std::map<std::string,int> eventLo;  // eventLo[hiChan]

    int numHits = channel->size();
    if(iEvent%10000==0)
      {std::cout << "  looping over " << numHits << " hits in event: " << iEvent << std::endl;}

    hg->resize(numHits);
    lg->resize(numHits);
    hiLoHit->resize(numHits);
    hitCount->resize(numHits);
    dT->resize(numHits);
    for(int ich = 0; ich<numHits; ich++)
    {
      int channelNum = channel->at(ich);
      int chanKey = channelNum - channelNum%2;  // this is the HG for the detector
      std::string name = detName->at(ich);

      hg->at(ich) = (channelNum%2)?0:1; // hg is 1 if channelNum is even, else 0
      lg->at(ich) = (channelNum%2)?1:0; // lg is 1 if channelNum is odd, else 0

      eventCount[name] += 1;
      eventChans[name] += 2*lg->at(ich) + hg->at(ich); // 0:HG-only, 1:LG-only, 2: both
      // eventHi[chanKey] += hg->at(ich);
      // eventLo[chanKey] += lg->at(ich);

      if(iEvent%10000==0)
        {std::cout << "    found channel " << channelNum << std::endl;}

      // calculate the time difference between the last event
      dT->at(ich) = timestamp->at(ich)*1e-8 - lastTimestamp[channelNum];
      lastTimestamp[channelNum] = timestamp->at(ich)*1e-8;
    }

    if(iEvent%10000==0)
      {std::cout << "  writing hiLoHits values for event " << iEvent << std::endl;}

    for(int ich = 0; ich<numHits; ich++)
    {
      int channelNum = channel->at(ich);
      std::string name = detName->at(ich);

      hiLoHit->at(ich) = eventChans[name] - 1;  // must subtract the 1 only once (at end)
      hitCount->at(ich) = eventCount[name];
      // subtracting 1 from this makes is 0:HG-only, 1:LG-only, 2: both
    }

    skimTree->Fill();
    hiLoHit->clear();
    hitCount->clear();
    dT->clear();
    hg->clear();
    lg->clear();

    eventChans.clear();
    eventHi.clear();
    eventLo.clear();
  }
  skimTree->Write();

  sprintf(cmd,"rm /global/u2/s/sjmeijer/status-hiLo/%d",runNumber);
  gSystem->Exec(cmd);

  timer.Stop();
	std::cout << "Executing this script took: " << std::endl;
	timer.Print();

	return (int)timer.RealTime();
}
