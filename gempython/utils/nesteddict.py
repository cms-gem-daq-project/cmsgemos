from collections import defaultdict as cdict
from collections import MutableMapping

def flatten(ndict, parent_key='', sep='_'):
    """
    flattens a nesteddict to one dimension, allows for easily
    determining the number of elements in the nesteddict

    keys of the dictionary and all it's nested dictionaries
    must be convertable to strings

    ## from https://stackoverflow.com/questions/6027558/flatten-nested-python-dictionaries-compressing-keys
    """
    items = []
    for key, value in ndict.items():
        new_key = parent_key + sep + str(key) if parent_key else str(key)
        if isinstance(value, MutableMapping):
            items.extend(flatten(value, new_key, sep=sep).items())
        else:
            items.append((new_key, value))
    return dict(items)

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
