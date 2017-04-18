# Phase-space volumes

CPU summary:

```
Linux ingrid-ui1 3.10.0-327.22.2.el7.x86_64 #1 SMP Thu Jun 23 11:11:12 CDT 2016 x86_64 x86_64 x86_64 GNU/Linux

Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                48
On-line CPU(s) list:   0-47
Thread(s) per core:    2
Core(s) per socket:    12
Socket(s):             2
NUMA node(s):          2
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 62
Stepping:              4
CPU MHz:               1248.000
BogoMIPS:              4794.05
Virtualization:        VT-x
L1d cache:             32K
L1i cache:             32K
L2 cache:              256K
L3 cache:              30720K
NUMA node0 CPU(s):     0-11,24-35
NUMA node1 CPU(s):     12-23,36-47
```

## Block A

  - Runtime: 100.36user 0.87system 0:18.03elapsed 561%CPU
  - 6.29898e-05 +- 6.07335e-08

## Block A + Secondary block C/D

  - Runtime: 89.85user 0.98system 0:13.83elapsed 656%CPU
  - 0.0166277 +- 5.78859e-05

## Block B

  - Runtime: 760.75user 0.98system 1:53.87elapsed 668%CPU
  - 6.29735e-05 +- 5.8105e-08

## Block B + Secondary block B

  - Runtime: 6825,44s user 15,48s system 711% cpu 16:01,97 total
  - 0.016607 +- 0.000105137 

## Block B + Secondary block E

  - Runtime: 7166,71s user 2,22s system 797% cpu 14:58,39 total
  - 0.0164879 +- 8.38759e-05

## Block D

  - Runtime: 15481.15user 7.44system 38:41.82elapsed
  - 645.063 +- 4.45919 (incorrect, we should tweak integration parameters)
 
## Block F

  - Runtime: 171.57user 1.10system 0:27.12elapsed 636%CPU
  - 0.016463 +- 7.05888e-05

## Block G

  - Runtime: 823.88user 1.93system 2:12.60elapsed 622%CPU
  - 0.0165968 +- 3.46336e-05
