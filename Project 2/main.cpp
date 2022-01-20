#include <stdio.h>
#include <mpi.h>

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

// linked list implementation
struct node
{
    int n_location;
    int m_location;
    int value;
    node *next;
};

class linked_list
{
    private:
        node *head;
        node *tail;
    public:
        linked_list() {
            head = NULL;
            tail = NULL;
        }
        void insert(int n, int m, int val) {
            node *tmp = new node;
            tmp->n_location = n;
            tmp->m_location = m;
            tmp->value = val;
            tmp->next = NULL;

            if (head == NULL) {
                head = tmp;
                tail = tmp;
            }
            else {
                tail->next = tmp;
                tail = tail->next;
            }
        }
        void print_list() {
            node *current = head;
            while (current != NULL) {
                std::cout << "Value : " << current->value;
                std::cout << " Row: " << current->n_location;
                std::cout << " Column: " << current->m_location << std::endl; 
                current = current->next; 
            }
        }
};

// start of program
int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // create linked list
    linked_list list;

    // for each process
    int array[100] = {}; // simple 1D array to hold entire matrix
    int elements_per_process = 0; // number of elements each process checks (excluding last process)
    int num_nonzero = 0; // number of nonzero elements in each matrix
    int root_nonzero[50] = {}; // array for root node to keep track of nonzero elements for each process
    int total_nonzero; // total number of non zero elements
    int result[50] = {}; // buffer to keep track of the nonzero elements and their position in each proecess
    int final[50] = {}; // buffer to keep reduce result arrays to


    // declare variables for matrix info
    int num_rows, num_columns, num_elements;

    if (rank == 0) {
        // read in matrix from file
        std::ifstream inFile;
        inFile.open("input.txt");

        std::string line;
        char * char_array;
        char * token;

        num_elements = 0;
        num_rows = 0;
        num_columns = 0;

        while (getline(inFile, line)) {
            char_array = &line[0];
            token = strtok(char_array, " ");
            num_columns = 0;
            while (token != NULL) {
                array[num_elements] = atoi(token);
                token = strtok(NULL, " ");
                num_elements++;
                num_columns++;
            }
            num_rows++;
        }

        inFile.close();

        // calculate how much of matrix to send to each process
        elements_per_process = num_elements / size;
    }

    // broadcast array and related info
    MPI_Bcast(&num_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_columns, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&array, num_elements, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&elements_per_process, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    int start_index = rank * elements_per_process;
    for (int i = start_index; i < start_index + elements_per_process; i++) {
        if (array[i] != 0) {
            result[num_nonzero * 3] = array[i];
            result[num_nonzero * 3 + 1] = i / num_columns; // row location
            result[num_nonzero * 3 + 2] = i % num_columns; // column location
            num_nonzero++;
        }
    }
    if (rank == size - 1) {
        // finish remaining elements
        int computed_so_far = elements_per_process * (rank + 1);
        int remaining = num_elements - computed_so_far;
        for (int i = computed_so_far; i < num_elements; i++) {
            if (array[i] != 0) {
                result[num_nonzero * 3] = array[i];
                result[num_nonzero * 3 + 1] = i / num_columns; // row location
                result[num_nonzero * 3 + 2] = i % num_columns; // column location
                num_nonzero++; 
            }
        }
    }

    // gather number of nonzero elements for each process
    MPI_Gather(&num_nonzero, 1, MPI_INT, &root_nonzero, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // gather results to root process
    MPI_Reduce(&num_nonzero, &total_nonzero, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // process info for gatherv
    int *displacement = new int[size];
    for (int i = 0; i < size; i++) {
        if (i == 0) {
            displacement[i] = 0;
        } else {
            displacement[i] = displacement[i - 1] + (root_nonzero[i - 1] * 3);
        }
    }
    for (int i = 0; i < size; i++) {
        root_nonzero[i] *= 3;
    }

    MPI_Gatherv(&result, num_nonzero * 3, MPI_INT, final, root_nonzero, displacement, MPI_INT, 0, MPI_COMM_WORLD);

    // construct linked list
    if (rank == 0) {
        for (int i = 0; i < total_nonzero * 3; i+=3) {
            list.insert(final[i+1], final[i+2], final[i]);
        }
        list.print_list();
    }
    
    MPI_Finalize();
    return 0;
}
