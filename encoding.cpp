/**
 * File: encoding.cpp
 * ------------------
 * Encoding class implementation that handles all the file
 * compression and decompression.
 */
 
#include <iostream>
#include "encoding.h"
#include "strlib.h"
#include "error.h"
using namespace std;
 
const string Encoding::headerIndicator = "^HUFFMANENCODING$$GRACEDO*";
 
Encoding::Encoding() {}
 
Encoding::~Encoding()
{
    freeNodes(infileQueue.peek());
    freeNodes(outfileQueue.peek());
    mapEntries.clear();
}
 
/* Compresses user-designated infile */
void Encoding::compress(ibstream &infile, obstream &outfile)
{
    readInput(infile);
    writeHeader(outfile);
    createPQueue(infileQueue);
    createTree(infileQueue);
    writeCompressed(infile, outfile);
    mapEntries.clear();
}
 
/* Decompresses user-designated infile */
void Encoding::decompress(ibstream &infile, obstream &outfile)
{
    readHeader(infile);
    createPQueue(outfileQueue);
    createTree(outfileQueue);
    writeDecompressed(infile, outfile);
    mapEntries.clear();
}
 
/* Reads infile and generates mapping of frequency of each byte */
void Encoding::readInput(ibstream &infile)
{
    int readByte = infile.get();
    mapEntries.clear();

    MappedFreq map;
    int index;
 
    while(readByte != EOF)
    {
        if(mapEntries.isEmpty())
        {
            map.byte = readByte;
            map.freq = 1;
            mapEntries.add(map);
        }
        else
        {
            for(index = 0; index < mapEntries.size(); index++)
            {
                if(mapEntries[index].byte == readByte) break;
            }
 
            if(index < mapEntries.size())
                mapEntries[index].freq++;
            else
            {
                map.byte = readByte;
                map.freq = 1;
                mapEntries.add(map);
            }
        }
        readByte = infile.get();
    }
 
    map.byte = PSEUDOEOF; //add psuedo-EOF with freq 1
    map.freq = 1;
    mapEntries.add(map);
    sortVector(); //sort vector so header is printed out in order of byte size
}
 
/* Writes a unique header at top of outfile */
void Encoding::writeHeader(obstream &outfile)
{
    outfile << headerIndicator; //write unique header indicator first
 
    int ascii, frequency;
    for(int i = 0; i < mapEntries.size(); i++)
    {
        ascii = mapEntries[i].byte;
        frequency = mapEntries[i].freq;
        outfile << ascii << "=" << frequency << ";";
    }
}
 
/* Creates a priority queue of the frequency mappings */
void Encoding::createPQueue(PQueue<Node*>& currQueue)
{
    Node *newNode;
    for(int i = 0; i < mapEntries.size(); i++)
    {
        newNode = new Node();
        newNode->byte = mapEntries[i].byte;
        newNode->freq = mapEntries[i].freq;
        newNode->isEmpty = false;
        newNode->left = NULL;
        newNode->right = NULL;
        currQueue.enqueue(newNode, mapEntries[i].freq);
    }
}
 
/* Generates the tree of bit patterns */
void Encoding::createTree(PQueue<Node*>& currQueue)
{
    Node *parent, *left, *right; //right node is weighted less (lower frequency)

    while(!currQueue.isEmpty())
    {
        parent = new Node();
        left = currQueue.extractMin();
        parent->left = left;
        parent->freq = left->freq;
        parent->isEmpty = true;
        if(!currQueue.isEmpty())
        {
            right = currQueue.extractMin();
            parent->right = right;
            parent->freq += right->freq;
        }
        currQueue.enqueue(parent, parent->freq);
        if(currQueue.size() == 1) break;
    }
}
 
/* Writes the compression to the outfile */
void Encoding::writeCompressed(ibstream &infile, obstream &outfile)
{
    infile.rewind();
    int readByte = infile.get();
    Node *parent = infileQueue.peek();;
    string bitPattern;
    int outBit;
 
    while(readByte != EOF)
    {
        bitPattern = findBitPattern(parent, readByte, "");
        for(int i = 0; i < bitPattern.size(); i++)
        {
            outBit = (int)bitPattern[i] - 48;
            outfile.writebit(outBit);
        }
        readByte = infile.get();
    }
 
    string psuedo = findBitPattern(parent, PSEUDOEOF, "");
    for(int i = 0; i < psuedo.size(); i++)
    {
        outBit = (int)psuedo[i] - 48;
        outfile.writebit(outBit);
    }
}
 
/* Recursively looks for the bit in the tree and returns the bit pattern */
string Encoding::findBitPattern(Node *curr, int readByte, string pattern)
{
    string updated;
    if(curr != NULL)
    {
        //if current node contains desired bit, return bit pattern
        if(!curr->isEmpty && curr->byte == readByte) return pattern;
         
        if(curr->left != NULL) //try left node first (higher priority)
        {
            updated = findBitPattern(curr->left, readByte, pattern+"0");
            if(updated != "") return updated;
        }
         
        if(!curr->isEmpty && curr->byte == readByte) return pattern;
         
        if(curr->right != NULL) //try right node
        {
            updated = findBitPattern(curr->right, readByte, pattern+"1");
            if(updated != "") return updated;
        }
    }
    return "";
}
 
