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

  // this is the list of ALL channels that were ever available in M1/P3JDY?.
  // float IDs[100] = {	576, 577,
  //   584, 585,
  //   592, 593,
  //   594, 595,
  //   598, 599,
  //   600, 601,
  //   608, 609,
  //   610, 611,
  //   614, 615,
  //   616, 617,
  //   624, 625,
  //   626, 627,
  //   628, 629,
  //   630, 631,
  //   632, 633,
  //   640, 641,
  //   642, 643,
  //   644, 645,
  //   646, 647,
  //   656, 657,
  //   662, 663,
  //   664, 665,
  //   674, 675,
  //   676, 677,
  //   680, 681,
  //   688, 689,
  //   690, 691,
  //   692, 693,
  //   696, 697
  // };

  uint32_t numChannels = IDs.size(); //58; // 51;//enabledIDs.size();
  // std::vector<double> lastTimestamp;  // lastTimestamp[IDs[n]]
  std::map<int,double> lastTimestamp;   // lastTimestamp[anID]

  for(int iCh=0; iCh<numChannels; iCh++)
  {
    int ch = IDs[iCh];
    lastTimestamp[ch] = 0;
  }

  std::cout << "Predefined the ID list to include all possible channels for P3JDY" << std::endl;



  std::cout << "Setting GAT branch addresses..." << std::endl;
  // variables I'll pull from the GAT-ified data
  gChain->SetBranchAddress("run", &run_p);
  gChain->SetBranchAddress("timestamp", &timestamp);
  gChain->SetBranchAddress("channel", &channel);
  gChain->SetBranchAddress("energy", &energy);
  gChain->SetBranchAddress("trapECal", &trapECal);
  gChain->SetBranchAddress("trapENFCal", &trapENFCal);
  gChain->SetBranchAddress("trapENFDBSGCal",&trapENFDBSGCal);
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

    std::map<int,int> eventChans; // eventChans[hiChan] gives the number of events in hi/lo for detector
    std::map<int,int> eventCount;
    std::map<int,int> eventHi;  // eventHi[hiChan]
    std::map<int,int> eventLo;  // eventLo[hiChan]
    // std::vector<int> presentKeys; // list channels present (only HG ch shown, regardless of which/both is actually present)

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
      if(chanKey == channelNum) // if even number, it is HG
      {
        hg->at(ich) = 1;
        lg->at(ich) = 0;
      }
      else                    // if odd number, it is LG
      {
        hg->at(ich) = 0;
        lg->at(ich) = 1;
      }
      eventCount[chanKey] += 1;
      eventChans[chanKey]+=(hg->at(ich)+(2*lg->at(ich)) - 1); // 0:HG-only, 1:LG-only, 2: both
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
      int chanKey = channelNum - channelNum%2;
      // hiLoHit->at(ich) = (eventHi[chanKey] + eventLo[chanKey]);
      // hiLoHit->at(ich) = (2*eventHi[chanKey] + eventLo[chanKey] - 1);

      hiLoHit->at(ich) = (eventChans[chanKey]);
      hitCount->at(ich) = eventCount[chanKey];
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
