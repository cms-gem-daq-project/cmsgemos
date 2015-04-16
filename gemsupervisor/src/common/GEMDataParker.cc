#include "gem/supervisor/GEMDataParker.h"
#include "gem/supervisor/GEMDataAMCformat.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include <boost/utility/binary.hpp>
#include <bitset>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

// Main constructor
gem::supervisor::GEMDataParker::GEMDataParker(gem::hw::vfat::HwVFAT2& vfatDevice, std::string& outFileName)
{
    vfatDevice_ = &vfatDevice;
    outFileName_ = outFileName;
    counter_ = 0;
}

int gem::supervisor::GEMDataParker::getGLIBData(gem::supervisor::ChannelData& ch, gem::supervisor::VFATData& vf)
{
    // Book event variables

    bool     isFirst = true;
    uint8_t  SBit, flags;
    uint16_t bcn, evn, chipid;
    uint32_t bxNum, bxExp, TrigReg, bxNumTr;
    uint64_t msData, lsData;

    // GLIB data buffer validation
    boost::format linkForm("LINK%d");
    uint32_t fifoDepth[3];
    vfatDevice_->setDeviceBaseNode("GLIB");
    fifoDepth[0] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
    fifoDepth[1] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
    fifoDepth[2] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");

    int bufferDepth = 0;
    if ( fifoDepth[0] != fifoDepth[1] || fifoDepth[0] != fifoDepth[2] || fifoDepth[1] != fifoDepth[2] ) {
       bufferDepth = std::min(fifoDepth[0],std::min(fifoDepth[1],fifoDepth[2]));
    }

    //right now only have FIFO on LINK1
    bufferDepth = fifoDepth[1];

    // For each event in GLIB data buffer
    while (bufferDepth) {
      std::vector<uint32_t> data;
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.TRK_DATA.COL1");

      if (vfatDevice_->readReg("DATA_RDY")) {
        for (int word = 0; word < 7; ++word) {
             std::stringstream ss9;
             ss9 << "DATA." << word;
             data.push_back(vfatDevice_->readReg(ss9.str()));
        }
      }

      // read trigger data
      vfatDevice_->setDeviceBaseNode("GLIB");
      //TrigReg = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRG_DATA.DATA");
      TrigReg = vfatDevice_->readReg("TRG_DATA.DATA");
      bxNumTr = TrigReg >> 6;
      SBit = TrigReg & 0x0000003F;

      if (!(
            (((data.at(5)&0xF0000000)>>28)==0xa) &&
            (((data.at(5)&0x0000F000)>>12)==0xc) &&
            (((data.at(4)&0xF0000000)>>28)==0xe)
          )) {
               vfatDevice_->setDeviceBaseNode("GLIB");
               bufferDepth = vfatDevice_->readReg("LINK1.TRK_FIFO.DEPTH");
               continue;
      }

      bxNum = data.at(6);

      if (isFirst) bxExp = bxNum;
      if (bxNum == bxExp) isFirst = false;

      // bxExp:28
      // bxNum  = (bxNum << 8 ) | (SBit); // bxNum:8  | SBit:8

      bcn    = (0x0fff0000 & data.at(5)) >> 16;
      evn    = (0x00000ff0 & data.at(5)) >> 4;
      chipid = (0x0fff0000 & data.at(4)) >> 16;
      flags  = (0x0000000f & data.at(5));

      uint64_t data1  = ((0x0000ffff & data.at(4)) << 16) | ((0xffff0000 & data.at(3)) >> 16);
      uint64_t data2  = ((0x0000ffff & data.at(3)) << 16) | ((0xffff0000 & data.at(2)) >> 16);
      uint64_t data3  = ((0x0000ffff & data.at(2)) << 16) | ((0xffff0000 & data.at(1)) >> 16);
      uint64_t data4  = ((0x0000ffff & data.at(1)) << 16) | ((0xffff0000 & data.at(0)) >> 16);

      lsData = (data3 << 32) | (data4);
      msData = (data1 << 32) | (data2);

      vf.BC     = ( ((data.at(5) & 0xF0000000)>>28) << 12 ) | (bcn);                // 1010     | bcn:12
      vf.EC     = ( ((data.at(5) & 0x0000F000)>>12) << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4 (zero?)
      vf.ChipID = ( ((data.at(4) & 0xF0000000)>>28) << 12 ) | (chipid);             // 1110     | ChipID:12
      ch.lsData = lsData;                                                           // lsData:64
      ch.msData = msData;                                                           // msData:64
      vf.crc    = 0x0000ffff & data.at(0);                                          // crc:16

      counter_++;

      /*
       * dump event
       gem::supervisor::PrintVFATDataBits(counter_, vf, ch);
      */

      vfatDevice_->setDeviceBaseNode("GLIB");
      bufferDepth = vfatDevice_->readReg("LINK1.TRK_FIFO.DEPTH");
    }
    return counter_;
}

int gem::supervisor::GEMDataParker::dumpDataToDisk()
{
    // Book GEM Data format
    gem::supervisor::ChannelData ch;
    gem::supervisor::VFATData vf;
    gem::supervisor::GEBData geb;
    gem::supervisor::GEMData gem;

    // get GLIB data from one VFAT chip
    counter_ = gem::supervisor::GEMDataParker::getGLIBData(ch, vf);
    geb.vfats.push_back(vf);
    cout << " counter= " << counter_ << endl;

    /*
     * One GEM bord loop, 24 VFAT chips maximum
     */

    /*
     * GEB, Chamber Data
     */

    // Chamber Header, Zero Suppression flags, Chamber ID
    uint64_t ZSFlag      = BOOST_BINARY( 1 ); // :24
    uint64_t ChamID      = 0xdea;             // :12

    geb.header  = ( (ZSFlag << 40) ) | ( ChamID << 28 );

    ZSFlag =  (0xffffff0000000000 & geb.header) >> 40; 
    ChamID =  (0x000000fff0000000 & geb.header) >> 28; 

    cout << " ZSFlag " << hex << ZSFlag << " ChamID " << ChamID << dec << endl;

    // Chamber Trailer, OptoHybrid: crc, wordcount, Chamber status
    uint64_t OHcrc       = BOOST_BINARY( 1 ); // :16
    uint64_t OHwCount    = BOOST_BINARY( 1 ); // :16
    uint64_t ChamStatus  = BOOST_BINARY( 1 ); // :16
    geb.trailer = ( (OHcrc << 48) | (OHwCount << 32 ) | (ChamStatus << 16) );

    OHcrc =      (0xffff000000000000 & geb.trailer) >> 48; 
    OHwCount =   (0x0000ffff00000000 & geb.trailer) >> 32; 
    ChamStatus = (0x00000000ffff0000 & geb.trailer) >> 16;

    cout << " OHcrc " << hex << OHcrc << " OHwCount " << OHwCount << " ChamStatus " << ChamStatus << dec << endl;

    int nVFATs = 2;
    for (int nume = 1; nume < nVFATs; nume++){
      vf.ChipID = (0x0E << 12) | (0xdea); // ChipID dead if data are missing
      geb.vfats.push_back(vf);
    }

    int nChip=0;
    // cout << " Number VFAT blocks on the GEM is " << geb.vfats.size() << endl;

    for (vector<VFATData>::iterator it=geb.vfats.begin(); it != geb.vfats.end(); ++it) {

      nChip++;
      cout << nChip << " ChipID  " << hex << (*it).ChipID << dec << endl; 

      //gem::supervisor::keepVFATData(outFileName_, counter_, vf, ch);
      //gem::supervisor::PrintVFATData(counter_, vf, ch);
      // gem::supervisor::keepVFATDataBinary(outFileName_, counter_, vf, ch);
      
      gem::supervisor::PrintVFATDataBits(counter_, vf, ch);
    }

    int nGEBs = 2;
    for (int nume = 1; nume < nGEBs; nume++){
      gem.gebs.push_back(geb);
    }

    return counter_;
}
