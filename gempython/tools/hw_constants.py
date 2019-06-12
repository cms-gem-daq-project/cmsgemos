# Known detector types
gemVariants = {
            "ge11":["short","long"],
            "ge21":["m{0}".format(ge21Type) for ge21Type in range(1,9) ],
            "me0":"null"}

vfatsPerGemVariant = {
            "ge11":24,
            "ge21":12,
            "me0":24}

gbtsPerGemVariant = {
            "ge11":3,
            "ge21":2,
            "me0":0}

# Size of VFAT3 DAC's
maxVfat3DACSize = {
        #ADC Measures Current
        #0:(0x3f, "CFG_IREF"), # This should never be scanned per VFAT3 Team's Instructions
        1:(0xff,"CFG_CAL_DAC"), # as current
        2:(0xff,"CFG_BIAS_PRE_I_BIT"),
        3:(0x3f,"CFG_BIAS_PRE_I_BLCC"),
        4:(0x3f,"CFG_BIAS_PRE_I_BSF"),
        5:(0xff,"CFG_BIAS_SH_I_BFCAS"),
        6:(0xff,"CFG_BIAS_SH_I_BDIFF"),
        7:(0xff,"CFG_BIAS_SD_I_BDIFF"),
        8:(0xff,"CFG_BIAS_SD_I_BFCAS"),
        9:(0x3f,"CFG_BIAS_SD_I_BSF"),
        10:(0x3f,"CFG_BIAS_CFD_DAC_1"),
        11:(0x3f,"CFG_BIAS_CFD_DAC_2"),
        12:(0x3f,"CFG_HYST"),
        14:(0xff,"CFG_THR_ARM_DAC"),
        15:(0xff,"CFG_THR_ZCC_DAC"),
        #16:(0xff,""),Don't know reg in CTP7 address space

        #ADC Measures Voltage
        #33:(0xff,"CFG_CAL_DAC"), # as voltage; removing, harder to convert to charge
        34:(0xff,"CFG_BIAS_PRE_VREF"),
        35:(0xff,"CFG_THR_ARM_DAC"),
        36:(0xff,"CFG_THR_ZCC_DAC"),
        39:(0x3,"CFG_VREF_ADC")
        #41:(0x3f,""))Don't know reg in CTP7 address space
        }

# VFAT3 Phase Settings
GBT_PHASE_RANGE = 16

# Best Guess of what default good phase "should be"
from gempython.utils.nesteddict import nesteddict as ndict
vfat3GBTPhaseLookupTable = ndict() #keys follow from gemVariants, values are a list with length = N_VFATs; each element is a phase value for that VFAT

# Provide Place holders
for ge11Type in gemVariants["ge11"]:
    vfat3GBTPhaseLookupTable["ge11"][ge11Type] = [ 0 for x in range(0,vfatsPerGemVariant["ge11"]) ]
for ge21Type in gemVariants["ge21"]:
    vfat3GBTPhaseLookupTable["ge21"][ge21Type] = [ 0 for x in range(0,vfatsPerGemVariant["ge21"]) ]
vfat3GBTPhaseLookupTable["me0"]["null"] = [ 0 for x in range(0,vfatsPerGemVariant["me0"]) ]

# Fill Info for GE21 - m3
vfat3GBTPhaseLookupTable["ge21"]["m3"][5]  = 8  #VFAT5
# Fill Info for GE11 - Short
# FIXME It would be great if this was in the DB and I could just load it from there...
vfat3GBTPhaseLookupTable["ge11"]["short"][0]  = 6  #VFAT0
vfat3GBTPhaseLookupTable["ge11"]["short"][1]  = 9  #VFAT1
vfat3GBTPhaseLookupTable["ge11"]["short"][2]  = 13 #VFAT2
vfat3GBTPhaseLookupTable["ge11"]["short"][3]  = 5  #VFAT3
vfat3GBTPhaseLookupTable["ge11"]["short"][4]  = 5  #VFAT4
vfat3GBTPhaseLookupTable["ge11"]["short"][5]  = 4  #VFAT5
vfat3GBTPhaseLookupTable["ge11"]["short"][6]  = 7  #VFAT6
vfat3GBTPhaseLookupTable["ge11"]["short"][7]  = 8  #VFAT7
vfat3GBTPhaseLookupTable["ge11"]["short"][8]  = 7  #VFAT8
vfat3GBTPhaseLookupTable["ge11"]["short"][9]  = 9  #VFAT9
vfat3GBTPhaseLookupTable["ge11"]["short"][10] = 12 #VFAT10
vfat3GBTPhaseLookupTable["ge11"]["short"][11] = 12 #VFAT11
vfat3GBTPhaseLookupTable["ge11"]["short"][12] = 6  #VFAT12
vfat3GBTPhaseLookupTable["ge11"]["short"][13] = 3  #VFAT13
vfat3GBTPhaseLookupTable["ge11"]["short"][14] = 2  #VFAT14
vfat3GBTPhaseLookupTable["ge11"]["short"][15] = 6  #VFAT15
vfat3GBTPhaseLookupTable["ge11"]["short"][16] = 9  #VFAT16
vfat3GBTPhaseLookupTable["ge11"]["short"][17] = 10 #VFAT17
vfat3GBTPhaseLookupTable["ge11"]["short"][18] = 8  #VFAT18
vfat3GBTPhaseLookupTable["ge11"]["short"][19] = 6  #VFAT19
vfat3GBTPhaseLookupTable["ge11"]["short"][20] = 10 #VFAT20
vfat3GBTPhaseLookupTable["ge11"]["short"][21] = 9  #VFAT21
vfat3GBTPhaseLookupTable["ge11"]["short"][22] = 12 #VFAT22
vfat3GBTPhaseLookupTable["ge11"]["short"][23] = 7  #VFAT23
