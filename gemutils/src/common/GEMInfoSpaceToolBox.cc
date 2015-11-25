#include "gem/utils/GEMInfoSpaceToolBox.h"

#include "gem/utils/exception/Exception.h"
//#include "log4cplus/logger.h"
//#include "log4cplus/loggingmacros.h"
//#include "gem/utils/GEMLogging.h"

#include "toolbox/string.h"

gem::utils::GEMInfoSpaceToolBox::GEMInfoSpaceToolBox(gem::base::GEMApplication* gemApp,
                                                     xdata::InfoSpace* infoSpace,
                                                     gem::base::GEMMonitor* gemMonitor,
                                                     bool autoPush) :
  p_gemApp(std::make_shared<gem::base::GEMApplication>(gemApp)),
  p_gemMonitor(std::make_shared<gem::base::GEMMonitor>(gemMonitor)),
  p_infoSpace(infoSpace)
{
}
      
gem::utils::GEMInfoSpaceToolBox::GEMInfoSpaceToolBox(gem::base::GEMApplication* gemApp,
                                                     std::string const& infoSpaceName,
                                                     gem::base::GEMMonitor* gemMonitor,
                                                     bool autoPush) :
  p_gemApp(std::make_shared<gem::base::GEMApplication>(gemApp)),
  p_gemMonitor(std::make_shared<gem::base::GEMMonitor>(gemMonitor))
{
}

gem::utils::GEMInfoSpaceToolBox::~GEMInfoSpaceToolBox()
{

}

