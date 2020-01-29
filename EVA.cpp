//**************************************************************
// EVA Cache Simulator
// By Brodie Gerloff
//
// A program implementing a new cache structure called EVA
// Written for ECGR 4101- Computer Architecture
// Note- Split instruction and data memory will set both cache sizes equal to your input- ex of you enter 32kb BOTH caches will be 32kb
//
// To run file: ./eva (total cache memory in kb) (block size) (file name)
// Ex: a 32kb, 16b block size for the file trace.din would be ./eva 32 16 trace.din
//
//**************************************************************

//Include files
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;

//Global Variables
char age[4096][4];
char groupAge[4096];
bool reused [4096][4];
int hits[8][2];
int evicts[8][2];
int evictPrior[8][2];
double evaTable[8][2];

//Functions
string hexToBin(string h);
int findTag(string s, int len);
int findIndex(string s, int len, int o);

void EVA(int n);
void argSort();

int main(int argc, char * argv[]) {
    
    //Input variables
    int cacheMemory = atoi(argv[1]) * 1024;         //Total cache memory
    char cacheComb = 'c';                           //Combined or seperated cache
    int blockSize = atoi(argv[2]);                  //Block size
    
    //Setup for input file
    ifstream trace (argv[3]);
    int type;
    string address;
    
    //Hit and miss count
    int hit = 0;
    int miss = 0;
    long long instructCount = 0;
    float hitRate = 0.0;
    
    //EVA variables
    bool hitFlag = false;
    int evictTemp[4];
    int tempMax = 0;
    int tempIndex = 0;
    
    //Memory variables
    int numBlocks = cacheMemory / blockSize;        //Number of blocks in cache
    int assc = 4;                                   //Associativity- either 1 or 4
    int offsetLen;                                  //Length of offset in bits
    int indexLen;                                   //Length of index in bits
    int tagLen;                                     //Length of tag in bits
    
    //Arrays
    long long tags[4096][4];                        //Tag array for data/combined
    long long tagsInstr[4096][4];                   //Tag array for instructions
    
    //Recieved parameters
    int requiredTag;
    int requiredIndex;
    
    //Display selected format
    cout << "Cache format:" << endl;
    cout << "Total cache size: " << cacheMemory << endl;
    cout << "Block size: " << blockSize << endl;
    
    //Calculating memory sizes
    offsetLen = log2(blockSize);
    indexLen = log2(numBlocks) - 2;
    tagLen = 32 - indexLen - offsetLen;
    
    //Clear tags
    for (int i = 0; i < 4096; i++) {
        groupAge[i] = 0;
        for (int j = 0; j < 4; j++) {
            age[i][j] = j;
            reused[i][j] = false;
            tags[i][j] = 0xffffffff;
            tagsInstr[i][j] = 0xffffffff;
        }
    }
    
    //Setup hit & evict
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 2; j++) {
            hits[i][j] = 0;
            evicts[i][j] = 0;
            evictPrior[i][j] = (2 * i) + j;
            evaTable[i][j] = 0.0;
        }
    }
    
    //Main Loop
    while(trace >> type >> address) {
        //Convert address to tag and index
        while(address.size() < 8) {
            address.insert(address.begin(),'0');
        }
        requiredTag = findTag(address, tagLen);
        requiredIndex = findIndex(address, indexLen, tagLen);
        
        hitFlag = false;
        
        //Check for a match
        for (int i = 0; i < assc; i++) {
            if (requiredTag == tags[requiredIndex][i]) {
                hitFlag = true;
                hit++;
                if (reused[requiredIndex][i] == true) {             //Determine if reused or not, and increments the respective counter
                    hits[age[requiredIndex][i]][0]++;
                }
                else {
                    hits[age[requiredIndex][i]][1]++;
                }
                reused[requiredIndex][i] = true;
                age[requiredIndex][i] = 0;
                i = assc;
            }
        }
        
        //If no match, evict the one with least value
        if (hitFlag == false) {
            miss++;
            for (int i = 0; i < assc; i++) {
                if (reused[requiredIndex][i] == true) {
                    evictTemp[i] = evictPrior[age[requiredIndex][i]][0];
                }
                else {
                    evictTemp[i] = evictPrior[age[requiredIndex][i]][1];
                }
            }
            
            tempMax = -1;
            for (int i = 0; i < 4; i++) {
                if (evictTemp[i] > tempMax) {
                    tempMax = evictTemp[i];
                    tempIndex = i;
                }
            }
        
            reused[requiredIndex][tempIndex] = false;
            age[requiredIndex][tempIndex] = 0;
            tags[requiredIndex][tempIndex] = requiredTag;
        }
        
        //Aging
        groupAge[requiredIndex]++;
        if (groupAge[requiredIndex] >= 16) {
            for(int i = 0; i < 4; i++) {
                if(age[requiredIndex][i] < 7) {
                    age[requiredIndex][i]++;
                }
            }
            groupAge[requiredIndex] = 0;
        }
        
        //Increment total instruction count
        instructCount++;
        
        //EVA check every 32k instructions
        if ((instructCount % 32768 == 0) && (instructCount != 0)) {
            EVA(cacheMemory);
        }
    }
    
    //Final math
    hitRate = 100 * (float)miss / (float)instructCount;
    cout << "Miss rate:" << miss << "/" << instructCount << "= " << hitRate << "%" << endl;
    
    
    return 0;
}

