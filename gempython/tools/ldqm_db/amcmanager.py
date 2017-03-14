from gempython.tools.glib_system_info_uhal import *
from gempython.tools.glib_user_functions_uhal import *
from gempython.tools.optohybrid_user_functions_uhal import *
from gempython.tools.vfat_functions_uhal import *

class AMCmanager:
  from gempython.utils.gemlogger import GEMLogger

  def __init__(self):
    self.connection_file = "file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"
    pass

  def connect(self,sn,shelf=1):
    manager = uhal.ConnectionManager( self.connection_file )
    msg = "Open new connection\n"
    gemlogger.info(msg)
    self.amc  = manager.getDevice( "gem.shelf%02d.amc%02d"%(shelf,int(sn)) )
    #check if amc is really connected
    fwv = getSystemFWRaw(self.amc)
    if fwv == 0x0:
      raise ValueError('AMC %s is missing!' %(sn))

  def reset(self):
    resetDAQLink(self.amc)
    #writeRegister(self.amc,"GEM_AMC.DAQ.CONTROL.DAQ_LINK_RESET",0x1)
    #writeRegister(self.amc,"GEM_AMC.DAQ.CONTROL.DAQ_LINK_RESET",0x0)
    #writeRegister(self.amc,"GEM_AMC.DAQ.CONTROL", 0x8)

  def checkGTX(self,link):
    fwv = getFirmwareVersionRaw(self.amc,link)
    if fwv != 0x0:
      return True
    else:
      return False

  def activateGTX(self):
    c = 0
    linkEnableMask = enableDaqLinkMask(0,0)
    for l in (0,1):#(0,1): currently hack to have only single OH connected twice
      fwv = getFirmwareVersionRaw(self.amc,l)
      if fwv != 0x0:
        c += 1
        linkEnableMask = enableDAQLinkMask(l,linkEnableMask)
        pass
      pass
    if c == 0:
      raise ValueError('No GTX connection present!')
    else:
      enableDAQLink(self.amc,linkEnableMask)
    #elif c == 1:
    #  writeRegister(self.amc, "GEM_AMC.DAQ.CONTROL", 0x181)# enable GTX link 0
    #elif c == 2:
    #  writeRegister(self.amc, "GEM_AMC.DAQ.CONTROL", 0x381)# enable both GTX links
    return c

  def getVFATs(self,gtx):
    return getAllChipIDs(self.amc,gtx)

  def getStatus(self, verbosity):
    #DAQ REGISTERS TABLE
    self.status = '<table border="1"><tr>\n<th class="name" colspan="2">DAQ REGISTERS</th>\n</tr>'
    self.status += '<tr><th>CONTROL</th><td>0x%08X</td></tr>'                      % (readRegister(self.amc,"GEM_AMC.DAQ.CONTROL"))
    self.status += '<tr><th>STATUS</th><td>0x%08X</td></tr>'                       % (readRegister(self.amc,"GEM_AMC.DAQ.STATUS"))
    self.status += '<tr><th>GTX NOT IN TABLE ERR COUNTER</th><td>0x%08X</td></tr>' % (readRegister(self.amc,"GEM_AMC.DAQ.EXT_STATUS.NOTINTABLE_ERR"))
    self.status += '<tr><th>GTX DISPERSION ERR COUNTER</th><td>0x%08X</td></tr>'   % (readRegister(self.amc,"GEM_AMC.DAQ.EXT_STATUS.DISPER_ERR"))
    self.status += '<tr><th>L1A ID</th><td>0x%08X</td></tr>'                       % (readRegister(self.amc,"GEM_AMC.DAQ.EXT_STATUS.L1AID"))
    self.status += '<tr><th>SENT EVENTS COUNTER</th><td>0x%08X</td></tr>'          % (readRegister(self.amc,"GEM_AMC.DAQ.EXT_STATUS.EVT_SENT"))
    self.status += '<tr><th>DAV TIMEOUT</th><td>0x%08X</td></tr>'                  % (readRegister(self.amc,"GEM_AMC.DAQ.CONTROL.DAV_TIMEOUT"))
    self.status += '<tr><th>RUN TYPE</th><td>0x%08X</td></tr>'                     % (readRegister(self.amc,"GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE"))
    self.status += '<tr><th>RUN PARAMS</th><td>0x%08X</td></tr>'                   % (readRegister(self.amc,"GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS"))
    self.status += '<tr><th>MAX DAV TIMER</th><td>0x%08X</td></tr>'                % (readRegister(self.amc,"GEM_AMC.DAQ.EXT_STATUS.MAX_DAV_TIMER"))
    self.status += '<tr><th>LAST DAV TIMER</th><td>0x%08X</td></tr>'               % (readRegister(self.amc,"GEM_AMC.DAQ.EXT_STATUS.LAST_DAV_TIMER"))
    self.status += '</table>'
    #LINK STATUS TABLE
    self.status += '<table border="1"><tr>\n<th class="name" colspan="3">LINK STATUS</th>\n</tr>'
    self.status += '<tr><th></th><th>GTX0</th><th>GTX1</th></tr>'
    self.status += '<tr><th>CORRUPTED VFAT BLOCKS COUNTER</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.amc,"GEM_AMC.DAQ.GTX0.COUNTERS.CORRUPT_VFAT_BLK_CNT"),readRegister(self.amc,"GEM_AMC.DAQ.GTX1.COUNTERS.CORRUPT_VFAT_BLK_CNT"))
    self.status += '<tr><th>EVENT COUNTER</th><td>0x%08X</td><td>0x%08X</td></tr>'  % (readRegister(self.amc,"GEM_AMC.DAQ.GTX0.COUNTERS.EVN"),          readRegister(self.amc,"GEM_AMC.DAQ.GTX1.COUNTERS.EVN"))
    self.status += '<tr><th>STATUS</th><td>0x%08X</td><td>0x%08X</td></tr>'         % (readRegister(self.amc,"GEM_AMC.DAQ.GTX0.STATUS"),                readRegister(self.amc,"GEM_AMC.DAQ.GTX1.STATUS"))
    self.status += '<tr><th>MAX DAV TIMER</th><td>0x%08X</td><td>0x%08X</td></tr>'  % (readRegister(self.amc,"GEM_AMC.DAQ.GTX0.COUNTERS.MAX_DAV_TIMER"),readRegister(self.amc,"GEM_AMC.DAQ.GTX1.COUNTERS.MAX_DAV_TIMER"))
    self.status += '<tr><th>LAST DAV TIMER</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.amc,"GEM_AMC.DAQ.GTX0.COUNTERS.MAX_DAV_TIMER"),readRegister(self.amc,"GEM_AMC.DAQ.GTX1.COUNTERS.MAX_DAV_TIMER"))
    self.status += '<tr><th>DAV TIMEOUT</th><td>0x%08X</td><td>0x%08X</td></tr>'    % (readRegister(self.amc,"GEM_AMC.DAQ.GTX0.CONTROL.DAV_TIMEOUT"),   readRegister(self.amc,"GEM_AMC.DAQ.GTX1.CONTROL.DAV_TIMEOUT"))
    self.status += '</table>'
    #DEBUG TABLE
    if verbosity>1:
      link0debug = readBlock(self.amc,"GEM_AMC.DAQ.OH%d.LASTBLOCK"%(0),7)
      link1debug = readBlock(self.amc,"GEM_AMC.DAQ.OH%d.LASTBLOCK"%(1),7)
      self.status += '<table border="1"><tr>\n<th class="name" colspan="3">DAQ_DEBUG</th>\n</tr>'
      self.status += '<tr><th></th><th>GTX0</th><th>GTX1</th></tr>'
      self.status += '<tr><th>DEBUG 0</th><td>0x%08X</td><td>0x%08X</td></tr>' % (link0debug[0],link1debug[0])
      self.status += '<tr><th>DEBUG 1</th><td>0x%08X</td><td>0x%08X</td></tr>' % (link0debug[1],link1debug[1])
      self.status += '<tr><th>DEBUG 2</th><td>0x%08X</td><td>0x%08X</td></tr>' % (link0debug[2],link1debug[2])
      self.status += '<tr><th>DEBUG 3</th><td>0x%08X</td><td>0x%08X</td></tr>' % (link0debug[3],link1debug[3])
      self.status += '<tr><th>DEBUG 4</th><td>0x%08X</td><td>0x%08X</td></tr>' % (link0debug[4],link1debug[4])
      self.status += '<tr><th>DEBUG 5</th><td>0x%08X</td><td>0x%08X</td></tr>' % (link0debug[5],link1debug[5])
      self.status += '<tr><th>DEBUG 6</th><td>0x%08X</td><td>0x%08X</td></tr>' % (link0debug[6],link1debug[6])
      self.status += '</table>'

    #SBITS TABLE
    self.status += '<table border="1"><tr>\n<th class="name" colspan="3">SBITS</th>\n</tr>'
    self.status += '<tr><th></th><th>GTX0</th><th>GTX1</th></tr>'
    self.status += '<tr><th>CLUSTERS 01</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.amc,"GEM_AMC.DAQ.GTX0_CLUSTER_01"),readRegister(self.amc,"GEM_AMC.DAQ.GTX1_CLUSTER_01"))
    self.status += '<tr><th>CLUSTERS 23</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.amc,"GEM_AMC.DAQ.GTX0_CLUSTER_23"),readRegister(self.amc,"GEM_AMC.DAQ.GTX1_CLUSTER_23"))
    self.status += '<tr><th>SBITS RATE</th><td colspan="2">0x%08X</td></tr>'     % (readRegister(self.amc,"GEM_AMC.DAQ.SBIT_RATE"))
    self.status += '</table>'

    return self.status

