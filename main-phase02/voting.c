#include "voting.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


// Enable in Makefile
#ifdef DEBUG_PRINTS_ENABLED
#define DebugPrint(...) printf(__VA_ARGS__);
#else
#define DebugPrint(...)
#endif

#define PRIMES_SZ 1024
#define DISTRICTS_SZ 56
#define PARTIES_SZ 5


typedef struct District District;
typedef struct Station Station;
typedef struct Voter Voter;
typedef struct Party Party;
typedef struct Candidate Candidate;
typedef struct ElectedCandidate ElectedCandidate;

struct District {
    int did;
    int seats;
    int blanks;
    int invalids;
    int partyVotes[5];
};

struct Station {
    int sid;
    int did;
    int registered;
    Voter* voters;
    Station* next;
};
struct Voter {
    int vid;
    bool voted;
    Voter* parent;
    Voter* lc;
    Voter* rc;
};

struct Party {
    int pid;
    int electedCount;
    Candidate* candidates;
};
struct Candidate {
    int cid;
    int did;
    int votes;
    bool isElected;
    Candidate* lc;
    Candidate* rc;
};

struct ElectedCandidate {
    int cid;
    int did;
    int pid;
    ElectedCandidate* next;
};

District Districts[DISTRICTS_SZ];
Station** StationsHT;
Party Parties[PARTIES_SZ];
ElectedCandidate* Parliament;

/* used to create the hash function*/
int HT_size = 0;
unsigned int a = 0,b;

const int DefaultDid = -1;
const int BlankDid = -1;
const int InvalidDid = -2;

const int Primes[PRIMES_SZ] = {
    0, 1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507, 4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591, 4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751, 4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861, 4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171, 5179, 5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279, 5281, 5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399, 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471, 5477, 5479, 5483, 5501, 5503, 5507, 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639, 5641, 5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711, 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827, 5839, 5843, 5849, 5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927, 5939, 5953, 5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073, 6079, 6089, 6091, 6101, 6113, 6121, 6131, 6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217, 6221, 6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311, 6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379, 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521, 6529, 6547, 6551, 6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607, 6619, 6637, 6653, 6659, 6661, 6673, 6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737, 6761, 6763, 6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841, 6857, 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971, 6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079, 7103, 7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207, 7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297, 7307, 7309, 7321, 7331, 7333, 7349, 7351, 7369, 7393, 7411, 7417, 7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499, 7507, 7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573, 7577, 7583, 7589, 7591, 7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687, 7691, 7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823, 7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919, 7927, 7933, 7937, 7949, 7951, 7963, 7993, 8009, 8011, 8017, 8039, 8053, 8059, 8069, 8081, 8087, 8089, 8093, 8101, 8111, 8117, 8123
};
int MaxStationsCount;
int MaxSid;

/*Util functions, called by events*/

