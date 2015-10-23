import socket
import os
import ConfigParser

import GEMDAQProcess


class GEMTestbeamDAQConfig:

    def __init__(self,configFile,fullConfig):
        self.xdaqProcesses = []
        self.configFilePath = "/tmp/GEMTestbeamDAQ_"+str(os.getpid())+".xml"
        self._config = ConfigParser.ConfigParser()
        self._config.optionxform=str
        self._config.read(configFile)
        self._fullConfig = fullConfig

        with open("/proc/sys/net/ipv4/ip_local_port_range") as portRange:
            self._validPortsMin,self._validPortsMax = [int(x) for x in portRange.readline().split()]

        enabledChips  = self.getEnabledChips()
        dataSource = self.getDataSource()
        writeData  = self.getWriteData()
        runNumber  = self.getRunNumber()
        xdaqPort   = self.getXdaqPort()

        with open(configFile,'wb') as configFile:
            self._config.write(configFile)

        self._xmlConfiguration = self.getConfiguration(runNumber,dataSource,fedId,xdaqPort,ferolSourceIP,ferolDestIP,ferolDestPort,writeData,useDummyFerol)

        with open(self.configFilePath,'wb') as xmlFile:
            xmlFile.write(self._xmlConfiguration)


    def __del__(self):
        os.remove(self.configFilePath)


    def getGEMGLIBSupervisorWebContext(self,xdaqHost,xdaqPort,enabledChips):
        xdaqProcess = GEMDAQProcess.GEMDAQProcess(xdaqHost,xdaqPort)
        xdaqProcess.addApplication("gem::supervisor::GEMGLIBSupervisorWeb",0)
        self.xdaqProcesses.append(xdaqProcess)
        chipLines = ""
        for slot in enabledChips:
            chipLines +="""	    <item xsi:type="xsd:string" soapenc:position="[%(slot)s]">VFAT%(slot)s</item>"""%{'slot':slot}

        return """
  <xc:Context url="http://%(xdaqHost)s:%(xdaqPort)s">
    <xc:Application class="gem::supervisor::GEMGLIBSupervisorWeb" id="254" instance="0" network="local">
      <properties xmlns="urn:xdaq-application:GEMGLIBSupervisorWeb" xsi:type="soapenc:Struct">
	<confParams xsi:type="soapenc:Struct">
	  <deviceIP xsi:type="xsd:string">192.168.0.162</deviceIP>
	  <deviceName xsi:type="soapenc:Array" soapenc:arrayType="xsd:ur-type[24]">
	    %(chipLines)s
	  </deviceName>
	  <latency xsi:type="xsd:unsignedShort">25</latency>
	  <triggerSource xsi:type="xsd:unsignedShort">0</triggerSource>
	  <deviceChipID xsi:type="xsd:unsignedShort">1</deviceChipID>
	  <deviceVT1 xsi:type="xsd:unsignedShort">50</deviceVT1>
	  <deviceVT2 xsi:type="xsd:unsignedShort">0</deviceVT2>
	</confParams>
      </properties>
    </xc:Application>

    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemhardware/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_hw.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemreadout/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_readout.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemsupervisor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_supervisor.so</xc:Module>

  </xc:Context>
        """ % {'xdaqHost':xdaqHost,'xdaqPort':xdaqPort,'chipLines':chipLines
               'dataSource':dataSource,'operationMode':operationMode}


    def getVFAT2ManagerContext(self,xdaqHost,xdaqPort,glibIP):
        xdaqProcess = GEMDAQProcess.GEMDAQProcess(xdaqHost,xdaqPort,glibIP)
        xdaqProcess.addApplication("gem::hw::vfat::VFAT2Manager",0)
        self.xdaqProcesses.append(xdaqProcess)

        return """
  <xc:Context url="http://%(xdaqHost)s:%(xdaqPort)s">

    <xc:Application class="gem::hw::vfat::VFAT2Manager" id="30" instance="3" network="local">
      <properties xmlns="urn:xdaq-application:gem::hw::vfat::VFAT2Manager"
		  xsi:type="soapenc:Struct">
	<device xsi:type="xsd:string">VFAT0</device>
	<ipAddr xsi:type="xsd:string">%(glibIP)s</ipAddr>
	<settingsFile xsi:type="xsd:string">${BUILD_HOME}/gemdaq-testing/gemhardware/xml/vfat/vfat_settings.xml</settingsFile>
      </properties>
    </xc:Application>

    <xc:Module>${BUILD_HOME}/gemdaq-testing/gembase/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_base.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemutils/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_utils.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemhardware/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_hw.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemsupervisor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_supervisor.so</xc:Module>

  </xc:Context>
        """ % {'xdaqHost':xdaqHost,'xdaqPort':xdaqPort,'glibIP':glibIP}


    def getVFAT2ManagerContext(self,xdaqHost,xdaqPort,glibIP):
        xdaqProcess = GEMDAQProcess.GEMDAQProcess(xdaqHost,xdaqPort)
        xdaqProcess.addApplication("gem::supervisor::tbutils::ThresholdScan",0)
        xdaqProcess.addApplication("gem::supervisor::tbutils::LatencyScan",0)
        self.xdaqProcesses.append(xdaqProcess)

        return """
  <xc:Context url="http://%(xdaqHost)s:%(xdaqPort)s">

    <xc:Application class="gem::supervisor::tbutils::ThresholdScan" id="254" instance="0" network="local">
      <properties xmlns="urn:xdaq-application:ThresholdScan" xsi:type="soapenc:Struct">
	<ipAddr xsi:type="xsd:string">%(glibIP)s</ipAddr>
      </properties>
    </xc:Application>


<!--
    <xc:Application class="gem::supervisor::tbutils::ADCScan" id="255" instance="0" network="local">
      <properties xmlns="urn:xdaq-application:ADCScan" xsi:type="soapenc:Struct">
	<ipAddr xsi:type="xsd:string">%(glibIP)s</ipAddr>
      </properties>
    </xc:Application>
-->

    <xc:Application class="gem::supervisor::tbutils::LatencyScan" id="256" instance="0" network="local">
      <properties xmlns="urn:xdaq-application:LatencyScan" xsi:type="soapenc:Struct">
	<ipAddr xsi:type="xsd:string">%(glibIP)s</ipAddr>
      </properties>
    </xc:Application>

    <xc:Module>${BUILD_HOME}/gemdaq-testing/gembase/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_base.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemutils/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_utils.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemhardware/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_hw.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemreadout/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_readout.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemsupervisor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_supervisor.so</xc:Module>

  </xc:Context>
        """%{'xdaqHost':xdaqHost,'xdaqPort':xdaqPort, 'glibIP':glibIP}


    def getGEMHwMonitorContext(self,xdaqHost,xdaqPort,runNumber):
        xdaqProcess = GEMDAQProcess.GEMDAQProcess(xdaqHost,xdaqPort)
        #xdaqProcess.addApplication("pt::frl::Application",1)
        xdaqProcess.addApplication("gem::hwMonitor::gemHwMonitorWeb",0)
        self.xdaqProcesses.append(xdaqProcess)

        context = """
  <xc:Context url="http://%(xdaqHost)s:%(xdaqPort)s">

    <xc:Application class="gem::hwMonitor::gemHwMonitorWeb" id="254" instance="0" network="local">
    </xc:Application>
    
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemHwMonitor/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_hwMonitor.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gembase/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_base.so</xc:Module>
    <xc:Module>${BUILD_HOME}/gemdaq-testing/gemhardware/lib/${XDAQ_OS}/${XDAQ_PLATFORM}/libgem_hw.so</xc:Module>
    
  </xc:Context>
        """ % {'xdaqHost':xdaqHost,'xdaqPort':xdaqPort,'runNumber':runNumber}
        return context


    def getConfiguration(self,runNumber,dataSource,fedId,xdaqPort,ferolSourceIP,ferolDestIP,ferolDestPort,writeData,useDummyFEROL):
        hostname = socket.gethostbyaddr(socket.gethostname())[0]

        config =
        """<?xml version='1.0'?>
<xc:Partition xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	      xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"
	      xmlns:xc="http://xdaq.web.cern.ch/xdaq/xsd/2004/XMLConfiguration-30">
"""

        if useDummyFEROL:
            config += self.getDummyFerolContext(hostname,xdaqPort,ferolDestPort,fedId)
        else:
            config += self.getFerolControllerContext(hostname,xdaqPort,dataSource,ferolDestIP,ferolDestPort,fedId,ferolSourceIP)

        config += self.getEvBContext(hostname,xdaqPort+1,ferolDestIP,ferolDestPort,runNumber,fedId,writeData)

        config += "</xc:Partition>"

        return config


    def askYesNo(self,question,default=None):
        if default is True:
            question += " [Yes]"
        elif default is False:
            question += " [No]"
        question += ":"

        answer = None
        while not answer:
            answer = raw_input(question)
            if not answer and default is not None:
                return default
            elif answer.lower()[0] == 'y':
                return True
            elif answer.lower()[0] == 'n':
                return False


    def getDataSource(self):
        dataSource = None
        possibleDataSources = ('L6G_SOURCE','L6G_CORE_GENERATOR_SOURCE','L6G_LOOPBACK_GENERATOR_SOURCE','SLINK_SOURCE')

        try:
            dataSource = self._config.get('Input','dataSource')
        except ConfigParser.NoSectionError:
            self._config.add_section('Input')
        except ConfigParser.NoOptionError:
            pass

        print("""Please select the data source to be used:
  1 - Real AMC13 data source        (L6G_SOURCE)
  2 - Generator core of the AMC13   (L6G_CORE_GENERATOR_SOURCE)
  3 - Loopback at the FEROL         (L6G_LOOPBACK_GENERATOR_SOURCE)
  4 - SLINK data source             (SLINK_SOURCE)""")

        prompt = "=> "
        try:
            default = possibleDataSources.index(dataSource) + 1
            prompt += '['+str(default)+'] '
        except ValueError:
            default = 0

        chosenDataSource = None
        while chosenDataSource is None:
            answer = raw_input(prompt)
            answer = answer or default
            try:
                index = int(answer) - 1
                if index < 0:
                    raise ValueError
                chosenDataSource = possibleDataSources[index]
            except (ValueError,NameError,TypeError,IndexError):
                print("Please enter a valid choice [1.."+str(len(possibleDataSources))+"]")

        dataSource = chosenDataSource
        self._config.set('Input','dataSource',dataSource)

        return dataSource


    def getEnabledChips(self):
        dataSource = None
        possibleDataSources = range(0,24)

        try:
            dataSource = self._config.get('Input','dataSource')
        except ConfigParser.NoSectionError:
            self._config.add_section('Input')
        except ConfigParser.NoOptionError:
            pass

        print("""Please select the VFAT chips to be used (comma and hyphen separated list:
  e.g., 0,4,6-9,20""")

        prompt = "=> "
        try:
            default = possibleDataSources.index(dataSource) + 1
            prompt += '['+str(default)+'] '
        except ValueError:
            default = 0

        chosenDataSource = None
        while chosenDataSource is None:
            answer = raw_input(prompt)
            answer = answer or default
            try:
                index = int(answer) - 1
                if index < 0:
                    raise ValueError
                chosenDataSource = possibleDataSources[index]
            except (ValueError,NameError,TypeError,IndexError):
                print("Please enter a valid choice [1.."+str(len(possibleDataSources))+"]")

        dataSource = chosenDataSource
        self._config.set('Input','dataSource',dataSource)

        return dataSource


    def askUserForFedId(self,defaultFedId):
        question = "Please enter the FED id you want to read out"
        if defaultFedId is not None:
            question += " ["+str(defaultFedId)+"]: "
        else:
            question += ": "
        answer = raw_input(question)
        answer = answer or defaultFedId
        try:
            fedId=int(answer)
        except (ValueError,NameError,TypeError):
            fedId=-1

        while fedId < 0 or fedId >= 1350:
            try:
                fedId=int(raw_input("Please enter a valid FED id [0..1350]: "))
            except (ValueError,NameError,TypeError):
                fedId=-1

        return fedId


    def getFedId(self):
        fedId = None
        try:
            fedId = self._config.getint('Input','fedId')
        except ConfigParser.NoSectionError:
            self._config.add_section('Input')
        except ConfigParser.NoOptionError:
            pass

        fedId = self.askUserForFedId(fedId)
        self._config.set('Input','fedId',fedId)

        return fedId


    def askUserForRunNumber(self,defaultRunNumber):
        question = "Please enter the run number ["+str(defaultRunNumber)+"]: "
        answer = raw_input(question)
        answer = answer or defaultRunNumber
        try:
            runNumber=int(answer)
        except (ValueError,NameError,TypeError):
            runNumber=-1

        while runNumber < 0:
            try:
                runNumber=int(raw_input("Please enter a positive integer as run number: "))
            except (ValueError,NameError,TypeError):
                runNumber=-1

        return runNumber


    def getRunNumber(self):
        runNumber = 1
        try:
            runNumber = self._config.getint('Input','runNumber') + 1
        except ConfigParser.NoSectionError:
            self._config.add_section('Input')
        except ConfigParser.NoOptionError:
            pass

        if self._fullConfig:
            runNumber = self.askUserForRunNumber(runNumber)

        self._config.set('Input','runNumber',runNumber)

        return runNumber


    def getWriteData(self):
        writeData = False
        try:
            writeData = self._config.getboolean('Output','writeData')
        except ConfigParser.NoSectionError:
            self._config.add_section('Output')
        except ConfigParser.NoOptionError:
            pass

        writeData = self.askYesNo("Do you want to write the data to disk?",writeData)

        self._config.set('Output','writeData',writeData)

        return writeData


    def getOutputDir(self):
        try:
            outputDir = self._config.get('Output','outputDir')
        except ConfigParser.NoSectionError:
            self._config.add_section('Output')
        except ConfigParser.NoOptionError:
            pass

        outputDir = raw_input("Please enter the directory where the data shall be written [/tmp]: ")
        outputDir = outputDir or '/tmp'
        self._config.set('Output','outputDir',outputDir)

        return outputDir


    def askUserForIP(self,defaultIP,forWhat):
        question = "Please enter the IP address of the "+forWhat+" ["+str(defaultIP)+"]: "
        ip = raw_input(question)
        ip = ip or defaultIP

        return ip


    def askUserForPort(self,defaultPort,forWhat):
        question = "Please enter the port of the "+forWhat+" ["+str(defaultPort)+"]: "
        answer = raw_input(question)
        answer = answer or defaultPort
        try:
            port=int(answer)
        except (ValueError,NameError,TypeError):
            port=-1

        while port < self._validPortsMin or port > self._validPortsMax:
            try:
                port=int(raw_input("Please enter a valid port number ["+str(self._validPortsMin)+".."+str(self._validPortsMax)+"]: "))
            except (ValueError,NameError,TypeError):
                port=-1

        return port


    def getXdaqPort(self):
        xdaqPort = ((self._validPortsMin // 1000) + 1) * 1000
        try:
            xdaqPort = self._config.getint('XDAQ','xdaqPort')
        except ConfigParser.NoSectionError:
            self._config.add_section('XDAQ')
        except ConfigParser.NoOptionError:
            pass

        if self._fullConfig:
            xdaqPort = self.askUserForPort(xdaqPort,"XDAQ application")

        self._config.set('XDAQ','xdaqPort',xdaqPort)

        return xdaqPort


    def getFerolSourceIP(self):
        ferolSourceIP = "10.0.0.4"
        try:
            ferolSourceIP = self._config.get('XDAQ','ferolSourceIP')
        except ConfigParser.NoSectionError:
            self._config.add_section('XDAQ')
        except ConfigParser.NoOptionError:
            pass

        if self._fullConfig:
            ferolSourceIP = self.askUserForIP(ferolSourceIP,"FEROL source")

        self._config.set('XDAQ','ferolSourceIP',ferolSourceIP)

        return ferolSourceIP


    def getFerolDestIP(self):
        ferolDestIP = "10.0.0.5"
        try:
            ferolDestIP = self._config.get('XDAQ','ferolDestIP')
        except ConfigParser.NoSectionError:
            self._config.add_section('XDAQ')
        except ConfigParser.NoOptionError:
            pass

        if self._fullConfig:
            ferolDestIP = self.askUserForIP(ferolDestIP,"FEROL destination")

        self._config.set('XDAQ','ferolDestIP',ferolDestIP)

        return ferolDestIP


    def getFerolDestPort(self):
        ferolDestPort = ((self._validPortsMin // 1000) + 2) * 1000
        try:
            ferolDestPort = self._config.getint('XDAQ','ferolDestPort')
        except ConfigParser.NoSectionError:
            self._config.add_section('XDAQ')
        except ConfigParser.NoOptionError:
            pass

        if self._fullConfig:
            ferolDestPort = self.askUserForPort(ferolDestPort,"FEROL destination")

        self._config.set('XDAQ','ferolDestPort',ferolDestPort)

        return ferolDestPort


if __name__ == "__main__":
    config = GEMTestbeamDAQConfig("tmp.cfg")
    print(config._xmlConfiguration)
