#include "gem/datachecker/GEMDataChecker.h"
//#include "gem/readout/GEMDataAMCformat.h"

#include <boost/utility/binary.hpp>
#include <bitset>

#include <sstream>
#include <vector>

int counter_vfats_ = 0;
int event_counter_ = 0;

// Main constructor
gem::datachecker::GEMDataChecker::GEMDataChecker(gem::readout::GEMData& gem, gem::readout::GEBData& geb, 
                                                 gem::readout::VFATData& vfat)
{
  event_counter_ = 0;
}

int gem::datachecker::GEMDataChecker::counterGEMdata(gem::readout::GEMData& gem, gem::readout::GEBData& geb, 
                                                     gem::readout::VFATData& vfat)
{
  return event_counter_;
}
