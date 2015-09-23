#include "gem/utils/GEMInfoSpaceToolBox.h"

#include "gem/utils/exception/Exception.h"
//#include "log4cplus/logger.h"
//#include "log4cplus/loggingmacros.h"
//#include "gem/utils/GEMLogging.h"

#include "toolbox/string.h"

//how to template this?
std::string gem::utils::GEMInfoSpaceToolBox::getString(xdata::InfoSpace* infoSpace,std::string const& itemName)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::String* res = dynamic_cast<xdata::String*>(s);
    infoSpace->unlock();
    return res->value_;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace String item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return "";
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::getBool(xdata::InfoSpace* infoSpace,std::string const& itemName)
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
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

double gem::utils::GEMInfoSpaceToolBox::getDouble(xdata::InfoSpace* infoSpace,std::string const& itemName)
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
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

int gem::utils::GEMInfoSpaceToolBox::getInteger(xdata::InfoSpace* infoSpace,std::string const& itemName)
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
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

uint32_t gem::utils::GEMInfoSpaceToolBox::getUInt32(xdata::InfoSpace* infoSpace,std::string const& itemName)
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
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

uint64_t gem::utils::GEMInfoSpaceToolBox::getUInt64(xdata::InfoSpace* infoSpace,std::string const& itemName)
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
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return 0;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setString(xdata::InfoSpace* infoSpace,std::string const& itemName, std::string const& value)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::String* res = dynamic_cast<xdata::String*>(s);
    *res = value;
    infoSpace->fireItemValueChanged(itemName);
    infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace String item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setBool(xdata::InfoSpace* infoSpace,std::string const& itemName, bool const& value)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::Boolean* res = dynamic_cast<xdata::Boolean*>(s);
    *res = value;
    infoSpace->fireItemValueChanged(itemName);
    infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Boolean item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setDouble(xdata::InfoSpace* infoSpace,std::string const& itemName, double const& value)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::Double* res = dynamic_cast<xdata::Double*>(s);
    *res = value;
    infoSpace->fireItemValueChanged(itemName);
    infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Double item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setInteger(xdata::InfoSpace* infoSpace,std::string const& itemName, int const& value)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::Integer* res = dynamic_cast<xdata::Integer*>(s);
    *res = value;
    infoSpace->fireItemValueChanged(itemName);
    infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace Integer item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setUInt32(xdata::InfoSpace* infoSpace,std::string const& itemName, uint32_t const& value)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::UnsignedInteger32* res = dynamic_cast<xdata::UnsignedInteger32*>(s);
    *res = value;
    infoSpace->fireItemValueChanged(itemName);
    infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace UnsignedInteger32 item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

bool gem::utils::GEMInfoSpaceToolBox::setUInt64(xdata::InfoSpace* infoSpace,std::string const& itemName, uint64_t const& value)
{
  try {
    infoSpace->lock();
    xdata::Serializable* s = infoSpace->find(itemName);
    infoSpace->fireItemValueRetrieve(itemName);
    xdata::UnsignedInteger64* res = dynamic_cast<xdata::UnsignedInteger64*>(s);
    *res = value;
    infoSpace->fireItemValueChanged(itemName);
    infoSpace->unlock();
    return true;
  } catch (...) {
    std::string msg = "Trying to read a non-existent InfoSpace UnsignedInteger64 item '" + itemName + "'.";
    XCEPT_RAISE(gem::utils::exception::InfoSpaceProblem, msg);
    return false;
  } 
}