/* Checks for unique header and generates mapping */
void Encoding::readHeader(ibstream &infile)
{
    int readByte = infile.get();
    string headerCheck = "";
 
    //keep track of bytes read and add to string, then compare to headerIndicator
    while(readByte != EOF)
    {
        headerCheck += readByte;
        if(readByte == '*') break;
        readByte = infile.get();
    }
 
    if(headerCheck != headerIndicator)
        error("Error: File not compressed with same version of application.");
 
    MappedFreq map;
    mapEntries.clear();
    string byteStream = "";
    int byteStreamInt;
 
    while((readByte = infile.get()) != EOF)
    {
        byteStream += readByte;
        if(readByte == '=')
        {
            byteStream = byteStream.substr(0,byteStream.size()-1);
            byteStreamInt = stringToInteger(byteStream);
            if(byteStreamInt == PSEUDOEOF) break;
            map.byte = byteStreamInt;
            byteStream = "";
        }
        else if(readByte == ';')
        {
            byteStream = byteStream.substr(0,byteStream.size()-1);
            byteStreamInt = stringToInteger(byteStream);
            map.freq = byteStreamInt;
            mapEntries.add(map);
            byteStream = "";
        }
    }
 
    //add PSEUDOEOF into map
    map.byte = PSEUDOEOF;
    map.freq = 1;
    mapEntries.add(map);
 
    do //finish reading header (up to ';') after finding PSEUDOEOF
    {
        readByte = infile.get();
    } while(infile.get() != ';');
}
 
/* Wrapper function that iterates through every bit to find corresponding byte from tree
 * and writes the byte to the outfile */
void Encoding::writeDecompressed(ibstream &infile, obstream &outfile)
{
    int startingBit = infile.readbit();
    Node* parent;
    int insert;
 
    while(startingBit != EOF)
    {
        parent = outfileQueue.peek();
        insert = getBitFromTree(infile, startingBit, parent);
        if(insert == PSEUDOEOF) break; //break out of while loop when "psuedo-EOF" is detected
        if(insert == (PSEUDOEOF+1)) error("Error with getting bit from tree.");
        outfile << (char)insert;
        startingBit = infile.readbit();
    }
}
 
/* Recursively looks for the byte in the tree */
int Encoding::getBitFromTree(ibstream &infile, int readBit, Node* curr)
{
    int nextBit;
    if(curr != NULL)
    {
        if(!curr->isEmpty) return curr->byte;
        if(readBit == 0 && curr->left != NULL)
        {
            if(!curr->left->isEmpty) return curr->left->byte;
            nextBit = infile.readbit();
            return getBitFromTree(infile, nextBit, curr->left);
        }
        if(readBit == 1 && curr->right != NULL)
        {
            if(!curr->right->isEmpty) return curr->right->byte;
            nextBit = infile.readbit();
            return getBitFromTree(infile, nextBit, curr->right);
        }
    }
    return (PSEUDOEOF+1);
}
 
/* Frees the allocated memory from the linked tree nodes */
void Encoding::freeNodes(Node* parent)
{
    if(parent != NULL)
    {
        freeNodes(parent->right);
        freeNodes(parent->left);
        delete parent;
    }
}
 
/* Implements the heapsort algorithm to sort the vector */
void Encoding::sortVector()
{
    buildHeap(); //create heap
    sortHeap(); //sort the heap
}
 
/* Builds the heap from bottom up */
void Encoding::buildHeap()
{
    int end = mapEntries.size();
    int start = (mapEntries.size()-2)/2; //index of last parent node
 
    //sift down node at index start to proper place such that all nodes below are in heap order
    while(start >= 0)
    {
        siftDown(start, end-1);
        start--;
    }
}
 
/* Builds the heap by making swaps between the parent and daughter nodes within the vector */
void Encoding::siftDown(int start, int end)
{
    int node = start;
    int daughter1, daughter2, parent;
    while((node*2 + 1) <= end)
    {
        daughter1 = node*2 + 1;
        daughter2 = daughter1 + 1;
        parent = node;
 
        if(mapEntries[parent].byte < mapEntries[daughter1].byte) parent = daughter1;
        if(daughter2 <= end && mapEntries[parent].byte < mapEntries[daughter2].byte) parent = daughter2;
        if(parent != node)
        {
            swap(mapEntries[node], mapEntries[parent]);
            node = parent;
        }
        else return;
    }
}
 
/* Sorts the heap by swapping the element at index [0] (the largest value) with
* the last element of the heap, and decreasing the size of the heap by 1 after each
* swap so that the previous max value stays in its correct position */
void Encoding::sortHeap() {
    int largestUnsorted = mapEntries.size()-1; //position of the largest unsorted index
    while(largestUnsorted > 0)
    {
        swap(mapEntries[0], mapEntries[largestUnsorted]); //swap first and last
        largestUnsorted--; //"remove" last node; it's in its correct, sorted position
        siftDown(0, largestUnsorted); //reorder heap so that the next largest number is on top
    }
}
 