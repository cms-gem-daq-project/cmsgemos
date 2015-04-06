#include "gem/supervisor/GEMDataParker.h"
#include "gem/supervisor/tbutils/ThresholdEvent.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include <iomanip>
#include <iostream>
#include <boost/format.hpp>

// Main constructor
gem::supervisor::GEMDataParker::GEMDataParker(gem::hw::vfat::HwVFAT2& vfatDevice, std::string& outFileName)
{
    vfatDevice_ = &vfatDevice;
    outFileName_ = outFileName;
    counter_ = 0;
}

int gem::supervisor::GEMDataParker::dumpDataToDisk()
{
    // Book event variables
    tbutils::ChannelData ch;
    tbutils::VFATEvent ev;
    int event=0;

    // GLIB data buffer validation
    boost::format linkForm("LINK%d");
    uint32_t fifoDepth[3];
    vfatDevice_->setDeviceBaseNode("GLIB");
    fifoDepth[0] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
    fifoDepth[1] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
    fifoDepth[2] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");

    int bufferDepth = 0;
    if (fifoDepth[0] != fifoDepth[1] || 
            fifoDepth[0] != fifoDepth[2] || 
            fifoDepth[1] != fifoDepth[2]) {
        bufferDepth = std::min(fifoDepth[0],std::min(fifoDepth[1],fifoDepth[2]));
    }
    //right now only have FIFO on LINK1
    bufferDepth = fifoDepth[1];

    bool isFirst = true;
    uint32_t bxNum, bxExp;

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

        uint32_t TrigReg, bxNumTr;
        uint8_t SBit;

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

        uint16_t bcn, evn, crc, chipid;
        uint64_t msData, lsData;
        uint8_t  flags;

        if (isFirst)
            bxExp = bxNum;

        if (bxNum == bxExp)
            isFirst = false;

        bxNum  = data.at(6);
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

        crc    = 0x0000ffff & data.at(0);

        ch.lsData = lsData;
        ch.msData = msData;

        ev.BC = ((data.at(5)&0xF0000000)>>28) << 12; // 1010
        ev.BC = (ev.BC | bcn);
        ev.EC = ((data.at(5)&0x0000F000)>>12) << 12; // 1100
        ev.EC = (ev.EC | evn) << 4;
        ev.EC = (ev.EC | flags);
        ev.bxExp = bxExp;
        ev.bxNum = bxNum << 6;
        ev.bxNum = (ev.bxNum | SBit);
        ev.ChipID = ((data.at(4)&0xF0000000)>>28) << 12; // 1110
        ev.ChipID = (ev.ChipID | chipid);
        ev.crc = crc;

        // dump event to disk
        keepEvent(outFileName_, event, ev, ch);
        counter_++;

        std::cout << "Received tracking data word:" << std::endl
            << "bxn     :: 0x" << std::setfill('0') << std::setw(4) << std::hex << bxNum  << std::dec << std::endl
            << "bcn     :: 0x" << std::setfill('0') << std::setw(4) << std::hex << bcn    << std::dec << std::endl
            << "evn     :: 0x" << std::setfill('0') << std::setw(4) << std::hex << evn    << std::dec << std::endl
            << "flags   :: 0x" << std::setfill('0') << std::setw(2) << std::hex << (unsigned)flags  << std::dec << std::endl
            << "chipid  :: 0x" << std::setfill('0') << std::setw(4) << std::hex << chipid << std::dec << std::endl
            << "<127:0> :: 0x" << std::setfill('0') << std::setw(8) << std::hex << msData <<
            std::dec << std::setfill('0') << std::setw(8) << std::hex << lsData << std::dec << std::endl
            << "<127:64>:: 0x" << std::setfill('0') << std::setw(8) << std::hex << msData << std::dec << std::endl
            << "<63:0>  :: 0x" << std::setfill('0') << std::setw(8) << std::hex << lsData << std::dec << std::endl
            << "crc     :: 0x" << std::setfill('0') << std::setw(4) << std::hex << crc    << std::dec << std::endl
            ;

        vfatDevice_->setDeviceBaseNode("GLIB");
        bufferDepth = vfatDevice_->readReg("LINK1.TRK_FIFO.DEPTH");
    }

    return counter_;
}
