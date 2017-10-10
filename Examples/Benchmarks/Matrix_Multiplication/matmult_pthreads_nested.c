#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

int thread_count;

double **a;
double **b;
double **res;
int dim;


typedef struct {
  int thread_lo;
  int thread_hi;
  pthread_t thread_id;
} thread_info_t;

thread_info_t thread_info[100];

double get_current_time()
{
  static int start = 0, startu = 0;
  struct timeval tval;
  double result;
  

  if (gettimeofday(&tval, NULL) == -1)
    result = -1.0;
  else if(!start) {
    start = tval.tv_sec;
    startu = tval.tv_usec;
    result = 0.0;
  }
  else
    result = (double) (tval.tv_sec - start) + 1.0e-6*(tval.tv_usec - startu);
  
  return result;
}

double multiply_row_by_column (double **mat1, int row, double **mat2, int col)
{
  int k;
  double sum=0;
  for (k=0; k<dim; k++)
    sum += mat1[row][k] * mat2[k][col];
}


void multiply_row_by_matrix (double **mat1, int row, double **mat2, double **res)
{
  for (int col=0; col<dim; col++)
    res[row][col] = multiply_row_by_column (mat1, row, mat2, col);
}

static void* inner_thread_function (void *arg)
{
  inner_thread_info_t *info = (inner_thread_info_t *) arg;
  double **mat1 = info->mat1;
  double **mat2 = info->mat2;
  int ind = info->ind;
  double **res = info->res;

  multiply_row_by_matrix(mat1, ind, mat2, res);

  return NULL;
}

static void *thread_function (void* arg)
{
  
  /*
   * Initializing performance counters
   */
  thread_info_t * info = (thread_info_t *) arg;
  int lo_bound = info -> thread_lo;
  int hi_bound = info -> thread_hi;

  pthread_attr_t attr;
  pthread_t inner_thread[100];
  status = pthread_attr_init(&attr);
  if (status != 0) {
    fprintf (stderr, "Error creating thread attribute set!\n");
    exit(1);
  }
  
  for (unsigned int i = lo_bound; i < hi_bound; i++){
	  multiply_row_by_matrix(a, i, b, res);
  }
  
  return (void *)0;
}

int main (int argc, char *argv[])
{
  if (argc<3) {
    fprintf (stderr, "Usage: matmult_pthreads <num_workers> <matrix_dim>\n");
    exit(1);
  }
  int dim = atoi(argv[2]);
  int num_workers = atoi(argv[1]);
  
  a = (double **) malloc (sizeof(double *) * dim);
  b = (double **) malloc (sizeof(double *) * dim);
  res = (double **) malloc (sizeof(double *) * dim);
  for (int i=0; i<dim; i++) {
    a[i] = (double *) malloc (sizeof(double) * dim);
    b[i] = (double *) malloc (sizeof(double) * dim);
    res[i] = (double *) malloc (sizeof(double) * dim);
    for (int j=0; j<dim; j++) {
      a[i][j] = 42.0;
      b[i][j] = 42.0;
    }
  }

  double t1 = get_current_time();
  
  thread_count = num_workers;

  pthread_attr_t attr;
  int status;
  int i;
  
  /*
   * The detach state attribute determines whether a thread created
   * using the thread attributes object 'attr' will be created in a joinable or a
   * detach state.
   *
   */
  status = pthread_attr_init(&attr);
  if (status != 0) {
    fprintf (stderr, "Error creating thread attribute set!\n");
    exit(1);
  }
  
  int chunk_size = dim / thread_count;
  int extra_pool = dim - (thread_count * chunk_size);
  int count = 0;
  
  for (unsigned int thread_ind = 0; thread_ind < thread_count; thread_ind++) {
    int extra;
    if (extra_pool>0) {
      extra = 1;
      extra_pool--;
    } else {
      extra = 0;
    }
    int chunk = chunk_size + extra;
    thread_info[thread_ind].thread_lo = count;
    thread_info[thread_ind].thread_hi = count + chunk;
    count += chunk;
    
    status = pthread_create(&thread_info[thread_ind].thread_id, &attr, &thread_function, &thread_info[thread_ind]);
  }

  for (unsigned int thread_ind = 0; thread_ind < thread_count; thread_ind++) 
    pthread_join(thread_info[thread_ind].thread_id, NULL);

  pthread_exit ( NULL );        /* Let threads finish */


}




