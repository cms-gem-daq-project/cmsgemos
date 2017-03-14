from collections import defaultdict as cdict

## from https://stackoverflow.com/questions/635483/what-is-the-best-way-to-implement-nested-dictionaries#652284
class nesteddict(dict):
    def __missing__(self, key):
        value = self[key] = type(self)() # retain local pointer to value
        return value                     # faster to return than dict lookup
