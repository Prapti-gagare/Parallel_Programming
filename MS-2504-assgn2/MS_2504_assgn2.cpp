#include<iostream>
#include<mpi.h>
#include<cmath>
#include<iomanip>
#include<fstream>
using namespace std;

double f(double x)
{
    return x*x*exp(-x);
}

int main(int argc,char* argv[])
{
    MPI_Init(&argc,&argv);

    int rank,size;

    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    if(size%2!=0)
    {
        if(rank==0)
            cout<<"Error: Number of processors must be even!"<<endl;

        MPI_Finalize();
        return 1;
    }

    int intervals[3]={16,24,36};

    double a=0.0,b=4.0;

    ofstream fout;

    if(rank==0)
    {
        fout.open("output.txt");

        if(!fout)
        {
            cout<<"Unable to open output.txt"<<endl;
            MPI_Abort(MPI_COMM_WORLD,1);
        }

        fout<<fixed<<setprecision(8);
        fout<<"Simpson's 3/8 Rule"<<endl;
        fout<<"Processors: "<<size<<endl;
        fout<<"----------------------------------"<<endl;
    }

    for(int k=0;k<3;k++)
    {
        int n=intervals[k];

        if(n%3!=0)
        {
            if(rank==0)
            {
                fout<<"Intervals: "<<n<<endl;
                fout<<"Invalid: Intervals must be divisible by 3"<<endl;
                fout<<"----------------------------------"<<endl;
            }

            continue;
        }

        double h=(b-a)/n;

        // Total points = n+1
        int total_points=n+1;

        // Distribute points among processors
        int base=total_points/size;
        int rem=total_points%size;

        int local_n=base+(rank<rem?1:0);

        int start=rank*base+(rank<rem?rank:rem);

        double local_sum=0.0;

        for(int i=start;i<start+local_n;i++)
        {
            double x=a+i*h;

            if(i==0 || i==n)
                local_sum+=f(x);

            else if(i%3==0)
                local_sum+=2*f(x);

            else
                local_sum+=3*f(x);
        }

        double total_sum=0.0;

        MPI_Reduce(
            &local_sum,
            &total_sum,
            1,
            MPI_DOUBLE,
            MPI_SUM,
            0,
            MPI_COMM_WORLD
        );

        if(rank==0)
        {
            double result=(3*h/8)*total_sum;

            fout<<"Intervals: "<<n<<endl;
            fout<<"Integral: "<<result<<endl;
            fout<<"----------------------------------"<<endl;
        }
    }

    if(rank==0)
    {
        fout.close();
        cout<<"Results stored in output.txt"<<endl;
    }

    MPI_Finalize();

    return 0;
}