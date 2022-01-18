#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>

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

void createTasksFile(int taskCount, int minTime, int maxTime)
{
    if(minTime < 0 || maxTime <= 0 || minTime > maxTime)
    {
        cout << "Time error!" << endl;
        return;
    }
    srand(time(NULL));
    ofstream file("tasks.txt");
    if(!file.is_open())
    {
        cout << "Create file error!" << endl;
        return;
    }
    for(int i = 0; i < taskCount; i++)
        file << "command" << i + 1 << " " << rand() % (maxTime - minTime) + minTime << endl;
    file.close();
}

vector<pair<string, int>> getTasks()
{
    ifstream file("tasks.txt");
    if(!file.is_open())
    {
        cout << "Read file error!" << endl;
        return {};
    }
    vector<pair<string, int>> times;
    string str;
    pair<string, int> command;
    while(getline(file, str))
    {
        int findSpace = str.find(' ');
        command.first = str.substr(0, findSpace);
        command.second = atoi(str.substr(findSpace + 1, str.length() - findSpace).c_str());
        times.push_back(command);
    }
    file.close();
    return times;
}

int main(int argc, char * argv[])
{
    if(argc != 11 && !cmdOptionsExists(argv, argv+argc, {"--p", "--n", "--min", "--max"}))
    {
        cout << "Input error!" << endl;
        return -1;
    }

    int childProcessCount, taskCount, minTime, maxTime;
    if(childProcessCount = atoi(cmdAfterKey(argv, argv+argc, "--p").c_str()); childProcessCount == 0)
    {
        cout << "Child process count error!" << endl;
        return -1;
    }
    if(taskCount = atoi(cmdAfterKey(argv, argv+argc, "--n").c_str()); taskCount == 0)
    {
        cout << "Task count error!" << endl;
        return -1;
    }
    if(minTime = atoi(cmdAfterKey(argv, argv+argc, "--min").c_str()); minTime == 0)
    {
        cout << "Min time error!" << endl;
        return -1;
    }
    if(maxTime = atoi(cmdAfterKey(argv, argv+argc, "--max").c_str()); maxTime == 0)
    {
        cout << "Max time error!" << endl;
        return -1;
    }

    createTasksFile(taskCount, minTime, maxTime);
    vector<pair<string, int>> commands = getTasks();

    pid_t processes[childProcessCount];
    pid_t mainProcess = getpid();
    cout << "Main process: " << mainProcess << endl;
    int i = 0;
    int parentToChildren[2];
    int childrenToParent[2];
    int empty = -1;
    pipe(parentToChildren);
    pipe(childrenToParent);

    while(i < childProcessCount)
    {
        if(getpid() == mainProcess)
            processes[i++] = fork();
        else
            break;
    }
    if(getpid() == mainProcess)
    {
        close(parentToChildren[0]);
        close(childrenToParent[1]);
        uint i = 0;
        write(parentToChildren[1], &i, sizeof(int));
        i++;
        string buf;
        while(read(childrenToParent[0], &buf, sizeof(int)))
        {
            if (i == commands.size())
            {
                for(int j = 0; j < childProcessCount; j++)
                    write(parentToChildren[1], &empty, sizeof(int));
                break;
            }
            else
            {
                write(parentToChildren[1], &i, sizeof(int));
                ++i;
            }
        }
        close(childrenToParent[0]);
    }
    else
    {
        close(parentToChildren[1]);
        close(childrenToParent[0]);
        int buf;
        while(read(parentToChildren[0], &buf, sizeof(int)))
        {
            if(buf == -1)
                break;

            write(childrenToParent[1], &empty, sizeof(int));
            sleep(commands[buf].second/100.);
            cout << "Process: " << getpid() << " did " << commands[buf].first << " " <<
                    commands[buf].second << " units of time" << endl;
        }
        close(parentToChildren[0]);
        close(childrenToParent[1]);
    }
    int status;
    if(mainProcess == getpid())
    {
        for(int j = 0; j < childProcessCount;)
        {
            waitpid(processes[j], &status, 0);
            if(WIFEXITED(status))
                j++;
        }
        close(parentToChildren[1]);
    }

    return 0;
}