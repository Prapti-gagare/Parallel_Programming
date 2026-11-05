#include<iostream>
#include<mpi.h>
#include<vector>
using namespace std;
int main(int argc,char * argv[])
{
 MPI_Init(&argc,&argv);
 int rank,size;
 MPI_Comm_rank(MPI_COMM_WORLD,&rank);
 MPI_Comm_size(MPI_COMM_WORLD,&size);
 const int rows=4;
 const int cols=8;
 int local_cols=cols/size;
 vector<double>result(rows,0.0);
//rows=matrix.size();
//cols=matrix[0].size();
     double matrix[rows][cols]={
              {1,2,3,4,5,6,7,8},
              {9,10,11,12,13,14,15,16,},
              {17,18,19,20,21,22,23,24,},
              {25,26,27,28,29,30,31,32}
              };
vector<double>vec={2,4,6,8,3,9,4,5};
vector<double>local_row_part(local_cols);
vector<double>local_vec(local_cols);
   MPI_Scatter(vec.data(),local_cols,MPI_DOUBLE,local_vec.data(),local_cols,MPI_DOUBLE,0,MPI_COMM_WORLD);
   for(int i=0;i<rows;i++)
    {
     
   MPI_Scatter(matrix[i],local_cols,MPI_DOUBLE,local_row_part.data(),local_cols,MPI_DOUBLE,0,MPI_COMM_WORLD);
  double local_sum=0.0;
  for(int j=0;j<local_cols;j++)
{

   local_sum+=local_row_part[j]*local_vec[j];

    }
   MPI_Reduce(&local_sum,&result[i],1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
 }

if(rank == 0)
{
cout<<"result";
for(int i=0;i<rows;i++)
 {
   cout<<result[i] <<" ";
 }
 }
MPI_Finalize();
return 0;
}