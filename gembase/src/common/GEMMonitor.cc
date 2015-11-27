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

#include "xgi/Input.h"
#include "xgi/Output.h"

#include "xdata/InfoSpace.h"

gem::base::GEMMonitor::GEMMonitor(log4cplus::Logger& logger, xdaq::ApplicationStub* stub, int const& index) : 
  m_gemLogger(logger)
{
  
}

gem::base::GEMMonitor::GEMMonitor(log4cplus::Logger& logger, GEMApplication* gemApp, int const& index) : 
  m_gemLogger(logger)
{
  addInfoSpace("Application",  gemApp->getAppISToolBox()); // update on state changes
  addInfoSpace("Configuration",gemApp->getCfgISToolBox()); // update on changes to parameters
  addInfoSpace("Monitoring",   gemApp->getMonISToolBox()); // update with interval try {
  try {
    DEBUG("Creating timer with name " << gemApp->m_urn << ":MonitoringTimer%d");
    m_timer = toolbox::task::getTimerFactory()->createTimer(toolbox::toString("%s:MonitoringTimer%d",
                                                                              gemApp->m_urn,
                                                                              index));
    m_timer->stop();
  } catch (toolbox::task::exception::Exception& te) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Cannot run GEMMonitor, timer already created", te);
  }
  
}

gem::base::GEMMonitor::GEMMonitor(log4cplus::Logger& logger, GEMFSMApplication* gemFSMApp, int const& index) : 
  m_gemLogger(logger)
{
  addInfoSpace("Application",  gemFSMApp->getAppISToolBox()); // update on state changes
  addInfoSpace("Configuration",gemFSMApp->getCfgISToolBox()); // update on changes to parameters
  addInfoSpace("Monitoring",   gemFSMApp->getMonISToolBox()); // update with interval
  addInfoSpace("AppState",     gemFSMApp->getAppStateISToolBox()); // update with interval for state changes
  try {
    DEBUG("Creating timer with name " << gemFSMApp->m_urn << ":MonitoringTimer%d");
    m_timer = toolbox::task::getTimerFactory()->createTimer(toolbox::toString("%s:MonitoringTimer%d",
                                                                              gemFSMApp->m_urn,
                                                                              index));
    m_timer->stop();
  } catch (toolbox::task::exception::Exception& te) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Cannot run GEMMonitor, timer already created", te);
  }
  
}

gem::base::GEMMonitor::~GEMMonitor()
{
  
}

void gem::base::GEMMonitor::startMonitoring()
{
  DEBUG("GEMMonitor::startMonitoring");
  
  m_timer->start();
  
  DEBUG("GEMMonitor::startMonitoring");
  for (auto infoSpace = m_infoSpaceMap.begin(); infoSpace != m_infoSpaceMap.end(); ++infoSpace) {
    toolbox::TimeInterval interval;
    interval.fromString("00:00:00:00:02");
    toolbox::TimeVal startTime;
    startTime = toolbox::TimeVal::gettimeofday();
    m_timer->scheduleAtFixedRate(startTime, this, infoSpace->second.second,
                                 infoSpace->second.first->getInfoSpace(),
                                 infoSpace->first);
  }
  
  updateMonitorables();
}

void gem::base::GEMMonitor::timeExpired(toolbox::task::TimerEvent& event)
{
  DEBUG("GEMMonitor::timeExpired received event:" << event.type());
  updateMonitorables();
}

void gem::base::GEMMonitor::addInfoSpace(std::string const& name,
                                         std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> infoSpace,
                                         toolbox::TimeInterval const& interval)
{
  // should we key by name or infoSpace->name(), such that the infoSpace could be retrieved from the infoSpaceFactory
  //std::unordered_map<std::string, gem::base::utils::GEMInfoSpaceToolBox*>::const_iterator it = m_infoSpaceMap.find(infoSpace->name());
  std::unordered_map<std::string,
    std::pair<std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox>,
    toolbox::TimeInterval> >::const_iterator it = m_infoSpaceMap.find(name);
  if (it != m_infoSpaceMap.end()) {
    // WARN( "GEMMonitor::addInfoSpace infospace " << infoSpace->name() << " already exists in monitor!" );
    WARN( "GEMMonitor::addInfoSpace infospace " << name << " already exists in monitor!" );
    return;
  }
  //DEBUG( "GEMMonitor::addInfoSpace adding infospace " << infoSpace->name() );
  //m_infoSpaceMap.insert(std::make_pair<std::string, gem::base::utils::GEMInfoSpaceToolBox*>(infoSpace->name(),infoSpace));
  DEBUG( "GEMMonitor::addInfoSpace adding infospace " << name );
  m_infoSpaceMap.insert(std::make_pair(name, std::make_pair(infoSpace, interval)));
  std::list<std::string> emptyList;
  m_monitorableSetInfoSpaceMap.insert(std::make_pair(name,emptyList));
}

void gem::base::GEMMonitor::addMonitorableSet(std::string const& setname, std::string const& infoSpaceName)
{
  std::unordered_map<std::string,
    std::list<std::pair<std::string, GEMMonitorable> > >::const_iterator it;
  it = m_monitorableSetsMap.find(setname);
  if (it != m_monitorableSetsMap.end()) {
    WARN( "GEMMonitor::addMonitorableSet monitorable set " << setname << " already exists in monitor!" );
    return;
  }
  std::list<std::pair<std::string, GEMMonitorable> > emptySet;
  m_monitorableSetsMap.insert(std::make_pair<std::string const&,
                              std::list<std::pair<std::string, GEMMonitorable> > >(setname, emptySet));
  
  m_monitorableSetInfoSpaceMap.find(infoSpaceName)->second.push_back(setname);
}

void gem::base::GEMMonitor::addMonitorable(std::string const& setname,
                                           std::string const& infoSpaceName,
                                           std::pair<std::string, std::string> const& monpair,
                                           gem::base::utils::GEMInfoSpaceToolBox::UpdateType type,
                                           std::string const& format)
{
  if (m_infoSpaceMap.find(infoSpaceName) == m_infoSpaceMap.end()) {
    ERROR( "GEMMonitor::addMonitorable infoSpace " << infoSpaceName << " does not exist in monitor!" );
    return;
  } else if (m_monitorableSetsMap.find(setname) == m_monitorableSetsMap.end()) {
    ERROR( "GEMMonitor::addMonitorable monitorable set " << setname << " does not exist in monitor!" );
    return;
  }
  
  std::shared_ptr<utils::GEMInfoSpaceToolBox> infoSpace = m_infoSpaceMap.find(infoSpaceName)->second.first;
  if (infoSpace->find(monpair.first)) {
    std::unordered_map<std::string,
      std::list<std::pair<std::string, GEMMonitorable> > >::iterator it;
    it = m_monitorableSetsMap.find(setname);
    GEMMonitorable monitem = {monpair.first, monpair.second, infoSpace, type, format};
    (*it).second.push_back(std::make_pair(monpair.first, monitem));
  } else {
    ERROR( "GEMMonitor::addMonitorablemonitorable " << monpair.first << " does not exist in infospace "
           << infoSpaceName << "!" );
    return;
  }
  
  addInfoSpace(setname, infoSpace);
}

