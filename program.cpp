#include "splashkit.h"
#include <string>
#include <algorithm>
#include <vector>
#include <fstream>
using namespace std;

// Student struct
struct Student {
    string name;
    string ID;
    int grade;
};

// Comparison functions for sorting
bool compareByName(const Student &a, const Student &b) { return a.name < b.name; }
bool compareByGrade(const Student &a, const Student &b) { return a.grade > b.grade; }

// Student Manager Class
class StudentManager {
private:
    Student* students;
    int capacity;
    int studentCount;

    // Resize dynamic array
    void resize() {
        int newCapacity = capacity * 2;
        Student* newArray = new Student[newCapacity];
        for (int i = 0; i < studentCount; i++) {
            newArray[i] = students[i];
        }
        delete[] students;
        students = newArray;
        capacity = newCapacity;
    }

    // Wait for back button or keyboard
    void wait_for_back() {
        draw_rectangle(COLOR_BLACK, 200, 350, 200, 40);
        draw_text("Back", COLOR_BLACK, "Arial", 20, 280, 360);
        refresh_screen();

        bool waiting = true;
        while (waiting) {
            process_events();
            if (window_close_requested("Student Manager")) exit(0);

            if (mouse_clicked(LEFT_BUTTON)) {
                if (mouse_x() >= 200 && mouse_x() <= 400 &&
                    mouse_y() >= 350 && mouse_y() <= 390) {
                    waiting = false;
                }
            }
            if (key_typed(RETURN_KEY) || key_typed(SPACE_KEY)) waiting = false;
        }
    }

public:
    // Constructor and Destructor 
    StudentManager(int initialCapacity = 100)
        : capacity(initialCapacity), studentCount(0) {
        students = new Student[capacity];
    }

    ~StudentManager() { delete[] students; }

    // Ultility Check if string is a number
    static bool isnumber(const string &s) {
        if (s.empty()) return false;
        for (char c : s)
            if (!isdigit(c)) return false;
        return true;
    }

    // Efficient text input with real-time display
    string get_text_input(const string &label, int x, int y, int width, int height) {
        string input;
        bool typing = true;

        while (any_key_pressed()) { process_events(); refresh_screen(60); }

        bool prev_key[256] = { false };
        int repeat_timer[256] = { 0 };
        int repeat_delay = 300, repeat_interval = 50;

        while (typing) {
            process_events();
            if (window_close_requested("Student Manager")) exit(0);

            for (int c = 32; c <= 126; c++) {
                bool curr = key_down(static_cast<key_code>(c));
                if (curr && (!prev_key[c] || repeat_timer[c] <= 0)) {
                    input += static_cast<char>(c);
                    repeat_timer[c] = prev_key[c] ? repeat_interval : repeat_delay;
                }
                prev_key[c] = curr;
                if (repeat_timer[c] > 0) repeat_timer[c] -= 16;
            }

            bool back_curr = key_down(BACKSPACE_KEY);
            if (back_curr && (!prev_key[BACKSPACE_KEY] || repeat_timer[BACKSPACE_KEY] <= 0)) {
                if (!input.empty()) input.pop_back();
                repeat_timer[BACKSPACE_KEY] = repeat_delay;
            }
            prev_key[BACKSPACE_KEY] = back_curr;
            if (repeat_timer[BACKSPACE_KEY] > 0) repeat_timer[BACKSPACE_KEY] -= 16;

            if (key_down(RETURN_KEY)) typing = false;

            clear_screen(COLOR_WHITE);
            draw_text(label, COLOR_BLACK, "Arial", 18, x, y - 30);
            draw_rectangle(COLOR_BLACK, x, y, width, height);
            draw_text(input + "_", COLOR_BLACK, "Arial", 18, x + 5, y + 5);
            refresh_screen(60);
        }
        return input;
    }

