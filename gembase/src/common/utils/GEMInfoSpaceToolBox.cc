#include "gem/base/utils/GEMInfoSpaceToolBox.h"
#include "gem/base/GEMApplication.h"
//#include "gem/base/GEMMonitor.h"

#include "gem/base/utils/exception/Exception.h"
#include "gem/utils/GEMLogging.h"

#include "xdaq/ApplicationStub.h"

#include "toolbox/string.h"

gem::base::utils::GEMInfoSpaceToolBox::GEMInfoSpaceToolBox(gem::base::GEMApplication* gemApp,
                                                           xdata::InfoSpace* infoSpace,
                                                           //gem::base::GEMMonitor* gemMonitor,
                                                           bool autoPush) :
  m_gemLogger(gemApp->getApplicationLogger()),
  p_gemApp(gemApp),
  //p_gemMonitor(gemMonitor),
  p_infoSpace(infoSpace)
{
  INFO("Created GEMInfoSpaceToolBox from application " << gemApp->getApplicationDescriptor()->getClassName() <<
       " and existing InfoSpace " << infoSpace->name());
}

gem::base::utils::GEMInfoSpaceToolBox::GEMInfoSpaceToolBox(gem::base::GEMApplication* gemApp,
                                                           std::string const& infoSpaceName,
                                                           //gem::base::GEMMonitor* gemMonitor,
                                                           bool autoPush) :
  m_gemLogger(gemApp->getApplicationLogger()),
  p_gemApp(gemApp)
  //p_gemMonitor),
{
  if (xdata::getInfoSpaceFactory()->hasItem(infoSpaceName)) {
    p_infoSpace = xdata::getInfoSpaceFactory()->get(infoSpaceName);
    INFO("Created GEMInfoSpaceToolBox from application " << gemApp->getApplicationDescriptor()->getClassName() <<
         " and existing InfoSpace " << infoSpaceName);
  } else {
    p_infoSpace = xdata::getInfoSpaceFactory()->create(infoSpaceName);
    INFO("Created GEMInfoSpaceToolBox from application " << gemApp->getApplicationDescriptor()->getClassName() <<
         " and new InfoSpace " << infoSpaceName);
  }
}

gem::base::utils::GEMInfoSpaceToolBox::~GEMInfoSpaceToolBox()
{

}

//how to template this?
bool gem::base::utils::GEMInfoSpaceToolBox::createString(std::string const& itemName, std::string const& value, UpdateType type, std::string const& docstring)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(STRING, type, itemName, docstring);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::String *ptr = new xdata::String(value);
    m_stringItems.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created string " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace String item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace String item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace String item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createBool(std::string const& itemName, bool const& value, UpdateType type, std::string const& docstring)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(BOOL, type, itemName, docstring);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::Boolean *ptr = new xdata::Boolean(value);
    m_boolItems.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created bool " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace Boolean item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace Boolean item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Boolean item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createDouble(std::string const& itemName, double const& value, UpdateType type, std::string const& docstring)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(DOUBLE, type, itemName, docstring);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::Double *ptr = new xdata::Double(value);
    m_doubleItems.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created double " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace Double item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace Double item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Double item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createInteger(std::string const& itemName, int const& value, UpdateType type, std::string const& docstring)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(INTEGER, type, itemName, docstring);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::Integer *ptr = new xdata::Integer(value);
    m_intItems.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created int " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace Integer item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace Integer item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Integer item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createUInt32(std::string const& itemName, uint32_t const& value, UpdateType type, std::string const& docstring)
{
  INFO("Trying to create uint32_t " << itemName << " in infoSpace " << p_infoSpace->name() << std::hex << p_infoSpace->hasItem(itemName));
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR(err);
      return false;
    }

    INFO("Creating GEMInfoSpaceItem");
    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(UINT32, type, itemName, docstring);
    m_itemMap.insert(std::make_pair(itemName, item));

    INFO("Creating xdata::UnsignedInteger32 *ptr");
    xdata::UnsignedInteger32 *ptr = new xdata::UnsignedInteger32(value);
    INFO("Inserting into map");
    m_uint32Items.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    INFO("Locking infoSpace");
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created uint32_t " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createUInt64(std::string const& itemName, uint64_t const& value, UpdateType type, std::string const& docstring)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR(err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(UINT64, type, itemName, docstring);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::UnsignedInteger64 *ptr = new xdata::UnsignedInteger64(value);
    m_uint64Items.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    INFO("Created uint64_t " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    ERROR(msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    ERROR(msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

std::string gem::base::utils::GEMInfoSpaceToolBox::getString(std::string const& itemName)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return "";
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::getBool(std::string const& itemName)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

double gem::base::utils::GEMInfoSpaceToolBox::getDouble(std::string const& itemName)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

int gem::base::utils::GEMInfoSpaceToolBox::getInteger(std::string const& itemName)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

uint32_t gem::base::utils::GEMInfoSpaceToolBox::getUInt32(std::string const& itemName)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

uint64_t gem::base::utils::GEMInfoSpaceToolBox::getUInt64(std::string const& itemName)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::setString(std::string const& itemName, std::string const& value)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::setBool(std::string const& itemName, bool const& value)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::setDouble(std::string const& itemName, double const& value)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::setInteger(std::string const& itemName, int const& value)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::setUInt32(std::string const& itemName, uint32_t const& value)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::setUInt64(std::string const& itemName, uint64_t const& value)
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
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

//////// static methods
std::string gem::base::utils::GEMInfoSpaceToolBox::getString(xdata::InfoSpace* infoSpace, std::string const& itemName)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::String* res = dynamic_cast<xdata::String*>(s);
    std::cout << "INFO found string " << res->toString() << std::endl;
    infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace String item '" + itemName + "'.";
    std::cout << "ERROR " << msg << std::endl;
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return "";
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::getBool(xdata::InfoSpace* infoSpace, std::string const& itemName)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::Boolean* res = dynamic_cast<xdata::Boolean*>(s);
    infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Boolean item '" + itemName + "'.";
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

double gem::base::utils::GEMInfoSpaceToolBox::getDouble(xdata::InfoSpace* infoSpace, std::string const& itemName)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::Double* res = dynamic_cast<xdata::Double*>(s);
    infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Double item '" + itemName + "'.";
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

int gem::base::utils::GEMInfoSpaceToolBox::getInteger(xdata::InfoSpace* infoSpace, std::string const& itemName)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::Integer* res = dynamic_cast<xdata::Integer*>(s);
    infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Integer item '" + itemName + "'.";
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

uint32_t gem::base::utils::GEMInfoSpaceToolBox::getUInt32(xdata::InfoSpace* infoSpace, std::string const& itemName)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::UnsignedInteger32* res = dynamic_cast<xdata::UnsignedInteger32*>(s);
    infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

uint64_t gem::base::utils::GEMInfoSpaceToolBox::getUInt64(xdata::InfoSpace* infoSpace, std::string const& itemName)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::UnsignedInteger64* res = dynamic_cast<xdata::UnsignedInteger64*>(s);
    infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

