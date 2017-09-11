/**
 * @author See Contributors.txt for code contributors and overview of BadgerDB.
 *
 * @section LICENSE
 * Copyright (c) 2012 Database Group, Computer Sciences Department, University of Wisconsin-Madison.
 */

#include <memory>
#include <iostream>
#include "buffer.h"
#include "exceptions/buffer_exceeded_exception.h"
#include "exceptions/page_not_pinned_exception.h"
#include "exceptions/page_pinned_exception.h"
#include "exceptions/bad_buffer_exception.h"
#include "exceptions/hash_not_found_exception.h"

namespace badgerdb { 

BufMgr::BufMgr(std::uint32_t bufs)
	: numBufs(bufs) {
	bufDescTable = new BufDesc[bufs];

  for (FrameId i = 0; i < bufs; i++) 
  {
  	bufDescTable[i].frameNo = i;
  	bufDescTable[i].valid = false;
  }

  bufPool = new Page[bufs];

  int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
  hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

  clockHand = bufs - 1;
}


BufMgr::~BufMgr() {
	//Must flush all dirty pages to disk
	for(std::uint32_t i = 0; i < numBufs; i++) {
		if(bufDescTable[i].valid == true) {
			if(bufDescTable[i].dirty == true) {
				bufDescTable[i].file->writePage(bufPool[i]);
				bufDescTable[i].dirty = false;
			}
		}
	}
	delete[] bufDescTable;
	delete[] bufPool;
	delete hashTable;
}

void BufMgr::advanceClock()
{
	clockHand = (clockHand + 1) % numBufs;
}

void BufMgr::allocBuf(FrameId & frame) 
{
	//Used to see if all pages are pinned
	std::uint32_t pinnedPages = 0;
	bool done = false;
	//TODO potentially change to a for loop
	while(!done) {
		advanceClock();
		//Check if valid
		if(bufDescTable[clockHand].valid ==  true) {
			//Check if frame has been accessed recently
			if(bufDescTable[clockHand].refbit == true) {
				bufDescTable[clockHand].refbit = false;
				if(bufDescTable[clockHand].pinCnt != 0) {
					pinnedPages++;
				}
			}
			else {
				//Check if Pin Count is 0
				if(bufDescTable[clockHand].pinCnt == 0) {
					//Check if page is dirty
					if(bufDescTable[clockHand].dirty == true) {
						bufDescTable[clockHand].file->writePage(bufPool[clockHand]);
						hashTable->remove(bufDescTable[clockHand].file, bufDescTable[clockHand].pageNo);
						bufDescTable[clockHand].Clear();
						frame = clockHand;
						done = true;
					}
					else {
						hashTable->remove(bufDescTable[clockHand].file, bufDescTable[clockHand].pageNo);
						bufDescTable[clockHand].Clear();
						frame = clockHand;
						done = true;
					}
				}
				else {
					pinnedPages++;
				}
			}
		}
		else {
			bufDescTable[clockHand].valid = true;
			frame = clockHand;
			done = true;
		}	
		if(pinnedPages == numBufs) {
			throw BufferExceededException();
		}
	}
}

	
void BufMgr::readPage(File* file, const PageId pageNo, Page*& page)
{
	//NOTHING IS DONE WITH page
	FrameId tempFrame;
	if(hashTable->lookup(file, pageNo, tempFrame) == true) {
		//Page is in Buffer Pool
		bufDescTable[tempFrame].refbit = 1;
		bufDescTable[tempFrame].pinCnt++;
		page = &(bufPool[tempFrame]);
	}
	else {
		//Page is not in buffer pool
		FrameId tempFrame;
		//TODO: handle when the buffer pool is full aka all pages are pinned
		allocBuf(tempFrame);
		//TODO May need to put something on the left side, read page returns a page
		bufPool[tempFrame] = file->readPage(pageNo);
		hashTable->insert(file, pageNo, tempFrame);
		bufDescTable[tempFrame].Set(file, pageNo);
		page = &(bufPool[tempFrame]);
	}
}


void BufMgr::unPinPage(File* file, const PageId pageNo, const bool dirty) 
{
	FrameId tempFrame;
	if(hashTable->lookup(file, pageNo, tempFrame) == true) {
		if(dirty == true) {
			//Sets dirty bit
			bufDescTable[tempFrame].dirty = true;
		}
		if(bufDescTable[tempFrame].pinCnt == 0) {
			throw PageNotPinnedException(file->filename(),pageNo, tempFrame);
		}
		else {
			bufDescTable[tempFrame].pinCnt--;
		}
	}
}

void BufMgr::flushFile(const File* file) 
{
	for(std::uint32_t i = 0; i < numBufs; i++) {
		if(bufDescTable[i].file == file) {
			if(bufDescTable[i].valid == false) {
				throw BadBufferException(bufDescTable[i].frameNo, bufDescTable[i].dirty, false, bufDescTable[i].refbit);
			}
			if(bufDescTable[i].pinCnt != 0) {
				throw PagePinnedException(bufDescTable[i].file->filename(), bufDescTable[i].pageNo, bufDescTable[i].frameNo);
			}
			if(bufDescTable[i].dirty == true) {
				bufDescTable[i].file->writePage(bufPool[i]);
				bufDescTable[i].dirty = false;
			}
			//TODO may need a try catch here
			hashTable->remove(file, bufDescTable[i].pageNo);
			bufDescTable[i].Clear();
		}
	}
}

void BufMgr::allocPage(File* file, PageId &pageNo, Page*& page) 
{
	FrameId frame;
	//TODO if error, probably this
 	Page page1 = file->allocatePage();
	pageNo = page1.page_number();
	allocBuf(frame);
	hashTable->insert(file, pageNo, frame);
	bufDescTable[frame].Set(file, pageNo);
	bufPool[frame] = page1;
	page = &(bufPool[frame]);
}

void BufMgr::disposePage(File* file, const PageId PageNo)
{
	FrameId tempFrame;
	if(hashTable->lookup(file, PageNo, tempFrame)) {
		hashTable->remove(file, PageNo);
		bufDescTable[tempFrame].Clear();	
	}
	file->deletePage(PageNo);	
}

void BufMgr::printSelf(void) 
{
  BufDesc* tmpbuf;
	int validFrames = 0;
  
  for (std::uint32_t i = 0; i < numBufs; i++)
  {
  	    tmpbuf = &(bufDescTable[i]);
		std::cout << "FrameNo:" << i << " ";
		tmpbuf->Print();

  	if (tmpbuf->valid == true)
    	validFrames++;
  }

	std::cout << "Total Number of Valid Frames:" << validFrames << "\n";
}

}
