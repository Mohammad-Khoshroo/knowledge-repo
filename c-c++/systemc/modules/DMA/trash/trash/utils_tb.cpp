#include "utils.hpp"

using namespace std;
int sc_main(int argc, char* argv[]) {
    try {
        // First partition: starts at 0, size 100
        MemoryPartition p1(0, 100);
        cout << "Start limit after p1: " << MemoryPartition::top() << endl;

        // Check if address 50 is inside p1
        cout << "Is 50 covered by p1? " << (p1.is_cover(50) ? "Yes" : "No") << endl;
        cout << "Is 150 covered by p1? " << (p1.is_cover(150) ? "Yes" : "No") << endl;

        // Second partition: starts where p1 ended
        MemoryPartition p2(100, 200);
        cout << "Start limit after p2: " << MemoryPartition::top() << endl;

        cout << "Is 150 covered by p2? " << (p2.is_cover(150) ? "Yes" : "No") << endl;
        cout << "Is 99 covered by p2? " << (p2.is_cover(99) ? "Yes" : "No") << endl;

        // Invalid test: wrong start address
        MemoryPartition p3(123, 50);  // This should trigger fatal error
    }
    catch (...) {
        cout << "Exception caught during test." << endl;
    }

    return 0;
}
