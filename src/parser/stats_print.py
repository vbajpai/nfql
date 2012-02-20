import pickle
f = open('./profile_stats1')

stats = pickle.load(f)

#for st in sorted(filter(lambda a: a[1][0]>1 and a[1][1]>10,  stats), key=lambda a: a[1][1]/a[1][0], reverse=True):
#    print st[0], st[1][1]/st[1][0], st[1][1], st[1][0]

for st in sorted(stats, key=lambda a: a[1][1], reverse=True):
    print st
#for st in sorted(stats, key=lambda a: a[1][0], reverse=True):
#    if st[0][1].find('flowy/src/flowy') != -1:
#        print (st[0][1].partition('flowy/src/flowy/'))[2], st[0][0], st[0][2], st[1][0], st[1][1]