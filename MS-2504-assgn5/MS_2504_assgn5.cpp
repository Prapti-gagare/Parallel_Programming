
#include<iostream>
#include<mpi.h>
#include<vector>
#include<fstream>
#include<iomanip>
#include<cmath>
#include<climits>
#include<algorithm>
using namespace std;

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = 0;
    int valid = 1;
    vector<double> A;

    if(rank == 0)
    {
        ifstream fin("input.txt");

        if(!fin || !(fin >> n) || n <= 0 || size >= n)
        {
            valid = 0;
        }
        else
        {
            A.resize(n * (n + 1));

            for(int i = 0; i < n * (n + 1); i++)
            {
                if(!(fin >> A[i]))
                {
                    valid = 0;
                    break;
                }
            }
        }

        fin.close();
    }

    MPI_Bcast(&valid, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(!valid)
    {
        if(rank == 0)
            cout << "Invalid input file or processor count!" << endl;

        MPI_Finalize();
        return 0;
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int width = n + 1;

    vector<int> rows(size), displs(size);
    vector<int> sendcounts(size), senddispls(size);

    int part = n / size;
    int rem = n % size;

    int offset = 0;

    for(int i = 0; i < size; i++)
    {
        rows[i] = part + (i < rem);
        displs[i] = offset;

        sendcounts[i] = rows[i] * width;
        senddispls[i] = offset * width;

        offset += rows[i];
    }

    int local_rows = rows[rank];

    vector<double> local_A(local_rows * width);

    MPI_Scatterv(
        A.data(), sendcounts.data(), senddispls.data(),
        MPI_DOUBLE,
        local_A.data(), local_rows * width,
        MPI_DOUBLE, 0, MPI_COMM_WORLD
    );

    auto owner = [&](int row)
    {
        for(int r = 0; r < size; r++)
        {
            if(row >= displs[r] &&
               row < displs[r] + rows[r])
                return r;
        }
        return -1;
    };

    vector<double> pivotrow(width);
    vector<double> temp(width);

    for(int k = 0; k < n; k++)
    {
        struct
        {
            double value;
            int index;
        } local_max, global_max;

        local_max.value = -1.0;
        local_max.index = INT_MAX;

        for(int i = 0; i < local_rows; i++)
        {
            int global_row = displs[rank] + i;

            if(global_row >= k)
            {
                double value = fabs(local_A[i * width + k]);

                if(value > local_max.value)
                {
                    local_max.value = value;
                    local_max.index = global_row;
                }
            }
        }

        MPI_Allreduce(
            &local_max, &global_max,
            1, MPI_DOUBLE_INT, MPI_MAXLOC,
            MPI_COMM_WORLD
        );

        int pivot_row = global_max.index;

        if(global_max.value < 1e-12)
        {
            if(rank == 0)
                cout << "Matrix is singular. No unique solution." << endl;

            break;
        }

        int owner_k = owner(k);
        int owner_p = owner(pivot_row);

        if(owner_k == owner_p)
        {
            if(rank == owner_k && pivot_row != k)
            {
                int r1 = k - displs[rank];
                int r2 = pivot_row - displs[rank];

                for(int j = 0; j < width; j++)
                    swap(local_A[r1 * width + j],
                         local_A[r2 * width + j]);
            }
        }
        else if(rank == owner_k || rank == owner_p)
        {
            int my_row;

            if(rank == owner_k)
                my_row = k - displs[rank];
            else
                my_row = pivot_row - displs[rank];

            MPI_Sendrecv(
                local_A.data() + my_row * width,
                width, MPI_DOUBLE, 
                (rank == owner_k ? owner_p : owner_k), 0,
                temp.data(), width, MPI_DOUBLE,
                (rank == owner_k ? owner_p : owner_k), 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );

            for(int j = 0; j < width; j++)
                local_A[my_row * width + j] = temp[j];
        }

        if(rank == owner_k)
        {
            int local_pivot = k - displs[rank];

            for(int j = 0; j < width; j++)
                pivotrow[j] = local_A[local_pivot * width + j];
        }

        MPI_Bcast(
            pivotrow.data(), width, MPI_DOUBLE,
            owner_k, MPI_COMM_WORLD
        );

        for(int i = 0; i < local_rows; i++)
        {
            int global_row = displs[rank] + i;

            if(global_row > k)
            {
                double factor =
                    local_A[i * width + k] / pivotrow[k];

                local_A[i * width + k] = 0;

                for(int j = k + 1; j < width; j++)
                {
                    local_A[i * width + j] -=
                        factor * pivotrow[j];
                }
            }
        }
    }

    MPI_Gatherv(
        local_A.data(), local_rows * width,
        MPI_DOUBLE,
        A.data(), sendcounts.data(), senddispls.data(),
        MPI_DOUBLE, 0, MPI_COMM_WORLD
    );

    if(rank == 0)
    {
        ofstream fout("output.txt");

        cout << "\nUpper triangular matrix:\n";
        fout << "Upper triangular matrix:\n";

        for(int i = 0; i < n; i++)
        {
            for(int j = 0; j < width; j++)
            {
                cout << setw(12) << setprecision(6)
                     << A[i * width + j] << " ";

                fout << setw(12) << setprecision(6)
                     << A[i * width + j] << " ";
            }

            cout << endl;
            fout << endl;
        }

        vector<double> x(n);

        bool singular = false;

        for(int i = n - 1; i >= 0; i--)
        {
            double sum = A[i * width + n];

            if(fabs(A[i * width + i]) < 1e-12)
            {
                singular = true;
                break;
            }

            for(int j = i + 1; j < n; j++)
                sum -= A[i * width + j] * x[j];

            x[i] = sum / A[i * width + i];
        }

        if(!singular)
        {
            cout << "\nSolution:\n";
            fout << "\nSolution:\n";

            for(int i = 0; i < n; i++)
            {
                cout << "x" << i + 1 << " = " << x[i] << endl;
                fout << "x" << i + 1 << " = " << x[i] << endl;
            }
        }

        fout.close();
    }

    MPI_Finalize();
    return 0;
}
