if __name__ == '__main__':
  import os, time, sys
  trace = sys.argv[1]
  query = sys.argv[2]
  base = "bin/engine %s %s --dirpath=/tmp/"%(query,trace)
  for level in range(0, 10):
    stmt = "%s -z%s"%(base,level)
    start = time.time()
    os.system(stmt)
    elapsed = time.time() - start
    print "-z%s: %s secs"%(level, elapsed)

