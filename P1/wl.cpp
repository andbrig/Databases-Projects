
// File: wl.h
// 
//  Description: Progam parses large text files. Shell and parse work flawlessly, 
// however I could not get the data structure to work and therefore cannot do anything with 
// the parsed input.
//  Student Name: Andrew Briggs 
//  UW Campus ID: 9071627591
//  enamil: abriggs5@wisc.edu
#include "wl.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
int insertCount = 0;
int numDuplicates = 8;
tree* tree = new tree;	
int main()
{
	//Stores duplicates
	//Loops until user inputs "end"
	while(1) {
		std::string word = "";
		std::string userInput = "";
		std::string userInput2 = "";
		std::string word1 = "";
		std::string word2 = "";
		std::string word3 = "";
		int count = 0;
		//Establish character array
		char userIn[100];
		//BEGIN SHELL
		std::cout<<">";
		//GET USER INPUT 
		std::getline(std::cin, userInput);
		//Convert to c string
		const char* user_in_line = userInput.c_str();
		//Get length of string
		int userInSize = userInput.length();
		//Copy line into userIn[]
		strncpy(userIn, user_in_line, userInSize);
		userIn[userInSize] = '\0';
		//Parse Individually by characters
		for(int i = 0; i < userInSize; i++) {
			userIn[i] = tolower(userIn[i]);
			if((userIn[i] == '\"') || (userIn[i] == '(') ||(userIn[i] == ')') || (userIn[i] == '#') || (userIn[i] == '!') 	
				|| (userIn[i] == '?') || (userIn[i] == ':') || (userIn[i] == ';') || (userIn[i] =='&') || (userIn[i] == '%')
				|| (userIn[i] == '?') || (userIn[i] == ',') || (userIn[i] == '-') || (userIn[i] == '<')
			 	|| (userIn[i] == '>')) {
				//Clear words and end loop, will cause error message to be printed
				word1.clear();
				word2.clear();
				word3.clear();
				break;
			}
			//Check to see if inbetween words or end of line
			else if(userIn[i] == ' ') {
				if(!userInput2.empty()) {
					if(count == 0) {
						word1 = userInput2;
						userInput2.clear();
						count++;
					}
					else if(count == 1) {
						word2 = userInput2;
						userInput2.clear();	
						count++;
					}
				}
				else {
				}
			}
			else {
				//Ensure word 3 is a number
				if(count == 2) {
					if(!isdigit(userIn[i])) {
						word1.clear();
						word2.clear();
						word3.clear();
						break;
					}
				}
				userInput2.push_back(userIn[i]);
			}
		}
		if(count == 0) {
			word1 = userInput2;
			userInput2.clear();
		}
		if(count == 1) {
			word2 = userInput2;
			userInput2.clear();
		}
		if(count == 2 && userInput2.length() != 0) {
			word3 = userInput2;
			userInput2.clear();
		}
		//REMEMBER TO CLOSE IFSTREAM
		//load
		if(word1 == "load") {	
			bool incorrectFile = false;
			//POSSIBLY ADD ERROR HANDLING HERE FOR A THIRD ARGUMEN
			//Continue parsing
			std::string filename = "";
			filename = word2;
			std::ifstream file;
			const char* c_filename = filename.c_str();
			file.open(c_filename);
			//Check if file exists
			if(!file || word3 != "") {
				//Display Error Message
				std::cout<< "ERROR: Invalid command"<<std::endl;
				incorrectFile = true;
			}
			std::string line = ""; 
			//Parse one line
			while((std::getline(file, line)) && (!incorrectFile)) {
				//Protects against case with ending on empty line
				int repeatCounter = 0;
				while(line.length() == 0) {
					if(repeatCounter > 50) {
						break;
					}					
					std::getline(file, line);
					repeatCounter++;
				}
				const char* c_line = line.c_str();
				char dup[400];
				//GET SIZE OF STRING
				int dupSize = strlen(c_line);
				//COPY STRING INTO CHAR ARRAY dup[]
				strncpy(dup, c_line, dupSize);
				dup[dupSize] = '\0';
				int j = 0;
				while(dup[j] != '\0') {
					dup[j] = tolower(c_line[j]);
					if((dup[j] == '\"') || (dup[j] == '.') || (dup[j] == ' ') ||(dup[j] == '(') ||(dup[j] == ')') || (dup[j] == '#') || (dup[j] == '!') || (dup[j] == '?') || (dup[j] == ':') || (dup[j] == ';') || (dup[j] =='&') || (dup[j] == '%') || (dup[j] == '?') || (dup[j] == ',') || (dup[j] == '-') || (dup[j] == '<') || (dup[j] == '>')) {	
						if(word.length() != 0) {
							tree->insert(word);
						}
						//Insert into Tree Here
						
						word.clear(); 
					}
					else {
						word.push_back(dup[j]);		
					}
					j++;
				}
				//Adds and clears final word of each line
				if(word.length() != 0) {
					//Insert into Tree Here
					tree->insert(word);
					word.clear();
				} 	
			}
		}
		//Locate
		else if(word1 == "locate") {
			if(word3 == "") {
				std::cout<<"ERROR: Invalid command"<<std::endl;
			}
			else {
				std::string locateString = word2;
				//NEED TO USE STOI
				std::cout<<"1"<<std::endl;
			}
		}
		//New
		else if(word1 == "new") {
	
			//Clear data structure
		}
		//End
		else if(word1 == "end") {
			break;
		}
		//Input error
		else {
			//Display Error Message
			std::cout<< "ERROR: Invalid command"<< std::endl;
		}
	}
}

