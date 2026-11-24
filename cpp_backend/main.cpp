#include <bits/stdc++.h>
using namespace std;

// ============================================================
// 1. DATA STRUCTURES
// ============================================================

struct Edge {
    int from, to;
    char symbol; // 0 represents epsilon (lambda)
};

struct NFA {
    int count = 0;
    int start = -1, accept = -1;
    vector<Edge> edges;

    int newState() { return count++; }
    void addEdge(int u, int v, char c) { edges.push_back({u, v, c}); }
};

struct Fragment {
    int start, accept;
};

struct DFA {
    vector<set<int>> states;       // Each DFA state is a set of NFA states
    vector<map<char, int>> trans;  // DFA transition function
    set<int> accepting;
    vector<char> alphabet;
};

using StateSet = set<int>;

// ============================================================
// 2. REGULAR EXPRESSION PREPROCESSING AND VALIDATION
// ============================================================

bool isOperand(char c) {
    return isalnum(static_cast<unsigned char>(c)) || c == '_';
}

bool canEnd(char c) {
    return isOperand(c) || c == ')' || c == '*';
}

bool canStart(char c) {
    return isOperand(c) || c == '(';
}

string preprocess(const string& input) {
    string result;
    for (char c : input) {
        if (isspace(static_cast<unsigned char>(c))) continue;
        if (!result.empty() && canEnd(result.back()) && canStart(c))
            result += '.'; // make implicit concatenation explicit
        result += c;
    }
    return result;
}

void validate(const string& regex) {
    if (regex.empty()) throw runtime_error("Regular expression is empty.");

    int depth = 0;
    bool needOperand = true;

    for (char c : regex) {
        if (isOperand(c)) {
            if (!needOperand) throw runtime_error("Missing operator between operands.");
            needOperand = false;
        }
        else if (c == '(') {
            if (!needOperand) throw runtime_error("Missing concatenation before '('.");
            ++depth;
            needOperand = true;
        }
        else if (c == ')') {
            if (needOperand || depth == 0)
                throw runtime_error("Empty or unmatched parentheses.");
            --depth;
            needOperand = false;
        }
        else if (c == '*') {
            if (needOperand) throw runtime_error("Kleene star has no operand.");
            needOperand = false;
        }
        else if (c == '+' || c == '.') {
            if (needOperand) throw runtime_error("Binary operator is missing its left operand.");
            needOperand = true;
        }
        else {
            throw runtime_error(string("Unsupported character: ") + c +
                                ". Use + for union, * for Kleene star.");
        }
    }

    if (depth != 0) throw runtime_error("Unmatched opening parenthesis.");
    if (needOperand) throw runtime_error("Expression ends with an operator.");
}

// ============================================================
// 3. INFIX TO POSTFIX (SHUNTING-YARD ALGORITHM)
// * has highest precedence, concatenation next, + has lowest.
// ============================================================

int precedence(char c) {
    if (c == '+') return 1; // union
    if (c == '.') return 2; // concatenation
    return 0;
}

string toPostfix(const string& regex) {
    string output;
    vector<char> operators;

    for (char c : regex) {
        if (isOperand(c)) output += c;
        else if (c == '*') output += c; // postfix unary operator
        else if (c == '(') operators.push_back(c);
        else if (c == ')') {
            while (!operators.empty() && operators.back() != '(') {
                output += operators.back();
                operators.pop_back();
            }
            if (operators.empty()) throw runtime_error("Mismatched parentheses.");
            operators.pop_back();
        }
        else {
            while (!operators.empty() && operators.back() != '(' &&
                   precedence(operators.back()) >= precedence(c)) {
                output += operators.back();
                operators.pop_back();
            }
            operators.push_back(c);
        }
    }

    while (!operators.empty()) {
        if (operators.back() == '(') throw runtime_error("Mismatched parentheses.");
        output += operators.back();
        operators.pop_back();
    }
    return output;
}

