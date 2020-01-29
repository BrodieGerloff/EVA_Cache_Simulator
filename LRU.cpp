//**************************************************************
// Homework 1- Basic Cache Simulator
// By Brodie Gerloff
//
// A program comparable to Dinero to simulate a level 1 cache. 
// Written for ECGR 4101- Computer Architecture
// Note- Split instruction and data memory will set both cache sizes equal to your input- ex of you enter 32kb BOTH caches will be 32kb
//
// To run file: ./cache (total cache memory in kb) (s for split, c for combined) (d for direct, a for 4 way associative) (block size)
// Ex: a 32kb combined cache, 4 way associativity, and 32b block size would be ./cache 32 c a 32
//
//**************************************************************

//Include files
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <cmath>

using namespace std;

//Functions
string hexToBin(string h);
int findTag(string s, int len);
int findIndex(string s, int len, int o);
int getLRU(int lru[], int ind);


int main(int argc, char * argv[]) {
    
    //Input variables
    int cacheMemory = atoi(argv[1]) * 1024;         //Total cache memory
    char cacheComb = argv[2][0];                    //Combined or seperated cache
    char cacheAssoc = argv[3][0];                   //Direct or 4 way associative
    int blockSize = atoi(argv[4]);                  //Block size
    
    //Setup for input file
    ifstream trace (argv[5]);
    int type;
    string address;
    
    //Hit and miss count
    int hit = 0;
    int miss = 0;
    int instructCount = 0;
    short hitFlag = 0;
    float hitRate = 0.0;
    
    //Memory variables
    int numBlocks = cacheMemory / blockSize;        //Number of blocks in cache
    int assc;                                       //Associativity- either 1 or 4
    int offsetLen;                                  //Length of offset in bits
    int indexLen;                                   //Length of index in bits
    int tagLen;                                     //Length of tag in bits
    
    //Arrays
    int lru[4096][4];                       //Lru for data/combined
    int lruInstr[4096][4];                  //lru for instructions
    long long tags[4096][4];                //Tag array for data/combined
    long long tagsInstr[4096][4];           //Tag array for instructions
    
    //Recieved parameters
    int requiredTag;
    int requiredIndex;
    int lruIndex;
    
    //Argv Validation
    if (cacheComb != 's' && cacheComb != 'c') {
        cout << "Incorrect format- use 's' for split cache or 'c' for combined cache." << endl;
        return 0;
    }
    else if (cacheAssoc != 'd' && cacheAssoc != 'a') {
        cout << "Incorrect format- use 'd' for direct or 'a' for 4 way associative." << endl;
        return 0;
    }
    
    //Display selected format
    cout << "Cache format:" << endl;
    cout << "Total cache size: " << cacheMemory << endl;
    cout << "Memory structure: ";
    if (cacheComb == 's') {
        cout << "Split memory" << endl;
    }
    else {
        cout << "Combined memory" << endl;
    }
    cout << "Block associativity: ";
    if (cacheAssoc == 'd') {
        cout << "Direct" << endl;
        assc = 1;
    }
    else {
        cout << "4-way associative" << endl;
        assc = 4;
    }
    cout << "Block size: " << blockSize << endl;
    
    //Calculating memory sizes
    offsetLen = log2(blockSize);
    if (cacheAssoc == 'd') {
        indexLen = log2(numBlocks);
    }
    else {
        indexLen = log2(numBlocks) - 2;
    }
    tagLen = 32 - indexLen - offsetLen;
    
    //Tag/type check- used for testing
    /*trace >> type >> address;
    while(address.size() < 8) {
        address.insert(address.begin(),'0');
    }
    cout << findTag(address, tagLen) << endl;
    cout << findIndex(address, indexLen, tagLen) << endl; */
    
    //Clear tags and lru
    //NOTE- the spots for the LRUs are filled 0, 1, 2, and 3. This prevents an issue with the LRU locator when the tags are entirely empty (the first four accesses to that cache layer)
    for (int i = 0; i < 4096; i++) {
        for (int j = 0; j < 4; j++) {
            lru[i][j] = j;
            lruInstr[i][j] = j;
            tags[numBlocks][assc] = 0xffffffff;
            tagsInstr[numBlocks][assc] = 0xffffffff;
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
        
        if (cacheComb == 'c' || ((cacheComb == 's') && (type != 2))) {          //Data cache, or combined cache
            for (int i = 0; i < assc; i++) {
                hitFlag = 0;
                if (requiredTag == tags[requiredIndex][i]) {
                    hitFlag = 1;
                    hit++;
                    i = assc;
                }
                
            }
            if (hitFlag == 0) {
                miss++;
                if (assc == 1) {
                    tags[requiredIndex][0] = requiredTag;
                }
                else {
                    lruIndex = getLRU(lru[requiredIndex], assc);
                    tags[requiredIndex][lruIndex] = requiredTag;
                }
            }
        } 
        else {                                                                  //Instruction cache
            for (int i = 0; i < assc; i++) {
                hitFlag = 0;
                if (requiredTag == tagsInstr[requiredIndex][i]) {
                    hitFlag = 1;
                    hit++;
                    i = assc;
                }
                
            }
            if (hitFlag == 0) {
                miss++;
                if (assc == 1) {
                    tagsInstr[requiredIndex][0] = requiredTag;
                }
                else {
                    lruIndex = getLRU(lruInstr[requiredIndex], assc);
                    tagsInstr[requiredIndex][lruIndex] = requiredTag;
                }
            }
        }
        
        //Increment total instruction count
        instructCount++;
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
// getLRU() function
//
//  -Searches the LRU array to find the point with "3" (0 is most recently used, 3 is LRU)
//
//====================================================================

int getLRU(int lru[], int ind) {
    int a;
    for (int i = 0; i < ind; i++) {
        if (lru[i] == 3) {                          //Match found, increment and return position
            a = i;
            for (int j = 0; j < ind; j++) {
                lru[j]++;
            }
            lru[a] = 0;
            return a;
        }
    }
}
