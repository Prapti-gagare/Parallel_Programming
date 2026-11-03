#include<iostream>
#include<vector>
#include<mpi.h>
#include<fstream>
using namespace std;
int main(int argc,char * argv[])
{
MPI_Init(&argc , &argv);
MPI_Status status;
int rank,size;
MPI_Comm_rank(MPI_COMM_WORLD,&rank);
MPI_Comm_size(MPI_COMM_WORLD,&size);
if(size%2!=0)
{
    if(rank==0)
    {
        cout<<"Error: Number of processors must be even!"<<endl;
    }

    MPI_Finalize();
    return 1;
}
int n=0;
vector<int>A;
if(rank==0)
{
ifstream fin("input.txt");
fin>>n;
fin.close();
A.resize(n);
for(int i=0;i<n;i++)
{
   A[i]=i+1;
}
}
MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
int part=n/size;
int rem = n%size;
int local_n = part + (rank < rem);
vector<int>local_A(local_n);
if(rank==0)
{ 
   int offset=0;
   for(int i=1;i<size;i++)
     {
        offset += part + (i-1 < rem);
        int count = part + (i < rem);
        MPI_Send(A.data() + offset,count,MPI_INT,i,0,MPI_COMM_WORLD);
     }
 int local_sum=0;
 for(int i=0;i<local_n;i++)
    {
      local_A[i]=A[i];
     local_sum +=local_A[i];
    }
   cout<<"local sum on rank"<<rank<<"="<<local_sum<<endl;
   long long globle_sum=local_sum;
   long long temp;
   for(int i=1;i<size;i++)
    {
    MPI_Recv(&temp,1,MPI_LONG_LONG,i,0,MPI_COMM_WORLD, &status);
    globle_sum+=temp;
     }
cout<<"sum of"<<n<<"natural number is"<<globle_sum<<endl;
ofstream fout("Output.txt");
fout<<"Number of natural numbers is"<<n<<endl;
fout<<"Number of processors is"<<size<<endl;
fout<<"Globle sum="<<globle_sum<<endl;
fout.close();
}
    else
   {
     MPI_Recv(local_A.data(),local_n,MPI_INT,0,0,MPI_COMM_WORLD, &status);
     long long local_sum=0;
for(int i=0;i<local_n;i++)
{
local_sum+=local_A[i];
}

cout<<"local sum on rank"<<rank<<"="<<local_sum<<endl;
        MPI_Send(&local_sum,1,MPI_LONG_LONG,0,0,MPI_COMM_WORLD);
}

MPI_Finalize();
return 0;
}