#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>

using namespace std;
#define int long long

// Function Prototypes
string toHex (int);
string HexToBinary (string, map <char, string>&);
void initializeMap (map <char, string>&);
int BinaryToInt (string);
int cache_read (vector <vector<int> >&, vector <vector<int> >&, int, int, string);
int cache_write (vector <vector<int> >&, vector <vector<int> >&, int, int, string, string);

int timecounter = 1;

signed main (void) {

    freopen ("output.txt", "w", stdout);

    map <char, string> binmap;
    initializeMap (binmap);

    srand ((unsigned) time (NULL));

    int size, block, assoc;
    string pol, write, ln;

    ifstream configfile ("cache.config");
    if (configfile.is_open() == false) {
        cout << "Error opening config file\n";
        return 1;
    }

    configfile >> size >> block >> assoc >> pol >> write;

    int setcount, totalhits = 0, totalmisses = 0;
    if (!assoc) setcount = 1;
    else setcount = size / (block * assoc);

    configfile.close();

    ifstream accessfile ("cache.access");
    if (accessfile.is_open() == false) {
        cout << "Error opening access file\n";
        return 1;
    }

    vector <int> emp;
    emp.push_back(1); emp.pop_back();
    vector<vector <int> > tags, lastaccess;
    for (int i = 0; i < setcount; i++){
        tags.push_back(emp);
        lastaccess.push_back(emp);
        for (int j = 0; j < assoc; j++){
            tags[i].push_back(-1);
            lastaccess[i].push_back(0);
        }
    }

    cout << "CS2323 Cache Simulation - CS22BTECH11020\n\n";
    while (getline (accessfile, ln)){
        char mode = ln[0];
        string hex = ln.substr (5, ln.length() - 5);
        while (hex.length() < 8) hex = "0" + hex;
        int address = BinaryToInt(HexToBinary(hex, binmap));
        
        // doing address / block removes the offset part
        int tag = (address / block) / setcount;
        int index = (address / block) % setcount;
        int hit;
        int offset = 0, tm = 1;
        while (tm != block){
            tm *= 2;
            offset++;
        }

        if (mode == 'R') hit = cache_read (tags, lastaccess, index, tag, pol);
        else hit = cache_write (tags, lastaccess, index, tag, pol, write);

        if (hit) totalhits++;
        else totalmisses++;

        cout << "Address: 0x" + hex << ", " << "Set: " << toHex (index)
        << (hit ? ", Hit" : ", Miss") << ", Tag: " << toHex (tag) << '\n';
        timecounter++;
    }

    cout << "\nTotal Hits: " << totalhits << '\n';
    cout << "Total Misses: " << totalmisses << '\n';
    cout << "Total Accesses: " << totalhits + totalmisses << '\n';

}

// Handles Cache reads
int cache_read (vector <vector<int> >& tags, vector <vector<int> >& la, int index, int tag, string pol){

    int freeloc = -1, ind = -1;

    for (int i = 0; i < tags[index].size(); i++){
        if (tags[index][i] == tag){
            ind = i;
            break;
        }
        if (tags[index][i] == -1) freeloc = i;
    }

    if (ind > -1){
        tags[index][ind] = tag;
        if (pol == "LRU") la[index][ind] = timecounter;
        return 1;
    }
    else if (freeloc > -1){
        tags[index][freeloc] = tag;
        la[index][freeloc] = timecounter;
        return 0;
    }
    else{

        if (pol == "RANDOM"){
            ind = rand() % tags[index].size();
            tags[index][ind] = tag;
            la[index][ind] = timecounter;
            return 0;
        }

        int mt = 1e17;
        for (int i = 0; i < tags[index].size(); i++){
            if (la[index][i] < mt){
                ind = i;
                mt = la[index][i];
            }
        }
        tags[index][ind] = tag;
        la[index][ind] = timecounter;
        return 0;
    }

}

// Handles Cache writes
int cache_write (vector <vector<int> >& tags, vector <vector<int> >& la, int index, int tag, string pol, string write){

    if (write == "WB") return cache_read (tags, la, index, tag, pol);
    else {
        int ind = -1;
        for (int i = 0; i < tags[index].size(); i++){
            if (tags[index][i] == tag){
                ind = i;
                break;
            }
        }
        if (ind == -1) return 0;
        else {
            if (pol == "LRU") la[index][ind] = timecounter;
            return 1;
        }
    }

}

// Converts Hexadecimal to Binary
string HexToBinary (string hex, map <char, string>& binmap){
    string bin = "", bad = "00000000000000000000000000000000";
    for (int i = 0; i < hex.length(); i++){
        hex[i] = tolower(hex[i]);
        if (!(hex[i] <= '9' && hex[i] >= '0') && !(hex[i] <= 'f' && hex[i] >= 'a')){
            bin = bad;
            break;
        }
        bin += binmap[hex[i]];
    }
    return bin;
}

// Initializes the Hexadecimal to Binary Map
void initializeMap (map <char, string>& binmap){
    binmap['1'] = "0001";   binmap['5'] = "0101";   binmap['9'] = "1001";   binmap['d'] = "1101";
    binmap['2'] = "0010";   binmap['6'] = "0110";   binmap['a'] = "1010";   binmap['e'] = "1110";
    binmap['3'] = "0011";   binmap['7'] = "0111";   binmap['b'] = "1011";   binmap['f'] = "1111";
    binmap['4'] = "0100";   binmap['8'] = "1000";   binmap['c'] = "1100";   binmap['0'] = "0000";
}


// Converts Binary to Integer
int BinaryToInt (string bin){
    int bit = 1, ans = 0;
    reverse (bin.begin(), bin.end());
    for (int i = 0; i < bin.length(); i++){
        ans += (bin[i] == '1') * bit;
        bit *= 2;
    }
    return ans;
}

// Converts Int to Hex
string toHex (int num){
    string hex = "";
    while (num){
        int r = num % 16;
        char c;
        if (r < 10) c = '0' + r;
        else c = 'a' + (r - 10);
        hex += c;
        num /= 16;
    }
    while (hex.length() < 8) hex += '0';
    hex += "x0";
    reverse (hex.begin(), hex.end());
    return hex; 
}