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

# Fill Info for GE21 - m1
vfat3GBTPhaseLookupTable["ge21"]["m1"][0]  = 6  #VFAT0
vfat3GBTPhaseLookupTable["ge21"]["m1"][1]  = 10 #VFAT1
vfat3GBTPhaseLookupTable["ge21"]["m1"][2]  = 6  #VFAT2
vfat3GBTPhaseLookupTable["ge21"]["m1"][3]  = 9  #VFAT3
vfat3GBTPhaseLookupTable["ge21"]["m1"][4]  = 8  #VFAT4
vfat3GBTPhaseLookupTable["ge21"]["m1"][5]  = 7  #VFAT5
vfat3GBTPhaseLookupTable["ge21"]["m1"][6]  = 4  #VFAT6
vfat3GBTPhaseLookupTable["ge21"]["m1"][7]  = 8  #VFAT7
vfat3GBTPhaseLookupTable["ge21"]["m1"][8]  = 12 #VFAT8
vfat3GBTPhaseLookupTable["ge21"]["m1"][9]  = 9  #VFAT9
vfat3GBTPhaseLookupTable["ge21"]["m1"][10] = 9  #VFAT10
vfat3GBTPhaseLookupTable["ge21"]["m1"][11] = 8  #VFAT11

# Fill Info for GE21 - m2
vfat3GBTPhaseLookupTable["ge21"]["m2"][0]  = 6  #VFAT0
vfat3GBTPhaseLookupTable["ge21"]["m2"][1]  = 6  #VFAT1
vfat3GBTPhaseLookupTable["ge21"]["m2"][2]  = 10 #VFAT2
vfat3GBTPhaseLookupTable["ge21"]["m2"][3]  = 7  #VFAT3
vfat3GBTPhaseLookupTable["ge21"]["m2"][4]  = 8  #VFAT4
vfat3GBTPhaseLookupTable["ge21"]["m2"][5]  = 7  #VFAT5
vfat3GBTPhaseLookupTable["ge21"]["m2"][6]  = 4  #VFAT6
vfat3GBTPhaseLookupTable["ge21"]["m2"][7]  = 8  #VFAT7
vfat3GBTPhaseLookupTable["ge21"]["m2"][8]  = 7  #VFAT8
vfat3GBTPhaseLookupTable["ge21"]["m2"][9]  = 5  #VFAT9
vfat3GBTPhaseLookupTable["ge21"]["m2"][10] = 8  #VFAT10
vfat3GBTPhaseLookupTable["ge21"]["m2"][11] = 7  #VFAT11

# Fill Info for GE21 - m3
vfat3GBTPhaseLookupTable["ge21"]["m3"][0]  = 7  #VFAT0
vfat3GBTPhaseLookupTable["ge21"]["m3"][1]  = 5  #VFAT1
vfat3GBTPhaseLookupTable["ge21"]["m3"][2]  = 7  #VFAT2
vfat3GBTPhaseLookupTable["ge21"]["m3"][3]  = 10 #VFAT3
vfat3GBTPhaseLookupTable["ge21"]["m3"][4]  = 9  #VFAT4
vfat3GBTPhaseLookupTable["ge21"]["m3"][5]  = 7  #VFAT5
vfat3GBTPhaseLookupTable["ge21"]["m3"][6]  = 8  #VFAT6
vfat3GBTPhaseLookupTable["ge21"]["m3"][7]  = 10 #VFAT7
vfat3GBTPhaseLookupTable["ge21"]["m3"][8]  = 7  #VFAT8
vfat3GBTPhaseLookupTable["ge21"]["m3"][9]  = 10 #VFAT9
vfat3GBTPhaseLookupTable["ge21"]["m3"][10] = 6  #VFAT10
vfat3GBTPhaseLookupTable["ge21"]["m3"][11] = 4  #VFAT11

# Fill Info for GE21 - m4
vfat3GBTPhaseLookupTable["ge21"]["m4"][0]  = 3  #VFAT0
vfat3GBTPhaseLookupTable["ge21"]["m4"][1]  = 3  #VFAT1
vfat3GBTPhaseLookupTable["ge21"]["m4"][2]  = 3  #VFAT2
vfat3GBTPhaseLookupTable["ge21"]["m4"][3]  = 9  #VFAT3
vfat3GBTPhaseLookupTable["ge21"]["m4"][4]  = 4  #VFAT4
vfat3GBTPhaseLookupTable["ge21"]["m4"][5]  = 3  #VFAT5
vfat3GBTPhaseLookupTable["ge21"]["m4"][6]  = 8  #VFAT6
vfat3GBTPhaseLookupTable["ge21"]["m4"][7]  = 8  #VFAT7
vfat3GBTPhaseLookupTable["ge21"]["m4"][8]  = 11 #VFAT8
vfat3GBTPhaseLookupTable["ge21"]["m4"][9]  = 6  #VFAT9
vfat3GBTPhaseLookupTable["ge21"]["m4"][10] = 6  #VFAT10
vfat3GBTPhaseLookupTable["ge21"]["m4"][11] = 4  #VFAT11

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

# Fill Info for GE11 - Long
vfat3GBTPhaseLookupTable["ge11"]["long"][0]  = 4  #VFAT0
vfat3GBTPhaseLookupTable["ge11"]["long"][1]  = 8  #VFAT1
vfat3GBTPhaseLookupTable["ge11"]["long"][2]  = 12 #VFAT2
vfat3GBTPhaseLookupTable["ge11"]["long"][3]  = 4  #VFAT3
vfat3GBTPhaseLookupTable["ge11"]["long"][4]  = 4  #VFAT4
vfat3GBTPhaseLookupTable["ge11"]["long"][5]  = 6  #VFAT5
vfat3GBTPhaseLookupTable["ge11"]["long"][6]  = 4  #VFAT6
vfat3GBTPhaseLookupTable["ge11"]["long"][7]  = 11 #VFAT7
vfat3GBTPhaseLookupTable["ge11"]["long"][8]  = 4  #VFAT8
vfat3GBTPhaseLookupTable["ge11"]["long"][9]  = 4  #VFAT9
vfat3GBTPhaseLookupTable["ge11"]["long"][10] = 10 #VFAT10
vfat3GBTPhaseLookupTable["ge11"]["long"][11] = 4  #VFAT11
vfat3GBTPhaseLookupTable["ge11"]["long"][12] = 5  #VFAT12
vfat3GBTPhaseLookupTable["ge11"]["long"][13] = 5  #VFAT13
vfat3GBTPhaseLookupTable["ge11"]["long"][14] = 7  #VFAT14
vfat3GBTPhaseLookupTable["ge11"]["long"][15] = 4  #VFAT15
vfat3GBTPhaseLookupTable["ge11"]["long"][16] = 2  #VFAT16
vfat3GBTPhaseLookupTable["ge11"]["long"][17] = 8  #VFAT17
vfat3GBTPhaseLookupTable["ge11"]["long"][18] = 11 #VFAT18
vfat3GBTPhaseLookupTable["ge11"]["long"][19] = 4  #VFAT19
vfat3GBTPhaseLookupTable["ge11"]["long"][20] = 5  #VFAT20
vfat3GBTPhaseLookupTable["ge11"]["long"][21] = 2  #VFAT21
vfat3GBTPhaseLookupTable["ge11"]["long"][22] = 6  #VFAT22
vfat3GBTPhaseLookupTable["ge11"]["long"][23] = 4  #VFAT23