    void add_new_student() {
        if (studentCount >= capacity) resize();

        string name;
        while (true) {
            name = get_text_input("Enter student name:", 50, 100, 400, 30);
            if (!name.empty()) break;  // valid name
            clear_screen(COLOR_WHITE);
            draw_text("Name cannot be empty. Please enter a valid name.", COLOR_RED, "Arial", 18, 50, 160);
            wait_for_back();
        }

        // --- Get student ID ---
        string id;
        while (true) {
            id = get_text_input("Enter student ID:", 50, 160, 400, 30);
            if (id.empty()) {
                clear_screen(COLOR_WHITE);
                draw_text("ID cannot be empty. Please enter a valid ID.", COLOR_RED, "Arial", 18, 50, 220);
                wait_for_back();
                continue;
            }
            if (find_student_by_id(id)) { 
                clear_screen(COLOR_WHITE);
                draw_text("This ID already exists. Please enter a different ID.", COLOR_RED, "Arial", 18, 50, 220);
                wait_for_back();
                continue;
            }
            break; // ID is valid and unique
        }
        
        string gradeStr;
        while (true) {
            gradeStr = get_text_input("Enter student grade (number):", 50, 220, 400, 30);
            if (isnumber(gradeStr)) break;
            clear_screen(COLOR_WHITE);
            draw_text("Invalid grade input. Please enter a number.", COLOR_RED, "Arial", 20, 50, 280);
            wait_for_back();
        }

        students[studentCount++] = { name, id, stoi(gradeStr) };
        clear_screen(COLOR_WHITE);
        draw_text("Student added successfully!", COLOR_GREEN, "Arial", 20, 50, 280);
        wait_for_back();
    }

    int display_method() {
        while (true) {
            process_events();
            clear_screen(COLOR_WHITE);

            int buttonWidth = 200;
            int buttonHeight = 50;
            int yPos = 20;
            int x1 = 50;  // Sort by name
            int x2 = 300; // Sort by grade
            int x3 = 175; // Display by name
            int x4 = 175; // Export CSV at bottom
            // Draw buttons
            draw_rectangle(COLOR_LIGHT_GRAY, x1, yPos, buttonWidth, buttonHeight);
            draw_rectangle(COLOR_LIGHT_GRAY, x2, yPos, buttonWidth, buttonHeight);
            draw_rectangle(COLOR_LIGHT_GRAY, x3, yPos + 90, buttonWidth, buttonHeight);
            draw_rectangle(COLOR_LIGHT_GRAY, x4, yPos + 320, buttonWidth, buttonHeight);

            draw_text("Sort by Name", COLOR_BLACK, "Arial", 20, x1 + 20, yPos + 15);
            draw_text("Sort by grade", COLOR_BLACK, "Arial", 20, x2 + 40, yPos + 15);
            draw_text("Display by Name", COLOR_BLACK, "Arial", 20, x3 + 40, yPos + 105);
            draw_text("Export to CSV", COLOR_BLACK, "Arial", 20, x4 + 30, yPos + 335);

            if (mouse_clicked(LEFT_BUTTON)) {
                int mx = mouse_x();
                int my = mouse_y();
                // Sort by Name button
                if (mx >= x1 && mx <= x1 + buttonWidth && my >= yPos && my <= yPos + buttonHeight) 
                    return 1;
                // Sort by Grade button  
                if (mx >= x2 && mx <= x2 + buttonWidth && my >= yPos && my <= yPos + buttonHeight) 
                    return 2;
                // Display by Name button
                if (mx >= x3 && mx <= x3 + buttonWidth && my >= yPos + 90 && my <= yPos + 90 + buttonHeight) 
                    return 3;
                // Export CSV button
                if (mx >= x4 && mx <= x4 + buttonWidth && my >= yPos + 320 && my <= yPos + 320 + buttonHeight) 
                    return 4;
            }

            if (key_typed(ESCAPE_KEY) || key_typed(RETURN_KEY)) return 0; // Cancel
            refresh_screen();
        }
    }

