from collections import defaultdict as cdict

class nesteddict(dict):
    """
    Helper class for creating dictionaries that self-initialize
    rather than having to create sub-level of a dictionary, one
    can simply index all the keys above the desired level
    Effectively, mkdir -p for dicts!

    Example:
    mydict = nesteddict()
    mydict[key1][subkey2] = value


    ## from https://stackoverflow.com/questions/635483/what-is-the-best-way-to-implement-nested-dictionaries#652284

    """

    def __missing__(self, key):
        value = self[key] = type(self)() # retain local pointer to value
        return value                     # faster to return than dict lookup
