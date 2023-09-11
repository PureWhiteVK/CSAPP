/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */

#endif
// 1
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  int a = (~x) & y;
  int b = x & (~y);
  return ~((~a) & (~b));
}
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) { return 1 << 31; }
// 2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  int y = x + x + 1;
  return (!(y ^ ~0)) & (!!(x ^ y));
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int y = (x & x >> 16);
  int z = (y & y >> 8);
  return !((z & 0xaa) ^ 0xaa);
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) { return (~x) + 1; }
// 3
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0'
 * to '9') Example: isAsciiDigit(0x35) = 1. isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  return (!((x >> 4) ^ 0x3)) &
         ((!((x & 0xF) >> 3)) | (!(((x & 0xF) >> 1) ^ 0x4)));
}
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int mask = ~0 + !x;
  return (mask & y) | ((~mask) & z);
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  // 计算 x - y
  int difference = x + ((~y) + 1);
  // 计算 x 的符号
  int sign_x = (x >> 31) & 1;
  // 计算 y 的符号
  int sign_y = (y >> 31) & 1;
  // 判断 x 和 y 的符号是否相同
  int diff_sign = (sign_x ^ sign_y);
  return (diff_sign & sign_x & (!sign_y)) |
         ((!diff_sign) & ((!difference) | ((difference >> 31) & 1)));
  // x 小于或等于 y 的几种情况，不管数的大小是多少，如果满足 sign_x == 1 &&
  // sign_y == 0，那么其返回结果一定是 1 同时如果有 sign_x == 0 && sign_y ==
  // 1，那么不论后续计算结果如何，其返回结果一定是 0
  // 当符号相同的时候，我们再计算二者的差值的符号，如果 sign_r == 1，则说明 a -
  // b < 0，则有 a < b 成立，返回1 如果 sign_r == 0，则说明 a - b > 0，则有 a >
  // b，则返回 0 如果二者相减的结果为 0，也满足条件，返回 0
}
// 4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x) {
  x = x | (x >> 16);
  x = x | (x >> 8);
  x = x | (x >> 4);
  x = x | (x >> 2);
  x = 1 ^ (1 & ((x | (x >> 1))));
  return x;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  // 这个的题的关键在于找到最小的返回
  // 包含 k 个bit的二的补码可以表示的数据范围为 [-2^(k-1),2^(k-1)-1]
  // 找规律 + 最大位计数
  // 首先计算出 x 的 绝对值，然后 - 1
  // x = x > 0 ? x : ~x
  // x > 0 就直接判断 !(x >> 31) -> x 为 负数时为 1， x 为正数时为 0
  // 0xffffffff
  int negative_one = ~0;
  int condition_mask = negative_one + !(x >> 31);
  int bit_mask = 0;
  int upper_half = 0;
  int count = 16;
  x = (~condition_mask & x) | (condition_mask & ~x);

  // step1. judge 16 bits level
  // 0b1111'1111'1111'1111
  bit_mask = 0xff | (0xff << 8);
  upper_half = (x >> 16) & bit_mask;
  condition_mask = negative_one + !upper_half;
  count += (~condition_mask & (~8 + 1)) | (condition_mask & 8);
  x = (~condition_mask & (x & bit_mask)) | (condition_mask & upper_half);
  // count += upper_half ? 8 : (~8 + 1);
  // x = upper_half ? upper_half : (x & bit_mask);
  // printf("count = %d\n",count);
  // step2. judge 8 bits level
  // 0b1111'1111
  upper_half = (x >> 8) & 0xff;
  condition_mask = negative_one + !upper_half;
  count += (~condition_mask & (~4 + 1)) | (condition_mask & 4);
  x = (~condition_mask & (x & bit_mask)) | (condition_mask & upper_half);
  // count += upper_half ? 4 : (~4 + 1);
  // x = upper_half ? upper_half : (x & 0xff);
  // printf("count = %d\n",count);

  // step3. judge 4 bits level
  // 0b1111
  upper_half = (x >> 4) & 0xf;
  condition_mask = negative_one + !upper_half;
  count += (~condition_mask & (~2 + 1)) | (condition_mask & 2);
  x = (~condition_mask & (x & bit_mask)) | (condition_mask & upper_half);
  // count += upper_half ? 2 : (~2 + 1);
  // x = upper_half ? upper_half : (x & 0xf);
  // printf("count = %d\n",count);

  // step4. judge 2 bits level
  // 0b11
  upper_half = (x >> 2) & 0x3;
  condition_mask = negative_one + !upper_half;
  count += (~condition_mask & (negative_one)) | (condition_mask & 1);
  x = (~condition_mask & (x & bit_mask)) | (condition_mask & upper_half);
  // count += upper_half ? 1 : negative_one;
  // x = upper_half ? upper_half : (x & 0x3);
  // printf("count = %d\n",count);

  // step5. judge 1 bits level
  // upper_half = (x >> 1) & 0x1;
  // count += !upper_half ? 0 : 1;
  count += (x >> 1) & 1;

  // step6. judge 0
  // count += x ? 0 : negative_one;
  count += (~(negative_one + !x) & negative_one);
  return count + 1;
}
// float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  int sign = uf >> 31;
  int exp = (uf >> 23) & 0xff;
  int frac = uf & ~(0x1ff << 23);
  frac = exp ? frac : frac << 1;
  exp = (exp > 0 && exp < 0xff) ? exp + 1 : exp;
  return (sign << 31) | (exp << 23) | frac;
}
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  int sign = uf >> 31;
  int exp = ((uf >> 23) & 0xff) - 0x7f;
  int frac = uf & ~(0x1ff << 23);
  int v = 0;
  // NaN / Infinity / Overflow
  if (exp == 0x7f || exp > 31) {
    return 1 << 31;
  }
  // Zero
  if (exp < 0) {
    return 0;
  }
  // Normal Case
  frac = (exp > 23) ? frac << (exp - 23) : frac >> (23 - exp);
  v = (1 << exp) | frac;
  v = sign ? (~v) + 1 : v;
  return v;
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  if (x > 127) {
    return 0xff << 23;
  }
  if (x < -149) {
    return 0;
  }
  // 对于 -126 到 127 而言，只需要填写 exp 部分即可
  if (x >= -126 && x <= 127) {
    return (x + 127) << 23;
  }
  // 对于 -127 到 -149，需要填写 frac 部分
  return 1 << (149 + x);
}
