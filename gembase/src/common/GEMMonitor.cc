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
#include "gem/base/GEMWebApplication.h"
#include "gem/base/GEMFSMApplication.h"

#include "xgi/Input.h"
#include "xgi/Output.h"

#include "xdata/InfoSpace.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

gem::base::GEMMonitor::GEMMonitor(log4cplus::Logger& logger, xdaq::Application* xdaqApp, int const& index) :
  m_gemLogger(logger)
{
  std::stringstream timerName;
  timerName << xdaqApp->getApplicationDescriptor()->getURN() << ":MonitoringTimer" << index;
  m_timerName = timerName.str();

  try {
    DEBUG("GEMMonitor::Creating timer with name " << m_timerName);
    if (toolbox::task::getTimerFactory()->hasTimer(m_timerName))
      p_timer = toolbox::task::getTimerFactory()->getTimer(m_timerName);
    else
      p_timer = toolbox::task::getTimerFactory()->createTimer(m_timerName);
    p_timer->stop();
  } catch (toolbox::task::exception::Exception& te) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Cannot run GEMMonitor, timer already created", te);
  }
}

gem::base::GEMMonitor::GEMMonitor(log4cplus::Logger& logger, GEMApplication* gemApp, int const& index) :
  m_gemLogger(logger)
{
  p_gemApp = gemApp;

  // update on state changes
  addInfoSpace("Application",   gemApp->getAppISToolBox(), toolbox::TimeInterval(3,  0));
  // update on changes to parameters
  addInfoSpace("Configuration", gemApp->getCfgISToolBox(), toolbox::TimeInterval(60, 0));
  // update with interval
  addInfoSpace("Monitoring",    gemApp->getMonISToolBox(), toolbox::TimeInterval(7,  0));

  std::stringstream timerName;
  timerName << gemApp->m_urn << ":MonitoringTimer" << index;
  m_timerName = timerName.str();

  try {
    DEBUG("GEMMonitor::Creating timer with name " << m_timerName);
    if (toolbox::task::getTimerFactory()->hasTimer(m_timerName))
      p_timer = toolbox::task::getTimerFactory()->getTimer(m_timerName);
    else
      p_timer = toolbox::task::getTimerFactory()->createTimer(m_timerName);
    p_timer->stop();
  } catch (toolbox::task::exception::Exception& te) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Cannot run GEMMonitor, timer already created", te);
  }
}

