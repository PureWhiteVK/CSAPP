/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include "cachelab.h"
#include <stdio.h>

#define MIN(x, y) ((x) > (y) ? (y) : (x))

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

char trans_32x32_desc[] = "32x32 matrix transpose";
void trans_32x32(int M, int N, int A[N][M], int B[M][N]) {
  int ii, jj, i;
  int a0, a1, a2, a3, a4, a5, a6, a7;
  for (ii = 0; ii < N; ii += 8) {
    for (jj = 0; jj < M; jj += 8) {
      for (i = 0; i < 8; i++) {
        // 一次加载矩阵一行到 cacheline 中（只会占用一个set）
        a0 = A[ii + i][jj + 0];
        a1 = A[ii + i][jj + 1];
        a2 = A[ii + i][jj + 2];
        a3 = A[ii + i][jj + 3];
        a4 = A[ii + i][jj + 4];
        a5 = A[ii + i][jj + 5];
        a6 = A[ii + i][jj + 6];
        a7 = A[ii + i][jj + 7];
        // 此时访问B是也会miss and
        // 32x32下cacheline可以存储8行
        // 关键在于访问时 cacheline 可以存多少行
        // eviction，尽可能把cache留出来给B（会尽可能把cache占满）
        B[jj + 0][ii + i] = a0;
        B[jj + 1][ii + i] = a1;
        B[jj + 2][ii + i] = a2;
        B[jj + 3][ii + i] = a3;
        B[jj + 4][ii + i] = a4;
        B[jj + 5][ii + i] = a5;
        B[jj + 6][ii + i] = a6;
        B[jj + 7][ii + i] = a7;
      }
    }
  }
}

char trans_64x64_desc[] = "64x64 matrix transpose";
void trans_64x64(int M, int N, int A[N][M], int B[M][N]) {
  int ii, jj, i;
  int a0, a1, a2, a3, a4, a5, a6, a7;
  for (ii = 0; ii < N; ii += 8) {
    for (jj = 0; jj < M; jj += 8) {
      for (i = 0; i < 4; i++) {
        a0 = A[ii + i][jj + 0];
        a1 = A[ii + i][jj + 1];
        a2 = A[ii + i][jj + 2];
        a3 = A[ii + i][jj + 3];
        // 拷贝 A 右上一行
        a4 = A[ii + i][jj + 4];
        a5 = A[ii + i][jj + 5];
        a6 = A[ii + i][jj + 6];
        a7 = A[ii + i][jj + 7];

        // 放置在 B 右上一列
        B[jj + 0][ii + i] = a0;
        B[jj + 0][ii + i + 4] = a4;

        B[jj + 1][ii + i] = a1;
        B[jj + 1][ii + i + 4] = a5;

        B[jj + 2][ii + i] = a2;
        B[jj + 2][ii + i + 4] = a6;

        B[jj + 3][ii + i] = a3;
        B[jj + 3][ii + i + 4] = a7;
      }
      for (i = 0; i < 4; i++) {
        // 关键在于减少列的访问
        // 拷贝 A 左下 一列
        a0 = A[ii + 4][jj + i];
        a1 = A[ii + 5][jj + i];
        a2 = A[ii + 6][jj + i];
        a3 = A[ii + 7][jj + i];
        // 拷贝 B 右上 一行
        a4 = B[jj + i][ii + 4];
        a5 = B[jj + i][ii + 5];
        a6 = B[jj + i][ii + 6];
        a7 = B[jj + i][ii + 7];
        // 放到 B 右上 一行
        B[jj + i][ii + 4] = a0;
        B[jj + i][ii + 5] = a1;
        B[jj + i][ii + 6] = a2;
        B[jj + i][ii + 7] = a3;
        // 放到 B 右下 一行
        B[jj + i + 4][ii + 0] = a4;
        B[jj + i + 4][ii + 1] = a5;
        B[jj + i + 4][ii + 2] = a6;
        B[jj + i + 4][ii + 3] = a7;
      }
      // 最后一块 4x4
      for (i = 4; i < 8; i++) {
        a0 = A[ii + i][jj + 4];
        a1 = A[ii + i][jj + 5];
        a2 = A[ii + i][jj + 6];
        a3 = A[ii + i][jj + 7];

        B[jj + 4][ii + i] = a0;
        B[jj + 5][ii + i] = a1;
        B[jj + 6][ii + i] = a2;
        B[jj + 7][ii + i] = a3;
      }
    }
  }
}

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
  if (M == 32 && N == 32) {
    trans_32x32(M, N, A, B);
  } else if (M == 64 && N == 64) {
    trans_64x64(M, N, A, B);
  } else {
    // perform block transpose
    int ii, jj, i, j;
    for (ii = 0; ii < N; ii += 16) {
      for (jj = 0; jj < M; jj += 16) {
        for (i = ii; i < MIN(ii + 16, N); i++) {
          for (j = jj; j < MIN(jj + 16, M); j++) {
            B[j][i] = A[i][j];
          }
        }
      }
    }
  }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
  int i, j, tmp;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; j++) {
      tmp = A[i][j];
      B[j][i] = tmp;
    }
  }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
  /* Register your solution function */
  registerTransFunction(transpose_submit, transpose_submit_desc);

  /* Register any additional transpose functions */
  registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
  int i, j;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; ++j) {
      if (A[i][j] != B[j][i]) {
        return 0;
      }
    }
  }
  return 1;
}
