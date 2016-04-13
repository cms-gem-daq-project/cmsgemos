#include "gem/base/utils/GEMInfoSpaceToolBox.h"
#include "gem/base/GEMApplication.h"
//#include "gem/base/GEMMonitor.h"

#include "gem/base/utils/exception/Exception.h"
#include "gem/utils/GEMLogging.h"
#include "gem/utils/GEMRegisterUtils.h"

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
  INFO("GEMInfoSpaceToolBox::Created GEMInfoSpaceToolBox from application " 
       << gemApp->getApplicationDescriptor()->getClassName()
       << " and existing InfoSpace " << infoSpace->name());
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
    INFO("GEMInfoSpaceToolBox::Created GEMInfoSpaceToolBox from application "
         << gemApp->getApplicationDescriptor()->getClassName()
         << " and existing InfoSpace " << infoSpaceName);
  } else {
    p_infoSpace = xdata::getInfoSpaceFactory()->create(infoSpaceName);
    INFO("GEMInfoSpaceToolBox::Created GEMInfoSpaceToolBox from application "
         << gemApp->getApplicationDescriptor()->getClassName()
         << " and new InfoSpace " << infoSpaceName);
  }
}

gem::base::utils::GEMInfoSpaceToolBox::~GEMInfoSpaceToolBox()
{

}