// ============================================================
// 4. THOMPSON'S CONSTRUCTION: REGULAR EXPRESSION TO NFA
// Epsilon-NFA is an internal intermediate representation.
// ============================================================

NFA buildNFA(const string& postfix) {
    NFA nfa;
    vector<Fragment> fragments;

    for (char c : postfix) {
        if (isOperand(c)) {
            int a = nfa.newState();
            int b = nfa.newState();
            nfa.addEdge(a, b, c);
            fragments.push_back({a, b});
        }
        else if (c == '.') { // concatenation
            if (fragments.size() < 2) throw runtime_error("Invalid concatenation.");
            Fragment right = fragments.back(); fragments.pop_back();
            Fragment left = fragments.back(); fragments.pop_back();
            nfa.addEdge(left.accept, right.start, 0);
            fragments.push_back({left.start, right.accept});
        }
        else if (c == '+') { // union, using course notation
            if (fragments.size() < 2) throw runtime_error("Invalid union.");
            Fragment right = fragments.back(); fragments.pop_back();
            Fragment left = fragments.back(); fragments.pop_back();
            int a = nfa.newState(), b = nfa.newState();
            nfa.addEdge(a, left.start, 0);
            nfa.addEdge(a, right.start, 0);
            nfa.addEdge(left.accept, b, 0);
            nfa.addEdge(right.accept, b, 0);
            fragments.push_back({a, b});
        }
        else if (c == '*') { // zero or more repetitions
            if (fragments.empty()) throw runtime_error("Kleene star has no operand.");
            Fragment f = fragments.back(); fragments.pop_back();
            int a = nfa.newState(), b = nfa.newState();
            nfa.addEdge(a, f.start, 0); // enter the expression
            nfa.addEdge(a, b, 0);       // zero occurrences
            nfa.addEdge(f.accept, f.start, 0); // repeat
            nfa.addEdge(f.accept, b, 0);       // exit
            fragments.push_back({a, b});
        }
    }

    if (fragments.size() != 1) throw runtime_error("Invalid regular expression.");
    nfa.start = fragments.back().start;
    nfa.accept = fragments.back().accept;
    return nfa;
}

// ============================================================
// 5. EPSILON CLOSURE AND MOVE
// ============================================================

StateSet epsilonClosure(const NFA& nfa, StateSet states) {
    vector<int> work(states.begin(), states.end());
    while (!work.empty()) {
        int u = work.back();
        work.pop_back();
        for (const Edge& edge : nfa.edges) {
            if (edge.from == u && edge.symbol == 0 && !states.count(edge.to)) {
                states.insert(edge.to);
                work.push_back(edge.to);
            }
        }
    }
    return states;
}

StateSet moveOn(const NFA& nfa, const StateSet& states, char symbol) {
    StateSet result;
    for (const Edge& edge : nfa.edges)
        if (edge.symbol == symbol && states.count(edge.from))
            result.insert(edge.to);
    return result;
}

// ============================================================
// 6. SUBSET CONSTRUCTION: NFA TO DFA
// ============================================================

string setKey(const StateSet& states) {
    string key;
    for (int state : states) key += to_string(state) + ",";
    return key;
}

DFA buildDFA(const NFA& nfa) {
    DFA dfa;
    set<char> symbols;
    for (const Edge& edge : nfa.edges)
        if (edge.symbol != 0) symbols.insert(edge.symbol);
    dfa.alphabet.assign(symbols.begin(), symbols.end());

    map<string, int> ids;
    StateSet initial = epsilonClosure(nfa, {nfa.start});
    dfa.states.push_back(initial);
    dfa.trans.emplace_back();
    ids[setKey(initial)] = 0;

    queue<int> pending;
    pending.push(0);

    while (!pending.empty()) {
        int current = pending.front();
        pending.pop();

        if (dfa.states[current].count(nfa.accept))
            dfa.accepting.insert(current);

        for (char symbol : dfa.alphabet) {
            StateSet next = epsilonClosure(nfa, moveOn(nfa, dfa.states[current], symbol));
            if (next.empty()) {
                dfa.trans[current][symbol] = -1; // implicit rejecting/dead transition
                continue;
            }

            string key = setKey(next);
            if (!ids.count(key)) {
                int newId = static_cast<int>(dfa.states.size());
                ids[key] = newId;
                dfa.states.push_back(next);
                dfa.trans.emplace_back();
                pending.push(newId);
            }
            dfa.trans[current][symbol] = ids[key];
        }
    }
    return dfa;
}

