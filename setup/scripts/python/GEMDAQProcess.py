#!/usr/bin/python

import httplib
import os
import signal
import subprocess
import time
from xml.dom import minidom

class SOAPexception(Exception):
    pass

class StateException(Exception):
    pass

class GEMDAQProcess:

    def __init__(self,host,port):
        self._host = host
        self._port = port
        self._process = None
        self._applications = []
        self._logfile = open('GEMTestbeamDAQ_'+str(port)+'.log','w')
        self._soapTemplate ="""<SOAP-ENV:Envelope SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Header/><SOAP-ENV:Body>%s</SOAP-ENV:Body></SOAP-ENV:Envelope>"""

    def __del__(self):
        if self._process is not None:
            os.kill(self._process.pid,signal.SIGKILL)
            self._logfile.flush()

    def getURL(self):
        return self._host+":"+str(self._port)

    def getURN(self,app,instance):
        try:
            next(a for a in self._applications if a == (app,instance))
        except StopIteration:
            print("I don't know anything about "+app+", instance "+str(instance))
            return None
        return "/urn:xdaq-application:class="+app+",instance="+str(instance)

    def addApplication(self,app,instance):
        self._applications.append((app,instance))

    def create(self,xmlConfig):
        runArgs = ["xdaq.exe",
                   "-p "+str(self._port),
                   "-lDEBUG",
                   "-c "+xmlConfig]
        self._process = subprocess.Popen(runArgs,stdout=self._logfile,stderr=self._logfile)

    def kill(self):
        if self._process is not None:
            os.kill(self._process.pid,signal.SIGTERM)
            self._process.wait()
            self._logfile.flush()
            self._process = None

    def sendSimpleCommand(self,command,app,instance):
        urn = "urn:xdaq-application:class="+app+",instance="+str(instance)
        soapMessage = self._soapTemplate%("<xdaq:"+command+" xmlns:xdaq=\"urn:xdaq-soap:3.0\"/>")
        headers = {"Content-Type":"text/xml",
                   "Content-Description":"SOAP Message",
                   "SOAPAction":urn}
        server = httplib.HTTPConnection(self._host,self._port)
        #server.set_debuglevel(4)
        server.request("POST","",soapMessage,headers)
        response = server.getresponse().read()
        xmldoc = minidom.parseString(response)
        xdaqResponse = xmldoc.getElementsByTagName('xdaq:'+command+'Response')
        if len(xdaqResponse) == 0:
            xdaqResponse = xmldoc.getElementsByTagName('xdaq:'+command.lower()+'Response')
        if len(xdaqResponse) != 1:
            raise(SOAPexception("Did not get a proper FSM response from "+app+":"+str(instance)+":\n"+xmldoc.toprettyxml()))
        try:
            newState = xdaqResponse[0].firstChild.attributes['xdaq:stateName'].value
        except (AttributeError,KeyError):
            raise(SOAPexception("FSM response from "+app+":"+str(instance)+" is missing state name:\n"+xmldoc.toprettyxml()))
        return newState


    def getParam(self,paramName,paramType,app,instance):
        urn = "urn:xdaq-application:class="+app+",instance="+str(instance)
        paramGet = """<xdaq:ParameterGet xmlns:xdaq=\"urn:xdaq-soap:3.0\"><p:properties xmlns:p="urn:xdaq-application:%(app)s" xsi:type="soapenc:Struct"><p:%(paramName)s xsi:type="xsd:%(paramType)s"/></p:properties></xdaq:ParameterGet>""" % {'paramName':paramName,'paramType':paramType,'app':app}
        soapMessage = self._soapTemplate%(paramGet)
        headers = {"Content-Type":"text/xml",
                   "Content-Description":"SOAP Message",
                   "SOAPAction":urn}
        server = httplib.HTTPConnection(self._host,self._port)
        server.request("POST","",soapMessage,headers)
        response = server.getresponse().read()
        xmldoc = minidom.parseString(response)
        xdaqResponse = xmldoc.getElementsByTagName('p:'+paramName)
        if len(xdaqResponse) != 1:
            raise(SOAPexception("ParameterGetResponse response from "+app+":"+str(instance)+" is missing "+paramName+":\n"+xmldoc.toprettyxml()))
        return xdaqResponse[0].firstChild.data


    def getStateName(self,app,instance):
        return self.getParam("stateName","string",app,instance)


    def configure(self):
        for app in self._applications:
            if self.getStateName(*app) not in ('Halted','uninitialized'):
                raise(StateException(app[0]+":"+str(app[1])+" is not in Halted state"))
            state = self.sendSimpleCommand("Configure",*app)
            tries=0
            while not state in ('Ready','Configured'):
                time.sleep(1)
                state = self.getStateName(*app)
                if state == 'Failed':
                    raise(StateException(app[0]+":"+str(app[1])+" has failed"))
                tries += 1
                if tries > 30:
                    raise(StateException(app[0]+":"+str(app[1])+" has not reached Ready state: "+state))


    def enable(self):
        for app in self._applications:
            if self.getStateName(*app) not in ('Ready','Configured'):
                raise(StateException(app[0]+":"+str(app[1])+" is not in Configured state"))
            state = self.sendSimpleCommand("Enable",*app)
            tries=0
            while not state in ('Enabled'):
                time.sleep(1)
                state = self.getStateName(*app)
                if state == 'Failed':
                    raise(StateException(app[0]+":"+str(app[1])+" has failed"))
                tries += 1
                if tries > 10:
                    raise(StateException(app[0]+":"+str(app[1])+" has not reached Enabled state: "+state))


    def stop(self):
        for app in self._applications:
            if self.getStateName(*app) not in ('Enabled'):
                raise(StateException(app[0]+":"+str(app[1])+" is not in Enabled state"))
            try:
                state = self.sendSimpleCommand("Stop",*app)
            except SOAPexception:
                continue
            tries=0
            while not state in ('Ready','Configured'):
                time.sleep(1)
                state = self.getStateName(*app)
                if state == 'Failed':
                    raise(StateException(app[0]+":"+str(app[1])+" has failed"))
                tries += 1
                if tries > 30:
                    raise(StateException(app[0]+":"+str(app[1])+" has not reached Ready state: "+state))


    def start(self):
        self.configure()
        self.enable()

        ## #for app in self._applications:
        ## app = self._applications[1]
        ## print self.getStateName(*app)
        ## self.sendSimpleCommand("Configure",*app)
