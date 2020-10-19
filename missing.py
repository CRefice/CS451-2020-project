import sys

supposed = set((sender, msg) for sender in range(1, 4) for msg in range(1, 1001))

for path in sys.argv[1:]:
    file = open(path)
    arrived = set()
    for line in file:
        toks = line.split()
        if len(toks) < 3:
            continue
        _, sender, num = toks
        arrived.add((int(sender), int(num)))
    missing = supposed.difference(arrived)
    print("{}: missing {}: {}".format(path, len(missing), missing))
