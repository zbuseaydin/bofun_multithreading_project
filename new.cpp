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

struct customer
{
    int amount;
    string company;
    int customerNo;
};

vector<string> inputLines;
vector<bool> prepaymentDone;
queue<customer> vendingQueues[10];

int companies[5];
int remainingCus;

ofstream output;

void *customerRoutine(void *itr);
void *vtmRoutine(void *itr);
void performPayment(string company, int amount);

int main(int argc, char *argv[])
{
    // initialize the mutexes for each vending ticket machine
    for (int i = 0; i < 10; i++)
        pthread_mutex_init(&vtmMutexes[i], 0);

    // initialize the mutexes for each company
    for (int i = 0; i < 5; i++)
        pthread_mutex_init(&companyMutexes[i], 0);

    pthread_mutex_init(&finishedMutex, 0); // initialize the mutex for remainingCus variable
    pthread_mutex_init(&outputLock, 0);    // initialize the mutex for file write

    ifstream input(argv[1]); // input file

    // create output file with the name <input_file_name>_log.txt
    int nameLen = strlen(argv[1]);
    char fileName[nameLen + 4];
    strncpy(fileName, argv[1], nameLen - 4);
    strcat(fileName, "_log.txt");
    output.open(fileName);

    int numCustomers;
    input >> numCustomers;
    remainingCus = numCustomers; // keep track of the number of remaining customers
    string line;
    while (input.peek() != EOF) // read the input file and put the lines of customer
    {                           // information into the global inputLines vector
        getline(input, line);
        if (line.size() < 4)
            continue;
        inputLines.push_back(line);
    }

    for (int i = 0; i < numCustomers; i++) // initially none of the customers are done
        prepaymentDone.push_back(false);

    int *itr; // pointer for sending the loop variable to the threads

    // creating the vending ticket machine threads
    pthread_t vtmIDs[10];
    for (int i = 0; i < 10; i++)
    {
        itr = (int *)malloc(sizeof(int));
        *itr = i;
        pthread_create(&(vtmIDs[i]), NULL, vtmRoutine, (void *)itr);
    }

    // create customer threads
    pthread_t customerIds[numCustomers];
    for (int i = 0; i < numCustomers; i++)
    {
        itr = (int *)malloc(sizeof(int));
        *itr = i;
        pthread_create(&(customerIds[i]), NULL, customerRoutine, (void *)itr);
    }

    for (int i = 0; i < numCustomers; i++)
        pthread_join(customerIds[i], NULL);

    for (int i = 0; i < 10; i++)
        pthread_join(vtmIDs[i], NULL);

    free(itr);

    // destroy all mutexes
    for (int i = 0; i < 10; i++)
        pthread_mutex_destroy(&vtmMutexes[i]);
    for (int i = 0; i < 5; i++)
        pthread_mutex_destroy(&companyMutexes[i]);
    pthread_mutex_destroy(&finishedMutex);
    pthread_mutex_destroy(&outputLock);

    output << "[Main]: All prepayments are completed.\n";
    output << "[Main]: Kevin: " << companies[0] << endl;
    output << "[Main]: Bob: " << companies[1] << endl;
    output << "[Main]: Stuart: " << companies[2] << endl;
    output << "[Main]: Otto: " << companies[3] << endl;
    output << "[Main]: Dave: " << companies[4] << endl;
}

void *customerRoutine(void *itr)
{
    int cusNo = *(int *)itr;
    stringstream line(inputLines[cusNo]);
    string currentInfo;
    customer cus;

    // input is given as sleep time, vending machine number, company name, amount of money
    getline(line, currentInfo, ','); // sleep for the given number of milliseconds
    sleep(stoi(currentInfo) / 1000);

    getline(line, currentInfo, ',');
    int vtmNo = stoi(currentInfo) - 1;

    getline(line, currentInfo, ',');
    cus.company = currentInfo;

    getline(line, currentInfo, ',');
    cus.amount = stoi(currentInfo);

    cus.customerNo = cusNo;

    // can put to the waiting queue of the vending machine only if
    // the thread has the lock
    pthread_mutex_lock(&vtmMutexes[vtmNo]);
    vendingQueues[vtmNo].push(cus);
    pthread_mutex_unlock(&vtmMutexes[vtmNo]);

    // wait until the prepayment is done before exit
    while (!prepaymentDone[cusNo]);
    pthread_exit(0);
}

void *vtmRoutine(void *itr)
{
    int vmNo = *(int *)itr;

    while (true) // run until all the customers are served
    {
        // serve if there is a customer waiting
        if (!vendingQueues[vmNo].empty())
        {
            pthread_mutex_lock(&vtmMutexes[vmNo]);

            // get the information of the customer that is being served
            int amount = vendingQueues[vmNo].front().amount;
            string company = vendingQueues[vmNo].front().company;
            int customerNo = vendingQueues[vmNo].front().customerNo;
            vendingQueues[vmNo].pop();

            performPayment(company, amount);
            prepaymentDone[customerNo] = true;

            pthread_mutex_unlock(&vtmMutexes[vmNo]);

            pthread_mutex_lock(&outputLock);
            output << "[vtm" << vmNo << "]: ";
            output << "Customer" << customerNo << ",";
            output << amount << "TL,";
            output << company << endl;
            pthread_mutex_unlock(&outputLock);

            pthread_mutex_lock(&finishedMutex);
            remainingCus--;
            pthread_mutex_unlock(&finishedMutex);
        }
        if (remainingCus <= 0)
            break;
    }
    pthread_exit(0);
}

void performPayment(string company, int amount)
{
    int companyNo;
    if (company == "Kevin")
        companyNo = 0;
    else if (company == "Bob")
        companyNo = 1;
    else if (company == "Stuart")
        companyNo = 2;
    else if (company == "Otto")
        companyNo = 3;
    else if (company == "Dave")
        companyNo = 4;

    pthread_mutex_lock(&companyMutexes[companyNo]);
    companies[companyNo] += amount;
    pthread_mutex_unlock(&companyMutexes[companyNo]);
}