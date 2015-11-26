/**
 * class: GEMMonitor
 * description: Generic GEM monitor, all GEM monitoring applications should inherit
 * from this class and define and extend as necessary
 * structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

// GEMMonitor.cc

#include "gem/base/GEMMonitor.h"
#include "gem/base/GEMApplication.h"
#include "gem/base/GEMFSMApplication.h"

#include "xdata/InfoSpace.h"

gem::base::GEMMonitor::GEMMonitor(log4cplus::Logger& logger, xdaq::ApplicationStub* stub) : 
  m_gemLogger(logger)
{
  
}

gem::base::GEMMonitor::GEMMonitor(log4cplus::Logger& logger, GEMApplication* gemApp) : 
  m_gemLogger(logger)
{
  addInfoSpace("Application",  gemApp->getAppISToolBox());
  addInfoSpace("Configuration",gemApp->getCfgISToolBox());
  addInfoSpace("Monitoring",   gemApp->getMonISToolBox());
}

gem::base::GEMMonitor::GEMMonitor(log4cplus::Logger& logger, GEMFSMApplication* gemFSMApp) : 
  m_gemLogger(logger)
{
  addInfoSpace("Application",  gemFSMApp->getAppISToolBox());
  addInfoSpace("AppState",     gemFSMApp->getAppStateISToolBox());
  addInfoSpace("Configuration",gemFSMApp->getCfgISToolBox());
  addInfoSpace("Monitoring",   gemFSMApp->getMonISToolBox());
  
}

gem::base::GEMMonitor::~GEMMonitor()
{
  
}

void gem::base::GEMMonitor::timeExpired(toolbox::task::TimerEvent&)
{
  updateMonitorables();
}

void gem::base::GEMMonitor::addInfoSpace(std::string const& name,
                                         std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> infoSpace)
{
  // should we key by name or infoSpace->name(), such that the infoSpace could be retrieved from the infoSpaceFactory
  //std::unordered_map<std::string, gem::base::utils::GEMInfoSpaceToolBox*>::const_iterator it = m_infoSpaceMap.find(infoSpace->name());
  std::unordered_map<std::string,
    std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> >::const_iterator it = m_infoSpaceMap.find(name);
  if (it != m_infoSpaceMap.end()) {
    // WARN( "GEMMonitor: infospace " << infoSpace->name() << " already exists in monitor!" );
    WARN( "GEMMonitor: infospace " << name << " already exists in monitor!" );
    return;
  }
  //DEBUG( "GEMMonitor: adding infospace " << infoSpace->name() );
  //m_infoSpaceMap.insert(std::make_pair<std::string, gem::base::utils::GEMInfoSpaceToolBox*>(infoSpace->name(),infoSpace));
  DEBUG( "GEMMonitor: adding infospace " << name );
  m_infoSpaceMap.insert(std::make_pair(name,infoSpace));
  std::list<std::string> emptyList;
  m_monitorableSetInfoSpaceMap.insert(std::make_pair(name,emptyList));
}

void gem::base::GEMMonitor::addMonitorableSet(std::string const& setname, std::string const& infoSpaceName)
{
  std::unordered_map<std::string,
    std::list<std::pair<std::string, GEMMonitorable> > >::const_iterator it;
  it = m_monitorableSetsMap.find(setname);
  if (it != m_monitorableSetsMap.end()) {
    WARN( "GEMMonitor: monitorable set " << setname << " already exists in monitor!" );
    return;
  }
  std::list<std::pair<std::string, GEMMonitorable> > emptySet;
  m_monitorableSetsMap.insert(std::make_pair<std::string const&,
                              std::list<std::pair<std::string, GEMMonitorable> > >(setname, emptySet));
  
}

void gem::base::GEMMonitor::addMonitorable(std::string const& setname,
                                           std::string const& infoSpaceName,
                                           std::pair<std::string, std::string> const& monpair,
                                           gem::base::utils::GEMInfoSpaceToolBox::UpdateType type,
                                           std::string const& format)
{
  if (m_infoSpaceMap.find(infoSpaceName) == m_infoSpaceMap.end()) {
    ERROR( "GEMMonitor: infoSpace " << infoSpaceName << " does not exist in monitor!" );
    return;
  } else if (m_monitorableSetsMap.find(setname) == m_monitorableSetsMap.end()) {
    ERROR( "GEMMonitor: monitorable set " << setname << " does not exist in monitor!" );
    return;
  }
  
  std::shared_ptr<utils::GEMInfoSpaceToolBox> infoSpace = m_infoSpaceMap.find(infoSpaceName)->second;
  if (infoSpace->find(monpair.first)) {
    std::unordered_map<std::string,
      std::list<std::pair<std::string, GEMMonitorable> > >::iterator it;
    it = m_monitorableSetsMap.find(setname);
    GEMMonitorable monitem = {monpair.first, monpair.second, infoSpace, type, format};
    (*it).second.push_back(std::make_pair(monpair.first, monitem));
  } else {
    ERROR( "GEMMonitor: monitorable " << monpair.first << " does not exist in infospace "
           << infoSpaceName << "!" );
    return;
  }
  
  addInfoSpace(setname, infoSpace);
}

