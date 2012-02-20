def minus(*args):
    res = args[0]
    for arg in args[1:]:
        res -= arg
    return res