#ifndef gemOnlineDQM_H
#define gemOnlineDQM_H
#define NVFAT 24
#define DEBUG_ 0
#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <memory>
#include <TFile.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TNtuple.h>
#include <TH2.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TFrame.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TRandom3.h>
#include <TBenchmark.h>
#include <TInterpreter.h>
#include <TApplication.h>
#include <TString.h>
#include <TObject.h>
#include <TClonesArray.h>
#include <TRefArray.h>
#include <TRef.h>
#include <TH1.h>
#include <TBits.h>
#include <TMath.h>
#include <TFile.h>
#include <TClassTable.h>
#include <TTree.h>
#include <TBranch.h>
#include <TError.h>

#include "gem/readout/GEMDataAMCformat.h"
#include "gem/datachecker/GEMDataChecker.h"
#include "gem/readout/GEMslotContents.h"
#include "GEMClusterization/GEMStrip.h"
#include "GEMClusterization/GEMStripCollection.h"
#include "GEMClusterization/GEMClusterContainer.h"
#include "GEMClusterization/GEMClusterizer.h"

namespace gem {
  namespace readout {
    class gemOnlineDQM {
      public:
        gemOnlineDQM(std::string slotFile){this->init(slotFile);}
        ~gemOnlineDQM(){}
        void Update(const gem::readout::GEMDataAMCformat::GEBData& geb){
          for (auto it = geb.vfats.begin(); it != geb.vfats.end(); ++it){
            hiVFATsn->Fill(this->sn(*it));
            for (unsigned i = 0; i < NVFAT; ++i){
              this->fillStrips(*it, i);
            }
            this->fillClusters();
            this->print();
          }
        }
        