//====================================================================
//
// hexToBin() function
//
//  -Converts a hex string to a binary string. Takes in a string and returns a string.
//  -Used for the findTag and findIndex functions
//
//====================================================================

string hexToBin(string h) {
    string temp = "";
    for (int i = 0; i < h.size(); i++) {
        switch (h[i]) {
        case '0': 
            temp.append("0000"); 
            break; 
        case '1': 
            temp.append("0001"); 
            break; 
        case '2': 
            temp.append("0010"); 
            break; 
        case '3': 
            temp.append("0011"); 
            break; 
        case '4': 
            temp.append("0100"); 
            break; 
        case '5': 
            temp.append("0101"); 
            break; 
        case '6': 
            temp.append("0110"); 
            break; 
        case '7': 
            temp.append("0111"); 
            break; 
        case '8': 
            temp.append("1000"); 
            break; 
        case '9': 
            temp.append("1001"); 
            break; 
        case 'A': 
        case 'a': 
            temp.append("1010"); 
            break; 
        case 'B': 
        case 'b': 
            temp.append("1011"); 
            break; 
        case 'C': 
        case 'c': 
            temp.append("1100"); 
            break; 
        case 'D': 
        case 'd': 
            temp.append("1101"); 
            break; 
        case 'E': 
        case 'e': 
            temp.append("1110"); 
            break; 
        case 'F': 
        case 'f': 
            temp.append("1111"); 
            break; 
        default: 
            cout << "Error- Incorrect hex digit" << endl;
        }
    } 
    
    return temp;
}

//====================================================================
//
// findTag() function
//
//  -Takes in an address string, converts to binary, sorts out the tag bits, and converts to an integer.
//  -
//
//====================================================================

int findTag(string s, int len) {
    string binary = hexToBin(s);
    binary = binary.substr(0, len);
    //cout << len << " " << binary << endl;
    return stoi(binary,0,2);
}

//====================================================================
//
// findIndex() function
//
//  -Takes in an address string, converts to binary, sorts out the index bits, and converts to an integer.
//  -Nearly identical to above, but has an offset
//
//====================================================================

int findIndex(string s, int len, int o) {
    string binary = hexToBin(s);
    binary = binary.substr(o, len);
    //cout << stoi(binary,0,2) << endl;
    return stoi(binary,0,2);
}

//====================================================================
//
// EVA() function
//
//  -Main bulk of the additions
//  -See algorithm block for more details
//
//====================================================================

void EVA(int n) {
    //Variables
    int hits_r = 0;
    int hits_nr = 0;
    int evict_r = 0;
    int evict_nr = 0;
    
    //Hitrate numbers
    double tableHitRate = 0.0;
    double reuseRate = 0.0;
    double notReuseRate = 0.0;
    double perAccessCost = 0.0;
    
    //Part 3 Variables
    int lifetime = 0;
    int hitSum = 0;
    int eventsSum = 0;
    double evaReused = 0;
    
    vector<pair<int,int>> order;
    
    //Part 1- determine access cost
    
    //Sum total hits and evicts
    for(int i = 0; i < 8; i++) {
        hits_r += hits[i][0];
        hits_nr += hits[i][1];
        evict_r += evicts[i][0];
        evict_nr += evicts[i][1];
    }
    
    //Hitrate for all versions
    reuseRate = (double)hits_r / (double)evict_r;
    notReuseRate = (double)hits_nr / (double)evict_nr;
    tableHitRate = ((double)hits_r + (double)hits_nr) / ((double)evict_r + (double)evict_nr);
    
    perAccessCost = tableHitRate * 15 / (double)n;
    
    //PART 2: EVA calculations
    
    //Reused calculations
    for (int i = 0; i < 8; i++) {
        hitSum += hits[i][0];
        eventsSum += hits[i][0] + evicts[i][0];
        lifetime += eventsSum;
        evaTable[i][0] = (hitSum - (perAccessCost * lifetime)) / eventsSum;
    }
    
    lifetime = 0;
    hitSum = 0;
    eventsSum = 0;
    
    //Not reused calculations
    for (int i = 0; i < 8; i++) {
        hitSum += hits[i][1];
        eventsSum += hits[i][1] + evicts[i][1];
        lifetime += eventsSum;
        evaTable[i][1] = (hitSum - (perAccessCost * lifetime)) / eventsSum;
    }
    
    //PART 3- Differentiate reused/ not reused
    
    //eva Reused 
    evaReused = evaTable[0][0] / (1 - reuseRate);
    
    //Recalculate EVA
    for (int i = 0; i < 8; i++) {
        evaTable[i][0] += (reuseRate - tableHitRate) * evaReused;
    }
    
    for (int i = 0; i < 8; i++) {
        evaTable[i][1] += (notReuseRate - tableHitRate) * evaReused;
    }
    
    //PART 4- Sort eviction priority
    
    argSort();
    
}

//====================================================================
//
//  ArgSort()
//
//  -Sorts eva table by index
//  -updates priority table
//
//====================================================================

void argSort() {
    double tempMin = 1000000;
    double lastMin = -1000000;
    pair<int,int> temp;
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 2; k++) {
                if ((evaTable[j][k] < tempMin) && (evaTable[j][k] > lastMin)) {
                    tempMin = evaTable[j][k];
                    temp.first = j;
                    temp.second = k;
                }
            }
        }
        lastMin = tempMin;
        tempMin = 1000000;
        evictPrior[temp.first][temp.second] = 1;
    }
}
