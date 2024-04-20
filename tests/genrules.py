# -*- coding: utf-8 -*- 
#!/usr/bin/env python
import collections

D = collections.defaultdict(int)

for line in open("domains.txt"):
	line = line.strip()
	if line.count('.') < 2:
		continue
	items = line.split(".", 1)
	D[items[-1]] += 1

for k,v in sorted(D.items(), key=lambda x: x[1], reverse=True):
	if v>1:
		print(k, "->", v)