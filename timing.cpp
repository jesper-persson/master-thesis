#include <iostream>
#include <map>
#include <string>
#include <iomanip>
#include <chrono>

using namespace std;

class Timing {
public:
    map<string, double> data;
    map<string, double> totalMap;
    map<string, double> begins;
    int numMeasurements = 0;

    Timing() {
        
    }

    void begin(string name) {
        if (numMeasurements < 100) {
            return;
        }
        // glFinish();
        if (!begins.count(name)) {
            totalMap[name] = 0;
        }
        begins[name] = glfwGetTime();

    }

    void end(string name) {
        if (numMeasurements < 100) {
            return;
        }
        if (begins.count(name)) {
            // glFinish();
            double begin = begins[name];
            double end = glfwGetTime();
            data[name] = (end-begin) * 1000;
            totalMap[name] += data[name];
        } else {
            cout << "Cant end" << endl;
        }   
    }
    
    void print() {
        cout << "\nTIMINGS" << endl;
        double total = 0;
        for (auto const& x : data) {
            double avg = totalMap[x.first] / (numMeasurements - 100);
            cout << left << setw(25) << x.first << " " << x.second << " ms" << ", avg: " << avg << endl;
            total += x.second;
        }
        cout << left << setw(25) << "TOTAL" << total << endl; 
        cout << "\n" << endl;
    }
}; 