/*addVoter: adds a voter node to the complete binary tree of a station*/
void addVoter(Station* ptr, int vid) {
    // Check for a valid station pointer
    if (!ptr) {
        fprintf(stderr, "Error: Invalid station pointer\n");
        return;
    }

    // Create a new voter node
    Voter* newVoter = (Voter*)malloc(sizeof(Voter));
    if (!newVoter) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newVoter->vid = vid;
    newVoter->voted = false;  // Default to not voted
    newVoter->parent = NULL;
    newVoter->lc = NULL;
    newVoter->rc = NULL;

    // If the tree is empty, set the new voter as the root
    if (!ptr->voters) {
        ptr->voters = newVoter;
        ptr->registered++;  // Increment the voter count
        return;
    }
    // Use a stack for traversal
    Voter** stack = malloc(log2((ptr->registered))+1 * sizeof(Voter*)); // biggest possible path is of length: height +1 = log2((ptr->registered)) +1
    int top = -1;

    // start by pushing the root onto the stack
    stack[++top] = ptr->voters;

    while (top >= 0) {
        // pop the top node from the stack
        Voter* current = stack[top--];

        // check if there's space for a new child
        if (!current->lc) {
            current->lc = newVoter;
            newVoter->parent = current;
            ptr->registered++;
            return;
        } else if (!current->rc) {
            current->rc = newVoter;
            newVoter->parent = current;
            ptr->registered++;
            return;
        }

        // Push children onto the stack (right first, then left)
        // This ensures left child is checked before the right child when popped
        stack[++top] = current->rc;
        stack[++top] = current->lc;
        printf("Added voter %d, child of %d", vid, newVoter->parent->vid);
    }
    free(stack);
}
/*printVoter: prints the voters tree according to inorder traversal*/
void printVoters(Voter* ptr){
    if(!ptr) return; // base case
    printVoters(ptr->lc);
    printf("%d, ", ptr->vid);
    printVoters(ptr->rc);

}
/*printVoters2: just like printVoters but formatted according to the printStations event*/
void printVoters2(Voter* ptr){
    if(!ptr) return; // base case
    printVoters2(ptr->lc);
    printf("\t%d %d, \n", ptr->vid, ptr->voted);
    printVoters2(ptr->rc);

}
/*printCandidates: prints the candidates tree according to inorder traversal*/
void printCandidates(Candidate* ptr){ // pointer to the root of the candidates tree of the party
    if(!ptr) return; // base case
    printCandidates(ptr->lc);
    printf("%d %d, ", ptr->cid, ptr->did);
    printCandidates(ptr->rc);
}
/*printCandidates2: same as above but formatted for printParty event*/
void printCandidates2(Candidate* ptr){ // pointer to the root of the candidates tree of the party
    if(!ptr) return; // base case
    printCandidates2(ptr->lc);
    printf("\t%d %d,\n", ptr->cid, ptr->votes);
    printCandidates2(ptr->rc);
}
/*voters_lookup: searched for voter with vid in a tree of a station*/
Voter* voters_lookup(Voter* ptr, int vid){
    if(!ptr) return NULL; // base case
    if(ptr->vid == vid) return ptr; // voter found
    Voter* found = voters_lookup(ptr->lc, vid); // left subtree
    if (found) return found;  // if found in left subtree, return the result
    return voters_lookup(ptr->rc, vid);     // right subtree

}
/* Initialize the hash parameters */ 
void initializeHashing() {
    srand(time(NULL));
    a = 1 + rand() % (HT_size - 1); //[1, HT_size-1]
    b = rand() % HT_size;          // [0, HT_size-1]
}
/* Universal hash function for station IDs */
unsigned int hash(int sid) {
    if(HT_size>0) return ((a * sid + b) % HT_size); 
    return 0; // return 0 if the hash table is corrupt
}

void EventAnnounceElections(int parsedMaxStationsCount, int parsedMaxSid) {
    DebugPrint("A %d %d\n", parsedMaxStationsCount, parsedMaxSid);

    /* save values to the global variables*/
    MaxStationsCount = parsedMaxStationsCount;
    MaxSid = parsedMaxSid;

    /* Initialize Districts array*/
    for(int i=0; i<DISTRICTS_SZ; i++){    //init every district
        Districts[i].did = -1;  //will change at event D
        Districts[i].seats = 0;
        Districts[i].blanks = 0;
        Districts[i].invalids = 0;
        for(int j = 0; j< PARTIES_SZ; j++){
            Districts[i].partyVotes[j] = 0;
        }    
    }

    /* Initialize Stations hash table.
     Size will be the first prime bigger than maxStationsCount, so the loard factor is barely under 1 */
    int count, key;
    while(count <PRIMES_SZ){    // find the first prime that is bigger than max_stations
        if(Primes[count] > parsedMaxStationsCount){ // suppose max_stations is always smaller than the biggest prime in the array
            key = count;    // save the prime
            break;  //exit loop
        } //else:
        count++;
    }
    HT_size = Primes[key]; //hashtable size
    StationsHT = (Station**)malloc(HT_size * sizeof(Station*));
    if (StationsHT == NULL) {
        perror("Failed to allocate memory for StationHT");
        return;
    }
    for (int i = 0; i < HT_size; i++) {
        StationsHT[i] = NULL;   // initialize every field to NULL
    }
    initializeHashing(); // initialize a and b parameters


    /* Initialize Parties array*/
    for(int i=0; i< PARTIES_SZ; i++){
        Parties[i].pid = i;
        Parties[i].electedCount = 0;
        Parties[i].candidates = NULL;
    }

    /* Initialize the Parliament*/
    Parliament = NULL;
      
/* Final print statement*/
printf("DONE\n");
}

void EventCreateDistrict(int did, int seats) {
    DebugPrint("D %d %d\n", did, seats);

    /* Binary search on Districts array for logn time complexity*/
    int left = 0, right = 55, middle, result = -1;  
    while(left <= right){
        middle = (right + left) / 2;
        if(Districts[middle].did == -1){
            result = middle;
            right = middle -1;
        }else{
            left = middle +1;
        }
    }

    if (result == -1) {
        printf("Error: No empty cell available in Districts array.\n");
        return;
    }

    /* 'Result' is the index of the new district cell*/
    Districts[result].did = did;
    Districts[result].seats = seats;
    
    /* Print message*/
    printf("\tDistricts:\n");
    int i = 0;
    while(i < 56 && Districts[i].did != -1){
        printf("\t%d", Districts[i].did);
        if (i < 55 && Districts[i + 1].did != -1) printf(", ");    
        i++;
    }
    printf("\n");

}