      private:
        std::map<int,int> strip_maps[NVFAT];
        std::map<int, GEMStripCollection> allstrips;
        std::string slot_file;
        TH1F* hiVFATsn;
        TH1F* hiClusterMult;
        TH1F* hiClusterSize;
        TH1F* hiStripsFired[NVFAT];
        TH2F* hiBeamProfile;
//=================================================================================================================
        void init(std::string slotFile_){
          slot_file = slotFile_;
          std::string type[NVFAT] = {"Slot0" , "Slot1" , "Slot2" , "Slot3" , "Slot4" , "Slot5" , "Slot6" , "Slot7", 
                                     "Slot8" , "Slot9" , "Slot10", "Slot11", "Slot12", "Slot13", "Slot14", "Slot15", 
                                     "Slot16", "Slot17", "Slot18", "Slot19", "Slot20", "Slot21", "Slot22", "Slot23"};
          char name[128], title[500];
          hiVFATsn       = new TH1F("VFATsn", "VFAT slot number", 24,  0., 24. );
          hiClusterMult  = new TH1F("ClusterMult", "Cluster multiplicity", 384,  0, 384 );
          hiClusterSize  = new TH1F("ClusterSize", "Cluster size", 384,  0, 384 );
          hiBeamProfile  = new TH2F("BeamProfile", "Beam Profile", 8, 0, 8, 384, 0, 384);
          for (unsigned i = 0; i < NVFAT; i++){
            sprintf (name , "hiStripsFired_%s", type[i].c_str());
            sprintf (title, "Strips fired for VFAT chip %s", type[i].c_str());
            hiStripsFired[i] = new TH1F(name, title, 20, 0., 20.);
            std::string path;
            path = std::getenv("BUILD_HOME");
            if (DEBUG_) std::cout << "[gemOnlineDQM]: path to maps : " << path << std::endl; 
            if (i < 2) {
              path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips0-1.csv";
            } else if (i < 16) {
              path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips2-15.csv";
            } else if (i < 18) {
              path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips16-17.csv";
            } else {
              path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips18-23.csv";
            }
            this->readMap(i,path);
          }
        }
        void fillClusters(){
          int ncl=0;
          for (std::map<int, GEMStripCollection>::iterator ieta=allstrips.begin(); ieta!= allstrips.end(); ieta++){
            GEMClusterizer clizer;
            GEMClusterContainer cls = clizer.doAction(ieta->second);
            ncl+=cls.size();
            for (GEMClusterContainer::iterator icl=cls.begin();icl!=cls.end();icl++){
              hiClusterSize->Fill(icl->clusterSize());	  
            }
          }
          hiClusterMult->Fill(ncl);	  
          allstrips.clear();
        }
        int sn(const gem::readout::GEMDataAMCformat::VFATData& vfat){
          std::unique_ptr<gem::readout::GEMslotContents> slotInfo_ = std::unique_ptr<gem::readout::GEMslotContents> (new gem::readout::GEMslotContents(slot_file));
          uint32_t t_chipID = static_cast<uint32_t>(0x0fff & vfat.ChipID);
          return slotInfo_->GEBslotIndex(t_chipID);
        }
        void fillStrips(const gem::readout::GEMDataAMCformat::VFATData& vfat, int m){
          uint16_t chan0xfFiredchip = 0;
          for (int chan = 0; chan < 128; ++chan) {
            if (chan < 64){
              chan0xfFiredchip = ((vfat.lsData >> chan) & 0x1);
              if(chan0xfFiredchip) {
                hiStripsFired[m]->Fill(strip_maps[m].find(chan+1)->second);
                int m_i = (int) m%8;
                int m_j = strip_maps[m].find(chan+1)->second + ((int) m/8)*128;
  		          if (allstrips.find(m_i) == allstrips.end()){
  		            GEMStripCollection strips;
  		            allstrips[m_i]=strips;
  		          }
  		          // bx set to 0...
  		          GEMStrip s(m_j,0);
  		          allstrips[m_i].insert(s);
                if (DEBUG_) std::cout << "[gemOnlineDQM]: Beam profile x : " << m_i << " Beam profile y : " << m_j <<  std::endl;
                hiBeamProfile->Fill(m_i,m_j);
              }
            } else {
              chan0xfFiredchip = ((vfat.msData >> (chan-64)) & 0x1);
              if(chan0xfFiredchip) {
                hiStripsFired[m]->Fill(strip_maps[m].find(chan+1)->second);
                int m_i = (int) m%8;
                int m_j = strip_maps[m].find(chan+1)->second + ((int) m/8)*128;
  		          if (allstrips.find(m_i) == allstrips.end()){
  		            GEMStripCollection strips;
  		            allstrips[m_i]=strips;
  		          }
  		          // bx set to 0...
  		          GEMStrip s(m_j,0);
  		          allstrips[m_i].insert(s);
                if (DEBUG_) std::cout << "[gemOnlineDQM]: Beam profile x : " << m_i << " Beam profile y : " << m_j <<  std::endl;
                hiBeamProfile->Fill(m_i,m_j);
              }
            }
          }
        }
        void readMap(int slot_, std::string ifpath)
        {
          std::string ifpath_ = ifpath;
          std::ifstream icsvfile_;
          icsvfile_.open(ifpath_);
          if(!icsvfile_.is_open()) {
            std::cout << "\nThe file: " << icsvfile_ << " is missing.\n" << std::endl;
            return;
          }  
          for (int il = 0; il < 128; il++) {
            std::string line;
            std::getline(icsvfile_, line);
            if (DEBUG_) std::cout << "[gemOnlineDQM]: Read line : " << line << std::endl; 
            std::istringstream iss(line);
            std::pair<int,int> map_;
            std::string val;
            std::getline(iss,val,',');
            if (DEBUG_) std::cout << "[gemOnlineDQM]: First val : " << val << std::endl; 
            std::stringstream convertor(val);
            convertor >> std::dec >> map_.second;
            if (DEBUG_) std::cout << "[gemOnlineDQM]: First val recorded : " << map_.second << std::endl; 
            std::getline(iss,val,',');
            if (DEBUG_) std::cout << "[gemOnlineDQM]: Second val : " << val << std::endl; 
            convertor.str("");
            convertor.clear();
            convertor << val;
            convertor >> std::dec >> map_.first;
            if (DEBUG_) std::cout << "[gemOnlineDQM]: Second val recorded : " << map_.first << std::endl; 
            strip_maps[slot_].insert(map_);
          }
        }
        void print(TString prefix="./temp_plots/")
        {
          TCanvas *c = new TCanvas("c","c", 600,600);
          c->cd();
          hiVFATsn->Draw();
          gROOT->ProcessLine(".!mkdir -p ./"+prefix);
          c->Print(prefix+hiVFATsn->GetTitle()+".png","png");
          hiClusterMult->Draw();
          c->Print(prefix+hiClusterMult->GetTitle()+".png","png");
          hiClusterSize->Draw();
          c->Print(prefix+hiClusterSize->GetTitle()+".png","png");
          for (unsigned i = 0; i < NVFAT; i++){
            hiStripsFired[i]->Draw();
            c->Print(prefix+hiStripsFired[i]->GetTitle()+".png","png");
          }
          hiBeamProfile->Draw();
          c->Print(prefix+hiBeamProfile->GetTitle()+".png","png");
        }
    };
  }
}
#endif
