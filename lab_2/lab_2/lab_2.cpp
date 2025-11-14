#include <iostream>
#include <random>
#include <set>
#include <vector>
#include <regex>
using namespace std;

const string alphabet = "ab";
const regex regular("b*((ab*a)*(aabb|(babb)*))*");

string generateRandomString(int n) {
    string s;
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<size_t> dist(0, alphabet.size() - 1);
    for (int i = 0; i < n; ++i) {
        s += alphabet[dist(gen)];
    }
    return s;
}

bool DFA(const string& s) {
    vector<bool> final_states(16, false);
    final_states[1] = final_states[2] = final_states[3] = final_states[5] = 
    final_states[6] = final_states[9] = final_states[14] = true;
    
    vector<vector<int>> transitions(16, vector<int>(2, -1));
    
    transitions[0][0] = 1;
    transitions[0][1] = 10;
    
    transitions[1][0] = 0;
    transitions[1][1] = 4;
    
    transitions[2][0] = 0;
    transitions[2][1] = 12;
    
    transitions[3][0] = 9;
    transitions[3][1] = 0;
    
    transitions[4][0] = 11;
    transitions[4][1] = 2;
    
    transitions[5][0] = 0;
    transitions[5][1] = 6;
    
    transitions[6][0] = 7;
    transitions[6][1] = 6;
    
    transitions[7][0] = 1;
    transitions[7][1] = 8;
    
    transitions[8][0] = 2;
    transitions[8][1] = 3;
    
    transitions[9][0] = 14;
    transitions[9][1] = 0;
    
    transitions[10][0] = 2;
    transitions[10][1] = 10;
    
    transitions[11][0] = -1;
    transitions[11][1] = 13;
    
    transitions[12][0] = 11;
    transitions[12][1] = -1;
    
    transitions[13][0] = -1;
    transitions[13][1] = 2;
    
    transitions[14][0] = 14;
    transitions[14][1] = 15;
    
    transitions[15][0] = 1;
    transitions[15][1] = 3;
    
    int current_state = 5;
    for (char c : s) {
        int symbol_index = (c == 'a') ? 0 : 1;
        int next_state = transitions[current_state][symbol_index];
        
        if (next_state == -1) {
            return false;
        }
        current_state = next_state;
    }
    
    return final_states[current_state];
}

bool NFA(const string &s) {
    vector<vector<vector<int>>> transitions(6, 
        vector<vector<int>>(2));
    transitions[0][0] = {1, 3};
    transitions[0][1] = {0, 3};
    transitions[1][0] = {2};
    transitions[1][1] = {1};
    transitions[2][0] = {1, 3};
    transitions[2][1] = {3};
    transitions[3][0] = {4};
    transitions[4][1] = {5};
    transitions[5][1] = {2};
    
    set<int> current = {0};
    for (char c : s) {
        set<int> next;
        int idx = (c == 'a') ? 0 : 1;
        for (int state : current) {
            for (int next_state : transitions[state][idx]) {
                next.insert(next_state);
            }
        }
        if (next.empty()) 
            return false;
        current = next;
    }
    return current.count(0) || current.count(2);
}

bool fuzz(string s) {
    return (regex_match(s, regular) == DFA(s) && DFA(s) == NFA(s));
}

int main() {
    string s;
    for (int i = 0; i < 10000; ++i) {
        s = generateRandomString(20);
        if (!fuzz(s)) {
            cout << "bad" << endl;
            cout << s << "\n" << regex_match(s, regular) << " " << DFA(s) << " " << NFA(s) << endl;
            return 0;
        }
    }
    cout << "good" << endl;
    return 0;
}