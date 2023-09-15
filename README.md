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

(trace file from [this repo](https://github.com/lsw8075/malloc-lab/blob/master/traces/README.md))

### Compile and Run
```bash
make mdriver
./mdriver -t ./traces -v -V -l
```

### Traces Files (under `MallocLab/traces`)

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
 0       yes   99%    5694  0.004892  1164
 1       yes   99%    5848  0.004561  1282
 2       yes   99%    6648  0.007152   930
 3       yes   99%    5380  0.005479   982
 4       yes   99%   14400  0.000096150785
 5       yes   92%    4800  0.005235   917
 6       yes   91%    4800  0.004949   970
 7       yes   54%   12000  0.074404   161
 8       yes   47%   24000  0.246816    97
 9       yes   27%   14401  0.035009   411
10       yes   34%   14401  0.001119 12874
Total          76%  112372  0.389712   288

Perf index = 46 (util) + 19 (thru) = 65/100
```

### Implicit Free Lists with Optimized Footer

we can see a slightly improvement on `util` field compared to native one 

```txt
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   99%    5694  0.004978  1144
 1       yes   99%    5848  0.004647  1258
 2       yes   99%    6648  0.007072   940
 3       yes   99%    5380  0.005321  1011
 4       yes   66%   14400  0.000106136235
 5       yes   91%    4800  0.005190   925
 6       yes   92%    4800  0.004901   979
 7       yes   55%   12000  0.074988   160
 8       yes   51%   24000  0.232132   103
 9       yes   27%   14401  0.033108   435
10       yes   34%   14401  0.001114 12930
Total          74%  112372  0.373557   301

Perf index = 44 (util) + 20 (thru) = 64/100
```

### Explicit Free Lists with LIFO order and stand-alone `realloc`

we can see a huge boost on `Kops` field compared with Implicit Free Lists method (but the Perf index mainly focuses on `util`) and better `util` metric under `realloc` trace file(due to stand-alone `realloc` implementation).

```txt
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   88%    5694  0.000156 36453
 1       yes   92%    5848  0.000099 58892
 2       yes   94%    6648  0.000215 30964
 3       yes   96%    5380  0.000150 35795
 4       yes   99%   14400  0.000092155676
 5       yes   87%    4800  0.000359 13356
 6       yes   85%    4800  0.000373 12855
 7       yes   54%   12000  0.000899 13354
 8       yes   47%   24000  0.000960 25005
 9       yes   37%   14401  0.020174   714
10       yes   45%   14401  0.000361 39859
Total          75%  112372  0.023839  4714

Perf index = 45 (util) + 40 (thru) = 85/100
```

### Segregated Free Lists with LIFO order, stand-alone `realloc`

size classes: {1\~32},{33\~64},{65\~128},{129\~256},{257\~512},{513\~1024},{1025\~2048},{2049\~4096},{4097\~inf}

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

## Cache Lab

LRU Cache and Optimized Matrix Transpose

```bash
make
python driver.py 
```
Note: the autograder need python2.7 to run auto-grade, please make sure you have python 2.7 installed (recommend installing it from miniconda)

```bash
conda create -n py27 python=2.7
conda activate py27
python driver.py
```

Result
```bash
Part A: Testing cache simulator
Running ./test-csim
                        Your simulator     Reference simulator
Points (s,E,b)    Hits  Misses  Evicts    Hits  Misses  Evicts
     3 (1,1,1)       9       8       6       9       8       6  traces/yi2.trace
     3 (4,2,4)       4       5       2       4       5       2  traces/yi.trace
     3 (2,1,4)       2       3       1       2       3       1  traces/dave.trace
     3 (2,1,3)     167      71      67     167      71      67  traces/trans.trace
     3 (2,2,3)     201      37      29     201      37      29  traces/trans.trace
     3 (2,4,3)     212      26      10     212      26      10  traces/trans.trace
     3 (5,1,5)     231       7       0     231       7       0  traces/trans.trace
     6 (5,1,5)  265189   21775   21743  265189   21775   21743  traces/long.trace
    27


Part B: Testing transpose function
Running ./test-trans -M 32 -N 32
Running ./test-trans -M 64 -N 64
Running ./test-trans -M 61 -N 67

Cache Lab summary:
                        Points   Max pts      Misses
Csim correctness          27.0        27
Trans perf 32x32           8.0         8         287
Trans perf 64x64           8.0         8        1179
Trans perf 61x67          10.0        10        1992
          Total points    53.0        53
```

## Proxy Lab

Result

```bash
*** Basic ***
Starting tiny on 16927
Starting proxy on 11890
1: home.html
   Fetching ./tiny/home.html into ./.proxy using the proxy
   Fetching ./tiny/home.html into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
2: csapp.c
   Fetching ./tiny/csapp.c into ./.proxy using the proxy
   Fetching ./tiny/csapp.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
3: tiny.c
   Fetching ./tiny/tiny.c into ./.proxy using the proxy
   Fetching ./tiny/tiny.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
4: godzilla.jpg
   Fetching ./tiny/godzilla.jpg into ./.proxy using the proxy
   Fetching ./tiny/godzilla.jpg into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
5: tiny
   Fetching ./tiny/tiny into ./.proxy using the proxy
   Fetching ./tiny/tiny into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
Killing tiny and proxy
basicScore: 40/40

*** Concurrency ***
Starting tiny on port 13073
Starting proxy on port 8335
Starting the blocking NOP server on port 31179
Trying to fetch a file from the blocking nop-server
Fetching ./tiny/home.html into ./.noproxy directly from Tiny
Fetching ./tiny/home.html into ./.proxy using the proxy
Checking whether the proxy fetch succeeded
Success: Was able to fetch tiny/home.html from the proxy.
Killing tiny, proxy, and nop-server
concurrencyScore: 15/15

*** Cache ***
Starting tiny on port 17277
Starting proxy on port 16756
Fetching ./tiny/tiny.c into ./.proxy using the proxy
Fetching ./tiny/home.html into ./.proxy using the proxy
Fetching ./tiny/csapp.c into ./.proxy using the proxy
Killing tiny
Fetching a cached copy of ./tiny/home.html into ./.noproxy
Success: Was able to fetch tiny/home.html from the cache.
Killing proxy
cacheScore: 15/15

totalScore: 70/70
```