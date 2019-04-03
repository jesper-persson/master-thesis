#include <iostream>
#include <map>
#include <string>
#include <iomanip>
#include <chrono>

using namespace std;

class Timing {
public:
    map<string, double> data;
    map<string, double> begins;

    Timing() {
        
    }

    void begin(string name) {
        // glFinish();
        begins[name] = glfwGetTime();
    }

    void end(string name) {
        if (begins.count(name)) {
            // glFinish();
            double begin = begins[name];
            double end = glfwGetTime();
            data[name] = (end-begin) * 1000;
        } else {
            cout << "Cant end" << endl;
        }   
    }
    
    void print() {
        cout << "\nTIMINGS" << endl;
        double total = 0;
        for (auto const& x : data) {
            cout << left << setw(25) << x.first << " " << x.second << " ms" << endl;
            total += x.second;
        }
        cout << left << setw(25) << "TOTAL" << total << endl; 
        cout << "\n" << endl;
    }
}; 