    void display_students() {
        if (studentCount == 0) {
            clear_screen(COLOR_WHITE);
            draw_text("No students to display.", COLOR_RED, "Arial", 20, 50, 100);
            wait_for_back();
            return;
        }

        vector<Student> temp(students, students + studentCount);
        vector<Student*> results;

        bool backToMethod = false;

        while (!backToMethod) {  // Loop to return to display_method menu
            int method = display_method();

            if (method == 0) {  // Cancel/back
                backToMethod = true;
                break;
            }

            clear_screen(COLOR_WHITE);

            if (method == 1) {
                sort(temp.begin(), temp.end(), compareByName);
                draw_text("Student List (Sorted by Name)", COLOR_BLACK, "Arial", 24, 50, 20);
            }
            else if (method == 2) {
                sort(temp.begin(), temp.end(), compareByGrade);
                draw_text("Student List (Sorted by Grade)", COLOR_BLACK, "Arial", 24, 50, 20);
            }
            else if (method == 3) {
                string name = get_text_input("Enter student name to display:", 50, 100, 400, 30);
                results = find_student_by_name(name);

                if (results.empty()) {
                    draw_text("No students with that name found.", COLOR_RED, "Arial", 20, 50, 160);
                    wait_for_back();
                    continue; // Go back to display_method menu
                }

                clear_screen(COLOR_WHITE);
                draw_text("Student Search Results", COLOR_BLACK, "Arial", 24, 50, 20);

                for (int i = 0; i < results.size(); i++) {
                    int y = 90 + i * 30;
                    draw_text(results[i]->name, COLOR_BLACK, "Arial", 18, 50, y);
                    draw_text(results[i]->ID, COLOR_BLACK, "Arial", 18, 250, y);
                    draw_text(to_string(results[i]->grade), COLOR_BLACK, "Arial", 18, 450, y);
                }

                wait_for_back();
                continue;  // Return to display_method menu
            }
            else if (method == 4) {
                export_to_csv();
                continue;  // Return to display_method menu after export
            }

            // Display student list for methods 1 or 2
            if (method == 1 || method == 2) {
                draw_text("Name", COLOR_BLUE, "Arial", 18, 50, 60);
                draw_text("ID", COLOR_BLUE, "Arial", 18, 250, 60);
                draw_text("Grade", COLOR_BLUE, "Arial", 18, 450, 60);

                for (int i = 0; i < temp.size(); i++) {
                    int y = 90 + i * 30;
                    draw_text(temp[i].name, COLOR_BLACK, "Arial", 18, 50, y);
                    draw_text(temp[i].ID, COLOR_BLACK, "Arial", 18, 250, y);
                    draw_text(to_string(temp[i].grade), COLOR_BLACK, "Arial", 18, 450, y);
                }

                wait_for_back();
            }
        }
    }



    void export_to_csv(const string &filename = "Student_report.csv"){
        ofstream file(filename);
        if(!file.is_open()){
            clear_screen(COLOR_WHITE);
            draw_text("Failed to open CSV file for writting", COLOR_RED, "Arial", 20, 50, 160);
            wait_for_back();
            return;
        }

        file << "Name, ID, Grade\n";
                // Write all student data
        for (int i = 0; i < studentCount; i++) {
            file << students[i].name << ","
                 << students[i].ID << ","
                 << students[i].grade << "\n";
        }

        file.close();

        clear_screen(COLOR_WHITE);
        draw_text("Student report exported to student_report.csv", COLOR_GREEN, "Arial", 20, 50, 160);
        wait_for_back();
    }

    Student* find_student_by_id(const string &id) {
        for (int i = 0; i < studentCount; i++)
            if (students[i].ID == id) return &students[i];
        return nullptr;
    }

    vector<Student*> find_student_by_name(const string& name) {
        vector<Student*> results;
        for (int i = 0; i < studentCount; i++) {
            if (students[i].name == name) {
                results.push_back(&students[i]);
            }
        }
        return results;
    }


    // Choose which methods to search
    int choose_search_method(){
        while (true)
        {
            process_events();
            clear_screen(COLOR_WHITE);
            draw_rectangle(COLOR_LIGHT_GRAY, 100, 150, 180, 50);
            draw_rectangle(COLOR_LIGHT_GRAY, 320, 150, 180, 50);
            draw_text("Search by ID", COLOR_BLACK, "Arial", 20, 120, 165);
            draw_text("Search by Name", COLOR_BLACK, "Arial", 20, 340, 165);
            if(mouse_clicked(LEFT_BUTTON)){
                int x = mouse_x();
                int y = mouse_y();
                if (x >= 100 && x <= 280 && y >= 150 && y <= 200) return 1; // ID
                if (x >= 320 && x <= 500 && y >= 150 && y <= 200) return 2; // Name
            }
            if (key_typed(ESCAPE_KEY) || key_typed(RETURN_KEY)) return 0; // Cancel
            refresh_screen();        
               
        }
        
    }

