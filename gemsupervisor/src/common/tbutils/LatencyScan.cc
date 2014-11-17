#include "gem/supervisor/tbutils/LatencyScan.h"

gem::supervisor::tbutils::LatencyScan::LatencyScan() :
  actSig_(0)
{
  initSig_  = toolbox::task::bind(this, &LatencyScan::initialize, "initialize");
  confSig_  = toolbox::task::bind(this, &LatencyScan::configure,  "configure" );
  startSig_ = toolbox::task::bind(this, &LatencyScan::start,      "start"     );
  stopSig_  = toolbox::task::bind(this, &LatencyScan::stop,       "stop"      );
  //Sig_ = toolbox::task::bind(this, &LatencyScan::initialize, "initialize");
  
}

gem::supervisor::tbutils::LatencyScan::~LatencyScan() :
  
{
  
}

//Actions
gem::supervisor::tbutils::LatencyScan::initialize() :
  
{
  //here the connection to the device should be made
  
}

gem::supervisor::tbutils::LatencyScan::configure() :
  
{
  
}

gem::supervisor::tbutils::LatencyScan::start() :
  
{
  
}

gem::supervisor::tbutils::LatencyScan::stop() :
  
{
  
}

