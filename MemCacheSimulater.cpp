#include "pin.H"
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
using namespace std;
#define ll long long
struct CacheLine {
    bool valid;
    ll tag;
    ll last_used_time;

    CacheLine() : valid(false), tag(0), last_used_time(0) {}
};

struct CacheSet {
    vector<CacheLine> lines;
    CacheSet(int associativity) {
        lines.resize(associativity);
    }
};

class CacheSimulator {
private:
    vector<CacheSet> cache;
    int associativity;
    ll global_time;

    int offset_bits;
    int index_bits;
    ll index_mask;

    ll accesses;
    ll hits;
    ll misses;

public:
    CacheSimulator(int cache_size, int block_size, int assoc) {
        associativity = assoc;
        accesses = 0;
        hits = 0;
        misses = 0;
        global_time = 0;

        int num_blocks = cache_size / block_size;
        int num_sets = num_blocks / associativity;

        for (int i = 0; i < num_sets; ++i) {
            cache.push_back(CacheSet(associativity));
        }

        offset_bits = log2(block_size);
        index_bits = log2(num_sets);
        
        index_mask = (1LL << index_bits) - 1; 
    }

    void access(ll address) {
        accesses++;
        global_time++;

        ll shifted_addr = address >> offset_bits;
        ll index = shifted_addr & index_mask;
        ll tag = shifted_addr >> index_bits;

        CacheSet& target_set = cache[index];

        for (int i = 0; i < associativity; ++i) {
            if (target_set.lines[i].valid && target_set.lines[i].tag == tag) {
                hits++;
                target_set.lines[i].last_used_time = global_time;
                return;
            }
        }

        misses++;

        int lru_index = 0;
        ll min_time =LLONG_MIN;

        for (int i = 0; i < associativity; ++i) {
            if (!target_set.lines[i].valid) {
                target_set.lines[i].valid = true;
                target_set.lines[i].tag = tag;
                target_set.lines[i].last_used_time = global_time;
                return;
            }

            if (target_set.lines[i].last_used_time < min_time) {
                min_time = target_set.lines[i].last_used_time;
                lru_index = i;
            }
        }

        target_set.lines[lru_index].tag = tag;
        target_set.lines[lru_index].last_used_time = global_time;
    }

    void printStats() const {
        cout << "CACHE SIMULATION RESULTS:-\n";
        cout << "Total Memory Accesses: " << accesses << "\n";
        cout << "Cache Hits: " << hits << "\n";
        cout << "Cache Misses: " << misses << "\n";
        
        if (accesses > 0) {
            double hit_rate = ((0.0+hits)/accesses)*100;
            double miss_rate = ((0.0+misses)/accesses)*100;
            cout << fixed << setprecision(2);
            cout << "Hit Rate: " << hit_rate << "%\n";
            cout << "Miss Rate: " << miss_rate << "%\n";
        }
    }
};

CacheSimulator* l1_cache;

VOID RecordMemAccess(VOID* addr) {
    ll address_val = (ll)addr;
    l1_cache->access(address_val);
}

VOID Instruction(INS ins, VOID* v) {
    if (INS_IsMemoryRead(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemAccess,
                       IARG_MEMORYREAD_EA, IARG_END);
    }

    if (INS_IsMemoryWrite(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemAccess,
                       IARG_MEMORYWRITE_EA, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v) {
    l1_cache->printStats();
    delete l1_cache;
}

int main(int argc, char* argv[]) {
    if (PIN_Init(argc, argv)) {
        cerr << "PIN Initialization failed" << endl;
        return -1;
    }

    l1_cache = new CacheSimulator(16384, 64, 4);

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();

    return 0;
}