#!/usr/bin/python

class VFAT2TrackingData:

    #contains the six 32 bit words

    data_ = []
    channelData_ = []
    headers_ = []

    def __init__(self, data):
        self.data_ = data
        self.getAllChannelData()
        self.getHeaders()
        return
        
    def getHeaders(self):
        self.headers_.append((self.data_[0]>>28)&0xF)
        self.headers_.append((self.data_[0]>>12)&0x0000F)
        self.headers_.append((self.data_[1]>>28)&0xF)
        return

    def getBC(self):
        #get the 16 most significant bits of the first word
        #does not include the 4 most significant bits
        mask = 0x0FFF0000
        return self.data_[0]&mask
        
    def getEC(self):
        #get the 16 least significant bits of the first word
        #does not include the 4 most/least significant bits
        mask = 0x00000FF0
        return self.data_[0]&mask

    def getFlags(self):
        #get the 4 least significant bits of the first word
        mask = 0x0000000F
        return self.data_[0]&mask

    def getChipID(self):
        #get the 16 most significant bits of the first word
        #does not include the 4 most significant bits
        mask = 0x0FFF0000
        return self.data_[1]&mask
    
    def getCRC(self):
        #get the 16 least significant bits of the last word
        mask = 0x0000FFFF
        return self.data_[5]&mask
        
    def getChannelData(self,channel):
        return self.channelData_[channel]
    
    def getAllChannelData(self):
##        channel[0] = bit 15 of data_[1]
##        channel[1] = bit 14 of data_[1]
##        channel[15] = bit 0 of data_[1]
##        channel[16] = bit 31 of data_[2]
##        channel[32] = bit 15 of data_[2]
##
##        0-15-1
##        1-14-1
##        
##        (32*data)
        for i in range(128):
            
            ibit = (31-16-i)%32
            #dbit = ((i+16)/32)+1
            #print i,ibit,dbit
            
            dataBlock = ((i+16)/32)+1
            dataWord = self.data_[dataBlock]

            self.channelData_.append((dataWord>>ibit)&1)
        return

    def printHits(self):
        localData = []
        for i in range(128):
            if i%16==0:
                print localData
                localData = []
                localData.append(self.getChannelData(i))
            else:
                localData.append(self.getChannelData(i))
        return

    def hitChannels(self):
        hits = []
        for i in range(128):
            if self.getChannelData(i):
                hits.append(i)
        return hits

    def printFlags(self):
        myFlags = self.getFlags()
        hamErr = (myFlags>>3)&1
        aFULL  = (myFlags>>2)&1
        seuLOG = (myFlags>>1)&1
        seuI2C = (myFlags>>0)&1
        print "Hamming Error %d"%hamErr
        print "AFULL %d"%aFULL
        print "SEULogic %d"%seuLOG
        print "SEUI2C %d"%seuI2C
        return
    
    
    def printReport(self):
        print "headers", "0x%x"%self.headers_[0],"0x%x"%self.headers_[1],"0x%x"%self.headers_[2]
        print "BC",    "0x%x"%self.getBC()
        print "EC",    "0x%x"%self.getEC()
        print "Flags", "0x%x"%self.getFlags()
        self.printFlags()
        print "ChipID","0x%x"%self.getChipID()
        print "channels",     self.hitChannels()
        #print "hits",         self.printHits()
        print "CRC",   "0x%x"%self.getCRC()