//how to template this?
bool gem::base::utils::GEMInfoSpaceToolBox::createString(std::string const& itemName, std::string const& value, UpdateType type, std::string const& docstring, std::string const& format)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR("GEMInfoSpaceToolBox::" << err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(STRING, type, itemName, docstring, format);
    m_itemMap.insert(std::make_pair(itemName, item));
    
    xdata::String *ptr = new xdata::String(value);
    m_stringItems.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::Created string " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace String item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace String item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace String item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createBool(std::string const& itemName, bool const& value, UpdateType type, std::string const& docstring, std::string const& format)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR("GEMInfoSpaceToolBox::" << err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(BOOL, type, itemName, docstring, format);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::Boolean *ptr = new xdata::Boolean(value);
    m_boolItems.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::Created bool " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace Boolean item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace Boolean item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Boolean item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createDouble(std::string const& itemName, double const& value, UpdateType type, std::string const& docstring, std::string const& format)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR("GEMInfoSpaceToolBox::" << err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(DOUBLE, type, itemName, docstring, format);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::Double *ptr = new xdata::Double(value);
    m_doubleItems.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::Created double " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace Double item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace Double item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Double item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createInteger(std::string const& itemName, int const& value, UpdateType type, std::string const& docstring, std::string const& format)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR("GEMInfoSpaceToolBox::" << err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(INTEGER, type, itemName, docstring, format);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::Integer *ptr = new xdata::Integer(value);
    m_intItems.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::Created int " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace Integer item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace Integer item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Integer item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createInteger32(std::string const& itemName, int32_t const& value, UpdateType type, std::string const& docstring, std::string const& format)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR("GEMInfoSpaceToolBox::" << err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(INTEGER32, type, itemName, docstring, format);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::Integer32 *ptr = new xdata::Integer32(value);
    m_int32Items.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::Created int32_t " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace Integer32 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace Integer32 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Integer32 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createInteger64(std::string const& itemName, int64_t const& value, UpdateType type, std::string const& docstring, std::string const& format)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR("GEMInfoSpaceToolBox::" << err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(INTEGER64, type, itemName, docstring, format);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::Integer64 *ptr = new xdata::Integer64(value);
    m_int64Items.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::Created int64_t " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace Integer64 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace Integer64 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace Integer64 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createUInt32(std::string const& itemName, uint32_t const& value, UpdateType type, std::string const& docstring, std::string const& format)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR("GEMInfoSpaceToolBox::" << err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(UINT32, type, itemName, docstring, format);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::UnsignedInteger32 *ptr = new xdata::UnsignedInteger32(value);
    m_uint32Items.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::Created uint32_t " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::createUInt64(std::string const& itemName, uint64_t const& value, UpdateType type, std::string const& docstring, std::string const& format)
{
  try {
    if (p_infoSpace->hasItem(itemName)) {
      std::string err = "An element with the name '" + itemName + "' exists already in this infospace.";
      XCEPT_DECLARE(gem::base::utils::exception::InfoSpaceProblem, top, err);
      p_gemApp->notifyQualified("fatal", top);
      ERROR("GEMInfoSpaceToolBox::" << err);
      return false;
    }

    GEMInfoSpaceItem* item = new GEMInfoSpaceItem(UINT64, type, itemName, docstring, format);
    m_itemMap.insert(std::make_pair(itemName, item));

    xdata::UnsignedInteger64 *ptr = new xdata::UnsignedInteger64(value);
    m_uint64Items.insert(std::make_pair(itemName, std::make_pair(value, ptr)));
    
    p_infoSpace->lock();
    p_infoSpace->fireItemAvailable(itemName, ptr);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::Created uint64_t " << itemName << " in infoSpace " << p_infoSpace->name());
    p_infoSpace->unlock();
    return true;
  } catch (xdata::exception::Exception const& err) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (std::exception const& err) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg << " " << err.what());
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } catch (...) {
    std::string msg = "Error trying to create InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
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
    DEBUG("GEMInfoSpaceToolBox::found string " << res->toString());
    p_infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace String item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
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
    DEBUG("GEMInfoSpaceToolBox::found bool " << res->value_);
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
    DEBUG("GEMInfoSpaceToolBox::found double " << res->value_);
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
    DEBUG("GEMInfoSpaceToolBox::found integer " << res->value_);
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Integer item '" + itemName + "'.";
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

int32_t gem::base::utils::GEMInfoSpaceToolBox::getInteger32(std::string const& itemName)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::Integer32* res = dynamic_cast<xdata::Integer32*>(s);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::found integer32 " << res->value_);
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Integer32 item '" + itemName + "'.";
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

int64_t gem::base::utils::GEMInfoSpaceToolBox::getInteger64(std::string const& itemName)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::Integer64* res = dynamic_cast<xdata::Integer64*>(s);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::found integer64 " << res->value_);
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Integer64 item '" + itemName + "'.";
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
    DEBUG("GEMInfoSpaceToolBox::found uint32_t " << res->value_);
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
    DEBUG("GEMInfoSpaceToolBox::found uint64_t " << res->value_);
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
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::set value to string " << res->toString());
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace String item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
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
    DEBUG("GEMInfoSpaceToolBox::set value to bool " << res->value_);
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace Boolean item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
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
    DEBUG("GEMInfoSpaceToolBox::set value to double " << res->value_);
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace Double item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
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
    DEBUG("GEMInfoSpaceToolBox::set value to integer " << res->value_);
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace Integer item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::setInteger32(std::string const& itemName, int32_t const& value)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::Integer32* res = dynamic_cast<xdata::Integer32*>(s);
    *res = value;
    p_infoSpace->fireItemValueChanged(itemName);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::set value to integer32 " << res->value_);
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace Integer32 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::base::utils::GEMInfoSpaceToolBox::setInteger64(std::string const& itemName, int64_t const& value)
{
  try {
    p_infoSpace->lock();
    xdata::Serializable* s = p_infoSpace->find(itemName);
    p_infoSpace->fireItemValueRetrieve(itemName);
    xdata::Integer64* res = dynamic_cast<xdata::Integer64*>(s);
    *res = value;
    p_infoSpace->fireItemValueChanged(itemName);
    p_infoSpace->unlock();
    DEBUG("GEMInfoSpaceToolBox::set value to integer64 " << res->value_);
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace Integer64 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
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
    DEBUG("GEMInfoSpaceToolBox::set value to uint32_t " << res->value_);
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
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
    DEBUG("GEMInfoSpaceToolBox::set value to uint64_t " << res->value_);
    return true;
  } catch (...) {
    std::string msg = "Trying to set a non-existent InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    ERROR("GEMInfoSpaceToolBox::" << msg);
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
    infoSpace->unlock();
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_DEBUG(gemLogger,"GEMInfoSpaceToolBox::found string " << res->toString());
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace String item '" + itemName + "'.";
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_ERROR(gemLogger,"GEMInfoSpaceToolBox::" << msg);
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
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_DEBUG(gemLogger,"GEMInfoSpaceToolBox::found bool " << res->value_);
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Boolean item '" + itemName + "'.";
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_ERROR(gemLogger,"GEMInfoSpaceToolBox::" << msg);
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
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_DEBUG(gemLogger,"GEMInfoSpaceToolBox::found double " << res->value_);
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Double item '" + itemName + "'.";
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_ERROR(gemLogger,"GEMInfoSpaceToolBox::" << msg);
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
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_DEBUG(gemLogger,"GEMInfoSpaceToolBox::found integer " << res->value_);
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Integer item '" + itemName + "'.";
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_ERROR(gemLogger,"GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

int32_t gem::base::utils::GEMInfoSpaceToolBox::getInteger32(xdata::InfoSpace* infoSpace, std::string const& itemName)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::Integer32* res = dynamic_cast<xdata::Integer32*>(s);
    infoSpace->unlock();
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_DEBUG(gemLogger,"GEMInfoSpaceToolBox::found integer32 " << res->value_);
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Integer32 item '" + itemName + "'.";
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_ERROR(gemLogger,"GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

int64_t gem::base::utils::GEMInfoSpaceToolBox::getInteger64(xdata::InfoSpace* infoSpace, std::string const& itemName)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::Integer64* res = dynamic_cast<xdata::Integer64*>(s);
    infoSpace->unlock();
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_DEBUG(gemLogger,"GEMInfoSpaceToolBox::found integer64 " << res->value_);
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Integer64 item '" + itemName + "'.";
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_ERROR(gemLogger,"GEMInfoSpaceToolBox::" << msg);
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
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_DEBUG(gemLogger,"GEMInfoSpaceToolBox::found uint32_t " << res->value_);
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_ERROR(gemLogger,"GEMInfoSpaceToolBox::" << msg);
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
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_DEBUG(gemLogger,"GEMInfoSpaceToolBox::found uint64_t " << res->value_);
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    log4cplus::Logger gemLogger(log4cplus::Logger::getInstance(infoSpace->name()));
    LOG4CPLUS_ERROR(gemLogger,"GEMInfoSpaceToolBox::" << msg);
    XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

std::string gem::base::utils::GEMInfoSpaceToolBox::getFormattedItem(std::string const& itemName, std::string const& format)
{
  DEBUG("GEMInfoSpaceToolBox::getFormattedItem(" << itemName << "," << format << ")");
  auto item = m_itemMap.find(itemName);
  if (item == m_itemMap.end()) {
    std::string err = itemName + "' does not exist in this infospace.";
    WARN("GEMInfoSpaceToolBox::" << err);
    return "Item not found";
  }
  std::stringstream result;
  GEMInfoSpaceItem* isItem = item->second;
  DEBUG(itemName << " found in infospace " << std::hex << isItem << std::dec);
  ItemType type = isItem->m_itype;
  
  if ( type == INTEGER ) {   
    int val = this->getInteger(itemName);
    DEBUG(itemName << " has value " << val << std::dec);
    if ( format == "dec" ) {
      result << std::dec << val;
    } else {
      WARN("Invalid format specified for INTEGER type item " << itemName << " formating as simple \"dec\"");
      result << std::dec << val;
    }
  } else if ( type == INTEGER32 ) {   
    int32_t val = this->getInteger32(itemName);
    DEBUG(itemName << " has value " << val << std::dec);
    if ( format == "dec" ) {
      result << std::dec << val;
    } else {
      WARN("Invalid format specified for INTEGER32 type item " << itemName << " formating as simple \"dec\"");
      result << std::dec << val;
    }
  } else if ( type == INTEGER64 ) {   
    int64_t val = this->getInteger64(itemName);
    DEBUG(itemName << " has value " << val << std::dec);
    if ( format == "dec" ) {
      result << std::dec << val;
    } else {
      WARN("Invalid format specified for INTEGER64 type item " << itemName << " formating as simple \"dec\"");
      result << std::dec << val;
    }
  } else if ( type == UINT32 ) {   
    uint32_t val = this->getUInt32(itemName);
    DEBUG(itemName << " has value " << std::hex << val << std::dec);
    if ( format == "" || format == "hex" ) {
      result << "0x" << std::setw(8) << std::setfill('0') << std::hex << val;
    } else if ( format == "dec" ) {
      result << std::dec << val;
    } else if ( format == "hex/dec" ) {
      result << "0x" << std::setw(8) << std::setfill('0') << std::hex
             << val << " / " << std::dec << val;
    } else if ( format == "raw/rate" ) { // for a counter, get the raw count, plus the rate
      result << "0x" << std::setw(8) << std::setfill('0') << std::hex << val;
    } else if ( format == "ip" ) {
      result << std::dec << gem::utils::uint32ToDottedQuad(val);
    } else if ( format == "date" ) {
      result <<         std::setfill('0') << std::setw(2) << (val&0x1f)
             << "-"  << std::setfill('0') << std::setw(2) << ((val>>5)&0x0f)
             << "-"  << std::setw(4) << 2000+((val>>9)&0x7f);
    } else if ( format == "id" ) {
      // expects four 8-bit chars
      result << std::dec << gem::utils::uint32ToString(val);
    } else if ( format == "fwver" ) {
      // expects Major(4).Minor(4).Build(8)
      result << ((val>>12)&0x0f) << "." 
             << ((val>>8) &0x0f) << "."
             << ((val)    &0xff);
    }
  } else if (type == UINT64) { // end of type == UINT32
    uint64_t val = this->getUInt64(itemName);
    DEBUG(itemName << " has value " << std::hex << val << std::dec);
    if ( format == "i2c/dec" ) {
      result << (val&(uint32_t)0xffffffff) << " (str.)" << std::endl
             << (val>>32) << " (ack.) ";
    } else if ( format == "i2c/hex" ) {
      result << "0x" << std::setw(8) << std::setfill('0') 
             << std::hex << (val&(uint32_t)0xffffffff) << " (str.)" << std::endl
             << "0x" << std::setw(8) << std::setfill('0') 
             << (val>>32) << " (ack.) " << std::dec;
    } else if ( format == "" || format == "hex" ) {
      result << "0x" << std::setw(8) << std::setfill('0') << std::hex << val;
    } else if ( format == "dec" ) {
      result << std::dec << val;
    } else if ( format == "hex/dec" ) {
      result << "0x" << std::setw(8) << std::setfill('0') << std::hex
             << val << " / " << std::dec << val;
    } else if ( format == "mac" ) {
      result << gem::utils::uint32ToGroupedHex((val>>32),val&(uint32_t)0xffffffff);
    }
    
  } else if (type == STRING) {  // end of type == UINT64
    std::string val = this->getString(itemName);
    DEBUG(itemName << " has value " << val);
    if ( format == "" ) {
      result << val;
    } else  {
      WARN("GEMInfoSpaceToolBox::Unsupported format " << format);
      result << val;
    }
  } // end of type == STRING
  
  return result.str();
}

std::string gem::base::utils::GEMInfoSpaceToolBox::getItemDocstring(std::string const& itemName)
{
  DEBUG("GEMInfoSpaceToolBox::getItemDocstring(" << itemName << ")");
  auto item = m_itemMap.find(itemName);
  if (item == m_itemMap.end()) {
    std::string err = itemName + "' does not exist in this infospace.";
    WARN("GEMInfoSpaceToolBox::" << err);
    return "Item not found";
  }
  std::stringstream result;
  GEMInfoSpaceItem* isItem = item->second;
  DEBUG(itemName << " found in infospace " << std::hex << isItem << std::dec);
  return isItem->m_docstring;
}


void gem::base::utils::GEMInfoSpaceToolBox::reset()
{
  // simply empty the itemMap, so it can be repopulated, or also revoke items in the infospace?
  for (auto item = m_itemMap.begin(); item != m_itemMap.end(); ++item) {
    p_infoSpace->lock();
    p_infoSpace->fireItemRevoked(item->first);
    p_infoSpace->unlock();
  }
  // now clear out the map so it could be repopulated
  m_itemMap.clear();
  
  // simply empty the item lists?
  m_uint32Items.clear();
  m_uint64Items.clear();
  m_boolItems.clear();
  m_intItems.clear();
  m_int32Items.clear();
  m_int64Items.clear();
  m_doubleItems.clear();
  m_stringItems.clear();

}
