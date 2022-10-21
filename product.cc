#include <iostream>
#include <tbb/tbb.h>
#include <unistd.h>
using namespace std;

int** create_matrix(int size){
    int** matrix = new int*[size];
    for(int i=0; i<size; ++i){
        matrix[i] = new int[size];
    }
    return matrix;
}
void initialize_matrix(int ** matrix, int size){
    for(int i=0; i<size; ++i){
        for(int j=0; j<size; ++j){
            matrix[i][j] = rand() % size;
        }
    }
}
void zero_matrix(int ** matrix, int size){
    for(int i=0; i<size; ++i){
        for(int j=0; j<size; ++j){
            matrix[i][j] = 0;
        }
    }
}
void print(int **A, int size){
    for(int i=0; i<size; ++i){
        for(int j=0; j<size; ++j){
            cout << A[i][j] << " ";
        }
        cout << endl;
    }
}
// matrix product with loop order ijk
void product_ijk(int** A, int** B, int** C, int size){
    int sum;
    for(int i=0; i<size; ++i){
        for(int j=0; j<size; j++){
            sum = C[i][j];
            for(int k=0; k<size; ++k){
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}
// matrix product using blocking for loop order ijk
void product_blocking(int** A, int** B, int** C, int size, int block){
 int sum;
 int en = block*(size/block);/* Amount that fits evenly into blocks */      
 for (int kk = 0; kk < en; kk += block) {
   for (int jj = 0; jj < en; jj += block) {
     for (int i = 0; i < size; i++) {
       for (int j = jj; j < jj + block; j++) {
         sum = C[i][j];
         for (int k = kk; k < kk + block; k++) {
          sum += A[i][k]*B[k][j];
         }
         C[i][j] = sum;
       }
     }
   }
 }
}
void usage() {
  using std::cout;
  cout << "Matrix Product\n";
  cout << "  Usage: product [options]\n";
  cout << "    -n <int> : number of rows/columns in the matrix (default 256)\n";
  cout << "    -b <int> : size of the block (default 16)\n";
  cout << "    -t <int> : number of threads (default 4)\n";
  cout << "    -c       : toggle verifying the result (default false)\n";
  cout << "    -h       : print this message\n";
}
void checkCorrectness(int** result1, int** result2, int size){
  for(int i=0; i<size; i++){
    for(int j=0; j<size; j++){
      if(result1[i][j] != result2[i][j]){
        std::cout << "Check correctness failed for elements (" << i << ", " << j << "): " 
             << result1[i][j] << "!=" << result2[i][j] << std::endl;
             return;
      }
    } 
  }
  std::cout << "Check correctness passed." << std::endl;

}
int main(int argc, char** argv){
    int size = 256;       // size of the matrix
    int block = 16;       // block size
    bool check = false;   // verify the results?

    // Parse the command line options:
    int o;
    while ((o = getopt(argc, argv, "n:b:ch")) != -1) {
      switch (o) {
        case 'n':
          size = atoi(optarg);
          break;
        case 'b':
          block = atoi(optarg);
          break;
        case 'c':
          check = !check;
          break;
        case 'h':
          usage();
          break;
        default:
          usage();
          exit(-1);
      }
    }
    int **A, **B, **C1, **C2;
    A = create_matrix(size);
    initialize_matrix(A, size);
    B = create_matrix(size);
    initialize_matrix(B, size);
    C1 = create_matrix(size);
    zero_matrix(C1, size);
    C2 = create_matrix(size);
    zero_matrix(C2, size);
    

    // Calling product ijk
    auto starttime = tbb::tick_count::now();
    product_ijk(A, B, C1, size);
    auto endtime = tbb::tick_count::now();
    std::cout << "Time (Matrix Product ijk): " << (endtime - starttime).seconds() << " seconds" << std::endl;
    
    // Calling product with blocking (block size = block)
    starttime = tbb::tick_count::now();
    product_blocking(A, B, C2, size, block);
    endtime = tbb::tick_count::now();
    std::cout << "Time (block size = " << block << "): " << (endtime - starttime).seconds() << " seconds" << std::endl;
    
    if(check){
      checkCorrectness(C1, C2, size);
    }
    return 0;
}