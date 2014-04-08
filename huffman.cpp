/**
 * File: huffman.cpp 
 * -------------------
 * This program uses the Huffman encoding algorithm to compress and decompress
 * any file type.
 */
 
#include <iostream>
#include "console.h"
#include "encoding.h"
#include "simpio.h"
using namespace std;
 
/* Function Prototypes */
void printWelcomeMsg();
static bool responseIsAffirmative(const string &prompt);
void compression(Encoding &module);
void decompression(Encoding &module);
void setupFiles(ibstream &infile, obstream &outfile, string command);
string getFilename(string text);
 
/* Main Program */
int main()
{
    Encoding module;
    printWelcomeMsg();
     
    while(true)
    {
        if(!responseIsAffirmative("Would you like to compress or decompress another file? "))
            break;
        if(responseIsAffirmative("Are we compressing? "))
            compression(module);
        else
            decompression(module);
    }
 
    return 0;
}
 
/* Prints a welcome message at the start of the program */
void printWelcomeMsg()
{
    cout << "Welcome! This program uses the Huffman coding algorithm for compression, " << endl;
    cout << "Any file can be compressed by this method, often with substantial savings. " << endl;
    cout << "Decompression will faithfully reproduce the original." << endl << endl;
}
 
/* Gets response from user and checks if yes or no */
static bool responseIsAffirmative(const string &prompt)
{
    while(true) {
        string answer = getLine(prompt);
        if(!answer.empty()) {
            switch(toupper(answer[0])) {
                case 'Y': return true;
                case 'N': return false;
            }
        }
        cout << "Please answer yes or no." << endl;
    }
}
 
/* Manages all the compression of indicated file */
void compression(Encoding &module)
{
    ibstream infile;
    obstream outfile;
     
    setupFiles(infile, outfile, "compress");
    module.compress(infile, outfile);
 
    //check sizes of original and compressed file
    cout << "Uncompressed size in bytes: " << infile.size() << endl;
    cout << "Compressed size in bytes: " << outfile.size() << endl;
    if(infile.size() > outfile.size())
    {
        double percentage = ((double)infile.size() - (double)outfile.size())/(double)infile.size();
        cout << "Compression: " << percentage*100 << "%" << endl << endl;
    }
    else if(infile.size() == outfile.size())
        cout << "Compressed file is the same size as the original file!" << endl << endl;
    else
        cout << "Compressed file is actually bigger!" << endl << endl;
 
    infile.close();
    outfile.close();
}
 
/* Manages all the decompression of indicated file */
void decompression(Encoding &module)
{
    ibstream infile;
    obstream outfile;
     
    setupFiles(infile, outfile, "decompress");
    module.decompress(infile, outfile);
    cout << "Your file has been decompressed!" << endl << endl;
    infile.close();
    outfile.close();
}
 
/* Gets infile/outfile names from user and opens the files */
void setupFiles(ibstream &infile, obstream &outfile, string command)
{
    string prompt, inName, outName;
 
    while(true)
    {
        prompt = "Enter the name of the file to " + command + ": ";
        inName = getFilename(prompt);
        infile.open(inName.c_str());
        if(infile.fail())
        {
            cout << "We couldn't find that file. Please try again." << endl;
            infile.clear();
        } else break;
    }
 
    //get name of file to create (for output)
    while(true)
    {
        prompt = "Enter the name we should give the " + command + "ed file: ";
        outName = getFilename(prompt);
        ibstream testInfile;
        testInfile.open(outName.c_str());
        if(!testInfile.fail())
        {
            cout << "File already exists, and we don't overwrite existing files!" << endl;
            testInfile.clear();
        } else
        {
            outfile.open(outName.c_str());
            if(outfile.fail())
            {
                cout << "Error opening outfile." << endl;
                outfile.clear();
            } else break;
        }
        testInfile.close();
    }
}
 
/* Asks user for filename and returns user input */
string getFilename(string text)
{
    cout << text;
    return getLine();
}