gem::base::GEMMonitor::GEMMonitor(log4cplus::Logger& logger, GEMFSMApplication* gemFSMApp, int const& index) :
  m_gemLogger(logger)
{
  p_gemApp = static_cast<gem::base::GEMApplication*>(gemFSMApp);
  // maybe it's really better to use the listener functionality... which we can put into the actionPerformed callback!
  // update on state changes
  addInfoSpace("Application",        gemFSMApp->getAppISToolBox(),      toolbox::TimeInterval(2.5, 0));
  // update on changes to parameters
  addInfoSpace("Configuration",      gemFSMApp->getCfgISToolBox(),      toolbox::TimeInterval(30,  0));
  // update with interval
  addInfoSpace("Monitoring",         gemFSMApp->getMonISToolBox(),      toolbox::TimeInterval(7,   0));
  // update with interval for state changes
  addInfoSpace("AppStateMonitoring", gemFSMApp->getAppStateISToolBox(), toolbox::TimeInterval(2.5, 0));

  std::stringstream timerName;
  timerName << gemFSMApp->m_urn << ":MonitoringTimer" << index;
  m_timerName = timerName.str();

  try {
    DEBUG("GEMMonitor::Creating timer with name " << m_timerName);
    if (toolbox::task::getTimerFactory()->hasTimer(m_timerName))
      p_timer = toolbox::task::getTimerFactory()->getTimer(m_timerName);
    else
      p_timer = toolbox::task::getTimerFactory()->createTimer(m_timerName);
    p_timer->stop();
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

  try {
    p_timer->stop();
  } catch (toolbox::task::exception::NotActive const& ex) {
    WARN("GEMMonitor::startMonitoring could not stop timer " << ex.what());
  }

  p_timer->start();

  DEBUG("GEMMonitor::startMonitoring");
  for (auto infoSpace = m_infoSpaceMap.begin(); infoSpace != m_infoSpaceMap.end(); ++infoSpace) {
    toolbox::TimeVal startTime;
    startTime = toolbox::TimeVal::gettimeofday();
    p_timer->scheduleAtFixedRate(startTime, this, infoSpace->second.second,
                                 infoSpace->second.first->getInfoSpace(),
                                 infoSpace->first);
  }

  updateMonitorables();
}

void gem::base::GEMMonitor::stopMonitoring()
{
  DEBUG("GEMMonitor::stopMonitoring");
  p_timer->stop();
}

void gem::base::GEMMonitor::setupMonitoring(bool isFSMApp)
{
  // create the values to be monitored in the info space
  addMonitorableSet("AppParameters", "Application");
  addMonitorable("AppParameters", "Application",
                 std::make_pair("RunNumber", ""),
                 GEMUpdateType::PROCESS, "");
  addMonitorable("AppParameters", "Application",
                 std::make_pair("RunType", ""),
                 GEMUpdateType::PROCESS, "");
  addMonitorable("AppParameters", "Application",
                 std::make_pair("CfgType", ""),
                 GEMUpdateType::PROCESS, "");
  if (isFSMApp)
    addMonitorable("AppParameters", "Application",
                   std::make_pair("State", ""),
                   GEMUpdateType::PROCESS, "");
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
  std::unordered_map<std::string,
    std::pair<std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox>,
    toolbox::TimeInterval> >::const_iterator it = m_infoSpaceMap.find(name);
  if (it != m_infoSpaceMap.end()) {
    WARN("GEMMonitor::addInfoSpace infospace " << name << " already exists in monitor!");
    return;
  }
  DEBUG("GEMMonitor::addInfoSpace adding infospace " << name);
  m_infoSpaceMap.insert(std::make_pair(name, std::make_pair(infoSpace, interval)));
  std::list<std::string> emptyList;
  m_infoSpaceMonitorableSetMap.insert(std::make_pair(name, emptyList));
}

void gem::base::GEMMonitor::addMonitorableSet(std::string const& setname, std::string const& infoSpaceName)
{
  std::unordered_map<std::string,
    std::list<std::pair<std::string, GEMMonitorable> > >::const_iterator it;
  it = m_monitorableSetsMap.find(setname);
  if (it != m_monitorableSetsMap.end()) {
    WARN("GEMMonitor::addMonitorableSet monitorable set " << setname << " already exists in monitor!");
    return;
  }
  std::list<std::pair<std::string, GEMMonitorable> > emptySet;
  m_monitorableSetsMap.insert(std::make_pair<std::string const&,
                              std::list<std::pair<std::string, GEMMonitorable> > >(setname, emptySet));

  m_infoSpaceMonitorableSetMap.find(infoSpaceName)->second.push_back(setname);

  m_monitorableSetInfoSpaceMap.insert(std::make_pair(setname, infoSpaceName));
}

void gem::base::GEMMonitor::addMonitorable(std::string const& setname,
                                           std::string const& infoSpaceName,
                                           std::pair<std::string, std::string> const& monpair,
                                           gem::base::utils::GEMInfoSpaceToolBox::UpdateType type,
                                           std::string const& format)
{
  if (m_infoSpaceMap.find(infoSpaceName) == m_infoSpaceMap.end()) {
    ERROR("GEMMonitor::addMonitorable infoSpace " << infoSpaceName << " does not exist in monitor!");
    return;
  } else if (m_monitorableSetsMap.find(setname) == m_monitorableSetsMap.end()) {
    ERROR("GEMMonitor::addMonitorable monitorable set " << setname << " does not exist in monitor!");
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
    ERROR("GEMMonitor::addMonitorablemonitorable " << monpair.first << " does not exist in infospace "
           << infoSpaceName << "!");
    return;
  }

  addInfoSpace(setname, infoSpace);
}

std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> gem::base::GEMMonitor::getInfoSpace(std::string const& setname)
{
  std::string infoSpaceName = m_monitorableSetInfoSpaceMap.find(setname)->second;
  auto infoSpacePair = m_infoSpaceMap.find(infoSpaceName)->second;
  auto infoSpace = infoSpacePair.first;
  return infoSpace;
}


std::list<std::vector<std::string> > gem::base::GEMMonitor::getFormattedItemSet(std::string const& setname)
{
  std::list< std::vector<std::string> > result;
  auto itemSet = m_monitorableSetsMap.find(setname);
  if (itemSet == m_monitorableSetsMap.end()) {
    ERROR("GEMMonitor::Set named " << setname << " does not exist in monitor");
    return result;
  }

  std::list<std::pair<std::string, GEMMonitorable> > itemList = itemSet->second;
  for (auto item = itemList.begin(); item != itemList.end(); ++item) {
    auto gemItem = item->second;
    std::vector<std::string> itl;
    auto gemIS = gemItem.infoSpace;
    std::string val = gemIS->getFormattedItem(gemItem.name, gemItem.format);
    std::string doc = gemIS->getItemDocstring(gemItem.name);
    itl.push_back(gemItem.name);
    itl.push_back(val);
    itl.push_back(doc);
    itl.push_back(gemItem.regname);
    result.push_back(itl);
    DEBUG("GEMMonitor::Set named " << setname << " has members"
          << " name: "    << itl.at(0)
          << " val: "     << itl.at(1)
          << " doc: "     << itl.at(2)
          << " regname: " << itl.at(3));
  }
  return result;
}

void gem::base::GEMMonitor::jsonUpdateItemSet(std::string const& setname, std::ostream *out)
{
  std::list< std::vector<std::string> > items = getFormattedItemSet(setname);
  std::list< std::vector<std::string> >::const_iterator it;

  std::list< std::vector<std::string> >::const_iterator end = items.end();
  for ( it = items.begin(); it != items.end(); ++it ) {
    std::string val = gem::base::GEMWebApplication::jsonEscape( (*it)[1] );
    *out << "{ \"name\":\"" << getInfoSpace(setname)->name() << "-" << (*it)[0]
         << "\",\"value\":\"" << val;
    // can't have a trailing comma for the last entry...
    if (std::distance(it, end) == 1) {
      *out << "\" }"  << std::endl;
    } else {
      *out << "\" }," << std::endl;
    }
  }
}

void gem::base::GEMMonitor::jsonUpdateItemSets(xgi::Output *out)
{
  auto end = m_monitorableSetsMap.end();
  for (auto iset = m_monitorableSetsMap.begin(); iset != m_monitorableSetsMap.end(); ++iset) {
    if (m_monitorableSetsMap.find(iset->first) == m_monitorableSetsMap.end()) {
      WARN("GEMMonitor::Monitorable set " << iset->first << " not found, not exporting as JSON");
      return;
    }

    if (m_monitorableSetsMap.find(iset->first)->second.empty()) {
      WARN("GEMMonitor::Monitorable set " << iset->first << " is empty, not exporting as JSON");
      return;
    }
    DEBUG("GEMMonitor::Found monitorable set " << iset->first << " while updating for JSON export");
    *out << "\"" << iset->first << "\" : [ " << std::endl;
    jsonUpdateItemSet(iset->first, out);
    // can't have a trailing comma for the last entry...
    if (std::distance(iset, end) == 1) {
      *out << " ]"  << std::endl;
    } else {
      *out << " ]," << std::endl;
    }
  }
}

void gem::base::GEMMonitor::jsonUpdateInfoSpaces(xgi::Output *out)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  *out << " } " << std::endl;
}

