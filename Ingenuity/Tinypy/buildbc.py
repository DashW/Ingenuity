out = []
for mod in ['tokenize','parse','encode','py2bc']:
    out.append("""unsigned char tp_%s[] = {"""%mod)
    fname = mod+".tpc"
    data = open(fname,'rb').read()
    cols = 16
    for n in xrange(0,len(data),cols):
        out.append(",".join([str(ord(v)) for v in data[n:n+cols]])+',')
    out.append("""};""")
out.append("")
f = open('bc.c','wb')
f.write('\n'.join(out))
f.close()