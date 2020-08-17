#pragma warning(disable : 4996)

#include <iostream>
#include <windows.h>
#include <ShObjIdl.h>
#include <chrono>
#include <ctime>

using namespace std;

class Jeg
{
public:
	Jeg() :state(0) { char buf[20]; itoa(state, buf, 10); name = new char[strlen(buf) + 1]; strcpy(name, buf); }
	Jeg(int s) :state(s) { char buf[20]; itoa(state, buf, 10); name = new char[strlen(buf) + 1]; strcpy(name, buf); }
	~Jeg() { cout << "destroyed " << state << " " << name << endl; }

	char* name = NULL;
	int state;
	void printState()
	{
		cout << state << endl;
	}
};

int main()
{
	//std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);
	cout << now->tm_sec;
}

