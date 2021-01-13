/*
 * CValerio_mem.cpp
 *
 *  Created on: Dec 4, 2020
 *  Author: Christopher Valerio
 */

// to compile: g++ -o CValerio_mem.exe CValerio_mem.cpp
// to execute: CValerio_mem.exe 1048576

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <list>

//#include <iterator>

using namespace std;

void RQ(string proc, string length);
void RL(string proc);
void STAT();
void C(int maxSize);

class memBlock { // my class representing a block of memory
	string status; // status is either holds value "free" or value of processes name
	int strt; // blocks starting index/position
	int end; // blocks ending index/position
	int size; // size of memory block

public:
	memBlock(string stat, int strtIndx, int endIndx) { // parameterized constructor
		status = stat;
		strt = strtIndx;
		end = endIndx;
		size = (end - strt) + 1;
	}
	string getStatus() {
		return status;
	}
	int getStrt() {
		return strt;
	}
	int getEnd() {
		return end;
	}
	int getSize() {
		return size;
	}
	void setStatus(string stat) {
		status = stat;
	}
	void setStrt(int indx) {
		strt = indx;
	}
	void setEnd(int indx) {
		end = indx;
	}
	void setSize(int newSize) {
		size = newSize;
	}
	void printBlock() {
		cout << status << ": " <<  strt << ", " << end << " size: " << size << endl;
	}
};

list<memBlock> freeList; // global free list variable, list of all available free blocks of memory
list<memBlock> usedList; // global list variable, representing a list of all currently allocated memory blocks
//list<memBlock> statList;

int main (int argc, char** argv) {

	int memSize = atoi(argv[1]); //integer representing memory size, specified by user when program is executed

	freeList.push_back(memBlock("Free", 0, memSize - 1)); // initially program starts with one large block of free memory

	cout << endl;
	cout << "Please Enter a Valid RQ, RL, STAT, C, or QUIT Command:";
	string input; // char* holding user input
	char* tok; // temp variable used to tokenize inputed string
	getline(cin, input);

	while(input.compare("QUIT") != 0) { // program keeps running until 'QUIT' is entered by user


		istringstream iss(input); // stringstream used to tokenize, user's input

		string tok;
		int numArgs = 0;
		int cmdType = -1; // integer used to know what type of command user has specified
		// 0 == RQ, 1 == RL, 2 == STAT, 3 == C

		string procNum = "";
		string procSize = "";

		while(iss >> tok) {

			tok.erase(tok.find_last_not_of("\n\r") + 1);

			if(numArgs == 0 && (tok == "RQ") ) { // if command starts with 'RQ', we indicate a RQ command is being inputed
				cmdType = 0;
			}
			else if(numArgs == 0 && (tok == "RL") ) { // if command starts with 'RL', we indicate a RL command is being inputed
				cmdType = 1;
			}
			else if(numArgs == 0 && (tok == "STAT") ) { // if command starts with 'STAT', we indicate a STAT command is being inputed, and execute STAT() function
				cmdType = 2;
				STAT();
			}
			else if(numArgs == 0 && (tok == "C") ) { // if command starts with 'C', we indicate a memory compaction command is being inputed, and execute C() function
				cmdType = 3;
				C(memSize - 1);
			}
			else if(numArgs == 0) { // otherwise an unknown/invalid command has been entered
				cout << "Invalid Command Entered" << endl;
				cmdType = -1;
				break;
			}

			if(numArgs == 1 && (cmdType == 0 || cmdType == 1)) { // upon parsing second parameter of RQ, RL command, we execute the RL() function if it has been specified
				procNum = tok;
				if(cmdType == 1) {
					RL(procNum);
				}
			}

			if(numArgs == 2 && cmdType == 0) { // upon parsing third parameter of RQ command, we execute the RQ() function
				procSize = tok;
				RQ(procNum, procSize);

				if(freeList.size() != 0) {
					//it = freeList.begin();
					//it->printBlock();
				}
				else
					cout << "freeList is empty" << endl;

			}

			numArgs++;
		}

		cout << flush;
		cout << "Please Enter a Valid RQ, RL, STAT, C, or QUIT Command:";
	
		getline(cin, input);

	}
	cout << input << endl;
	return 0;
}

