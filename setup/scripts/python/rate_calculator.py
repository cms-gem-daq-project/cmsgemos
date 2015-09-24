
def errorRate(errorCounts,sampleTime):
    """
    Calculates the error rate provided in a list of pairs of measurements, taken with delay sampleTime
    errorCounts = [pair1, pair2, ...]
    pair1 = [first, second]
    returns a pair which is the rate (averaged over the number of trials) and the modifier string (k, M, G)
    """
    rate = 0.
    modifier = ""
    for trial in errorCounts:
        tmprate = ((trial[1]-trial[0])/(1.0*sampleTime))
        rate = rate + tmprate

    rate = rate / len(errorCounts)
    if rate > 1000000000:
        modifier = "G"
        rate = rate/1000000000.
    elif rate > 1000000:
        modifier = "M"
        rate = rate/1000000.
    elif rate > 1000:
        modifier = "k"
        rate = rate/1000.
        
    return rate,modifier
