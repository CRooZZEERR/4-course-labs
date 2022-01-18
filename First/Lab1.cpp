#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

bool cmdOptionsExists(char** begin, char** end, const vector<string>& options)
{
    for(const string& option : options)
        if(find(begin, end, option) == end)
            return false;
    return true;
}

string cmdAfterKey(char** begin, char** end, const std::string& option)
{
    if(find(begin, end, option) + 1 != end && *(find(begin, end, option) + 1)[0] != '-')
        return *(find(begin, end, option) + 1);
    return "";
}

string createInFile(string prefixIn, int vectorCount, int dimension)
{
    srand(time(NULL) * getpid());
    string filename = prefixIn + "_" + to_string(getpid()) + ".txt";
    ofstream file(filename);
    if(!file.is_open())
    {
        cout << "Create file error!" << endl;
        return "";
    }
    for(int j = 0; j < vectorCount; j++)
    {
        for(int k = 0; k < dimension; k++)
        {
            file << rand() % 10 << " ";
        }
        file << endl;
    }
    file.close();
    return filename;
}

void createOutFile(string prefixOut, int vectorCount, int dimension, string fileNameIn)
{
    ifstream fileIn(fileNameIn);
    if(!fileIn.is_open())
    {
        cout << "Create file error!" << endl;
        return;
    }
    vector<int> result(dimension, 0);
    int term;
    for(int i = 0; i < vectorCount; i++)
    {
        for(int j = 0; j < dimension; j++)
        {
            fileIn >> term;
            result[j] += term;
        }
    }
    fileIn.close();

    string filename = prefixOut + "_" + to_string(getpid()) + ".txt";
    ofstream fileOut(filename);
    if(!fileOut.is_open())
    {
        cout << "Create file error!" << endl;
        return;
    }
    for(int i = 0; i < dimension; i++)
        fileOut << result[i] << " ";

    fileOut.close();
}

int main(int argc, char * argv[])
{
    if(argc != 11 && !cmdOptionsExists(argv, argv+argc, {"-i", "-o", "-p", "-m", "-t"}))
    {
        cout << "Input error!" << endl;
        return -1;
    }

    string prefixIn, prefixOut;
    int fileCount, vectorCount, dimension;
    if(prefixIn = cmdAfterKey(argv, argv+argc, "-i"); prefixIn == "")
    {
        cout << "Prefix In error!" << endl;
        return -1;
    }
    if(prefixOut = cmdAfterKey(argv, argv+argc, "-o"); prefixOut == "")
    {
        cout << "Prefix Out error!" << endl;
        return -1;
    }
    if(fileCount = atoi(cmdAfterKey(argv, argv+argc, "-p").c_str()); fileCount == 0)
    {
        cout << "File count error!" << endl;
        return -1;

    }
    if(vectorCount = atoi(cmdAfterKey(argv, argv+argc, "-m").c_str()); vectorCount == 0)
    {
        cout << "Vector count error!" << endl;
        return -1;

    }
    if(dimension = atoi(cmdAfterKey(argv, argv+argc, "-t").c_str()); dimension == 0)
    {
        cout << "Dimesion error!" << endl;
        return -1;

    }

    pid_t processes[fileCount];
    pid_t mainProcess = getpid();
    int i = 0;
    while(i < fileCount)
    {
        if(getpid() == mainProcess)
            processes[i++] = fork();
        else
            break;
    }
    if(getpid() != mainProcess)
    {
        string fileNameIn = createInFile(prefixIn, vectorCount, dimension);
        createOutFile(prefixOut, vectorCount, dimension, fileNameIn);
    }

    int status;
    if(mainProcess == getpid())
    {
        for(int j = 0; j < fileCount;)
        {
            waitpid(processes[j], &status, 0);
            if(WIFEXITED(status))
                j++;
        }
    }

    return 0;
}