void RQ(string proc, string length) { // new process request, given process name & size
	// using worst fit allocation, so we check for the largest available "hole" to use
	// free list head should point to memory block of greatest size.
	// free memory block is updated and newly allocated block is added to global usedList

	const char* procLen = length.c_str();
	int intLen = atoi(procLen); // integer value of string "length" argument

	list<memBlock>::iterator tempIt = freeList.begin();
	list<memBlock>::iterator maxFreeBlock = tempIt;

	while(tempIt != freeList.end()) { // loop through find greatest free block
		if(tempIt->getSize() >= maxFreeBlock->getSize())
			maxFreeBlock = tempIt;

		tempIt++;
	}
	tempIt = maxFreeBlock;

	if(tempIt->getSize() < intLen) { // if no available space
		cout << "Process: " << proc << ", cannot be allocated because there is currently no big enough free space" << endl;
	}
	else if (tempIt->getSize() == intLen) {
		// if size of new process is the same size as the largest hole
		// then the entire memBlock is filled and removed from freeList

		usedList.push_back(memBlock(proc, tempIt->getStrt(), tempIt->getEnd())); // add new process information to allocated memory list

		freeList.erase(tempIt); // remove previously free memory block from free list
	}
	else {
		// grab largest hole and allocate new process within it


		int newProcStrt = tempIt->getStrt();
		int newProcEnd = newProcStrt + intLen - 1;

		tempIt->setStrt(newProcEnd + 1); // move free memory blocks starting position up, ending position remains unchanged
		tempIt->setSize(   (tempIt->getEnd() - (newProcEnd + 1)) + 1   );

		usedList.push_back(memBlock(proc, newProcStrt, newProcEnd)); // add new process information to allocated memory list
	}
}

void RL(string proc) { // release process request, given name of process to release

	// check usedList for specified process if it doesn't exist do nothing
	// otherwise memory block allocated to specified memory block is added back to our free list
	// and removed from usedList;

	list<memBlock>::iterator tempIt = usedList.begin(); // temp iterator used to traverse usedList
	bool found = false;
	while(tempIt != usedList.end()) {
		if(tempIt->getStatus() == proc) {

			// release this memory block
			// add this memory block back onto freeList and remove from usedList
			// if adjacent memory block is free then merge the two adjacent blocks

			int startingIndx = tempIt->getStrt(); // starting and ending indexes for memory block to be freed
			int endingIndx = tempIt->getEnd();


			// add memory block back onto freeList and remove from usedList
			list<memBlock>::iterator tempIt2 = freeList.begin();
			bool isGreater = false;
			while(tempIt2 != freeList.end()) { // loop through memory free list to where where to place


				if(  tempIt2->getSize() <= tempIt->getSize()  ) { // find where to place newly released free block into the free list for future use. and remove the process from occupied/used memory list
				
					freeList.insert(tempIt2, memBlock("Free", startingIndx, endingIndx));
					tempIt = usedList.erase(tempIt);
					isGreater = true;
					break;
				}

				tempIt2++;
			}
			if(isGreater == false) { // push to back of freeList if smallest free memory block
				freeList.push_back(memBlock("Free", startingIndx, endingIndx));
				tempIt = usedList.erase(tempIt);
			}

			found = true;
		}

		tempIt++;
	}
	if(found == false) {
		cout << proc << " does not exist in memory, therefore could not released" << endl;
		return;
	}
	else
		cout << proc << " has been released from memory" << endl;




	// here we check if any adjacent free memory blocks need to be merged
	// loop through freeList
	// and for every free memory block check if there exist an adjacent block that is also free
	cout << "merging" << endl;
	list<memBlock>::iterator freeIt= freeList.begin(); // free list iterator
	while(freeIt != freeList.end()) {
		cout << "outLoop TOP" << endl;
		list<memBlock>::iterator freeIt2= freeList.begin();
		while(freeIt2 != freeList.end()) {
			cout << "inner loop TOP" << endl;

			if((freeIt != freeIt2) && (freeIt2->getStrt() == (freeIt->getEnd() + 1))) { // if found memBlock that is right adjacent

				cout << "if found adjacent free block" << endl;
				freeIt->setEnd(freeIt2->getEnd()); // merge mem blocks by expanding one of them and removing the other
				freeIt->setSize(  ((freeIt->getEnd() - freeIt->getStrt()) + 1)    );
				cout << "before erase" << endl;
				list<memBlock>::iterator tempRetIterator = freeList.erase(freeIt2);
				cout << "after erase" << endl;
			}

			freeIt2++;
		}

		freeIt++;
	}


}


