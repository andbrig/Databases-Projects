/*
//Custom Methods
template<class T>

 * @author See*ge* meta;
*ge* meta;
 Contributors.txt for code contributors and overview of BadgerDB.
 *
 * @section LICENSE
 * Copyright (c) 2012 Database Group, Computer Sciences Department, University of Wisconsin-Madison.
 */

#include "btree.h"
#include "filescan.h"
#include "exceptions/bad_index_info_exception.h"
#include "exceptions/bad_opcodes_exception.h"
#include "exceptions/bad_scanrange_exception.h"
#include "exceptions/no_such_key_found_exception.h"
#include "exceptions/scan_not_initialized_exception.h"
#include "exceptions/index_scan_completed_exception.h"
#include "exceptions/file_not_found_exception.h"
#include "exceptions/end_of_file_exception.h"


//#define DEBUG

namespace badgerdb
{

// -----------------------------------------------------------------------------
// BTreeIndex::BTreeIndex -- Constructor
// -----------------------------------------------------------------------------

BTreeIndex::BTreeIndex(const std::string & relationName,
		std::string & outIndexName,
		BufMgr *bufMgrIn,
		const int attrByteOffset,
		const Datatype attrType)
{
	this->bufMgr = bufMgrIn;
	std::ostringstream btreeStream;
	btreeStream << relationName << '.' << attrByteOffset;
	std::string indexString = btreeStream.str();
	//Update Occupancies of BTree
	if(attrType == INTEGER) {
		this->nodeOccupancy = INTARRAYNONLEAFSIZE;
		this->leafOccupancy = INTARRAYLEAFSIZE;
	}
	else if(attrType == DOUBLE) {
		this->nodeOccupancy = DOUBLEARRAYNONLEAFSIZE;
		this->leafOccupancy = DOUBLEARRAYLEAFSIZE;	
	}
	else {
		this->nodeOccupancy = STRINGARRAYNONLEAFSIZE;
		this->leafOccupancy = STRINGARRAYLEAFSIZE;
	}
	//The File exists
	if(File::exists(indexString) == true) {
		this->file = new BlobFile(indexString, true);
		//TODO:: Open Index File
		Page* meta;
		IndexMetaInfo* metaInfo;
		this->headerPageNum = file->getFirstPageNo();
		this->bufMgr->readPage(this->file, this->headerPageNum, meta);
		metaInfo = (IndexMetaInfo*) meta;
		//Compare metaInfo with parameters passed in
		if(this->attrByteOffset != metaInfo->attrByteOffset || this->attributeType != metaInfo->attrType) {
			throw BadIndexInfoException("Index info not matched");
		}
		else {
			this->attributeType = attrType;
			this->attrByteOffset = attrByteOffset;
			this->rootPageNum = metaInfo->rootPageNo;
		}
	}
	//The File does not exist, create new file
	else {
		this->file = new BlobFile(indexString, true);
		this->attributeType = attrType;
		this->attrByteOffset = attrByteOffset;
		//TODO::Create new Index File
		Page* meta;
		Page* treeRoot;
		Page* leftChild;
		PageId leftChildNo;
		Page* rightChild;
		PageId rightChildNo;
		IndexMetaInfo* metaInfo;
		//Popuplate IndexMetaInfo
		this->bufMgr->allocPage(this->file, this->headerPageNum, meta);
		this->bufMgr->allocPage(this->file, this->rootPageNum, treeRoot);
		metaInfo = (IndexMetaInfo*) meta;
		metaInfo->attrByteOffset = this->attrByteOffset;
		metaInfo->attrType = this->attributeType;
		metaInfo->rootPageNo = this->rootPageNum;
		memcpy(metaInfo->relationName, relationName.c_str(), 30);
		//Create the correct type of Non Leaf Root Node and two leaf children
		if(attrType == INTEGER) {
			NonLeafNodeInt* rootNode = (NonLeafNodeInt*) treeRoot;
			rootNode->level = 1;
			memset(rootNode->keyArray, -1, sizeof(rootNode->keyArray));
			memset(rootNode->pageNoArray, -1, sizeof(rootNode->pageNoArray));
			bufMgr->allocPage(this->file, leftChildNo, leftChild);
			bufMgr->allocPage(this->file, rightChildNo, rightChild); 
			LeafNodeInt* leftChildLeaf = (LeafNodeInt*) leftChild;
			LeafNodeInt* rightChildLeaf = (LeafNodeInt*) rightChild;
			leftChildLeaf->rightSibPageNo = rightChildNo;
			rightChildLeaf->rightSibPageNo = 0;
			memset(leftChildLeaf->ridArray, -1, sizeof(leftChildLeaf->ridArray));
			memset(rightChildLeaf->ridArray, -1, sizeof(rightChildLeaf->ridArray));
			rootNode->pageNoArray[0] = leftChildNo;
			rootNode->pageNoArray[1] = rightChildNo;		
		}
		else if(attrType == DOUBLE) {
			NonLeafNodeDouble* rootNode = (NonLeafNodeDouble*) treeRoot;
			rootNode->level = 1;
			memset(rootNode->keyArray, -1, sizeof(rootNode->keyArray));
			memset(rootNode->pageNoArray, -1, sizeof(rootNode->pageNoArray));
			bufMgr->allocPage(this->file, leftChildNo, leftChild);
			bufMgr->allocPage(this->file, rightChildNo, rightChild);
			LeafNodeDouble* leftChildLeaf = (LeafNodeDouble*) leftChild;
			LeafNodeDouble* rightChildLeaf = (LeafNodeDouble*) rightChild;
			leftChildLeaf->rightSibPageNo = rightChildNo;
			memset(leftChildLeaf->ridArray, -1, sizeof(leftChildLeaf->ridArray));
			memset(rightChildLeaf->ridArray, -1, sizeof(rightChildLeaf->ridArray));
			rightChildLeaf->rightSibPageNo = 0;
			rootNode->pageNoArray[0] = leftChildNo;
			rootNode->pageNoArray[1] = rightChildNo;
		}
		else {
			NonLeafNodeString* rootNode = (NonLeafNodeString*) treeRoot;
			rootNode->level = 1;
			memset(rootNode->keyArray, -1, sizeof(rootNode->keyArray));
			memset(rootNode->pageNoArray, -1, sizeof(rootNode->pageNoArray));
			bufMgr->allocPage(this->file, leftChildNo, leftChild);
			bufMgr->allocPage(this->file, rightChildNo, rightChild);
			LeafNodeString* leftChildLeaf = (LeafNodeString*) leftChild;
			LeafNodeString* rightChildLeaf = (LeafNodeString*) rightChild;
			leftChildLeaf->rightSibPageNo = rightChildNo;
			rightChildLeaf->rightSibPageNo = 0;
			rootNode->pageNoArray[0] = leftChildNo;
			rootNode->pageNoArray[1] = rightChildNo;
			memset(leftChildLeaf->ridArray, -1, sizeof(leftChildLeaf->ridArray));
			memset(rightChildLeaf->ridArray, -1, sizeof(rightChildLeaf->ridArray));
		}
		//File Scan to insert tuples
		FileScan* scanner = new FileScan(relationName, this->bufMgr);
		std::string tempString;
		RecordId tempRecordId;
		const char* tempRecord;
		void* key;
		try{
			for(int i = 0; i < 10000; i++) {
				scanner->scanNext(tempRecordId);
				tempString = scanner->getRecord();
				tempRecord = tempString.c_str();
				key = (void*)(tempRecord + this->attrByteOffset);
				insertEntry(key, tempRecordId);
			}
		}
		catch(EndOfFileException e) {
			std::cout<< "All records have been inserted" <<std::endl;
		}
		//Unpin the pages used
		this->bufMgr->unPinPage(this->file, this->headerPageNum, true);	
	}
	outIndexName = indexString;
}


// -----------------------------------------------------------------------------
// BTreeIndex::~BTreeIndex -- destructor
// -----------------------------------------------------------------------------

BTreeIndex::~BTreeIndex()
{
	bufMgr->flushFile(this->file);
	this->scanExecuting = false;
	delete this->file;
}

// -----------------------------------------------------------------------------
// BTreeIndex::insertEntry
// -----------------------------------------------------------------------------

const void BTreeIndex::insertEntry(const void *key, const RecordId rid) 
{
	Page* newPage;
	bufMgr->readPage(file, rootPageNum, newPage);
	if(this->attributeType == INTEGER) {
		RIDKeyPair<int> intPair;
		int intKey = *((int*) key);
		intPair.set(rid, intKey);
	}
	else if(this->attributeType == DOUBLE) {
		RIDKeyPair<double> doublePair;
		double doubleKey = *((int*) key);
		doublePair.set(rid, doubleKey);
	}
	else {
		RIDKeyPair<char*> stringPair;
		char stringKey[10];
		memcpy(stringKey, key, 10);
		stringPair.set(rid, stringKey);
	}
}

// -----------------------------------------------------------------------------
// BTreeIndex::startScan
// -----------------------------------------------------------------------------

const void BTreeIndex::startScan(const void* lowValParm,
				   const Operator lowOpParm,
				   const void* highValParm,
				   const Operator highOpParm)
{

}

// -----------------------------------------------------------------------------
// BTreeIndex::scanNext
// -----------------------------------------------------------------------------

const void BTreeIndex::scanNext(RecordId& outRid) 
{

}
// -----------------------------------------------------------------------------
// BTreeIndex::endScan
// -----------------------------------------------------------------------------
//
const void BTreeIndex::endScan() 
{

}
//Custom Methods
template<class T>
int BTreeIndex::keyCompare(T key1, T key2) {
	if(key1 > key2) {
		return 1;
	}
	else if(key1 < key2) {
		return -1;
	}
	else {
		return 0;
	}
}
template<class T>
void BTreeIndex::insertNonEmptyNode() {

}
template<class T>
void BTreeIndex::insertEmptyNode() {

}
template<class T>
void BTreeIndex::insertSplitLeaf() {

}
template<class T>
void BTreeIndex::insertFullLeafAndNode() {

}	
}
