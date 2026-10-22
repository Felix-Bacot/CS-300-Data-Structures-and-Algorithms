// CS-300 Project Two: ABCU Advising Assistance Program
// Data structure: Binary Search Tree
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

struct Course {
    string number;
    string title;
    vector<string> prerequisites;
};

string trim(const string& value) {
    const string whitespace = " \t\r\n";
    size_t start = value.find_first_not_of(whitespace);
    if (start == string::npos) return "";
    size_t end = value.find_last_not_of(whitespace);
    return value.substr(start, end - start + 1);
}

string uppercase(string value) {
    transform(value.begin(), value.end(), value.begin(),
              [](unsigned char ch) { return static_cast<char>(toupper(ch)); });
    return value;
}

class CourseTree {
private:
    struct Node {
        Course course;
        unique_ptr<Node> left;
        unique_ptr<Node> right;
        explicit Node(const Course& value) : course(value) {}
    };
    unique_ptr<Node> root;

    static void insertNode(unique_ptr<Node>& node, const Course& course) {
        if (!node) {
            node = make_unique<Node>(course);
        } else if (course.number < node->course.number) {
            insertNode(node->left, course);
        } else if (course.number > node->course.number) {
            insertNode(node->right, course);
        } else {
            node->course = course;
        }
    }

    static void printInOrder(const Node* node) {
        if (!node) return;
        printInOrder(node->left.get());
        cout << node->course.number << ", " << node->course.title << '\n';
        printInOrder(node->right.get());
    }

public:
    void insert(const Course& course) { insertNode(root, course); }

    const Course* search(const string& number) const {
        const Node* current = root.get();
        while (current) {
            if (number == current->course.number) return &current->course;
            current = number < current->course.number
                          ? current->left.get() : current->right.get();
        }
        return nullptr;
    }

    void printAll() const { printInOrder(root.get()); }
    void swap(CourseTree& other) { root.swap(other.root); }
};

bool parseCourse(const string& line, Course& course) {
    stringstream stream(line);
    string field;
    vector<string> fields;
    while (getline(stream, field, ',')) fields.push_back(trim(field));
    if (fields.size() < 2 || fields[0].empty() || fields[1].empty()) return false;

    course.number = uppercase(fields[0]);
    course.title = fields[1];
    course.prerequisites.clear();
    for (size_t i = 2; i < fields.size(); ++i) {
        if (fields[i].empty()) return false;
        course.prerequisites.push_back(uppercase(fields[i]));
    }
    return true;
}

bool loadCourses(const string& filename, CourseTree& tree) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open file '" << filename << "'.\n";
        return false;
    }

    vector<Course> courses;
    unordered_set<string> numbers;
    string line;
    int lineNumber = 0;
    while (getline(file, line)) {
        ++lineNumber;
        if (trim(line).empty()) continue;
        Course course;
        if (!parseCourse(line, course)) {
            cout << "Error: Invalid course data on line " << lineNumber << ".\n";
            return false;
        }
        if (!numbers.insert(course.number).second) {
            cout << "Error: Duplicate course " << course.number << ".\n";
            return false;
        }
        courses.push_back(course);
    }
    if (courses.empty()) {
        cout << "Error: The course file is empty.\n";
        return false;
    }

    // Check all prerequisites against the full set of course numbers.
    for (const Course& course : courses) {
        for (const string& prerequisite : course.prerequisites) {
            if (numbers.count(prerequisite) == 0) {
                cout << "Error: " << course.number << " references missing prerequisite "
                     << prerequisite << ".\n";
                return false;
            }
        }
    }

    // Build a replacement tree so a failed load never destroys existing data.
    CourseTree replacement;
    for (const Course& course : courses) replacement.insert(course);
    tree.swap(replacement);
    cout << "Successfully loaded " << courses.size() << " courses.\n";
    return true;
}

void printCourseDetails(const CourseTree& tree, const string& number) {
    const Course* course = tree.search(uppercase(trim(number)));
    if (!course) {
        cout << "Course not found. Please enter a valid course number.\n";
        return;
    }
    cout << course->number << ", " << course->title << '\n';
    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None\n";
        return;
    }
    cout << "Prerequisites:\n";
    for (const string& number : course->prerequisites) {
        const Course* prerequisite = tree.search(number);
        cout << "  " << number;
        if (prerequisite) cout << ", " << prerequisite->title;
        cout << '\n';
    }
}

int main() {
    CourseTree tree;
    bool loaded = false;
    string choice;
    cout << "Welcome to the course planner.\n";

    while (true) {
        cout << "\n1. Load Data Structure.\n"
             << "2. Print Course List.\n"
             << "3. Print Course.\n"
             << "9. Exit\n\n"
             << "What would you like to do? ";
        if (!getline(cin, choice)) break;
        choice = trim(choice);

        if (choice == "1") {
            cout << "Enter the course data filename (including path if needed): ";
            string filename;
            if (!getline(cin, filename)) break;
            if (loadCourses(trim(filename), tree)) loaded = true;
        } else if (choice == "2") {
            if (!loaded) {
                cout << "Please load the course data first (option 1).\n";
            } else {
                cout << "Here is a sample schedule:\n\n";
                tree.printAll();
            }
        } else if (choice == "3") {
            if (!loaded) {
                cout << "Please load the course data first (option 1).\n";
            } else {
                cout << "What course do you want to know about? ";
                string number;
                if (!getline(cin, number)) break;
                printCourseDetails(tree, number);
            }
        } else if (choice == "9") {
            cout << "Thank you for using the course planner!\n";
            break;
        } else {
            cout << choice << " is not a valid option.\n";
        }
    }
    return 0;
}
