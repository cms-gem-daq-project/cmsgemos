import os
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "db.settings")
import django
django.setup()
from ldqm_db.models import *
from ldqm_db.amcmanager import *

def configure_db(amc_list, geb_list):
    zlist = zip(amc_list, geb_list)
    for amcN, gtx_list in zlist:
      print "Trying to connect to AMC # %s\n" %(amcN)
      m_AMCmanager = AMCmanager()
      try:
        m_AMCmanager.connect(int(amcN))
      except ValueError as ve:
        continue
      # retrieve VFAT slot numberd and ChipIDs from HW
      for gtx in gtx_list:
        chipids = m_AMCmanager.getVFATs(gtx)
        # retrieve VFAT slot numberd and ChipIDs from DB
        vfats = VFAT.objects.all()
        # Check if the VFATs are in DB, add if not
        v_list = []
        for chip in chipids.keys():
          t_chipid = "0x%04x"%(chipids[chip])
          if t_chipid in vfats.filter(Slot=chip).values_list("ChipID", flat=True):
            pass
          else:
            print "Adding VFAT(ChipID = %s, Slot = %d)"%(t_chipid,chip)
            v = VFAT(ChipID = t_chipid, Slot = chip)
            v.save()
          v_list.append(VFAT.objects.get(ChipID = t_chipid, Slot = chip))
        #t_chamberID = 'OHv2aM'#hard code now, read from HW later when available
        t_chamberID = 'GTX-'+str(gtx) #use gtx link number now, read from HW later when available
        print "t_chamberID = %s" %(t_chamberID)
        g_list = []
        gebs = GEB.objects.filter(ChamberID=t_chamberID)
        t_flag = False
        for geb in gebs:
          if v_list == list(geb.vfats.all()):
            t_flag = True
            g_list.append(geb)
            break
        if t_flag:
          pass
        else:
          print "Update DB"
          g = GEB(Type="Long",ChamberID = t_chamberID)
          g.save()
          for v in v_list:
            g.vfats.add(v)
            g_list.append(g)
    
      t_flag = False
      t_boardID = "AMC-"+str(amcN)#hard code now, read from HW later when available
      a_list = []
      amcs = AMC.objects.filter(BoardID = t_boardID)
      for amc in amcs:
        if g_list == list(amc.gebs.all()):
          t_flag = True
          a_list.append(amc)
      if t_flag:
        pass
      else:
        print "Update DB"
        a = AMC(Type="GLIB",BoardID = t_boardID)
        a.save()
        for g in g_list:
          a.gebs.add(g)
          a_list.append(a)
    
    # create a new run. Some values are hard-coded for now
    runs = Run.objects.filter(Period = "2016T", Type = "teststand", Station = "904")
    rns = list(int(x) for x in list(runs.values_list("Number", flat=True)))
    nrs = u'%s'%(max(rns)+1)
    nrs = nrs.zfill(6)
    t_date = str(datetime.date.today())
    m_filename = "run"+str(nrs)+""+"_teststand_904"+t_date
    newrun = Run(Name=m_filename, Type = "teststand", Number = str(nrs), Date = datetime.date.today(), Period = "2016T", Station = "904")
    newrun.save()
    for a in a_list:
      newrun.amcs.add(a)
