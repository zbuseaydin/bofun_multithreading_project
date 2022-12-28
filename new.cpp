#include <stdio.h>
#include <pthread.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <queue>
#include <string.h>
using namespace std;

pthread_mutex_t companyMutexes[5];
pthread_mutex_t vtmMutexes[10];
pthread_mutex_t finishedMutex;
pthread_mutex_t outputLock;

vector<string> inputLines;
vector<bool> prepaymentDone;

int companies[5];

int finished;
ofstream output;

struct customer{
    int amount;
    string company;
    int customerNo;
};

queue<customer> vendingQueues[10];

int kevin = 0;
int bob = 0;
int stuart = 0;
int otto = 0;
int dave = 0;

void* processInfo(void* itr);
void* paymentRoutine(void* itr);
void performPayment(string company, int amount);

int main(int argc, char *argv[]){
    int numCustomers;
    ifstream input(argv[1]);

    output.open("logFile");

    input >> numCustomers;
    finished = numCustomers;

    for(int i; i < 10; i++)
        pthread_mutex_init(&vtmMutexes[i], 0);

    for(int i=0; i<5; i++)
        pthread_mutex_init(&companyMutexes[i], 0);

    pthread_mutex_init(&finishedMutex, 0);
    pthread_mutex_init(&outputLock, 0);

    int *itr;
    pthread_t vendingIds[10];

    for (int i = 0; i < 10; i++)
    {
        itr = (int *)malloc(sizeof(int));
        *itr = i;
        pthread_create(&(vendingIds[i]), NULL, paymentRoutine, (void *)itr);
    }

    pthread_t customerIds[numCustomers];
    string line;
    while(input.peek()!=EOF)
    {
        getline(input, line);
        if(line.size()<4)
            continue;
        inputLines.push_back(line);
    }

    for(int i=0; i<numCustomers; i++)
        prepaymentDone.push_back(false);

    // create customer processes
    for (int i=0; i<numCustomers; i++)
    {
        itr = (int *)malloc(sizeof(int));
        *itr = i;
        pthread_create(&(customerIds[i]), NULL, processInfo, (void *)itr);
    }
    
    for (int i=0; i<numCustomers; i++)
        pthread_join(customerIds[i], NULL);

    for (int i = 0; i < 10; i++)
        pthread_join(vendingIds[i], NULL);
    
    for (int i = 0; i<10; i++)
        pthread_mutex_destroy(&vtmMutexes[i]);
    for (int i = 0; i<5; i++)
        pthread_mutex_destroy(&companyMutexes[i]);
    pthread_mutex_destroy(&finishedMutex);
    pthread_mutex_destroy(&outputLock);

    output << "All prepayments are completed.\n";
    output << "Kevin: ";
    output << kevin;
    output << "TL\n";
    output << "Bob: ";
    output << bob;
    output << "TL\n";
    output << "Stuart: ";
    output << stuart;
    output << "TL\n";
    output << "Otto: ";
    output << otto;
    output << "TL\n";
    output << "Dave: ";
    output << dave;
    output << "TL\n";
}

void* processInfo(void *itr)
{
    vector<string> infoList;
    string info;
    int cusNo = *(int*)itr;
    stringstream line(inputLines[cusNo]);

    while(getline(line, info, ',')){
        infoList.push_back(info);
    }
    string company = infoList[2];
    int amount = stoi(infoList[3]);
    
    int sleepTime = stoi(infoList[0]);
    sleep(sleepTime/1000);
    int vendingMachine = stoi(infoList[1])-1;

    pthread_mutex_lock(&vtmMutexes[vendingMachine]);
    customer cus;
    cus.amount = amount;
    cus.company = company;
    cus.customerNo = cusNo;
    vendingQueues[vendingMachine].push(cus);
    pthread_mutex_unlock(&vtmMutexes[vendingMachine]);

    while(!prepaymentDone[cusNo]);
    
    pthread_exit(0);
}

void* paymentRoutine(void *itr)
{
    int vmNo = *(int*) itr;

    while(true)
    {
        if(!vendingQueues[vmNo].empty()){
            pthread_mutex_lock(&vtmMutexes[vmNo]);

            int amount = vendingQueues[vmNo].front().amount;
            string company = vendingQueues[vmNo].front().company;
            int customerNo = vendingQueues[vmNo].front().customerNo;
            vendingQueues[vmNo].pop();
            performPayment(company, amount);
            prepaymentDone[customerNo] = true;
            pthread_mutex_unlock(&vtmMutexes[vmNo]);

            pthread_mutex_lock(&outputLock);
            output << "Customer";
            output << customerNo;
            output << ",";
            output << amount;
            output << "TL,";
            output << company;
            output << endl;
            pthread_mutex_unlock(&outputLock);

            pthread_mutex_lock(&finishedMutex);
            finished--;    
            pthread_mutex_unlock(&finishedMutex);
        }
        if(finished<=0){
            break;
        }
        
    }
    pthread_mutex_lock(&outputLock);
    cout << "exiting..";
    cout << vmNo;
    cout << endl;
    pthread_mutex_unlock(&outputLock);

    pthread_exit(0);
}

void performPayment(string company, int amount)
{   
    int companyNo;
    if(company == "Kevin")
        companyNo = 0;
    else if (company == "Bob")
        companyNo = 1;
    else if(company == "Stuart")
        companyNo = 2;
    else if(company == "Otto")
        companyNo = 3;
    else if(company == "Dave")
        companyNo = 4;
    pthread_mutex_lock(&companyMutexes[companyNo]);
    companies[companyNo] += amount;
    pthread_mutex_unlock(&companyMutexes[companyNo]);

}