//how to template this?
bool gem::utils::GEMInfoSpaceToolBox::createString(std::string const& itemName, std::string const& value, UpdateType type)
{
  try {
    if (p_infoSpace->find(itemName)) {
      std::string err = "An element with the name \"" + name + "\" exists already in this infospace.";
      XCEPT_DECLARE( gem::utils::exception::InfoSpaceProblem, top, err );
      p_gemApp->notifyQualified( "fatal", top );
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(itemName, STRING, type, "");
    m_itemMap.insert(std::make_pair<std::string, GEMInfoSpaceItem&>(itemName, *item));

    xdata::String *ptr = new xdata::String(value);
    m_stringItems.insert(std::make_pair<std::string, std::pair<std::string, xdata::String*> >
                         (itemName, std::make_pair<std::string, xdata::String*> (value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemValueAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created string " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace String item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::createBool(std::string const& itemName, bool const& value, UpdateType type)
{
  try {
    if (p_infoSpace->find(itemName)) {
      std::string err = "An element with the name \"" + name + "\" exists already in this infospace.";
      XCEPT_DECLARE( gem::utils::exception::InfoSpaceProblem, top, err );
      p_gemApp->notifyQualified( "fatal", top );
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(itemName, BOOL, type, "");
    m_itemMap.insert(std::make_pair<std::string, GEMInfoSpaceItem&>(itemName, *item));

    xdata::Boolean *ptr = new xdata::Boolean(value);
    m_boolItems.insert(std::make_pair<std::string, std::pair<bool, xdata::Boolean*> >
                         (itemName, std::make_pair<bool, xdata::Boolean*> (value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemValueAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created bool " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Boolean item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::createDouble(std::string const& itemName, double const& value, UpdateType type)
{
  try {
    if (p_infoSpace->find(itemName)) {
      std::string err = "An element with the name \"" + name + "\" exists already in this infospace.";
      XCEPT_DECLARE( gem::utils::exception::InfoSpaceProblem, top, err );
      p_gemApp->notifyQualified( "fatal", top );
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(itemName, DOUBLE, type, "");
    m_itemMap.insert(std::make_pair<std::string, GEMInfoSpaceItem&>(itemName, *item));

    xdata::Double *ptr = new xdata::Double(value);
    m_doubleItems.insert(std::make_pair<std::string, std::pair<double, xdata::Double*> >
                         (itemName, std::make_pair<double, xdata::Double*> (value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemValueAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created double " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Double item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::createInteger(std::string const& itemName, int const& value, UpdateType type)
{
  try {
    if (p_infoSpace->find(itemName)) {
      std::string err = "An element with the name \"" + name + "\" exists already in this infospace.";
      XCEPT_DECLARE( gem::utils::exception::InfoSpaceProblem, top, err );
      p_gemApp->notifyQualified( "fatal", top );
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(itemName, INT, type, "");
    m_itemMap.insert(std::make_pair<std::string, GEMInfoSpaceItem&>(itemName, *item));

    xdata::Integer *ptr = new xdata::Integer(value);
    m_intItems.insert(std::make_pair<std::string, std::pair<int, xdata::Integer*> >
                         (itemName, std::make_pair<int, xdata::Integer*> (value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemValueAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created int " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Integer item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::createUInt32(std::string const& itemName, uint32_t const& value, UpdateType type)
{
  try {
    if (p_infoSpace->find(itemName)) {
      std::string err = "An element with the name \"" + name + "\" exists already in this infospace.";
      XCEPT_DECLARE( gem::utils::exception::InfoSpaceProblem, top, err );
      p_gemApp->notifyQualified( "fatal", top );
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(itemName, UINT32, type, "");
    m_itemMap.insert(std::make_pair<std::string, GEMInfoSpaceItem&>(itemName, *item));

    xdata::UnsignedInteger32 *ptr = new xdata::UnsignedInteger32(value);
    m_uint32Items.insert(std::make_pair<std::string, std::pair<uint32_t, xdata::UnsignedInteger32*> >
                         (itemName, std::make_pair<uint32_t, xdata::UnsignedInteger32*> (value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemValueAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created uint32_t " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::createUInt64(std::string const& itemName, uint64_t const& value, UpdateType type)
{
  try {
    if (p_infoSpace->find(itemName)) {
      std::string err = "An element with the name \"" + name + "\" exists already in this infospace.";
      XCEPT_DECLARE( gem::utils::exception::InfoSpaceProblem, top, err );
      p_gemApp->notifyQualified( "fatal", top );
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(itemName, UINT64, type, "");
    m_itemMap.insert(std::make_pair<std::string, GEMInfoSpaceItem&>(itemName, *item));

    xdata::UnsignedInteger64 *ptr = new xdata::UnsignedInteger64(value);
    m_uint64Items.insert(std::make_pair<std::string, std::pair<uint64_t, xdata::UnsignedInteger64*> >
                         (itemName, std::make_pair<uint64_t, xdata::UnsignedInteger64*> (value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemValueAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created uint64_t " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

std::string gem::utils::GEMInfoSpaceToolBox::getString(std::string const& itemName)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::String* res = dynamic_cast<xdata::String*>(s);
    std::cout << "INFO found string " << res->toString() << std::endl;
    p_infoSpace->unlock();
      return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace String item '" + itemName + "'.";
    std::cout << "ERROR " << msg << std::endl;
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return "";
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::getBool(std::string const& itemName)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::Boolean* res = dynamic_cast<xdata::Boolean*>(s);
    p_infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Boolean item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

double gem::utils::GEMInfoSpaceToolBox::getDouble(std::string const& itemName)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::Double* res = dynamic_cast<xdata::Double*>(s);
    p_infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Double item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

int gem::utils::GEMInfoSpaceToolBox::getInteger(std::string const& itemName)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::Integer* res = dynamic_cast<xdata::Integer*>(s);
    p_infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Integer item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

uint32_t gem::utils::GEMInfoSpaceToolBox::getUInt32(std::string const& itemName)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::UnsignedInteger32* res = dynamic_cast<xdata::UnsignedInteger32*>(s);
    p_infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

uint64_t gem::utils::GEMInfoSpaceToolBox::getUInt64(std::string const& itemName)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::UnsignedInteger64* res = dynamic_cast<xdata::UnsignedInteger64*>(s);
    p_infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setString(std::string const& itemName, std::string const& value)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::String* res = dynamic_cast<xdata::String*>(s);
    *res = value;
    p_infoSpace->fireItemValueChanged(itemName);
    std::cout << "INFO set value to string " << res->toString() << std::endl;
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace String item '" + itemName + "'.";
    std::cout << "ERROR " << msg << std::endl;
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setBool(std::string const& itemName, bool const& value)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::Boolean* res = dynamic_cast<xdata::Boolean*>(s);
    *res = value;
    p_infoSpace->fireItemValueChanged(itemName);
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace Boolean item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setDouble(std::string const& itemName, double const& value)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::Double* res = dynamic_cast<xdata::Double*>(s);
    *res = value;
    p_infoSpace->fireItemValueChanged(itemName);
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace Double item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setInteger(std::string const& itemName, int const& value)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::Integer* res = dynamic_cast<xdata::Integer*>(s);
    *res = value;
    p_infoSpace->fireItemValueChanged(itemName);
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace Integer item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setUInt32(std::string const& itemName, uint32_t const& value)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::UnsignedInteger32* res = dynamic_cast<xdata::UnsignedInteger32*>(s);
    *res = value;
    p_infoSpace->fireItemValueChanged(itemName);
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setUInt64(std::string const& itemName, uint64_t const& value)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::UnsignedInteger64* res = dynamic_cast<xdata::UnsignedInteger64*>(s);
    *res = value;
    p_infoSpace->fireItemValueChanged(itemName);
    p_infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