void STAT() { // generate output request

	list<memBlock> statList; // list of memory blocks used for output when 'STAT' command is inputed
	list<memBlock>::iterator whereTo;

	if(usedList.size() != 0) { // loop through used/occupied memory list and add to statList in appropriate place based on memory addresses
		list<memBlock>::iterator tempIt;
		tempIt = usedList.begin();
		statList.insert(statList.begin(), memBlock(tempIt->getStatus(), tempIt->getStrt(), tempIt->getEnd()));
		tempIt++;
		while(tempIt != usedList.end()) { // loop through all occupied memory blocks

			list<memBlock>::iterator tempIt2 = statList.begin();

			whereTo = statList.begin();
			while(tempIt2 != statList.end()) { // loop through statList to find where to place

				if(tempIt->getStrt() > tempIt2->getEnd()) {
					whereTo = tempIt2;
				}
				tempIt2++;

			}

			statList.insert(++whereTo, memBlock(tempIt->getStatus(), tempIt->getStrt(), tempIt->getEnd()));
			tempIt++;
		}

	}


	if(freeList.size() != 0) { // loop through free memory list and add to statList in appropriate place based on memory addresses

		list<memBlock>::iterator tempIt;
		tempIt = freeList.begin();

		while(tempIt != freeList.end()) { // loop through all occupied memory blocks

			list<memBlock>::iterator tempIt2 = statList.begin();

			whereTo = statList.begin();
			while(tempIt2 != statList.end()) { // loop through statList to find where to place

				if(tempIt->getStrt() > tempIt2->getEnd()) {
					whereTo = tempIt2;
				}
				tempIt2++;
			}
			statList.insert(++whereTo, memBlock(tempIt->getStatus(), tempIt->getStrt(), tempIt->getEnd()));

			tempIt++;
		}

	}


	list<memBlock>::iterator tempIt = statList.begin(); // temporary iterator used to traverse statList and print all address spaces upon a 'STAT' command
	cout << flush << endl;
	while(tempIt != statList.end()) {
		cout << "Addresses [" << tempIt->getStrt() << " : " << tempIt->getEnd() << "] " << tempIt->getStatus() << endl;
		tempIt++;
	}
	cout << flush << endl;

}

void C(int maxSize) { // memory compaction request
	// push all processes down towards low memory, so they are all flush against each other

	list<memBlock> compactedList;
	list<memBlock>::iterator tempIt = usedList.begin(); //temporary iterator, to travese list of occupied memory blocks

	int prevStrt = -1;
	int prevEnd = -1;
	int tempSize = 0;
	while(tempIt != usedList.end()) {

		if(prevStrt == -1 && prevEnd == -1){
			tempSize = tempIt->getSize();
			prevStrt = 0;
			prevEnd = prevStrt + tempSize -1;
			compactedList.push_back(memBlock(tempIt->getStatus(), prevStrt, prevEnd));
		}
		else {
			tempSize = tempIt->getSize();
			prevStrt = prevEnd + 1;
			prevEnd = prevStrt + tempSize -1;
			compactedList.push_back(memBlock(tempIt->getStatus(), prevStrt, prevEnd));
		}
		tempIt++;
	}

	usedList.clear();
	usedList = compactedList;

	freeList.clear();
	freeList.push_back(memBlock("Free", prevEnd + 1, maxSize));

	cout << "Memory Compacted" << endl;
}
