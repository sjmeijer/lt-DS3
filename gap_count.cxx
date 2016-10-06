??=include <iostream>
#include "TFile.h"
#include "TH2F.h"
#include "TChain.h"
#include "TImage.h"
#include "TLegend.h"
#include "TCanvas.h"

#include <string>
#include <cerrno>
#include <sstream>      // std::ostringstream
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <functional>
#include <iomanip>

#pragma link C++ class std::map<int, std::map<int,int> >
#pragma link C++ class std::map<int, std::map<int,double> >

#define getDigitizer(ID) ((ID&0xFF)>>4)
#define getChannel(ID) (ID&0xF)

#define getID(run,chan) ( chan + (16*16*run) ) 	// single int to denote run/channel
#define getRun(ID) (ID>>8)

int freq_out_energy(int inputChan=0)
{
	int energyLimit = 0; // in keV
	double threshold = 8.5;		// seconds we can go between hits that I consider the run to be "bad"


	std::cout << "Gap threshold of " << threshold << " seconds" << std::endl;


	int theRunNumber;
	int theChannel;
	double theRunTime;
	std::vector<int>* theID;
	std::vector<double>* theDeltaTime;
	std::vector<double>* theTimestamp;
	std::vector<double>* theEnergy;

	TTree *resultTree = new TTree("resultsTree","outData");

	resultTree->Branch("runNumber",&theRunNumber);
	resultTree->Branch("channel",&theChannel);
	resultTree->Branch("ID",&theID);
	resultTree->Branch("runTime",&theRunTime);
	resultTree->Branch("timestamp",&theTimestamp);
	resultTree->Branch("deltaTime",&theDeltaTime);
	resultTree->Branch("trapEnergies",&theEnergy);


	// int dig = 4;

	// int chans[] = {582,580,578};		// P1
	// int chans[] = {692,648,640};		// P2
	// int chans[] = {616,610,608,664};	// P3
	// int chans[] = {624,628,694,614};	// P4
	// int chans[] = {678,672};			// P5
	// int chans[] = {632,626,690};		// P6
	// int chans[] = {600,598,594,592};	// P7



	// int chans[] = {692,690,688,640,674,576,610,608,598,600,594,592,664,662,656,696,626,624,646,644,642};	// P3JDY HG Good, from Clara
	// int chans[] = {693,691,689,641,675,577,611,609,599,601,595,593,665,663,657,697,627,625,647,645,643};	// P3JDY LG Good, from Clara

	static const int chansArr[] = {692,693,690,691,688,689,640,641,674,675,576,577,610,611,608,609,598,599,600,601,594,595,
		592,593,664,665,662,663,656,657,696,697,626,627,624,625,646,647,644,645,642,643}; // P3JDY Total Good HG/LG
	std::vector<int> chans (chansArr, chansArr + sizeof(chansArr) / sizeof(chansArr[0]) );
	int cNum = 42; 				// number of channels we're looking at

	std::map<int,int> IDs;

	for(int k=0; k<cNum; k++)
	{
		IDs[chans[k]] = k;
	}

	static const int questionableRunsArr[] = {3557,3559,3561,3563,3565,3567,3569,3571,3573,3575,3579,
		3596,3598,3600,3602,3604,3606,3608,3610,3612,3614,3616,3618,3620,3622,3624,3626,3628,3630,3632,3634,3636,3638,3640,3642,3644,
		4034,4038,4040,4045,4047,4049,
		4045,4047,4049,4051,4053,4055,4057,4059,4061,4063,4065,4067,4069,4071,4073,4075,4077,4079,4081,4083,4085,4087,4089,4091,4093,4095,4097,4099,4101,4103,4105,4107,4109,4111,4113,4115,4117,4119,4121,4123,4125,4127,4129,4131,4133,
		4239,4241,4243,4245,4247,4249,4251,4253,4255,4257,4259,4261,4263,4265,4267,4269,4271,4273,4275,4277,4279,4281,4283,4285,4287,4289,4291,4293,4295,4297,4299,4301,4303,4305,4307,4309,4311,4313,4315,4317,4319,4321,4323,4325,4327,4329,4331,4333,4335,4337,4339,4341,4343,4345,4347,4349,4351,4353,4355,4357,4359,4361,4363,4365,4367,4369,4371,4373,4375,4377,4379,4381,4383,4385,4387,4389,4391,4393,4395,4397,4399,4401,4403,4405,4407,4409,4411,4413,4415,4417,4419,4421,4423,4425,4427,
		4445, 4447, 4449, 4451, 4453, 4455, 4457, 4459, 4461, 4463, 4465, 4467, 4469, 4471, 4473, 4475, 4477, 4479, 4481, 4483,
		4549, 4551, 4553, 4555, 4557, 4559, 4561, 4563, 4565, 4567, 4569, 4571
	};

	std::vector<int> questionableRuns (questionableRunsArr, questionableRunsArr + sizeof(questionableRunsArr) / sizeof(questionableRunsArr[0]) );

	std::vector<unsigned int> eventCount;

	if(inputChan>cNum)
	{
		std::cout << "ERROR, invalid input channel number..." << std::endl;
		return -2;
	}

	TChain* rtTree = new TChain("outTree", "outTree");
	rtTree->Add(TString::Format("runtimes_2335-8183.root"));

	float rNum;		// run number
	float runtime;	// run time

	rtTree->SetBranchAddress("runNumber",&rNum);
	rtTree->SetBranchAddress("runtime",&runtime);

	std::map<int,double> times;
	std::map<float,double> timesChan;		// total times for each channel

	std::vector<double> timesUsed;			// the times of each of the runs we looked at
	std::vector<int> runsUsed;					// the runs we looked at

	int nRuns = rtTree->GetEntries();
	for(int i=0;i<nRuns;i++)
	{
		rtTree->GetEntry(i);
		times[rNum] = runtime;
	}


	TH2F *hTDiffs[42];
	TH2F *hGapRuns[42];
	TH1F *hGap[42];

	TH2F* hGaps = new TH2F("hGaps","Gaps",6963-2335+2,2335,6963,696-576+2,576,696);
	TH2F* hZeros = new TH2F("hZeros","Zeros",6963-2335+2,2335,6963,696-576+2,576,696);

	TCanvas *c1 = new TCanvas("c1", "canvas", 1200, 800);
	TCanvas *c2 = new TCanvas("c2", "canvas", 1200, 800);

	TLegend* leg = new TLegend(0.92,0.7,0.98,0.9);
	leg->SetHeader("Channels");


	// variables for holding input chain branches
	std::vector<double>* timestamps;
	std::vector<double>* energies;
	float runNumber;
	int channel;
	int ID;

	// variables for bookkeeping
	// std::vector<double> tDiff;  		// time difference in seconds
	std::vector<double> tDiffRun;		// for only a single run
	std::vector<double> EE;  			// energy to go along with each value
	std::map<int,int> problems;		// how many runs had problems (too long of a period without an event)
	std::map<int,int> problemPeriods;		// how many total times we had problems (could be more than once per run)
	std::map<int,int> runCount;		// how many runs we looked at for this channel

	std::map<int,int> zeros;			// number of times I saw 0 events in a complete run
	std::map<int,int> modZeros; 		// modified zeros map (removing alternating pulser runs)

	std::map<int, std::vector<int>> zeroRuns;	// map of vectors showing all runs which are zero rate for each channel
	std::map<int, std::vector<int>> modZeroRuns;	// zeroRuns, excluding alternating pulsers

	std::map<int,int> gapCount;				// gapCount[run][channel] = number of times in run that gap exceeded threshold
	std::map<unsigned long, double> gapCumulative;	// gapCumulative[run][channel] = cumulative gap size of gaps>thresh for run

	std::map<int, std::vector<int> >	badStart;	// badStart[channel].size() is the runs with bad starts (long gaps at beginning)
	std::map<int, std::vector<int> >	badStop;	// badStop[channel].size() is the runs with bad endings (long gaps at end)

	std::map<int,double> cumulative;		// total channel cumulative gaptime over all runs

	for(int i = 0; i<cNum; i++)		// initialize some channel counting stats
	{
		problems[chans[i]] = 0;
		problemPeriods[chans[i]] = 0.0;
		zeros[chans[i]] = 0;
		modZeros[chans[i]] = 0;
		timesChan[chans[i]] = 0;
		eventCount.push_back(0);		//sets all `cNum` channels to 0 events to begin with
		cumulative[chans[i]] = 0;
	}


	TImage *img = TImage::Create();
	TFile f("lt-dt-AllGood-out.root","recreate");


	std::cout << "Putting together the arrays of histograms..." << std::endl;
	for(int j=0;j<cNum;j++)
	{
		hTDiffs[j] = 	new TH2F(TString::Format("hTDiffs%d",chans[j]),"Hit #Delta t",1e4,0,1000,1e3,0,1e4);
		hGap[j] = 		new TH1F(TString::Format("hGap%3d",chans[j]), "Gap Time",36e3,0,3600);
		hGapRuns[j] = 	new TH2F(TString::Format("hGap%3d-Runs",chans[j]), TString::Format("Gap Time, Ch%d",chans[j]),36000,0,3600,5000,2500,7500);

		// hTDiffsBin[j] = new TH2F(n2,"Hit #Delta t",1e3,-0.1,10,1e3,0,1e4);
	}

	std::cout << "Starting loops over all channels..." << std::endl;

	for(int j=0;j<cNum;j++)
	{
		std::cout << "--   " << j << "   --   Channel " << chans[j] << ", digitizer " << getDigitizer(chans[j]) << std::endl;

		std::cout << "Adding runs to chain..." << std::endl;
		TChain* ch = new TChain("outTree", "outTree");
		ch->Add(TString::Format("out/final/lt-dt_0keV_*_%d.root",chans[j]));

		ch->SetBranchAddress("timestamps",&timestamps);
		ch->SetBranchAddress("channel",&channel);
		// ch->SetBranchAddress("rawEnergies",&energies);
		ch->SetBranchAddress("trapEnergies",&energies);
		ch->SetBranchAddress("runNumber",&runNumber);

		hTDiffs[j]->SetTitle(TString::Format("Subsequent Hit #Delta t, Channel %d",chans[j]));

		int nRuns = ch->GetEntries();

		std::cout << "This chain contains " << nRuns << " entries (runs)." << std::endl;

		runCount[chans[j]] = nRuns;

		if(nRuns==0)
		{
			std::cout << "That's weird, there were no runs associated with this channel in the selected series of runs... Maybe it wasn't turned on for this run series? Skipping..." << std::endl;
			continue;
		}

		for(int k=0; k < nRuns; k++)
		{
			ch->GetEntry(k);
			int num = timestamps->size();
			tDiffRun.reserve(num);
			EE.reserve(num);
			double diff;

			gapCount[ getID(runNumber,channel)] = 0.0;	// initialize this channel/run
			gapCumulative[ getID(runNumber,channel) ] = 0.0;

			if(j==0)
			{
				runsUsed.push_back(runNumber);
				timesUsed.push_back(times[runNumber]);
			}

			// std::cout << num <<  " events in run " << runNumber << " " << std::endl;
			if( num > 0)
			{
				// std::cout << " first timestamp is " << (1e-8)*timestamps->at(0) << std::endl;

				// handle the start situation (time between run start and first event)
				diff = (1e-8)*timestamps->at(0);
				tDiffRun.push_back(diff);
				EE.push_back( -1 );		// not quite, but close enough

				hGap[j]->Fill( diff );
				hGapRuns[j]->Fill( diff,runNumber );
				hTDiffs[j]->Fill( diff, EE.back() );

				if(diff > threshold)
				{
					gapCount[ getID(runNumber,channel) ] += 1;
					gapCumulative[ getID(runNumber,channel) ] += diff; // gapCumulative[(int)runNumber][channel] += diff;
					cumulative[channel] += diff;
					problemPeriods.at(channel) += 1;
					badStart[channel].push_back(runNumber);
					std::cout << "    bad start, r " << runNumber << ", gap of " << diff << std::endl;
				}

				// handle the "usual" middle cases
				for(int i=1; i<num; i++)
				{
					diff = (1e-8)*(timestamps->at(i) - timestamps->at(i-1));

					tDiffRun.push_back(diff);
					EE.push_back( energies->at(i) );

					hGap[j]->Fill( diff );
					hGapRuns[j]->Fill( diff,runNumber );
					hTDiffs[j]->Fill( diff, EE.back() );

					if(diff > threshold)
					{
						gapCount[ getID(runNumber,channel) ] += 1;
						gapCumulative[ getID(runNumber,channel) ] += diff; // gapCumulative[(int)runNumber][channel] += diff;
						cumulative[channel] += diff;
						problemPeriods.at(channel) += 1;
						badStart[channel].push_back(runNumber);
					}
				}

				// handle the end situation (time between last event and run stop)
				diff = times[runNumber] - (1e-8)*timestamps->at(num-1);
				tDiffRun.push_back(diff);
				EE.push_back( -1 );	// not quite, but close enough...

				hGap[j]->Fill( diff );
				hGapRuns[j]->Fill( diff,runNumber );
				hTDiffs[j]->Fill( diff, EE.back() );					// there's no nergy associated with the lack of an event, I'll call that -1 energy...

				if(diff > threshold)
				{
					gapCount[ getID(runNumber,channel) ] += 1;
					gapCumulative[ getID(runNumber,channel) ] += diff; // gapCumulative[(int)runNumber][channel] += diff;
					cumulative[channel] += diff;
					problemPeriods.at(channel) += 1;
					badStop[channel].push_back(runNumber);
					std::cout << "    bad stop, r " << runNumber << ", gap of " << diff << std::endl;
				}
			}

			if(num == 0)	// handle the case where there are no events in an entire run
			{
				diff = times[runNumber];	// says that there was a gap of the entire run length

				hGap[j]->Fill(diff);
				hGapRuns[j]->Fill( diff,runNumber );
				hTDiffs[j]->Fill( diff, -1 );
			}

			std::cout << k << "     Run " << runNumber << " contained " << num << " events, ";
			std::cout << " " << gapCumulative.at(getID(runNumber,channel)) << " / " << times[runNumber] << " seconds lost here, ";
			std::cout <<  " " << cumulative.at(channel) << " lost total" << std::endl; //

			timesChan[channel] += times[runNumber];


			eventCount[j] += num;
			if(num == 0)
			{
				zeros[channel]++;
				zeroRuns[channel].push_back(runNumber);
				hZeros->Fill(runNumber,channel,0.1);

				if (std::binary_search(questionableRuns.begin(), questionableRuns.end(), runNumber))
				{
					modZeros[channel]++;
					modZeroRuns[channel].push_back(runNumber);
				}
			}
			else{
				hZeros->Fill(runNumber,channel,100);
			}




			if(gapCount[getID(runNumber,channel)] > 1)
				problems[channel]++;

			hGaps->Fill(runNumber,channel,gapCount[getID(runNumber,channel)]);

			// Prepare the output tree info
			theRunNumber 	= runNumber;
			theChannel 		= channel;
			theDeltaTime 	= &tDiffRun;
			theTimestamp 	= timestamps;
			theRunTime		= times[runNumber];
			theEnergy 		= &EE;
			theID->assign(timestamps->size(),IDs[channel]);	// this is hacky and a waste of space, but it makes Draw("ID:time") much easier

			resultTree->Fill();

			tDiffRun.clear();		// reset this for each run, since we copied it already
			EE.clear();

			theDeltaTime->clear();
			theTimestamp->clear();
			theEnergy->clear();
			theID->clear();
		}

		// std::cout << "Ch " << channel << " result: " << cumulative.at(channel) << " / " << timesChan.at(channel) << std::endl;
		printf(" Ch %d result: %10.2f / %10.2f = %4.4f, or %4.6f uptime \n", channel, cumulative.at(channel),timesChan.at(channel), cumulative.at(channel)/timesChan.at(channel), 1-(cumulative.at(channel)/timesChan.at(channel)));
		// double frac = timesChan.at(channel)/cumulative.at(channel);
		// std::cout << "   Fractionally: " << frac << std::endl;
		// std::cout << "\n   tDiff has " << tDiff.size() << " entries..." << std::endl;
		// std::cout << "   Energy vector  has " << EE.size() << " entries..." << std::endl;

		c1->cd();
		c1->SetGrid(1,1);

		std::cout << "   Drawing histograms..." << std::endl;

		hTDiffs[j]->GetXaxis()->SetTitle("Time [seconds]");
		hTDiffs[j]->GetYaxis()->SetTitle("Energy [eV]");
		// hTDiffs[j]->SetStats(0);
		// hTDiffs[j]->SetLineColor(dChan[j]+1);
		hTDiffs[j]->Draw("colz");
		c1->Update();

		hTDiffs[j]->GetXaxis()->SetRangeUser(0,10);
		// hTDiffs[j]->SetMinimum(maxScale);	// This must be a bug that this works to set the MAXIMUM
		c1->Update();

		// std::cout << "Writing figure 1 as an image..." << std::endl;
		// img->FromPad(c1);
		// img->WriteImage(TString::Format("P3KJR_dT-Chan%d_0-2ms_readout.png",chans[j]));

		std::cout << "Writing histograms for channel " << chans[j] << " to disk..." << std::endl;

		hGap[j]->GetXaxis()->SetTitle("Time [seconds]");
		hGap[j]->GetYaxis()->SetTitle("Count");

		hGapRuns[j]->GetXaxis()->SetTitle("Time [seconds]");
		hGapRuns[j]->GetYaxis()->SetTitle("Run");

		hGap[j]->Write();
		hGapRuns[j]->Write();
		hTDiffs[j]->Write();

		std::cout << "   Deleting things..." << std::endl;

		ch->Reset();
	}

	std::cout << "Saving results to disk..." << std::endl;
	hZeros->Write();
	hGaps->Write();

	resultTree->Write();

	std::cout << "Summary:" << std::endl;
	std::cout << "Chan   		| Count  |  zeroCountRuns  badPeriods  badRuns  totalRuns" << std::endl;
	for(int j = 0; j<cNum; j++)
	{
		printf("%2d: %3d (%2d:%2d) %11d  %10d  %10d  %10d  %10d \n", j, chans[j], (chans[j]&0xFF)>>4, (chans[j]&0xF), eventCount[j], zeros.at(chans[j]), problemPeriods.at(chans[j]), problems.at(chans[j]), runCount.at(chans[j]) );

		// std::cout << j << ":  " << std::setw(5) << std::left << chans[j] << " (" << std::setw(2) << std::to_string(getDigitizer(chans[j])) << ":";
		// std::cout << std::to_string(getChannel(chans[j])) << ") " << eventCount[j] << std::setw(11) << " " << zeros.at(chans[j]) << std::setw(10) << " ";
		// std::cout << problemPeriods.at(chans[j]) << std::setw(10) << " ";
		// std::cout << problems.at(chans[j]) << std::setw(10) << " " << runCount.at(chans[j]) << " " << std::endl;
	}

	std::cout << "\n\nAlternatively..." << std::endl << std::endl;
	for(int dd=4; dd<12; dd++)
	{
		std::cout << "Digitizer slot " << dd << std::endl;
		for(int j = 0; j<cNum; j++)
		{
			if(getDigitizer(chans[j]) == dd)
			{
				printf("%2d: %3d (%2d:%2d) %11d  %10d  %10d  %10d  %10d \n", j, chans[j], (chans[j]&0xFF)>>4, (chans[j]&0xF), eventCount[j], zeros.at(chans[j]), problemPeriods.at(chans[j]), problems.at(chans[j]), runCount.at(chans[j]) );
				std::cout << " 	Zeros: ";
				for(int k=0;k<zeroRuns[chans[j]].size();k++)
					std::cout << zeroRuns[chans[j]].at(k) << ", ";
				std::cout << " " << std::endl;

			// 	std::cout << j << ":  " << chans[j] << " (" << std::to_string(getDigitizer(chans[j])) << ":";
			// 	std::cout << std::to_string(getChannel(chans[j])) << ")	" << eventCount[j] << "		" << zeros.at(chans[j]);
			// 	std::cout << "	" << problemPeriods.at(chans[j]) << "	";
			// 	std::cout << "	 " << problems.at(chans[j]) << "	 "  << runCount.at(chans[j]) << std::endl;
			}

		}

		std::cout << std::endl;
	}


	std::cout << "\n\nExcluding alternating pulser runs..." << std::endl << std::endl;

	for(int dd=4; dd<12; dd++)
	{
		std::cout << "Digitizer slot " << dd << std::endl;
		for(int j = 0; j<cNum; j++)
		{
			if(getDigitizer(chans[j]) == dd)
			{
				printf("%2d: %3d %11d  %10d  %10d  %10d  %10d \n", j, chans[j], eventCount[j], modZeros.at(chans[j]), problemPeriods.at(chans[j]), problems.at(chans[j]), runCount.at(chans[j]) );
				std::cout << " 	Zeros: ";
				for(int k=0;k<modZeroRuns[chans[j]].size();k++)
					std::cout << modZeroRuns[chans[j]].at(k) << ", ";
				std::cout << " " << std::endl;

			// 	std::cout << j << ":  " << chans[j] << " (" << std::to_string(getDigitizer(chans[j])) << ":";
			// 	std::cout << std::to_string(getChannel(chans[j])) << ")	" << eventCount[j] << "		" << zeros.at(chans[j]);
			// 	std::cout << "	" << problemPeriods.at(chans[j]) << "	";
			// 	std::cout << "	 " << problems.at(chans[j]) << "	 "  << runCount.at(chans[j]) << std::endl;
			}

		}

		std::cout << std::endl;
	}


	// std::cout << "\n\nBad starts/stops..." << std::endl << std::endl;

	// std::cout << "  Chan   | cumalativeGap  totalRunTime  totalLiveFraction totalRuns" << std::endl;
	// for(int j = 0; j<cNum; j++)
	// {
	// 	printf("%2d: %3d    %10.4f    %10.2f   %10.4f   %5d\n",
	// 		j,
	// 		chans[j],
	// 		cumulative[chans[j]],
	// 		timesChan[chans[j]],
	// 		1-(cumulative.at(chans[j])/timesChan.at(chans[j])),
	// 		runCount.at(chans[j])
	// 	);
	// }


	std::cout << "\n\nFinally..." << std::endl << std::endl;


	std::cout << "  Chan   | badStarts badStops cumalativeGap  totalRunTime  totalLiveFraction totalRuns" << std::endl;
	for(int j = 0; j<cNum; j++)
	{
		printf("%2d: %3d   %4lu     %4lu    %10.4f    %10.2f   %10.4f   %5d\n",
			j,
			chans[j],
			badStart[chans[j]].size(),
			badStop[chans[j]].size(),
			cumulative[chans[j]],
			timesChan[chans[j]],
			1-(cumulative.at(chans[j])/timesChan.at(chans[j])),
			runCount.at(chans[j])
		);
	}

	std::cout << "Producing output of all data:" << std::endl;

	// print a 2D array that python can take in as array[channel,runTime,lostTime]
	int nRunsUsed = runsUsed.size(); // they're all the same, I picked the first one

	std::cout << "fullOut = np.array(\n[";
	for(int cc=0; cc<cNum; cc++)
	{
		for(int rr=0; rr<nRunsUsed; rr++)
		{
			printf("[%d,%.5f,%.5f,%d],",
			chans[cc],
			timesUsed[rr],
			gapCumulative.at(getID(runsUsed[rr],chans[cc])),
			runsUsed[rr]
		);
		}
	}
std::cout << "\n]\n)\n\n";



	std::cout << "Producing summary of all data:" << std::endl;


	std::cout << "\n\n Runs with bad start gaps" << std::endl;
	for(int j = 0; j<cNum; j++)
	{
		std::cout << j << ": " << chans[j] << " had " << badStart[chans[j]].size() << ": \n    ";
		for(int kk=0; kk<badStart[chans[j]].size(); kk++)
		{
			std::cout << badStart[chans[j]].at(kk) << ", ";
		}
		std::cout << std::endl;
	}

	std::cout << " Runs with bad finish gaps" << std::endl;
	for(int j = 0; j<cNum; j++)
	{
		std::cout << j << ": " << chans[j] << " had " << badStop[chans[j]].size() << ": \n    ";
		for(int kk=0; kk<badStop[chans[j]].size(); kk++)
		{
			std::cout << badStop[chans[j]].at(kk) << ", ";
		}
		std::cout << std::endl;
	}


	std::cout << "chans = [";
	for(int pp=0; pp<cNum; pp++)
	{
		std::cout << chans[pp];
		if(pp<cNum-1) std::cout << ",";
	}
	std::cout << "]" << std::endl;


	std::cout << "numBadStarts[chan] = [";
	for(int pp=0; pp<cNum-1; pp++)
	{
		std::cout << badStart[chans[pp]].size();
		if(pp<cNum-1) std::cout << ",";
	}
	std::cout << badStart[chans[cNum-1]].size() << "]" << std::endl;


	std::cout << "numBadStops[chan] = [";
	for(int pp=0; pp<cNum-1; pp++)
	{
		std::cout << badStop[chans[pp]].size();
	if(pp<cNum-1) std::cout << ",";
	}
	std::cout << badStop[chans[cNum-1]].size() << "]" << std::endl;

	std::cout << "numBadPeriods[chan] = [";
	for(int pp=0; pp<cNum-1; pp++)
	{
		std::cout << problemPeriods.at(chans[pp]);
		if(pp<cNum-1) std::cout << ",";
	}
	std::cout << problemPeriods.at(chans[cNum-1]) << "]" << std::endl;

	std::cout << "cumulative[chan] = [";
	for(int pp=0; pp<cNum; pp++)
	{
		std::cout << cumulative[chans[pp]];
		if(pp<cNum-1) std::cout << ",";
	}
	std::cout << "]" << std::endl;









	std::cout << "Done." << std::endl;

	return 0;

}


int main()
{
	freq_out_energy(0);

	return 0;
}
