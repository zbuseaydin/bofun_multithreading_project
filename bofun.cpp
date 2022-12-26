#include <stdio.h>
#include <pthread.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
using namespace std;

pthread_mutex_t companyMutexes[5];
pthread_mutex_t customerMutexes[10];
vector<string> inputLines;

int finished;

pair<int, string> customers[10];

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

    input >> numCustomers;
    finished = numCustomers;

    for(int i,j = 0; i < 10; i++, j+=2)
        pthread_mutex_init(&customerMutexes[i], 0);

    for(int i=0; i<5; i++)
        pthread_mutex_init(&companyMutexes[i], 0);

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
        pthread_mutex_destroy(&customerMutexes[i]);
    for (int i = 0; i<5; i++)
        pthread_mutex_destroy(&companyMutexes[i]);

    cout << "All prepayments are completed.\n";
    cout << "Kevin: ";
    cout << kevin;
    cout << "TL\n";
    cout << "Bob: ";
    cout << bob;
    cout << "TL\n";
    cout << "Stuart: ";
    cout << stuart;
    cout << "TL\n";
    cout << "Otto: ";
    cout << otto;
    cout << "TL\n";
    cout << "Dave: ";
    cout << dave;
    cout << "TL\n";
}

void* processInfo(void *itr)
{
    vector<string> infoList;
    string info;
    stringstream line(inputLines[*(int*)itr]);

    while(getline(line, info, ',')){
        infoList.push_back(info);
    }
    string company = infoList[2];
    int amount = stoi(infoList[3]);
    cout << "Customer";
    cout << *(int*)itr+1;
    cout << ",";
    cout << amount;
    cout << "TL,";
    cout << company;
    cout << endl;

    int sleepTime = stoi(infoList[0]);
    sleep(sleepTime/1000);
    int vendingMachine = stoi(infoList[1])-1;
    pthread_mutex_lock(&customerMutexes[vendingMachine]);
    customers[vendingMachine].first = amount;
    customers[vendingMachine].second = company;
    pthread_mutex_unlock(&customerMutexes[vendingMachine]);
    
    pthread_exit(0);
}

void* paymentRoutine(void *itr)
{
    int customerNo = *(int*) itr;
    while(finished>0)
    {
        if(customers[customerNo].second!="")
        {
            pthread_mutex_lock(&customerMutexes[customerNo]);
            int amount = customers[customerNo].first;
            string company = customers[customerNo].second;
        
            performPayment(company, amount);
            finished--;
            cout << finished;
            cout << endl;

            pthread_mutex_unlock(&customerMutexes[customerNo]);
        }
    }
    free(itr);
    pthread_exit(0);
}

void performPayment(string company, int amount)
{   
    if(company == "Kevin")
    {
        pthread_mutex_lock(&companyMutexes[0]);
        kevin += amount;
        cout << "Kevin: ";
        cout << kevin;
        cout << endl;
        pthread_mutex_unlock(&companyMutexes[0]);
    }
    else if(company == "Bob")
    {
        pthread_mutex_lock(&companyMutexes[1]);
        bob += amount;
        cout << "Bob: ";
        cout << bob;
        cout << endl;
        pthread_mutex_unlock(&companyMutexes[1]);
    }
    else if(company == "Stuart")
    {
        pthread_mutex_lock(&companyMutexes[2]);
        stuart += amount;
        cout << "Stuart: ";
        cout << stuart;
        cout << endl;
        pthread_mutex_unlock(&companyMutexes[2]);
    }
    else if(company == "Otto")
    {
        pthread_mutex_lock(&companyMutexes[3]);
        otto += amount;
        cout << "Otto: ";
        cout << otto;
        cout << endl;
        pthread_mutex_unlock(&companyMutexes[3]);
    }
    else if(company == "Dave")
    {
        pthread_mutex_lock(&companyMutexes[4]);
        dave += amount;
        cout << "Dave: ";
        cout << dave;
        cout << endl;
        pthread_mutex_unlock(&companyMutexes[4]);
    }
    else
    {
        cout << company;
        cout << "Company name not found.";
        exit(1);
    }

}