    Student* choose_from_multiple(vector<Student*> options){
        int selected = 0;
        while (true) {
            process_events();

            clear_screen(COLOR_WHITE);
            draw_text("Multiple students found. Select one:", COLOR_BLACK, "Arial", 20, 50, 50);
            draw_text("Name", COLOR_BLUE, "Arial", 18, 50, 60);
            draw_text("ID", COLOR_BLUE, "Arial", 18, 250, 60);
            draw_text("Grade", COLOR_BLUE, "Arial", 18, 450, 60);
            for (int i = 0; i < options.size(); i++) {
                int y = 100 + i * 30;
                string marker = (i == selected) ? "> " : "  ";
                draw_text(marker + options[i]->name + " | " + options[i]->ID + " | " + to_string(options[i]->grade),
                        COLOR_BLACK, "Arial", 18, 50, y);
            }
            if (key_typed(UP_KEY) && selected > 0) selected--;
            if (key_typed(DOWN_KEY) && selected < options.size() - 1) selected++;
            if (key_typed(RETURN_KEY)) return options[selected];
            refresh_screen();
        }
    }

    void update_student_grade() {
        while (true) {   // keep looping until update succeeds or user cancels
            int method = choose_search_method();
            if (method == 0) return;  // ESC → exit the function to menu

            Student* s = nullptr;

            if (method == 1) { // --- Search by ID ---
                string id;
                while (true) {
                    id = get_text_input("Enter student ID to update:", 50, 100, 400, 30);
                    if (id.empty()) {
                        clear_screen(COLOR_WHITE);
                        draw_text("ID cannot be empty. Please enter a valid ID.", COLOR_RED, "Arial", 18, 50, 160);
                        wait_for_back();
                        continue; // re-ask
                    }
                    break;
                }

                s = find_student_by_id(id);
                if (!s) {
                    clear_screen(COLOR_WHITE);
                    draw_text("Student ID not found.", COLOR_RED, "Arial", 20, 50, 160);
                    wait_for_back();
                    continue;  // instead of return → go back to choose_search_method()
                }

            } else { // --- Search by Name ---
                string name;
                while (true) {
                    name = get_text_input("Enter student name to update:", 50, 100, 400, 30);
                    if (name.empty()) {
                        clear_screen(COLOR_WHITE);
                        draw_text("Name cannot be empty. Please enter a valid name.", COLOR_RED, "Arial", 18, 50, 160);
                        wait_for_back();
                        continue; // re-ask
                    }
                    break;
                }

                vector<Student*> matches = find_student_by_name(name);
                if (matches.empty()) {
                    clear_screen(COLOR_WHITE);
                    draw_text("No students with that name found.", COLOR_RED, "Arial", 20, 50, 160);
                    wait_for_back();
                    continue; // back to choose_search_method()
                }

                s = choose_from_multiple(matches);
            }

            // --- Update grade ---
            string gradeStr;
            while (true) {
                gradeStr = get_text_input("Enter new grade (number):", 50, 160, 400, 30);
                if (isnumber(gradeStr)) break;
                clear_screen(COLOR_WHITE);
                draw_text("Invalid grade input. Please enter a number.", COLOR_RED, "Arial", 20, 50, 220);
                wait_for_back();
            }

            s->grade = stoi(gradeStr);

            clear_screen(COLOR_WHITE);
            draw_text("Student grade updated successfully!", COLOR_GREEN, "Arial", 20, 50, 220);
            wait_for_back();

            return; // exit function after success
        }
    }



