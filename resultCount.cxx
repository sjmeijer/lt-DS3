// ??=include <iostream>
// #include "TFile.h"
// #include "TH2F.h"
// #include "TChain.h"
// #include "TImage.h"
// #include "TLegend.h"
// #include "TCanvas.h"
//
// #include <string>
// #include <cerrno>
// #include <sstream>      // std::ostringstream
// #include <vector>
// #include <map>
// #include <set>
// #include <memory>
// #include <functional>
// #include <iomanip>
//

int resultCount(int module)
{

  TFile f("lt_result.root","recreate");

  TChain* ch = new TChain("skimTree","skimTree");
  TChain* cha= new TChain("auxTree","auxTree");

  ch->Add(TString::Format("M%d/*.root",module));
  cha->Add(TString::Format("M%d/*.root",module));

  ch->Show(0);
  cha->Show(0);
  int run = 0;


  Double_t        run_p = 0;
  Double_t        run_pA = 0;     // the version in the aux tree
  Double_t        startTime = 0;
  Double_t        stopTime = 0;
  std::vector<double>* energyCal = 0;



  std::vector<double>*  timestamp   = 0;
  std::vector<int>*     channel     = 0;
  std::vector<double>*  energy      = 0;
  std::vector<double>*  trapECal    = 0;
  std::vector<double>*  trapENFCal  = 0;
  std::vector<double>*  trapENFDBSGCal = 0;
  std::vector<std::string>* detName = 0;
  std::vector<int>*     hiLoHit     = 0;
  std::vector<int>*     hiLoHitD    = 0;
  std::vector<int>*     hitCount    = 0;
  std::vector<int>*     hg          = 0;
  std::vector<int>*     lg          = 0;
  std::vector<double>*  dT          = 0;


  std::vector<double>* timesChan = new std::vector<double>(100);		// total times for each channel

  std::vector<int>* problemPeriods = new std::vector<int>(100);		// how many total times we had problems (could be more than once per run)
  std::vector<int>* zeros = new std::vector<int>(100);			// did I see 0 events in a complete run
  std::vector<int>* gapCount = new std::vector<int>(100);				// gapCount[detName] = number of times in run that gap exceeded threshold
  std::vector<double>* gapCumulative = new std::vector<double>(100);	// gapCumulative[channel] = cumulative gap size of gaps>thresh for run
  int	badStart;	// badStart[channel].size() is the runs with bad starts (long gaps at beginning)
  int	badStop;	// badStop[channel].size() is the runs with bad endings (long gaps at end)

  std::vector<std::string>* pName = new std::vector<std::string>(100);    // position name, mirroring IDs ordering
  std::vector<std::string>* detPName = new std::vector<std::string>(100); // this is the one that gets put into the tree

  std::vector<int>* IDs;
  Double_t runTime;

  ch->SetBranchAddress("run",&run_p);
  ch->SetBranchAddress("timestamp",&timestamp);
  ch->SetBranchAddress("channel",&channel);
  ch->SetBranchAddress("energy", &energy);
  ch->SetBranchAddress("trapECal",&trapECal);
  ch->SetBranchAddress("trapENFCal",&trapENFCal);
  ch->SetBranchAddress("trapENFDBSGCal",&trapENFDBSGCal);
  ch->SetBranchAddress("detName",&detName);
  // skimTree->SetBranchAddress("detPName",&detPName);

  // calculated vars
  // skimTree->Branch("runtime",&runTime);
  ch->SetBranchAddress("hiLoHit",&hiLoHit);
  ch->SetBranchAddress("hiLoHitD",&hiLoHitD);
  ch->SetBranchAddress("hitCount",&hitCount);
  ch->SetBranchAddress("hg",&hg);
  ch->SetBranchAddress("lg",&lg);
  ch->SetBranchAddress("dT",&dT);

  // aux tree vars
  cha->SetBranchAddress("run",&run_pA);
  cha->SetBranchAddress("problemPeriods",&problemPeriods);
  cha->SetBranchAddress("zeros",&zeros);
  cha->SetBranchAddress("gapCount",&gapCount);
  cha->SetBranchAddress("gapCumulative",&gapCumulative);
  cha->SetBranchAddress("badStart",&badStart);
  cha->SetBranchAddress("badStop",&badStop);
  cha->SetBranchAddress("IDs",&IDs);
  cha->SetBranchAddress("runTime",&runTime);
  cha->SetBranchAddress("startTime",&startTime);
  cha->SetBranchAddress("stopTime",&stopTime);


  int nEvents = ch->GetEntries();
  int nRuns = cha->GetEntries();

  std::cout << "skimTree has " << nEvents << " entries" << std::endl;
  std::cout << "auxTree has " << nRuns << " entries" << std::endl;

  ch->GetEntry(0);
  cha->GetEntry(0);

  std::cout << "This run has " << IDs->size() << " detectors in it, and I'm looking at 58 of them, hardcoded" << std::endl;
  std::vector<double>* cumulativeTotal = new std::vector<double>(58);
  double runTotal;


	TH2F *hGapRuns[58];

  int startRun = 16797;
  int endRun = 17493;
  if(module == 2)  {
    startRun = 60000802;
    endRun = 60001506;
  }


  std::cout << "Putting together the arrays of histograms..." << std::endl;
	for(int j=0;j<IDs->size();j++)
	{
    // std::cout << IDs->at(j) << std::endl;
		// hTDiffs[j] = 	new TH2F(TString::Format("hTDiffs%d",chans[j]),"Hit #Delta t",1e4,0,1000,1e3,0,1e4);
		// hGap[j] = 		new TH1F(TString::Format("hGap%3d",chans[j]), "Gap Time",36e3,0,3600);
		hGapRuns[j] = 	new TH2F(TString::Format("hGap%03d-Runs",IDs->at(j)), TString::Format("Gap Time, Ch%d",IDs->at(j)),endRun-startRun,startRun,endRun,36000,0,3600);

		// hTDiffsBin[j] = new TH2F(n2,"Hit #Delta t",1e3,-0.1,10,1e3,0,1e4);
	}

  // TH2F* hGapRuns = new TH2F("hGapRuns","Gap by run",)

  for(int iEntry=0; iEntry<nRuns; iEntry++ )
  {
    cha->GetEntry(iEntry);
    int nDetectors = IDs->size();
    for(int iDetector = 0; iDetector<nDetectors; iDetector++)
    {
      cumulativeTotal->at(iDetector) += gapCumulative->at(iDetector);
      runTotal += runTime;

      hGapRuns[iDetector]->Fill( run,gapCumulative->at(iDetector) );
    }
  }


  for(int iDetector = 0; iDetector<IDs->size(); iDetector++)
  {
    hGapRuns[iDetector]->Write();

    std::cout << IDs->at(iDetector) << ": " << cumulativeTotal->at(iDetector) << ", " << cumulativeTotal->at(iDetector)/runTotal << ", " << 1-cumulativeTotal->at(iDetector)/runTotal<< std::endl;
  }

  // return 1;
}

// int main(int argc, char *argv[])
// {
//   int mod = atoi(argv[1]);
//
// 	resultCount(mod);
//
// 	return 0;
// }