void EventCreateStation(int sid, int did) {
    DebugPrint("S %d %d\n", sid, did);

    if (a == 0) {
        fprintf(stderr, "Structures have not been initialized\n");
        return;
    }

    int position = hash(sid);
    Station* prev = NULL;
    Station* curr = StationsHT[position];

    // Traverse the sorted list and find the correct position
    while (curr && curr->sid > sid) {
        prev = curr;
        curr = curr->next;
    }

    // Allocate memory for the new station
    Station* newNode = malloc(sizeof(Station));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed for new station\n");
        return;
    }

    // Initialize the new station
    newNode->did = did;
    newNode->sid = sid;
    newNode->voters = NULL;

    // Insert the new node into the sorted list
    if (!prev) {
        // Inserting at the head of the list
        newNode->next = curr;
        StationsHT[position] = newNode;
    } else {
        // Inserting somewhere after the head
        prev->next = newNode;
        newNode->next = curr;
    }

    printf("Stations[%d]\n", position);
    curr = StationsHT[position];
    while(curr){
        printf("%d, ", curr->sid);
        curr = curr->next;
    }
    printf("\nDONE\n");
}

void EventRegisterVoter(int vid, int sid) {
    DebugPrint("R %d %d\n", vid, sid);
    int pos1 = hash(sid); // position of station in the hashtable
    Station* curr = StationsHT[pos1]; // pointer at the station
    while(curr && curr->sid!=sid){ // look for station in the chain
        curr = curr->next;
    }
    if(!curr){
        printf("Station not found!"); //  reached the end of the chain
        return;
    }
    addVoter(curr, vid);
    printf("\tVoters[%d]\n\t", sid);
    printVoters(curr->voters);
    printf("\nDONE\n"); 
}   

void EventRegisterCandidate(int cid, int pid, int did) {
    DebugPrint("C %d %d %d\n", cid, pid, did);
    int pos = -1;
    
    for(int i= 0; i < PARTIES_SZ; i++){ // find the party based on pid
        if(pid == Parties[i].pid){
            pos = i;
            break; // found the party
        }
    }
    if(pos == -1){
        printf("Event C: Party not found.\n");
        return; // case: party not found
    } 

    Candidate* root = Parties[pos].candidates; // root of the candidates BST

    Candidate* newCandidate = malloc(sizeof(Candidate)); // make space for candidate
    if(!newCandidate) return; // case: malloc fails
    newCandidate->cid = cid; // fill the fields
    newCandidate->did = did;
    newCandidate->isElected = false;
    newCandidate->votes = 0;
    newCandidate->lc = NULL;
    newCandidate->rc = NULL;

    if(!root){ // case: candidates tree is empty for the party
        Parties[pos].candidates = newCandidate;
        return;
    }

    Candidate* curr = root; // init pointers
    Candidate* prev = NULL;

    while(curr){ // find the correct position for the candidate 
        prev = curr;
        if(curr->cid > cid) curr = curr->lc; // move left
        else curr = curr->rc; // move right
    }
    if(prev->cid > cid) prev->lc = newCandidate; // case candidate is the smallest child 
    else prev->rc = newCandidate; // candidate is the biggest child, so it is the rc of its parent

    printf("\tCandidates %d\n", pid);
    printCandidates(root);
    printf("\nDONE\n");

}

