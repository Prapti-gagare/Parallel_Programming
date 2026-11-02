#include<iostream>
#include<vector>
#include<mpi.h>
using namespace std;
int main(int argc,char * argv[])
{
MPI_Init(&argc , &argv);
MPI_Status status;
int rank,size;
MPI_Comm_rank(MPI_COMM_WORLD,&rank);
MPI_Comm_size(MPI_COMM_WORLD,&size);

int n=100;
vector<int>A(n);
int part = n/size;
vector<int>local_A(part);
if(rank==0)
{
   for(int i=0;i<n;i++)
     {
        A[i]=i+1;
     }
     int offset=part;
     for(int i=1;i<size;i++)
     {
        MPI_Send(A.data() + offset,part,MPI_INT,i,0,MPI_COMM_WORLD);
        offset=offset + part;
     }
 int local_sum=0;
 for(int i=0;i<part;i++)
    {
     local_sum +=A[i];
    }
   cout<<"local sum on rank"<<rank<<"="<<local_sum<<endl;
   int globle_sum=local_sum;
   int temp;
   for(int i=1;i<size;i++)
    {
    MPI_Recv(&temp,1,MPI_INT,i,0,MPI_COMM_WORLD, &status);
    globle_sum+=temp;
     }
cout<<"sum of"<<n<<"natural number is"<<globle_sum<<endl;
}
    else
   {
     MPI_Recv(local_A.data(),part,MPI_INT,0,0,MPI_COMM_WORLD, &status);
     int local_sum=0;
for(int i=0;i<part;i++)
{
local_sum+=local_A[i];
}

cout<<"local sum on rank"<<rank<<"="<<local_sum<<endl;
        MPI_Send(&local_sum,1,MPI_INT,0,0,MPI_COMM_WORLD);
}

MPI_Finalize();
return 0;
}