void gem::base::GEMMonitor::reset()
{
  // have to get rid of the timer
  DEBUG("GEMMonitor::reset");
  for (auto infoSpace = m_infoSpaceMap.begin(); infoSpace != m_infoSpaceMap.end(); ++infoSpace) {
    DEBUG("GEMMonitor::reset removing " << infoSpace->first << " from p_timer");
    try {
      p_timer->remove(infoSpace->first);
    } catch (toolbox::task::exception::Exception& te) {
      ERROR("GEMMonitor::Caught exception while removing timer task " << infoSpace->first << " " << te.what());
    }
  }
  stopMonitoring();
  DEBUG("GEMMonitor::reset removing timer " << m_timerName << " from timerFactory");
  try {
    toolbox::task::getTimerFactory()->removeTimer(m_timerName);
  } catch (toolbox::task::exception::Exception& te) {
    ERROR("GEMMonitor::Caught exception while removing timer " << m_timerName << " " << te.what());
  }

  // is this necessary? how to do for some applications and not others?
  // make this simply an interface and force every derived application to implement it properly
  // without this, then the json updates will continue and eventually fail
  /*
  DEBUG("GEMMonitor::reset - clearing all maps");
  m_infoSpaceMap.clear();
  m_infoSpaceMonitorableSetMap.clear();
  m_monitorableSetInfoSpaceMap.clear();
  m_monitorableSetsMap.clear();
  */
}
