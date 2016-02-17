Class AMC
{
  private:
    //AMC header #1	
      uint8_t  m_AMCnum;      //0000:4   AMC#:4
                              //Slot number of AMC(GLIB/MP7/EC7, etc.)
      uint32_t m_L1A;         //L1A ID:24        (8 0's):8
                              //basically like event number, but reset by resync
      uint16_t m_BX:          //BX ID:12         0000:4
                              //Bunch crossing ID
      uint32_t m_Dlength;     //Data length:20   (12 0's):12
                              //Overall size of this FED event fragment in 64bit words (including headers and trailers)

    //AMC header #2
      uint8_t m_Rtype;        //Format Version:4  Run Type:4    
                              //current version = 0x0;  Could be used to encode run types like physics, cosmics, threshold scan, etc.
      uint8_t m_Param1;       //Run param1:8 
      uint8_t m_Param2;       //Run param2:8
      uint8_t m_Param3;       //Run param3:8 
      uint16_t m_Onum;        //Orbit number:16 
      uint16_t m_BID          //Board ID:16
                              //This is currently filled with 8bit long GLIB serial number


    //AMC_trailer
	
  public:
      uint8_t  AMCnum()  {return m_AMCnum};
      uint32_t L1A()     {return m_L1A};
      uint16_t BX()      {return m_BX};
      uint32_t Dlength() {return m_Dlength};

      uint8_t  Rtype()   {return m_Rtype};
      uint8_t  Param1()  {return m_Param1};
      uint8_t  Param2()  {return m_Param2};
      uint8_t  Param3()  {return m_Param3};
      uint16_t Onum()    {return m_Onum};
      uint16_t BID()     {return m_BID};


	
};

Class GEM
{
  private:
    //GEM event header
      uint32_t m_GEMDAV;   //GEM DAV list:24    (8 zeroes):8
                           //Bitmask indicating which inputs/chambers have data
      uint64_t m_Bstatus;  // Buffer Status:34  (30 zeroes):30
                           //Bitmask indicating buffer error in given inputs
      uint8_t  m_GDcount;  //GEM DAV count:5    000:3
                           //Number of chamber blocks
      uint8_t  m_Tstate;   //TTS state:4        0000:4
                           //Debug: GLIB TTS state at the moment when this event was built

    //GEM chamber header

      uint32_t m_ZeroSup;  //Zero Suppression Flags:24  (8 zeroes):8
                           //Bitmask indicating if certain VFAT blocks have been zero suppressed
      uint8_t m_InputID;   //Input ID:5                 000:3
                           //GLIB input ID (starting at 0)
      uint16_t m_Vwh;      //VFAT word count:12         0000:4
                           //Size of VFAT payload in 64 bit words
      uint16_t m_ErrorC;
                           //EvtFIFO full:1       InFIFO full:1      L1AFIFO full:1    Even size overflow:1  EvtFIFO near full:1  InFIFO near full:1 
                           //L1AFIFO near full:1  Event size warn:1  No VFAT marker:1  OOS GLIB VFAT:1       OOS GLIB OH:1        
                           //BX mismatch GLIB VFAT:1                 BX mismatch GLIB OH:1                   
                           //000:3
	


    //GEM chamber trailer
		
      uint16_t m_OHCRC;    //OH CRC:16
                           //CRC of OH data (currently not available)
      uint16_t m_Vwt;      //VFAT word count:12   0000:4
                           //Same as in header. This one actually counts the number of valid words that were sent to AMC13; the one in header is 
                           //what we expected to send to AMC13
      uint8_t m_InFu;      //InFIFO underflow:1   (7 0's):7
                           //Input status (critical): Input FIFO underflow occured while sending this event
      uint8_t m_Stuckd;    //Stuck data:1    (7 0's):7
                           //Input status (warning): data in InFIFO or EvtFIFO when L1A FIFO was empty. Only resets with resync or reset
		
    //GEM event trailer
      uint32_t m_ChamT;    //Chamber timeout:24   (8 0's):8
                           //Bitmask indicating if GLIB did not recieve data from particular input for this L1A in X amount of GTX clock cycles
      uint8_t  m_OOSG;     //OOS GLIB:1    (7 0's):7
                           //GLIB is out-of-sync (critical): L1A ID is different for different chambers in this event.

  public:
		
      uint32_t GEMDAV ()  {return m_GEMDAV};
      uint64_t Bstatus()  {return m_Bstatus};
      uint8_t  GDcount()  {return m_GDcount};
      uint8_t  Tstate()   {return m_Tstate};

      uint32_t ZeroSup()  {return m_ZeroSup};
      uint8_t  InputID()  {return m_InputID};
      uint16_t Vwh()      {return m_Vwh};
      uint16_t ErrorC()   {return m_ErrorC};

      uint16_t OHCRC()    {return m_OHCRC};
      uint16_t Vwt()      {return m_Vwt};
      uint8_t  InFu()     {return m_InFu};
      uint8_t  Stuckd()   {return m_Stuckd};

      uint32_t ChamT()    {return m_ChamT};
      uint8_t  OOSG()     {return m_OOSG};

};

Class VFAT
{

};
