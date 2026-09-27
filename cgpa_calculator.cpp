// CodeAlpha - Task 1: CGPA Calculator
// Calculates semester GPA and overall CGPA on a 4.0 scale.

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <limits>
#include <cctype>

using namespace std;

struct Course {
    string name;
    string letterGrade;
    double gradePoint;
    double creditHours;
};

// ---------- Input helpers ----------

int readInt(const string &prompt, int minVal) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minVal) {
            return value;
        }
        cout << "  Invalid input. Enter a whole number >= " << minVal << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double readCredits(const string &prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value && value > 0 && value <= 10) {
            return value;
        }
        cout << "  Invalid input. Credit hours must be between 0 and 10.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Converts a letter grade (A, A-, B+, ...) to grade points.
// Returns -1.0 if the grade is not recognised.
double gradeToPoints(string grade) {
    for (char &c : grade) c = toupper(static_cast<unsigned char>(c));

    if (grade == "A+" || grade == "A") return 4.0;
    if (grade == "A-") return 3.7;
    if (grade == "B+") return 3.3;
    if (grade == "B")  return 3.0;
    if (grade == "B-") return 2.7;
    if (grade == "C+") return 2.3;
    if (grade == "C")  return 2.0;
    if (grade == "C-") return 1.7;
    if (grade == "D+") return 1.3;
    if (grade == "D")  return 1.0;
    if (grade == "F")  return 0.0;
    return -1.0;
}

string readGrade(const string &prompt, double &points) {
    string grade;
    while (true) {
        cout << prompt;
        cin >> grade;
        points = gradeToPoints(grade);
        if (points >= 0.0) {
            for (char &c : grade) c = toupper(static_cast<unsigned char>(c));
            return grade;
        }
        cout << "  Invalid grade. Use A, A-, B+, B, B-, C+, C, C-, D+, D or F.\n";
    }
}

// ---------- Display helpers ----------

void printGradeScale() {
    cout << "\nGrade scale (4.0):\n"
         << "  A/A+ = 4.0   A- = 3.7   B+ = 3.3   B = 3.0   B- = 2.7\n"
         << "  C+   = 2.3   C  = 2.0   C- = 1.7   D+ = 1.3   D  = 1.0   F = 0.0\n\n";
}

void printSemesterTable(const vector<Course> &courses) {
    cout << "\n" << left
         << setw(5)  << "No."
         << setw(22) << "Course"
         << setw(8)  << "Grade"
         << setw(10) << "Points"
         << setw(10) << "Credits"
         << setw(12) << "Grade x Cr" << "\n";
    cout << string(67, '-') << "\n";

    for (size_t i = 0; i < courses.size(); ++i) {
        const Course &c = courses[i];
        cout << left
             << setw(5)  << (i + 1)
             << setw(22) << c.name.substr(0, 20)
             << setw(8)  << c.letterGrade
             << fixed << setprecision(2)
             << setw(10) << c.gradePoint
             << setw(10) << c.creditHours
             << setw(12) << (c.gradePoint * c.creditHours) << "\n";
    }
    cout << string(67, '-') << "\n";
}

// ---------- Main program ----------

int main() {
    cout << "===================================\n";
    cout << "        CGPA CALCULATOR\n";
    cout << "===================================\n";
    printGradeScale();

    int numSemesters = readInt("Enter number of semesters: ", 1);

    double overallCredits = 0.0;
    double overallPoints = 0.0;
    vector<double> semesterGPAs;

    for (int s = 1; s <= numSemesters; ++s) {
        cout << "\n----------- Semester " << s << " -----------\n";
        int numCourses = readInt("Enter number of courses: ", 1);

        vector<Course> courses;
        double semCredits = 0.0;
        double semPoints = 0.0;

        for (int i = 1; i <= numCourses; ++i) {
            Course c;
            cout << "\nCourse " << i << ":\n";

            cout << "  Course name: ";
            cin >> ws;
            getline(cin, c.name);
            if (c.name.empty()) c.name = "Course " + to_string(i);

            c.letterGrade = readGrade("  Grade (e.g. A, B+): ", c.gradePoint);
            c.creditHours = readCredits("  Credit hours: ");

            semCredits += c.creditHours;
            semPoints += c.gradePoint * c.creditHours;
            courses.push_back(c);
        }

        double semGPA = semPoints / semCredits;
        semesterGPAs.push_back(semGPA);
        overallCredits += semCredits;
        overallPoints += semPoints;

        cout << "\n=========== Semester " << s << " Result ===========";
        printSemesterTable(courses);
        cout << fixed << setprecision(2)
             << "Total credits      : " << semCredits << "\n"
             << "Total grade points : " << semPoints << "\n"
             << "Semester GPA       : " << semGPA << "\n";
    }

    double cgpa = overallPoints / overallCredits;

    cout << "\n===================================\n";
    cout << "         FINAL SUMMARY\n";
    cout << "===================================\n";
    cout << fixed << setprecision(2);
    for (size_t i = 0; i < semesterGPAs.size(); ++i) {
        cout << "Semester " << (i + 1) << " GPA : " << semesterGPAs[i] << "\n";
    }
    cout << "-----------------------------------\n";
    cout << "Total credits      : " << overallCredits << "\n";
    cout << "Total grade points : " << overallPoints << "\n";
    cout << "Overall CGPA       : " << cgpa << "\n";
    cout << "===================================\n";

    return 0;
}
