# Utility to reset the StartAllBack trial.

## Explanation

It works by deleting empty subkeys at `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\CLSID`.
The application always acts upon HKCU when run.

Intent of the developers was obviously to waste time of people
trying to figure it out on their own, but with the help of
[Process Monitor](https://learn.microsoft.com/en-us/sysinternals/downloads/procmon)
it was pretty trivial in terms of difficulty to quickly see
where things are headed.

## Building

1. Set-up [Tiny C Compiler (TCC)](https://bellard.org/tcc/) and the `winapi` that ships alongside it.
2. `git clone https://github.com/imshvc/start-all-back-trial-reset`
3. `cd start-all-back-trial-reset`
4. `make`

If everything is setup correctly, you should get a `sabtr.exe` executable.

## In action

### 1. Subkey enumeration

1. Subkey matched.

```
[+] INFO: Querying subkeys ...
 | INFO: Found 6 subkeys
 | PASS: Found a matching subkey: {ded2c0ff-11e4-9661-6b20-929ffc0033c}
 | INFO: Absolute Path: Software\Microsoft\Windows\CurrentVersion\Explorer\CLSID\{ded2c0ff-11e4-9661-6b20-929ffc0033c}
 | PASS: Successfully deleted the subkey
```

---

2. Subkey not matched.

```
[+] INFO: Querying subkeys ...
 | INFO: Found 5 subkeys
 | FAIL: No subkey was matched.
```

---

3. Multiple subkeys matched (rare).

You'll need to run the command multiple times to get rid of all matching subkeys.
The utility only deletes the first subkey that matched.

```
[+] INFO: Querying subkeys ...
 | INFO: Found 7 subkeys

 +---------------------------------------------+
 | WARN: Multiple subkeys match the criteria   |
 | (first encountered subkey will be affected) |
 +---------------------------------------------+

 | PASS: Found a matching subkey: WeirdKey1234
 | INFO: Absolute Path: Software\Microsoft\Windows\CurrentVersion\Explorer\CLSID\WeirdKey1234
 | PASS: Successfully deleted the subkey
```

### 2. Manual review

Opening **StartAllBack configuration** window and navigating to the About section should yield:

```
Program evaluation. Trial days left: 100
```
