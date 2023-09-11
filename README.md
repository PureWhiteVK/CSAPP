# CSAPP
CS:APP Lab Assignments

## Environment

Ubuntu 20.04

GCC    9.4.0

## Data Lab

before compile, make sure you have gcc-multilib installed (for `-m32` gcc complie flag)

```bash
sudo apt-get update
sudo apt-get install gcc-multilib -y
```
Compliance with dlc

```bash
./dlc bits.c
```

Output

```bash
dlc:bits.c:148:bitXor: 8 operators
dlc:bits.c:155:tmin: 1 operators
dlc:bits.c:167:isTmax: 9 operators
dlc:bits.c:180:allOddBits: 7 operators
dlc:bits.c:188:negate: 2 operators
dlc:bits.c:201:isAsciiDigit: 12 operators
dlc:bits.c:212:conditional: 7 operators
dlc:bits.c:237:isLessOrEqual: 18 operators
dlc:bits.c:254:logicalNeg: 12 operators
dlc:bits.c:331:howManyBits: 82 operators
dlc:bits.c:351:floatScale2: 15 operators
dlc:bits.c:382:floatFloat2Int: 21 operators
dlc:bits.c:409:floatPower2: 12 operators
```

Testing

```bash
make all && ./btest
```

Output

```bash
Score   Rating  Errors  Function
 1      1       0       bitXor
 1      1       0       tmin
 1      1       0       isTmax
 2      2       0       allOddBits
 2      2       0       negate
 3      3       0       isAsciiDigit
 3      3       0       conditional
 3      3       0       isLessOrEqual
 4      4       0       logicalNeg
 4      4       0       howManyBits
 4      4       0       floatScale2
 4      4       0       floatFloat2Int
 4      4       0       floatPower2
Total points: 36/36
```

## Shell Lab

run the following command to generate trace result

```bash
make
./scripts/test.sh
./scripts/diff.sh
```

all output (along with diff result) generate with `tsh` and reference `tshref` will in `output` dir

Testing

```bash
diff output/rtest15 output/test15
```

Result


```diff
1c1
< ./sdriver.pl -t trace15.txt -s ./tshref -a "-p"
---
> ./sdriver.pl -t trace15.txt -s ./tsh -a "-p"
8c8
< Job [1] (27457) terminated by signal 2
---
> Job [1] (26933) terminated by signal 2
10c10
< [1] (27471) ./myspin 3 &
---
> [1] (26968) ./myspin 3 &
12c12
< [2] (27473) ./myspin 4 &
---
> [2] (26970) ./myspin 4 &
14,15c14,15
< [1] (27471) Running ./myspin 3 &
< [2] (27473) Running ./myspin 4 &
---
> [1] (26968) Running ./myspin 3 &
> [2] (26970) Running ./myspin 4 &
17c17
< Job [1] (27471) stopped by signal 20
---
> Job [1] (26968) stopped by signal 20
19,20c19,20
< [1] (27471) Stopped ./myspin 3 &
< [2] (27473) Running ./myspin 4 &
---
> [1] (26968) Stopped ./myspin 3 &
> [2] (26970) Running ./myspin 4 &
24c24
< [1] (27471) ./myspin 3 &
---
> [1] (26968) ./myspin 3 &
26,27c26,27
< [1] (27471) Running ./myspin 3 &
< [2] (27473) Running ./myspin 4 &
---
> [1] (26968) Running ./myspin 3 &
> [2] (26970) Running ./myspin 4 &

```

## Malloc Lab

the free block matching policy is `First Fit`

### Compile and Run
```bash
make mdriver
./mdriver -t ./traces -v -V -l
```

### Traces Files (under `./Malloc Lab/traces`)

 0. `amptjp-bal.rep`
 1. `cccp-bal.rep`
 2. `cp-decl-bal.rep`
 3. `expr-bal.rep`
 4. `coalescing-bal.rep`
 5. `random-bal.rep`
 6. `random2-bal.rep`
 7. `binary-bal.rep`
 8. `binary2-bal.rep`
 9. `realloc-bal.rep`
10. `realloc2-bal.rep`

### Implicit Free Lists

```txt
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   99%    5694  0.004763  1195
 1       yes   99%    5848  0.004549  1286
 2       yes   99%    6648  0.006962   955
 3       yes   99%    5380  0.005181  1038
 4       yes   66%   14400  0.000073196721
 5       yes   93%    4800  0.004629  1037
 6       yes   91%    4800  0.004741  1013
 7       yes   54%    6000  0.016835   356
 8       yes   47%    7200  0.020737   347
 9       yes   27%   14401  0.032043   449
10       yes   34%   14401  0.001033 13946
Total          73%   89572  0.101547   882

Perf index = 44 (util) + 40 (thru) = 84/100
```

### Implicit Free Lists with Optimized Footer

we can see a slightly improvement on `util` field compared to native one 

```txt
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   99%    5694  0.004824  1180
 1       yes   99%    5848  0.004451  1314
 2       yes   99%    6648  0.006826   974
 3       yes   99%    5380  0.005047  1066
 4       yes   66%   14400  0.000083172662
 5       yes   93%    4800  0.004592  1045
 6       yes   92%    4800  0.004688  1024
 7       yes   55%    6000  0.017217   348
 8       yes   51%    7200  0.019659   366
 9       yes   27%   14401  0.032257   446
10       yes   34%   14401  0.001046 13773
Total          74%   89572  0.100689   890

Perf index = 44 (util) + 40 (thru) = 84/100
```

### Explicit Free Lists with LIFO order and stand-alone `realloc`

we can see a huge boost on `Kops` field compared with Implicit Free Lists method (but the Perf index mainly focuses on `util`) and better `util` metric under `realloc` trace file(due to stand-alone `realloc` implementation).

```txt
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   88%    5694  0.000169 33772
 1       yes   92%    5848  0.000096 61044
 2       yes   94%    6648  0.000204 32556
 3       yes   96%    5380  0.000152 35371
 4       yes   66%   14400  0.000078183673
 5       yes   87%    4800  0.000387 12410
 6       yes   85%    4800  0.000387 12419
 7       yes   54%    6000  0.000236 25456
 8       yes   47%    7200  0.000113 64000
 9       yes   37%   14401  0.019387   743
10       yes   45%   14401  0.000344 41900
Total          72%   89572  0.021551  4156

Perf index = 43 (util) + 40 (thru) = 83/100
```

### Segregated Free Lists with LIFO order, stand-alone `realloc`

size classes: {1~32},{33~64},{65~128},{129~256},{257~512},{513~1024},{1025~2048},{2049~4096},{4097~inf}

```txt
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   98%    5694  0.000175 32630
 1       yes   97%    5848  0.000197 29761
 2       yes   99%    6648  0.000210 31672
 3       yes   99%    5380  0.000167 32293
 4       yes   98%   14400  0.000228 63241
 5       yes   88%    4800  0.000292 16444
 6       yes   86%    4800  0.000251 19093
 7       yes   54%    6000  0.000155 38610
 8       yes   47%    7200  0.000151 47588
 9       yes   44%   14401  0.019650   733
10       yes   46%   14401  0.000347 41442
Total          78%   89572  0.021822  4105

Perf index = 47 (util) + 40 (thru) = 87/100
```