void insert(std::string word) {
	if(root) {
		this->addHelper(root, word);
	}
	else {
		root = new node;
		root->word = word;
	}
 /*
	if(nod == NULL) {
		root = nod;
	}	
	else {
		node* curr = root; 
		if(nod->word < curr->word) {
			insert(&(curr->left), word);
		}
		else {
			insert(curr->right, word);
		} 
	}
*/

}
void addHelper(node* root, std::string word) {
	if(root->word > word) {
		if(!root->left) {
			root->left = new node;
			root->left->word = word;
			std::cout<<root->left->word<<std::endl;
		}
		else {
			addHelper(root->left, word);
		}
	}
	else {
		if(!root->right) {
			root->right = new node;
			root->right->word = word;
		}
		else {
			addHelper(root->right, word);
		}
	}
}
void search(std::string wordToFind, int wordNum) {
/*
	if(root->word == "") {
		std::cout<<"No matching entry"<<std::endl;
		return;
	}
	int counter = 0;
	node* tempNode;
	tempNode = root;
	while(tempNode != NULL && counter == 0) {
		if(tempNode->word == wordToFind) {
			counter++;
			for(int i = 0; i < tempNode->numRepeats; i++) {
				if(i == wordNum) {
					std::cout<<tempNode->wordNums[i];
					return;
				}
			}
			std::cout<<"No matching entry"<<std::endl; 
			return;
		}
		if(tempNode->word < wordToFind) {
			tempNode = tempNode->left;
		}
		else {
			tempNode = tempNode->right;
		}
	}
	std::cout<<"No matching entry"<<std::endl;
*/
}
void expandArray(node *nod, int count) {
	/*
	//STILL NEED TO INSERT COUNT INTO NODE
	int newSize = 2*(nod->numRepeats);
	int *temp = new int[newSize];
	std::copy(nod->wordNums, nod->wordNums + nod->numRepeats, temp);
	delete [] nod->wordNums;
	nod->wordNums = temp;
	nod->wordNums[nod->numRepeats] = count;
	nod->numRepeats++;
*/
} 
void addToArray(node *nod) {
/*
	for(int i = 0; i < nod->numRepeats; i++) {
		if(nod->wordNums[i] == 0) {
			nod->wordNums[i] = insertCount;
			nod->numRepeats++;
		}
	}
	expandArray(nod, insertCount);
*/
}
