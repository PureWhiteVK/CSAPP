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