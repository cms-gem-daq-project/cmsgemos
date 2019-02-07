/**
 * class: GEMOnlineDBManager
 * description: OnlineDBManager application for GEM system
 * author: J. Sturdy
 * date:
 */

#include "gem/onlinedb/GEMOnlineDBManager.h"

#include <cstdlib>
#include <iomanip>

#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "gem/onlinedb/GEMOnlineDBManagerWeb.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/exception/Exception.h"


XDAQ_INSTANTIATOR_IMPL(gem::onlinedb::GEMOnlineDBManager);

gem::onlinedb::GEMOnlineDBManager::GEMOnlineDBManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMApplication(stub)
{
  p_gemWebInterface = new gem::onlinedb::GEMOnlineDBManagerWeb(this);

}

gem::onlinedb::GEMOnlineDBManager::~GEMOnlineDBManager()
{

}

void gem::onlinedb::GEMOnlineDBManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    CMSGEMOS_DEBUG("GEMOnlineDBManager::actionPerformed() setDefaultValues"
                   << "Default configuration values have been loaded from xml profile");
  }

  // item is changed, update it
  if (event.type() == "ItemChangedEvent" || event.type() == "urn:xdata-event:ItemChangedEvent") {
    CMSGEMOS_DEBUG("GEMOnlineDBManager::actionPerformed() ItemChangedEvent");
  }

  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::onlinedb::GEMOnlineDBManager::init()
{
}
