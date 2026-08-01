#include "pin.H"
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <climits>
#include <fstream>
using namespace std;
#define ll long long

ll associativity = 0, offset_size = 0, index_size = 0, counter = 0, mask = 0, accesses = 0, hit = 0, miss = 0, writeBack = 0;
ofstream csvFile;
ll wSize = 1000, wAccess = 0, wMiss = 0, wId = 0;

struct Line {
    bool valid;
    bool dirty;
    ll tag;
    ll cnt;
    
    Line() : valid(false), dirty(false), tag(0), cnt(0) {}
};

struct Set {
    vector<Line> lines;
    Set(ll assoc) {
        lines.resize(assoc);
    }
};

vector<Set> cache;

void initCache(ll cache_size, ll block_size, ll assoc) {
    associativity = assoc;
    accesses = 0;
    hit = 0;
    miss = 0;
    counter = 0;
    wAccess = 0;
    wMiss = 0;
    wId = 0;

    ll num_blocks = cache_size / block_size;
    ll num_sets = num_blocks / associativity;
    
    cache.clear();
    for (int i = 0; i < num_sets; i++) {
        cache.push_back(Set(associativity));
    }
    
    offset_size = log2(block_size);
    index_size = log2(num_sets);
    mask = (1LL << index_size) - 1;
    
    csvFile.open("miss.csv");
    csvFile << "window,accesses,misses\n";
}

void access(ll address, bool isWrite) {
    accesses++;
    wAccess++;
    counter++;

    ll shifted_addr = address >> offset_size;
    ll index = shifted_addr & mask;
    ll tag = shifted_addr >> index_size;

    Set& inSet = cache[index];

    // Check for Hit
    for (int i = 0; i < associativity; i++) {
        if (inSet.lines[i].valid && inSet.lines[i].tag == tag) {
            hit++;
            inSet.lines[i].cnt = counter;
            inSet.lines[i].dirty |= isWrite;
            
            if (wAccess >= wSize) {
                csvFile << wId << "," << wAccess << "," << wMiss << "\n";
                wId++;
                wAccess = 0;
                wMiss = 0;
            }
            return;
        }
    }

    // Miss Handler
    miss++;
    wMiss++;
    int idx = 0;
    ll mn = LLONG_MAX;

    for (int i = 0; i < associativity; i++) {
        if (!inSet.lines[i].valid) {
            idx = i;
            break;
        }
        if (inSet.lines[i].cnt < mn) {
            mn = inSet.lines[i].cnt;
            idx = i;
        }
    }

    if (inSet.lines[idx].dirty) {
        writeBack++;
    }

    inSet.lines[idx].valid = true;
    inSet.lines[idx].tag = tag;
    inSet.lines[idx].cnt = counter;
    inSet.lines[idx].dirty = isWrite;

    if (wAccess >= wSize) {
        csvFile << wId << "," << wAccess << "," << wMiss << "\n";
        wId++;
        wAccess = 0;
        wMiss = 0;
    }
}

VOID RecordMemAccess(VOID* addr, bool isWrite = false) {
    ll address_val = (ll)addr;
    access(address_val, isWrite);
}

VOID Instruction(INS ins, VOID* v) {
    if (INS_IsMemoryRead(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemAccess, IARG_MEMORYREAD_EA, IARG_BOOL, false, IARG_END);
    }

    if (INS_IsMemoryWrite(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemAccess, IARG_MEMORYWRITE_EA, IARG_BOOL, true, IARG_END);
    }
}

void flushCache() {
    for (auto& i : cache) {
        for (auto& j : i.lines) {
            j.valid = false;
            j.dirty = false;
        }
    }
}

VOID Fini(INT32 code, VOID* v) {
    cout << "CACHE SIMULATION RESULTS:-\n";
    cout << "Total Memory Accesses: " << accesses << "\n";
    cout << "Cache hit: " << hit << "\n";
    cout << "Cache miss: " << miss << "\n";
    cout << "Write-backs: " << writeBack << "\n";
    
    if (accesses > 0) {
        cout << fixed << setprecision(2);
        cout << "Hit Rate: " << ((0.0 + hit) / accesses) * 100 << "%\n";
        cout << "Miss Rate: " << ((0.0 + miss) / accesses) * 100 << "%\n";
    }
    
    flushCache();
    
    if (wAccess > 0) {
        csvFile << wId << "," << wAccess << "," << wMiss << "\n";
    }
    
    if (csvFile.is_open()) {
        csvFile.close();
    }
}

int32_t main(int argc, char* argv[]) {
    if (PIN_Init(argc, argv)) {
        cerr << "PIN Initialization error" << endl;
        return -1;
    }

    initCache(16384, 64, 4);

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();

    return 0;
}
