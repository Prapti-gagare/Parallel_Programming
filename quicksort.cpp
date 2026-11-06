#include<iostream>
#include<mpi.h>
using namespace std;

int partition(int A[],int s,int e)
{

  int pivot = A[s];
  int i=s+1;

  for(int j=s+1;j<=e;j++)
  {
   if(A[j]<pivot)
   {
    swap(A[i],A[j]);
    i++;
   }
  }
    swap(A[s],A[i-1]);
  return i-1;
}

void qs(int A[],int s,int e)
{
  if(s<e)
   {
    int p=partition(A,s,e);
    qs(A,s,p-1);
    qs(A,p+1,e);
   }
}

int main(int argc,char * argv[])
{
   MPI_Init(&argc,&argv);

   int rank,size,pivot,recev_size;

   MPI_Comm_rank(MPI_COMM_WORLD,&rank);0
   MPI_Comm_size(MPI_COMM_WORLD,&size);

   int n = 16;
   int A[16]={13,7,15,2,18,11,5,29,1,20,23,9,26,4,17,38};

    int B[16];
    int part = n/size;

    MPI_Scatter(A, part, MPI_INT,A,part,MPI_INT,0,MPI_COMM_WORLD);
    qs(B,0,part-1);

   MPI_Gather(A,part,MPI_INT,B,part,MPI_INT,0,MPI_COMM_WORLD);

   if(rank==0)
   {
      qs(A,0,n-1);
      cout<<"sorted Array";
      for(int i=0;i<n;i++)
       {
        cout<<A[i]<<" ";
       }
    cout<<endl;
   }

MPI_Finalize();
return 0;
}

