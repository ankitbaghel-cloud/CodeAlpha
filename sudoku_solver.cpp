// CodeAlpha - Task 3: Sudoku Solver
// Solves a 9x9 Sudoku puzzle using recursive backtracking.

#include <iostream>
#include <string>
#include <cctype>

using namespace std;

const int N = 9;

// The Sudoku grid is stored as a 2D array. 0 means "empty cell".
int grid[N][N];

// ---------- Display ----------

void printGrid(const int g[N][N]) {
    for (int r = 0; r < N; ++r) {
        if (r % 3 == 0) cout << "+-------+-------+-------+\n";
        for (int c = 0; c < N; ++c) {
            if (c % 3 == 0) cout << "| ";
            if (g[r][c] == 0) cout << ". ";
            else              cout << g[r][c] << " ";
        }
        cout << "|\n";
    }
    cout << "+-------+-------+-------+\n";
}

// ---------- Rule checking ----------

// Can `num` be placed at (row, col) without breaking any Sudoku rule?
// The cell itself is ignored, so this also works for validating filled cells.
bool isSafe(int g[N][N], int row, int col, int num) {
    // Row constraint
    for (int c = 0; c < N; ++c)
        if (c != col && g[row][c] == num) return false;

    // Column constraint
    for (int r = 0; r < N; ++r)
        if (r != row && g[r][col] == num) return false;

    // 3x3 sub-grid constraint
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int r = startRow; r < startRow + 3; ++r)
        for (int c = startCol; c < startCol + 3; ++c)
            if ((r != row || c != col) && g[r][c] == num) return false;

    return true;
}

// Checks that the starting puzzle doesn't already contain a conflict.
bool isInitialGridValid(int g[N][N]) {
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            if (g[r][c] != 0 && !isSafe(g, r, c, g[r][c]))
                return false;
    return true;
}

// ---------- Backtracking solver ----------

// Finds the next empty cell. Returns false if the grid is full.
bool findEmptyCell(int g[N][N], int &row, int &col) {
    for (row = 0; row < N; ++row)
        for (col = 0; col < N; ++col)
            if (g[row][col] == 0) return true;
    return false;
}

bool solveSudoku(int g[N][N]) {
    int row, col;

    // Base case: no empty cells left -> puzzle solved
    if (!findEmptyCell(g, row, col)) return true;

    // Try digits 1-9 in the empty cell
    for (int num = 1; num <= 9; ++num) {
        if (isSafe(g, row, col, num)) {
            g[row][col] = num;          // place the number

            if (solveSudoku(g))         // recurse
                return true;

            g[row][col] = 0;            // backtrack
        }
    }
    return false;                       // trigger backtracking in the caller
}

// ---------- Input ----------

void loadSamplePuzzle() {
    int sample[N][N] = {
        {5, 3, 0, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}
    };
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            grid[r][c] = sample[r][c];
}

// Reads 9 rows of 9 characters. Digits 1-9 are clues; 0 or '.' means empty.
void readPuzzleFromUser() {
    cout << "\nEnter the puzzle row by row (9 rows).\n"
         << "Each row: 9 characters, digits 1-9 for clues, 0 or . for empty cells.\n"
         << "Example: 530070000\n\n";

    for (int r = 0; r < N; ++r) {
        while (true) {
            string line;
            cout << "Row " << (r + 1) << ": ";
            cin >> line;

            bool ok = (line.size() == N);
            for (size_t i = 0; ok && i < line.size(); ++i) {
                if (!isdigit(static_cast<unsigned char>(line[i])) && line[i] != '.')
                    ok = false;
            }

            if (!ok) {
                cout << "  Invalid row. Enter exactly 9 characters (digits or '.').\n";
                continue;
            }

            for (int c = 0; c < N; ++c)
                grid[r][c] = (line[c] == '.') ? 0 : line[c] - '0';
            break;
        }
    }
}

// ---------- Main ----------

int main() {
    cout << "===================================\n";
    cout << "          SUDOKU SOLVER\n";
    cout << "===================================\n";
    cout << "1. Enter my own puzzle\n";
    cout << "2. Use sample puzzle\n";
    cout << "Choose (1/2): ";

    int choice;
    cin >> choice;

    if (choice == 2) loadSamplePuzzle();
    else             readPuzzleFromUser();

    cout << "\nInput puzzle:\n";
    printGrid(grid);

    if (!isInitialGridValid(grid)) {
        cout << "\nThe puzzle breaks Sudoku rules (duplicate number in a row, "
             << "column or 3x3 box). Please check your input.\n";
        return 1;
    }

    if (solveSudoku(grid)) {
        cout << "\nSolved puzzle:\n";
        printGrid(grid);
    } else {
        cout << "\nNo solution exists for this puzzle.\n";
    }

    return 0;
}
