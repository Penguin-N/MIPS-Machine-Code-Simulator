#include <map>
using namespace std;
// Define a data block used to store data in the data part.
struct charPart{
    char c1;
    char c2;
    char c3;
    char c4;
};
struct halfPart{
    short s1;
    short s2;
};
union dataBlock{
    int intData;
    charPart charData;
    halfPart halfData;
};

// Point to the start of simulated memory.
void* memoryPointer;
// Point to the end of the text part.
int* textPointer;
// Point to the end of static data part.
dataBlock* staticDataPointer;
// Point to the end of dynamic data part.
dataBlock* dynamicDataPointer;
// Point to the stack.
string* stackPointer;

// Define a map to store the register number and its pointer.
extern map<string,int*> Registers;
