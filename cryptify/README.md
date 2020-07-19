### how to use the one-way encryption program (cryptify.c)
Linux:
```bash
# Encrypt the word 'Hello' with the salt 'f0'
./cryptify Hello f0
```

Dependencies: crypt, the standard library

Notes: Only works on Linux, and not all Linux distros (Android being the only one I've tested it with that didn't work)
