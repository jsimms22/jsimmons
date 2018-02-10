/* 
 *     Please include compiler name below (you may also include any other modules you would like to be loaded)
 *
 *     COMPILER= gnu
 *
 *         Please include All compiler flags and libraries as you want them run. You can simply copy this over from the Makefile's first few lines
 *          
 *          CC = cc
 *          OPT = -O3
 *          CFLAGS = -Wall -std=gnu99 $(OPT)
 *          MKLROOT = /opt/intel/composer_xe_2013.1.117/mkl
 *          LDLIBS = -lrt -Wl,--start-group $(MKLROOT)/lib/intel64/libmkl_intel_lp64.a $(MKLROOT)/lib/intel64/libmkl_sequential.a $(MKLROOT)/lib/intel64/libmkl_core.a -Wl,--end-group -lpthread -lm
 *
 *          */

const char* dgemm_desc = "Simple blocked dgemm.";

/*#if !defined(BLOCK_SIZiE)
#define BLOCK_SIZE 100
#endif*/

#define min(a,b) (((a)<(b))?(a):(b))

/* This auxiliary subroutine performs a smaller dgemm operation
 *  C := C + A * B
 * where C is M-by-N, A is M-by-K, and B is K-by-N. */
static void do_block (int lda, int M, int N, int K, double* A, double* B, double* C)
{
    register double ff[4];
    if (K % 2 == 0) {
      /* For each row i of A */
      for (int i = 0; i < M; ++i)
        /* For each column j of B */
        for (int j = 0; j < N; ++j)
        {
          /* Compute C(i,j) */
          register double cij = C[i+j*lda];
          for (int k = 0; k < K; k += 2)
          {
        	ff[0] = A[i+k*lda];
        	ff[1] = A[i+(k+1)*lda];
        	ff[2] = B[k+j*lda];
        	ff[3] = B[(k+1)+j*lda];
            cij += ff[0] * ff[2];
        	cij += ff[1] * ff[3];
          }
          C[i+j*lda] = cij;
        }
    }
    else {
      /* For each row i of A */
      for (int i = 0; i < M; ++i)
        /* For each column j of B */
        for (int j = 0; j < N; ++j)
        {
          /* Compute C(i,j) */
          register double cij = C[i+j*lda];
          for (int k = 0; k < K; ++k)
          {
        	ff[0] = A[i+k*lda];
        	ff[1] = B[k+j*lda];
            cij += ff[0] * ff[1];
          }
          C[i+j*lda] = cij;
        }
    }
}

/* This routine performs a dgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format. 
 * On exit, A and B maintain their input values. */
void square_dgemm (/*int BLOCK_SIZE,*/int lda, double* A, double* B, double* C)
{
  register int BLOCK_SIZE = 72;
  /* For each block-row of A */
  for (int i = 0; i < lda; i += BLOCK_SIZE)
    /* For each block-column of B */
    for (int j = 0; j < lda; j += BLOCK_SIZE)
      /* Accumulate block dgemms into block of C */
      for (int k = 0; k < lda; k += BLOCK_SIZE)      {
	/* Correct block dimensions if block "goes off edge of" the matrix */
	int M = min (BLOCK_SIZE, lda-i);
	int N = min (BLOCK_SIZE, lda-j);
	int K = min (BLOCK_SIZE, lda-k);

	/* Perform individual block dgemm */
	do_block(lda, M, N, K, A + i + k*lda, B + k + j*lda, C + i + j*lda);
      }
}