    void delete_student() {
        while (true) {   // keep looping until delete succeeds or user cancels
            int method = choose_search_method();
            if (method == 0) return; // ESC → exit the function to menu

            int index = -1;

            if (method == 1) { // --- Search by ID ---
                string id = get_text_input("Enter student ID to delete:", 50, 100, 400, 30);

                if (id.empty()) {
                    clear_screen(COLOR_WHITE);
                    draw_text("ID cannot be empty. Please enter a valid ID.", COLOR_RED, "Arial", 20, 50, 160);
                    wait_for_back();
                    continue; // re-ask
                }

                for (int i = 0; i < studentCount; i++) {
                    if (students[i].ID == id) { 
                        index = i; 
                        break; 
                    }
                }

                if (index == -1) {
                    clear_screen(COLOR_WHITE);
                    draw_text("Student ID not found.", COLOR_RED, "Arial", 20, 50, 160);
                    wait_for_back();
                    continue; // go back to choose_search_method()
                }

            } else { // --- Search by Name ---
                string name = get_text_input("Enter student name to delete:", 50, 100, 400, 30);

                if (name.empty()) {
                    clear_screen(COLOR_WHITE);
                    draw_text("Name cannot be empty. Please enter a valid name.", COLOR_RED, "Arial", 20, 50, 160);
                    wait_for_back();
                    continue; // re-ask
                }

                vector<Student*> matches;
                for (int i = 0; i < studentCount; i++) {
                    if (students[i].name == name) matches.push_back(&students[i]);
                }

                if (matches.empty()) {
                    clear_screen(COLOR_WHITE);
                    draw_text("No students with that name found.", COLOR_RED, "Arial", 20, 50, 160);
                    wait_for_back();
                    continue; // back to choose_search_method()
                }

                Student* s = choose_from_multiple(matches);
                for (int i = 0; i < studentCount; i++) {
                    if (&students[i] == s) { 
                        index = i; 
                        break; 
                    }
                }
            }

            // --- Perform deletion ---
            for (int i = index; i < studentCount - 1; i++) {
                students[i] = students[i + 1];
            }
            studentCount--;

            clear_screen(COLOR_WHITE);
            draw_text("Student deleted successfully!", COLOR_GREEN, "Arial", 20, 50, 160);
            wait_for_back();

            return; // exit after successful deletion
        }
    }
};

// Show menu
int show_menu() {
    string choiceStr;
    while (true) {
        process_events();
        if (window_close_requested("Student Manager")) exit(0);

        clear_screen(COLOR_WHITE);
        draw_text("=== Student Grade Manager ===", COLOR_BLACK, "Arial", 24, 50, 20);
        draw_text("1. Add new student", COLOR_BLACK, "Arial", 20, 50, 80);
        draw_text("2. Display all students", COLOR_BLACK, "Arial", 20, 50, 120);
        draw_text("3. Update student grade", COLOR_BLACK, "Arial", 20, 50, 160);
        draw_text("4. Delete student", COLOR_BLACK, "Arial", 20, 50, 200);
        draw_text("5. Exit", COLOR_BLACK, "Arial", 20, 50, 240);
        draw_text("Enter choice and press Enter:", COLOR_BLACK, "Arial", 18, 50, 280);
        draw_text(choiceStr + "_", COLOR_BLACK, "Arial", 18, 50, 350);
        refresh_screen(60);

        for (char c = '1'; c <= '6'; c++) {
            if (key_typed(static_cast<key_code>(c))) choiceStr = c;
        }
        if (key_typed(RETURN_KEY) && !choiceStr.empty()) return stoi(choiceStr);
    }
}

int main() {
    open_window("Student Manager", 600, 400);
    StudentManager manager;

    while (true) {
        int choice = show_menu();
        switch (choice) {
            case 1: manager.add_new_student(); break;
            case 2: manager.display_students(); break;
            case 3: manager.update_student_grade(); break;
            case 4: manager.delete_student(); break;
            case 5:
                clear_screen(COLOR_WHITE);
                draw_text("Bye!", COLOR_BLACK, "Arial", 30, 250, 180);
                return 0;
            default:
                clear_screen(COLOR_WHITE);
                draw_text("Invalid choice.", COLOR_RED, "Arial", 20, 50, 200);
                refresh_screen(60);
        }
    }
}
