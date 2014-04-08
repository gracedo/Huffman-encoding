/**
 * File: encoding.h
 * ----------------
 * Defines the Encoding class to manage all aspects of the
 * Huffman compression and decompression algorithms.
 */
 
#ifndef _encoding_
#define _encoding_
 
#include "bstream.h"
#include "pqueue.h"
 
class Encoding {
public:
    Encoding();
    ~Encoding();
 
    void compress(ibstream& infile, obstream& outfile);
    void decompress(ibstream& infile, obstream& outfile);
 
private:
    struct MappedFreq
    {
        int byte;
        int freq;
    };
 
    struct Node
    {
        int byte;
        int freq;
        bool isEmpty;
        Node *right, *left;
    };
 
    void readInput(ibstream &infile);
    void writeHeader(obstream &outfile);
    void createPQueue(PQueue<Node*>& currQueue);
    void createTree(PQueue<Node*>& currQueue);
    void writeCompressed(ibstream &infile, obstream &outfile);
    string findBitPattern(Node *curr, int readBit, string pattern);
    void readHeader(ibstream &infile);
    void writeDecompressed(ibstream &infile, obstream &outfile);
    int getBitFromTree(ibstream &infile, int readBit, Node* curr);
    void freeNodes(Node* parent);
 
    void sortVector();
    void buildHeap();
    void siftDown(int start, int end);
    void sortHeap();
 
    static const int PSEUDOEOF = 256;
    static const string headerIndicator;
    Vector<MappedFreq> mapEntries;
    PQueue<Node*> infileQueue, outfileQueue;
};
 
#endif