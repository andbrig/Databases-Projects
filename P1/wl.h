//
// @file: wl.h
// @author Andrew Briggs <abriggs5@wisc.edu>
//  @section Description: This program accepts file inputs, parses the file, then allows the user to search for words on the file.
//  UW Campus ID: 9071627591 
//  email: abriggs5@wisc.edu
#ifndef WL_H
#define WL_H
#include <string>
/*!
 * \brief struct to hold strings from a file
 */
struct node {
	std::string word;
	int* wordNums;
	node* right;
	node* left;
	node* parent;
	int numRepeats;
	}; 
class tree {

	private:
		node* root;
		/*! \brief Inserts a given word into a node and stores it according to the implementation
		@param string to be inserted
		*/
		void insert(std::string word);
		/*! \brief Enables user to search for any occurrence of any word
		@param string to be searched, int number of occurrence
		*/
		void search(std::string word, int num);
		/*! \brief Allows array to be expanded
		@param node pointer to the node holding the array, int count to be inserted	
		*/
		void expandArray(node* temp, int count);
		/*! \brief adds different occurrences of already stored words to be added
		@param node pointer of the node which needs to have a number added to its array
		*/
		void addToArray(node* node);
		void addHelper(node* root, std::string word);
};
#endif	
