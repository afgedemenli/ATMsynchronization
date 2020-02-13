#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>

using namespace std;

// threads
pthread_t atms[11];
pthread_t custs[333];

// global output file
ofstream outFile;

// variable for constraining atm work time
int atms_working=1;

// is data given to that atm?
int atm_data_taken[11]={0};

// counting variables for each bill type
int cableTV_sum;
int electricity_sum;
int gas_sum;
int telecommunication_sum;
int water_sum;

// mutexes, initialized
pthread_mutex_t print=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t atm_mut[11]={PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t sum_mut[6]={PTHREAD_MUTEX_INITIALIZER};

// data given to atms will be stored in this array
int atm_data[11][6];


// customer thread function
void *cust_t(void *p){

	// get the input from parameters
	int customer_data[6];
	int *tt = (int *)p;
    for (int i=1;i<=5;i++){
        customer_data[i]=tt[i];
    }
    int atm_id = customer_data[3];
    // calculate sleeping time in nano
    // sleep
    int sleep_time = customer_data[2];
    timespec time;
    time.tv_nsec = 1000000*sleep_time;
    nanosleep(0,&time);
    // cout <<"geldiii" <<endl;

    // lock
    pthread_mutex_lock(&atm_mut[atm_id]);   
    // cout <<"geldiiimmmmmmmmm" <<endl;
    // copy customer data to atm data
    for (int i=1;i<=5;i++){
        atm_data[atm_id][i]=customer_data[i];
    }
    // data is given to that atm
    atm_data_taken[atm_id]=1;

    // wait for atm to process data
    // atm thread runs simultaneously
    // cout << atm_id << endl;
    while(atm_data_taken[atm_id]);
    // cout << "gelmedi" << endl;
    // release the mutex
    pthread_mutex_unlock(&atm_mut[atm_id]);
}
// array for input
int customer[333][11];

// atm thread function
void *atm(void *p) {
	int atm_id=(long)p;
	// cout << atm_id << endl;
	// repeat while atms are working, until all is done
    while(atms_working){

    	// wait until the data is given
    	while(atm_data_taken[atm_id]==0 && atms_working);
    	// it atms not working, then quit
    	if(atms_working == 0){
    		break;
    	}
    	else{
        	// cout << "bok" << endl;

        	// lock mutexes
            pthread_mutex_lock(&sum_mut[atm_data[atm_id][4]]);
            pthread_mutex_lock(&print);
            // cout << "bokkkkk" << endl;

            // bill amount
            int add =atm_data[atm_id][5];

            // find the corresponding bill type
            switch(atm_data[atm_id][4]){
            	case 0: cableTV_sum+=add; break;
            	case 1: electricity_sum+=add; break;
            	case 2: gas_sum+=add; break;
            	case 3: telecommunication_sum+=add; break;
            	case 4: water_sum+=add; break;
            }

            // output
            outFile << "Customer" << atm_data[atm_id][1]<<","<<atm_data[atm_id][5]<<"TL,";
            
            // find the corresponding bill type
            // print it to the output file as the type that customer payed
            switch(atm_data[atm_id][4]){
            	case 0: outFile<<"cableTV"<<endl; break;
            	case 1: outFile<<"electricity\n"<<endl; break;
            	case 2: outFile<<"gas\n"<<endl; break;
            	case 3: outFile<<"telecommunication\n"<<endl; break;
            	case 4: outFile<<"water\n"<<endl; break;
            	// default: printf("bok\n");
            }

            // release mutexes
            pthread_mutex_unlock(&print);
            pthread_mutex_unlock(&sum_mut[atm_data[atm_id][4]]);
    		// cout << "bok" << endl;

    		// release the data, dont process anymore
            atm_data_taken[atm_id]=0;
        }
    }
}

int main(int argc, char *argv[]){
	// read input file given as argument
	string inFileName = argv[1];
	freopen(inFileName.c_str(), "r", stdin);
	int N;
	string line;
	getline(cin, line);
	N=stoi(line);
	// open output file
	outFile.open(inFileName + "_log.txt");

	// creating threads for 10 atms
	for(int i=1;i<=10;i++){
		// int *cid = (int*)(malloc(sizeof(int)));
  //       *cid = i;
		pthread_create(&atms[i], NULL, &atm, (void*)i);
	}
	// cout << "geldi" << endl;
	for(int i=1;i<=N;i++){
		getline(cin, line);
		// parsing the input
		// first one is the customer id
		// second one is the sleeping duration
		// third one is the atm id
		// fourth one is the bill type
		// fifth one is the bill amount
		customer[i][1] = i;
		customer[i][2] = stoi(line.substr(0, line.find(","))); line = line.substr(line.find(",")+1);
		customer[i][3] = stoi(line.substr(0, line.find(","))); line = line.substr(line.find(",")+1);
		string billname = line.substr(0, line.find(",")); line = line.substr(line.find(",")+1);
		customer[i][5] = stoi(line.substr(0, line.find(","))); line = line.substr(line.find(",")+1);
		
		// bill types
		if(billname=="cableTV")customer[i][4]=0;
		if(billname=="electricity") customer[i][4]=1;
		if(billname=="gas") customer[i][4]=2;
		if(billname=="telecommunication") customer[i][4]=3;
		if(billname=="water") customer[i][4]=4;

		// create customer threads
		pthread_create(&custs[i], NULL, &cust_t, customer[i]);
	}
	// cout << "geldi" << endl;
	for(int i=1;i<=N;i++){
		pthread_join(custs[i], NULL);
	}
	// cout << "geldi" << endl;
	atms_working = 0;
	// cout << "geldi" << endl;
	for(int i=1;i<=10;i++){
		pthread_join(atms[i], NULL);
	}

	// output, bill sums
	outFile << "All payments are completed." << endl;
	outFile << "cableTV: " << cableTV_sum << "TL" << endl;
	outFile << "electricity: " << electricity_sum << "TL" << endl;
	outFile << "gas: " << gas_sum << "TL" << endl;
	outFile << "telecommunication: " << telecommunication_sum << "TL" << endl;
	outFile << "water: " << water_sum << "TL" << endl;

	return 0;
}