// CodeAlpha - Task 2: Login and Registration System
//
// Features
//  - Registration with username/password validation and duplicate check
//  - Passwords are NEVER stored in plain text: each password is salted and
//    hashed (SHA-256, repeated 10,000 times) before being written to file
//  - Login verifies the entered password against the stored salt + hash
//  - Password input is hidden while typing
//  - Login is locked after 3 failed attempts
//
// Credentials are stored in "users.txt" in the format:  username:salt:hash

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <random>
#include <cstdint>
#include <cctype>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

const string USER_FILE = "users.txt";
const int HASH_ROUNDS = 10000;
const int MAX_LOGIN_ATTEMPTS = 3;

// =====================================================================
//  SHA-256 (self-contained, no external libraries needed)
// =====================================================================

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

string sha256(const string &input) {
    uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                     0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

    // Pre-processing: append 0x80, pad with zeros, append 64-bit length
    vector<uint8_t> msg(input.begin(), input.end());
    uint64_t bitLen = static_cast<uint64_t>(input.size()) * 8;
    msg.push_back(0x80);
    while (msg.size() % 64 != 56) msg.push_back(0x00);
    for (int i = 7; i >= 0; --i) msg.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xff));

    // Process each 512-bit block
    for (size_t off = 0; off < msg.size(); off += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(msg[off + 4 * i]) << 24) |
                   (static_cast<uint32_t>(msg[off + 4 * i + 1]) << 16) |
                   (static_cast<uint32_t>(msg[off + 4 * i + 2]) << 8) |
                   (static_cast<uint32_t>(msg[off + 4 * i + 3]));
        }
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t t1 = hh + S1 + ch + K[i] + w[i];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t t2 = S0 + maj;

            hh = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    ostringstream oss;
    oss << hex << setfill('0');
    for (int i = 0; i < 8; ++i) oss << setw(8) << h[i];
    return oss.str();
}

// =====================================================================
//  Password security helpers
// =====================================================================

// Random 16-byte salt written as 32 hex characters.
string generateSalt() {
    random_device rd;
    ostringstream oss;
    oss << hex << setfill('0');
    for (int i = 0; i < 16; ++i) oss << setw(2) << (rd() & 0xff);
    return oss.str();
}

// Salted, repeated hashing makes brute-forcing and rainbow tables much harder.
string hashPassword(const string &password, const string &salt) {
    string digest = sha256(salt + password);
    for (int i = 0; i < HASH_ROUNDS; ++i) {
        digest = sha256(digest + salt + password);
    }
    return digest;
}

// Compares strings without stopping at the first mismatch (avoids timing leaks).
bool safeEquals(const string &a, const string &b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); ++i) diff |= (a[i] ^ b[i]);
    return diff == 0;
}

// =====================================================================
//  Input helpers
// =====================================================================

string readLine(const string &prompt) {
    cout << prompt << flush;
    string line;
    getline(cin, line);
    return line;
}

// Reads a password without showing it on screen.
string readPassword(const string &prompt) {
    cout << prompt << flush;
    string pw;

#ifdef _WIN32
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == 0 || ch == 0xE0) { _getch(); continue; }   // ignore arrow/function keys
        if (ch == '\b') {
            if (!pw.empty()) { pw.pop_back(); cout << "\b \b" << flush; }
        } else if (isprint(ch)) {
            pw.push_back(static_cast<char>(ch));
            cout << '*' << flush;
        }
    }
    cout << endl;
#else
    termios oldt;
    bool isTerminal = (tcgetattr(STDIN_FILENO, &oldt) == 0);
    if (isTerminal) {
        termios newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    getline(cin, pw);
    if (isTerminal) {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        cout << endl;
    }
#endif
    return pw;
}

