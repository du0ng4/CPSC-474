// Andy Duong
// CPSC 474 - Project 1

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

void calculate(std::vector<std::vector<std::string>> array, int n, int m);
void verify(std::vector<std::vector<std::string>> array, int n, int m);

template <class T>
void print_array(T array, int n, int m);
template <class T>
void output_array_to_file(T array, int n, int m);
template <class T, class T2>
bool contains(T array, int n, int m, T2 value);

int main()
{
    std::ifstream inFile;
    inFile.open("input.txt");

    // Find out number of processes from user
    int n, m;
    std::cout << "Enter the number of processes N: ";
    std::cin >> n;

    // Read file into matrix
    std::vector<std::vector<std::string>> input(n);
    std::string line;
    char* char_array;
    char* token;
    int i = 0;
    while (getline(inFile, line)) {
        std::vector<std::string> temp;
        char_array = &line[0];
        token = strtok(char_array, " ");
        while (token != NULL) {
            temp.push_back(token);
            token = strtok(NULL, " ");
        }
        m = temp.size();
        input[i] = temp;
        i++;
    }

    // get user input on which algorithm to run
    int algorithm;
    do {
        std::cout << "Enter 1 for Calculate or 2 for Verify: ";
        std::cin >> algorithm;
    } while (algorithm != 1 && algorithm != 2);

    // run the algorithm
    if (algorithm == 1) {
        calculate(input, n, m);
    }
    else {
        verify(input, n, m);
    }

    inFile.close();

    return 0;
}

void calculate(std::vector<std::vector<std::string>> input, int n, int m) {
    // map for easier checking
    std::unordered_map<std::string, int> values;

    // result matrix
    std::vector<std::vector<int>> result;
    result.resize(n, std::vector<int>(m, -1));

    // continues to loop until all events are given a value
    while (contains(result, n, m, -1)) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                std::string current = input[i][j];
                // process is done executing
                if (current == "NULL") {
                    result[i][j] = 0;
                    continue;
                }
                // is either a internal event or send event
                if (current[0] != 'r') {
                    int temp;
                    if (j == 0) {
                        temp = j + 1;
                    }
                    else {
                        temp = std::max(j, result[i][j - 1]) + 1;
                    }
                    result[i][j] = temp;
                    values[current] = temp;

                }
                // is a receive event
                else {
                    // we have seen the respective send event
                    if (values.count(std::string("s") + current[1])) {
                        int temp;
                        if (j == 0) {
                            temp = std::max(values[std::string("s") + current[1]], j) + 1;
                        }
                        else {
                            temp = std::max(values[std::string("s") + current[1]], result[i][j - 1]) + 1;
                        }
                        result[i][j] = temp;
                        values[current] = temp;
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }

    print_array(result, n, m);
    output_array_to_file(result, n, m);

}

void verify(std::vector<std::vector<std::string>> input, int n, int m) {
    std::vector<std::vector<std::string>> result;
    result.resize(n, std::vector<std::string>(m, "0"));
    char current_letter = 'a'; // used later to fill internal events

    std::vector<std::vector<int>> receives;

    // look for obvious problems and gaps
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            // recognize process has ended
            if (input[i][j][0] == '0') {
                result[i][j] = "NULL";
                continue;
            }

            // if current lc value is less than previous
            // or if process continues after already ended
            if (j != 0 && (input[i][j][0] <= input[i][std::max(0, j - 1)][0] || input[i][std::max(0, j - 1)][0] == '0')) {
                std::cout << "INCORRECT" << std::endl;
                goto functionExit;
            }

            // identify gaps
            if (j != 0 && (input[i][j][0] != input[i][std::max(0, j - 1)][0] + 1)) {
                receives.push_back({ i, j });
            }
            else if (j == 0 && input[i][j][0] != '1') {
                receives.push_back({ i, j });
            }
        }
    }

    // find respective sends for each gap
    for (int i = 0; i < receives.size(); i++) {
        // find location with receive value - 1
        char send_value = input[receives[i][0]][receives[i][1]][0] - 1;
        bool found = false;
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < m; k++) {
                if (input[j][k][0] == send_value) {
                    found = true;
                    result[j][k] = std::string("s") + std::to_string(i+1);
                    result[receives[i][0]][receives[i][1]] = std::string("r") + std::to_string(i+1);
                    goto stopSearch;
                }
            }
        }
        stopSearch:
        if (!found) {
            std::cout << "INCORRECT" << std::endl;
            goto functionExit;
        }
    }

    // rest can be considered internal events
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (result[i][j][0] == '0') {
                result[i][j] = current_letter;
                current_letter++;
            }
        }
    }

    print_array(result, n, m);
    output_array_to_file(result, n, m);

    functionExit:
    std::cout << std::endl;
}

// helper functions

// returns true if given array contains specific value, false otherwise
template <class T, class T2>
bool contains(T array, int n, int m, T2 value) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (array[i][j] == value) {
                return true;
            }
        }
    }
    return false;
}

// prints out given array
template <class T>
void print_array(T array, int n, int m) {
    std::cout << std::endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            std::cout << array[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

// outputs array to file output.txt
template <class T>
void output_array_to_file(T array, int n, int m) {
    std::ofstream outFile;
    outFile.open("output.txt");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            outFile << array[i][j] << " ";
        }
        outFile << "\n";
    }
    outFile.close();
}
