### how to use the compressor (compressor.c)
Linux:
```bash
# Compress random.txt to random.txt.z
./compressor -c < random.txt > random.txt.z
# Decompress random.txt.z to random.txt
./compressor -d < random.txt.z > random.txt
```

Dependencies: zlib, the standard library

Notes: Only has a noticable effect on files ~2MB or larger in size