// ============================================================
// 7. DFA DIAGRAM GENERATION (Graphviz DOT)
// ============================================================

void writeDFADot(const DFA& dfa, const string& path) {
    ofstream file(path);
    if (!file) throw runtime_error("Could not write outputs/dfa.dot.");

    file << "digraph DFA {\nrankdir=LR;\nnode [shape=circle];\nstart [shape=point];\nstart -> q0;\n";
    for (int state : dfa.accepting)
        file << "q" << state << " [shape=doublecircle];\n";

    for (int state = 0; state < static_cast<int>(dfa.trans.size()); ++state) {
        for (char symbol : dfa.alphabet) {
            auto it = dfa.trans[state].find(symbol);
            if (it != dfa.trans[state].end() && it->second >= 0)
                file << "q" << state << " -> q" << it->second
                     << " [label=\"" << symbol << "\"];\n";
        }
    }
    file << "}\n";
}

// ============================================================
// 8. MAIN: INPUT, CONVERSION, DFA TABLE AND STRING SIMULATION
// ============================================================

int main() {
    string rawRegex, inputString;
    getline(cin, rawRegex);
    getline(cin, inputString);

    try {
        string processed = preprocess(rawRegex);
        validate(processed);
        string postfix = toPostfix(processed);

        NFA nfa = buildNFA(postfix);
        DFA dfa = buildDFA(nfa);

        // Simple machine-readable output consumed by the Python interface.
        cout << "POSTFIX=" << postfix << "\n";
        cout << "DFA_SYMBOLS=";
        for (int i = 0; i < static_cast<int>(dfa.alphabet.size()); ++i) {
            if (i) cout << ",";
            cout << dfa.alphabet[i];
        }
        if (dfa.alphabet.empty()) cout << "-";
        cout << "\n";

        for (int state = 0; state < static_cast<int>(dfa.states.size()); ++state) {
            cout << "DFA_ROW=" << state;
            for (char symbol : dfa.alphabet) {
                auto it = dfa.trans[state].find(symbol);
                cout << "," << symbol << ":";
                if (it == dfa.trans[state].end() || it->second < 0) cout << "-";
                else cout << it->second;
            }
            cout << "\n";
        }

        cout << "DFA_ACCEPT=";
        if (dfa.accepting.empty()) cout << "-";
        else {
            bool first = true;
            for (int state : dfa.accepting) {
                if (!first) cout << ",";
                cout << state;
                first = false;
            }
        }
        cout << "\n";

        // Simulate the input string on the generated DFA.
        int current = 0;
        vector<int> trace = {0};
        for (char c : inputString) {
            if (isspace(static_cast<unsigned char>(c))) continue;
            auto it = dfa.trans[current].find(c);
            if (it == dfa.trans[current].end() || it->second < 0) {
                current = -1;
                trace.push_back(-1);
                break;
            }
            current = it->second;
            trace.push_back(current);
        }

        bool accepted = current >= 0 && dfa.accepting.count(current);
        cout << "TRACE=";
        for (int i = 0; i < static_cast<int>(trace.size()); ++i) {
            if (i) cout << "->";
            if (trace[i] < 0) cout << "DEAD";
            else cout << "q" << trace[i];
        }
        cout << "\nRESULT=" << (accepted ? "ACCEPTED" : "REJECTED") << "\n";

        writeDFADot(dfa, "outputs/dfa.dot");
    }
    catch (const exception& error) {
        cout << "ERROR=" << error.what() << "\n";
        return 1;
    }
    return 0;
}
