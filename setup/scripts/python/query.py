import os
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "db.settings")
import django
django.setup()
import datetime
from time import sleep
from ldqm_db.models import *
from ldqm_db.amcmanager import *
from gemlogger import GEMLogger
gemlogger = GEMLogger("query").gemlogger

def configure_db(station="TIF",setuptype="teststand",runperiod="2016T"):
    amc_list=[1,2,3,4,5,6,7,8,9,10,11,12]
    geb_list=[[0,1],[0,1],[0,1],[0,1],[0,1],[0,1],[0,1],[0,1],[0,1],[0,1],[0,1],[0,1],]
    zlist = zip(amc_list, geb_list)
    a_list = []
    for amcN, gtx_list in zlist:
      msg = "Trying to connect to AMC # %s\n" %(amcN)
      gemlogger.info(msg)
      m_AMCmanager = AMCmanager()
      g_list = []
      try:
        m_AMCmanager.connect(int(amcN))
      except ValueError as ve:
        continue
      # retrieve VFAT slot numberd and ChipIDs from HW
      for gtx in gtx_list:
        if m_AMCmanager.checkGTX(gtx):
          chipids = m_AMCmanager.getVFATs(gtx)
          # retrieve VFAT slot numberd and ChipIDs from DB
          vfats = VFAT.objects.all()
          # Check if the VFATs are in DB, add if not
          v_list = []
          for chip in chipids.keys():
            t_chipid = "0x%04x"%(chipids[chip])
            if t_chipid in vfats.filter(Slot=chip).values_list("ChipID", flat=True):
              pass # closes if t_chipid
            else:
              msg = "Adding VFAT(ChipID = %s, Slot = %d)"%(t_chipid,chip)
              gemlogger.info(msg)
              v = VFAT(ChipID = t_chipid, Slot = chip)
              v.save()
              pass # closese else (if t_chipid)
            v_list.append(VFAT.objects.get(ChipID = t_chipid, Slot = chip))
            pass # closes for chip in chipids.keys
          #t_chamberID = 'OHv2aM'#hard code now, read from HW later when available
          t_chamberID = 'GTX-'+str(gtx) #use gtx link number now, read from HW later when available
          msg = "t_chamberID = %s" %(t_chamberID)
          gemlogger.info(msg)
          gebs = GEB.objects.filter(ChamberID=t_chamberID)
          t_flag = False
          for geb in gebs:
            if v_list == list(geb.vfats.all()):
              t_flag = True
              g_list.append(geb)
              break
            pass # closes for geb in gebs
          if t_flag:
            pass # bypass if t_flag
          else:
            msg = "Update DB"
            gemlogger.info(msg)
            g = GEB(Type="Long",ChamberID = t_chamberID)
            g.save()
            for v in v_list:
              g.vfats.add(v)
              g_list.append(g)
              pass # closes for v in v_list
            pass # closes else (if t_flag)
          pass # closes if m_AMCmanager
        pass # closes for gtx in gtx_list

      t_flag = False
      t_boardID = "AMC-"+str(amcN)#hard code now, read from HW later when available
      amcs = AMC.objects.filter(BoardID = t_boardID)
      for amc in amcs:
        if g_list == list(amc.gebs.all()):
          t_flag = True
          a_list.append(amc)
          msg = "Adding to a_list : %s" %(amc.BoardID)
          gemlogger.debug(msg)
          pass # closes if g_list
        pass # closes for amc in amcs
      if t_flag:
        pass # closes if t_flag
      else:
        msg = "Update DB"
        gemlogger.info(msg)
        a = AMC(Type="GLIB",BoardID = t_boardID)
        a.save()
        for g in g_list:
          a.gebs.add(g)
          a_list.append(a)
          msg = "Adding to a_list : %s" %(amc.BoardID)
          gemlogger.debug(msg)
          pass # closes for g in g_list
        pass # closes else (if t_flag)
      pass # closes for amcN, gtx_list in zlist

    # create a new run. Some values are hard-coded for now
    runs = Run.objects.filter(Period = runperiod, Type = setuptype, Station = station)
    rns = list(int(x) for x in list(runs.values_list("Number", flat=True)))
    try:
      nrs = u'%s'%(max(rns)+1)
    except ValueError as ve:
      nrs = u'%s'%(1)
      pass
    nrs = nrs.zfill(6)
    t_date = str(datetime.date.today())
    m_filename = "run"+str(nrs)+""+"_"+setuptype+"_"+station+"_"+t_date
    newrun = Run(Name=m_filename, Type = setuptype, Number = str(nrs), Date = datetime.date.today(), Period = runperiod, Station = station)
    newrun.save()
    for a in a_list:
        msg = "Adding AMC: %s" %(a.BoardID)
        gemlogger.info(msg)
        newrun.amcs.add(a)
        for g in a.gebs.all():
            msg = "Adding GEB: %s" %(g.ChamberID)
            gemlogger.info(msg)
            pass
        pass
    sleep(2)
