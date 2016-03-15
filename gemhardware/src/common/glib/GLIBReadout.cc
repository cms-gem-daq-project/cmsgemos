/**
 * class: GLIBReadout
 * description: Application to handle reading out from FIFO on GLIB
 *              structure borrowed from HCAL
 * author: J. Sturdy
 * date: 14/03/2016
 */

#include <gem/hw/glib/GLIBReadout.h>

XDAQ_INSTANTIATOR_IMPL(gem::hw::glib::GLIBReadout);

gem::hw::glib::GLIBReadout::GLIBReadout(xdaq::ApplicationStub* stub, ) :
  GEMReadoutApplication(stub)
{
  xoap::bind(this,&GLIBReadout::updateScanParameters,"UpdateScanParameter","urn:GLIBReadout-soap:1");

}

xoap::MessageReference gem::hw::glib::GLIBReadout::updateScanParameters(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  INFO("GLIBReadout::updateScanParameters()");
  if (msg.isNull()) {
    XCEPT_RAISE(xoap::exception::Exception,"Null message received!");
  }
  
  std::string commandName    = "undefined";
  std::string parameterValue = "-1";
  try {
    std::pair<std::string, std::string> command
      = gem::utils::soap::GEMSOAPToolBox::extractCommandWithParameter(msg);
    commandName = command.first;
    parameterValue = command.second;
    INFO("GLIBReadout received command " << commandName);
  } catch(xoap::exception::Exception& err) {
    std::string msgBase = toolbox::toString("Unable to extract command from CommandWithParameter SOAP message");
    ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception_history(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::readout::exception::SOAPCommandParameterProblem, top,
                         toolbox::toString("%s.", msgBase.c_str()), err);
    //p_gemApp->notifyQualified("error", top);
    std::string faultString = toolbox::toString("%s failed", commandName.c_str());
    std::string faultCode   = "Client";
    std::string detail      = toolbox::toString("%s: %s.",
                                                msgBase.c_str(),
                                                err.message().c_str());
    //this has to change to something real, but will come when data parker becomes the gem readout application
    std::string faultActor = "";
    xoap::MessageReference reply =
      gem::utils::soap::GEMSOAPToolBox::makeSOAPFaultReply(faultString, faultCode, detail, faultActor);
    return reply;
  }
  //this has to be injected into the GEM header
  m_runParams = std::stoi(parameterValue);
  DEBUG(toolbox::toString("GLIBReadout::updateScanParameters() received command '%s' with value. %s",
                          commandName.c_str(), parameterValue.c_str()));
  return gem::utils::soap::GEMSOAPToolBox::makeFSMSOAPReply(commandName, "ParametersUpdated");
}