void EventVote(int vid, int sid, int cid, int pid) {
    DebugPrint("V %d %d %d %d\n", vid, sid, cid, pid);

    /* first we locate the station with 'sid' in the hash table */
    Station* sPtr = StationsHT[hash(sid)]; // station pointer
    while(sPtr && sPtr->sid!=sid){
        sPtr = sPtr->next;
    }
    if(!sPtr){ fprintf(stderr, "Station not found.\n");  return; } // station not found

    /* step 2: we locate the voter in the voters tree */
    Voter* vPtr = sPtr->voters; // voter pointer
    vPtr = voters_lookup(vPtr,vid);
    if(!vPtr) { fprintf(stderr, "Voter not found.\n");  return; }
    vPtr->voted = true; // voter voted
    int did = sPtr->did; // district of the voter

    /* step 3: apply the effects of the vote */
    int i = 0;
    bool found = false;
    while(i<DISTRICTS_SZ && !found){
        if(Districts[i].did == did){
            found = true; 
            break;
        }
        i++; // step
    }
    
    if(cid == -1) {
        Districts[i].blanks++;
    }else if(cid == -2) {
        Districts[i].invalids++;
    }else{
        int j = 0;
        found = false;
        while(j < PARTIES_SZ && !found){ // find party index
            if(Parties[j].pid == pid){
                found = true;
                break;
            }
            j++; // step
        }

        Candidate* cPtr = Parties[j].candidates;
        while(cPtr && cPtr->cid!=cid){ // find candidate
            if(cPtr->cid < cid) cPtr = cPtr->rc;
            else cPtr = cPtr->lc;
        }
        if(!cPtr){
            printf("Candidate not found.\n");
            return;
        }
        cPtr->votes++; // increment candidate votes
        Districts[i].partyVotes[j]++; // increment party votes
        
    }
    


    

    /* finally, print whatever necessary */
    printf("\tDistrict[%d]\n", did);
    printf("\tblanks %d\n", Districts[i].blanks);
    printf("\tinvalids %d\n", Districts[i].invalids);
    printf("\tParty votes: \n");
    for(int k = 0; k < PARTIES_SZ; k++){
        printf("\t%d %d,\n", Parties[k].pid, Districts[i].partyVotes[k]);
    }
    printf("DONE\n");

}

void EventCountVotes(int did) {
    DebugPrint("M %d\n", did);

    /* calculate eklogiko metro*/
    int count = 0, found = 0;
    while(count < 56 && found == 0){ // find the district
        if(Districts[count].did == did){
            found = 1;
            break;
        }
        count++;
    }
    if(found == 0) return; // district not found
    int index = count;
    
    double eklogiko_metro;
    int valid_votes = 0;
    for(int i = 0; i < PARTIES_SZ; i++){ // calculate the votes
        valid_votes+= Districts[index].partyVotes[i];
    }

    if(Districts[index].seats == 0){
        eklogiko_metro = 0;
    }else{
        eklogiko_metro = valid_votes / Districts[index].seats;
    }

    int partyElected[5];
    for(int i = 0; i < PARTIES_SZ; i++){
        if(eklogiko_metro == 0){
            partyElected[i] = 0;
        }else{
            partyElected[i] = floor(Districts[index].partyVotes[i] / eklogiko_metro);
        }
        //TODO: case partyElected is bigger than party Candidates

        Parties[i].electedCount += partyElected[i]; // add to party elected field
        Districts[index].seats -= partyElected[i];  // subtract from district seats

        
    }



}

void EventFormParliament(void) {
    DebugPrint("N\n");
    // TODO
}

void EventPrintDistrict(int did) {
    DebugPrint("I %d\n", did);
    int count = 0, found = 0;
    while(count < DISTRICTS_SZ){
        if(Districts[count].did == did){
            found = 1;
            break;
        }
        count++;
    }
    if(found == 0){
        printf("District with did %d not found!\n", did);
        return;
    }
    
    printf("\tseats: %d\n", Districts[count].seats);
    printf("\tblanks: %d\n", Districts[count].blanks);
    printf("\tinvalids: %d\n", Districts[count].invalids);
    printf("\tpartyVotes\n");
    for(int i=0; i<PARTIES_SZ;i++){
        printf("\t%d %d,\n", Parties[i].pid ,Districts[count].partyVotes[i]);
    }
    printf("DONE\n");   
}

void EventPrintStation(int sid) {
    DebugPrint("J %d\n", sid);
    Station* ptr = StationsHT[hash(sid)];
    while(ptr){
        if(ptr->sid == sid) break;
        ptr = ptr->next;
    }
    if(!ptr){
        printf("Station not found.\nDONE\n");
        return;
    }
    printf("\tregistered %d\n", ptr->registered);
    printf("\tvoters\n");
    printVoters2(ptr->voters);
    printf("DONE\n");
}

void EventPrintParty(int pid) {
    DebugPrint("K %d\n", pid);
    
    printf("\telected\n");
    int count = 0, found = 0;
    while(count < PARTIES_SZ){
        if(Parties[count].pid == pid){
            found = 1;
            break;
        }
        count++;
    }
    if(found == 0) return; // party not found
    printCandidates2(Parties[count].candidates);
    printf("DONE\n");

}

void EventPrintParliament(void) {
    DebugPrint("L\n");
    // TODO
}

void EventBonusUnregisterVoter(int vid, int sid) {
    DebugPrint("BU %d %d\n", vid, sid);
    // TODO
}

void EventBonusFreeMemory(void) {
    DebugPrint("BF\n");
    // TODO
}