string toLower(string s) {
    for (char &c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

// =====================================================================
//  Validation
// =====================================================================

// Username: 3-20 characters, letters, digits and underscore only.
bool isValidUsername(const string &username, string &error) {
    if (username.size() < 3 || username.size() > 20) {
        error = "Username must be 3-20 characters long.";
        return false;
    }
    for (char c : username) {
        if (!isalnum(static_cast<unsigned char>(c)) && c != '_') {
            error = "Username can only contain letters, digits and underscore (_).";
            return false;
        }
    }
    return true;
}

// Password: at least 8 characters, with a letter and a digit, no spaces.
bool isValidPassword(const string &password, const string &username, string &error) {
    if (password.size() < 8) {
        error = "Password must be at least 8 characters long.";
        return false;
    }
    bool hasLetter = false, hasDigit = false;
    for (char c : password) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (isspace(uc)) {
            error = "Password must not contain spaces.";
            return false;
        }
        if (isalpha(uc)) hasLetter = true;
        if (isdigit(uc)) hasDigit = true;
    }
    if (!hasLetter || !hasDigit) {
        error = "Password must contain at least one letter and one digit.";
        return false;
    }
    if (toLower(password) == toLower(username)) {
        error = "Password must not be the same as the username.";
        return false;
    }
    return true;
}

// =====================================================================
//  File storage
// =====================================================================

// Looks up a user. Returns true if found and fills in the stored salt and hash.
bool findUser(const string &username, string &salt, string &hash) {
    ifstream file(USER_FILE);
    if (!file.is_open()) return false;    // no users registered yet

    string line;
    while (getline(file, line)) {
        size_t p1 = line.find(':');
        size_t p2 = (p1 == string::npos) ? string::npos : line.find(':', p1 + 1);
        if (p2 == string::npos) continue; // skip malformed lines

        if (line.substr(0, p1) == username) {
            salt = line.substr(p1 + 1, p2 - p1 - 1);
            hash = line.substr(p2 + 1);
            return true;
        }
    }
    return false;
}

bool saveUser(const string &username, const string &salt, const string &hash) {
    ofstream file(USER_FILE, ios::app);
    if (!file.is_open()) return false;
    file << username << ':' << salt << ':' << hash << '\n';
    return file.good();
}

// =====================================================================
//  Registration and Login
// =====================================================================

void registerUser() {
    cout << "\n----------- REGISTER -----------\n";

    string username = readLine("Choose a username: ");
    string error;

    if (!isValidUsername(username, error)) {
        cout << "[Error] " << error << "\n";
        return;
    }

    username = toLower(username);          // usernames are not case-sensitive

    string s, h;
    if (findUser(username, s, h)) {
        cout << "[Error] Username '" << username << "' is already taken. Try another one.\n";
        return;
    }

    string password = readPassword("Choose a password: ");
    if (!isValidPassword(password, username, error)) {
        cout << "[Error] " << error << "\n";
        return;
    }

    string confirm = readPassword("Confirm password: ");
    if (password != confirm) {
        cout << "[Error] Passwords do not match.\n";
        return;
    }

    string salt = generateSalt();
    string hash = hashPassword(password, salt);

    if (saveUser(username, salt, hash)) {
        cout << "[Success] Registration successful! You can now log in, " << username << ".\n";
    } else {
        cout << "[Error] Could not write to " << USER_FILE << ". Check file permissions.\n";
    }
}

void loginUser() {
    cout << "\n------------- LOGIN -------------\n";

    for (int attempt = 1; attempt <= MAX_LOGIN_ATTEMPTS; ++attempt) {
        string username = toLower(readLine("Username: "));
        string password = readPassword("Password: ");

        string salt, storedHash;
        bool found = findUser(username, salt, storedHash);

        // Always do the hashing work, so response time doesn't reveal
        // whether a username exists.
        if (!found) salt = generateSalt();
        string enteredHash = hashPassword(password, salt);

        if (found && safeEquals(enteredHash, storedHash)) {
            cout << "[Success] Login successful! Welcome back, " << username << ".\n";
            return;
        }

        // Generic message: don't tell the user whether the username or password was wrong
        cout << "[Error] Invalid username or password.";
        if (attempt < MAX_LOGIN_ATTEMPTS) {
            cout << " Attempts left: " << (MAX_LOGIN_ATTEMPTS - attempt) << "\n\n";
        } else {
            cout << "\n[Locked] Too many failed attempts. Returning to main menu.\n";
        }
    }
}

// =====================================================================
//  Main menu
// =====================================================================

int main() {
    while (true) {
        cout << "\n===================================\n";
        cout << "   LOGIN & REGISTRATION SYSTEM\n";
        cout << "===================================\n";
        cout << "1. Register\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";

        string choice = readLine("Choose an option (1-3): ");

        if (choice == "1")      registerUser();
        else if (choice == "2") loginUser();
        else if (choice == "3") { cout << "Goodbye!\n"; break; }
        else                    cout << "[Error] Invalid option. Please enter 1, 2 or 3.\n";

        if (!cin) break;    // input stream closed
    }
    return 0;
}
