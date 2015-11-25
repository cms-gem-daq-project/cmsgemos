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
  addInfoSpace("Application",gemApp->getApplicationInfoSpace());
}

gem::base::GEMMonitor::GEMMonitor(log4cplus::Logger& logger, GEMFSMApplication* gemFSMApp) : 
  m_gemLogger(logger)
{
  addInfoSpace("Application",gemFSMApp->getApplicationInfoSpace());
  
}

gem::base::GEMMonitor::~GEMMonitor()
{
  
}

void gem::base::GEMMonitor::timeExpired(toolbox::task::TimerEvent&)
{
  updateMonitorables();
}

void gem::base::GEMMonitor::addInfoSpace(std::string const& name, xdata::InfoSpace* infoSpace)
{
  // should we key by name or infoSpace->name(), such that the infoSpace could be retrieved from the infoSpaceFactory
  std::unordered_map<std::string, xdata::InfoSpace*>::const_iterator it = m_infoSpaceMap.find(infoSpace->name());
  // std::unordered_map<std::string, xdata::InfoSpace*>::const_iterator it = m_infoSpaceMap.find(name);
  if (it != m_infoSpaceMap.end()) {
    // ERROR( "GEMMonitor: infospace " << infoSpace->name() << " already exists in monitor!" );
    ERROR( "GEMMonitor: infospace " << name << " already exists in monitor!" );
    return;
  }
  DEBUG( "GEMMonitor: adding infospace " << infoSpace->name() );
  m_infoSpaceMap.insert(std::make_pair<std::string, xdata::InfoSpace*>(infoSpace->name(),infoSpace));
  // DEBUG( "GEMMonitor: adding infospace " << name );
  // m_infoSpaceMap.insert(std::make_pair<std::string, xdata::InfoSpace*>(name,infoSpace));
}

void gem::base::GEMMonitor::addMonitorableSet(std::string const& setname, xdata::InfoSpace* infoSpace)
{
  std::unordered_map<std::string,
    std::list<std::pair<std::string, GEMMonitorable> > >::const_iterator it;
  it = m_monitorableSetsMap.find(setname);
  if (it != m_monitorableSetsMap.end()) {
    ERROR( "GEMMonitor: monitorable set " << setname << " already exists in monitor!" );
    return;
  }
  std::list<std::pair<std::string, GEMMonitorable> > emptySet;
  m_monitorableSetsMap.insert(std::make_pair<std::string const&,
                              std::list<std::pair<std::string, GEMMonitorable> > >(setname, emptySet));
  
}

void gem::base::GEMMonitor::addMonitorable(std::string const& setname,
                                           xdata::InfoSpace* infoSpace,
                                           std::pair<std::string const&, std::string const&> monpair,
                                           gem::utils::GEMInfoSpaceToolBox::UpdateType type,
                                           std::string const& format)
{
  std::unordered_map<std::string,
    std::list<std::pair<std::string, GEMMonitorable> > >::iterator it;
  it = m_monitorableSetsMap.find(setname);
  if (it == m_monitorableSetsMap.end()) {
    ERROR( "GEMMonitor: monitorable set " << setname << " does not exist in monitor!" );
    return;
  }

  // std::list<std::pair<std::string, GEMMonitorable> >::const_iterator setit = m_monitorableSetsMap.find(monpair.first);
  //if (it->find(monpair.first) != it->end()) {
  //  ERROR( "GEMMonitor: monitorable set " << setname << " does not exist in monitor!" );
  //  return;
  //}

  GEMMonitorable monitem = {monpair.first, monpair.second, infoSpace, type, format};

  it->second.push_back(std::make_pair<std::string const&, GEMMonitorable>(monpair.first, monitem));
  //it->insert(monpair.first,monitem);
  addInfoSpace(setname, infoSpace);
}

