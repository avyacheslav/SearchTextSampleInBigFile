// mtfind.cpp: определяет точку входа для приложения.
//

//#include <iostream>
#include <string> 
#include <fstream>
#include <thread>
#include <vector>
#include "mtfind.h"

using namespace std;

constexpr int maxThreаdNumber = 10;
constexpr int minThreadStep = 1000000;

struct FoundSampleData
{
    int nStr;
    size_t nPosInStr;
    shared_ptr<char[]> sample;
};

struct SearchThreadData
{
    static string path;
    static string searchSample;
    static size_t fileSize;

    size_t startPos = 0;
    size_t endPos;
    shared_ptr<thread> searchThread = nullptr;

    int rowCount = 0;

    void Search();

    void SearchInLine(string str, int& lineNum);
    
    void CreateThread()
    {
        searchThread = shared_ptr<thread>(new thread(&SearchThreadData::Search, this));
    }
    
    vector<FoundSampleData> foundSamples;
};

string SearchThreadData::path;
size_t SearchThreadData::fileSize;
string SearchThreadData::searchSample;

void SearchThreadData::Search()
{
    ifstream file(path);
    file.seekg(startPos);
    size_t pos = 0;
    string s;
    int nLine = 0;
    size_t searchEndPosition = endPos;
    
    if (endPos + searchSample.length() < fileSize)
    {
        endPos += searchSample.length() - 1;
    }

    while (pos < endPos && pos != -1)
    {
        getline(file, s);
        rowCount++;

        pos = file.tellg();

        if (pos >= endPos)
        {
            s = s.substr(0, endPos - 1);
        }

        SearchInLine(s, ++nLine);
    }

    if (!s.empty() && s[s.length() - 1] != '\n')
    {
        rowCount--;
    }
}

void SearchThreadData::SearchInLine(string str, int& lineNum)
{
    size_t pos = 0;

    while (pos + searchSample.length() <= str.length())
    {
        int i = 0;

        for (; i < searchSample.length(); i++)
        {
            if (searchSample[i] == '?')
            {
                continue;
            }

            if (searchSample[i] != str[pos + i])
            {
                break;
            }
        }
        
        if (i == searchSample.length())
        {
            FoundSampleData foundSampleData;
            foundSampleData.nStr = lineNum;
            foundSampleData.nPosInStr = pos;
            foundSampleData.sample = shared_ptr<char[]>(new char[searchSample.length() + 1]);
            strcpy(foundSampleData.sample.get(), str.substr(pos, searchSample.length()).data());
            foundSampleData.sample[searchSample.length()] = '\0';

            foundSamples.push_back(foundSampleData);
        }

        pos++;
    }
}

int main(int argc, char** argv)
{/*
    if (argc < 3)
    {
        cout << "Not enough arguments: " << argc << endl;
        return 0;
    }
    */
    SearchThreadData::path = "C:/Users/1/Desktop/Financier.txt";// argv[1];
    SearchThreadData::searchSample = "for";// argv[2];

    ifstream file(SearchThreadData::path); 

    file.seekg(0, std::ios::end);
    SearchThreadData::fileSize = file.tellg();
    size_t threadStep = SearchThreadData::fileSize / maxThreаdNumber < minThreadStep ? minThreadStep : SearchThreadData::fileSize / 10;
   
    //Create threads data
    vector<SearchThreadData> searchThreads;
    size_t pos = 0;

    while (pos < SearchThreadData::fileSize - 1)
    {
        size_t end = pos + threadStep < SearchThreadData::fileSize - 1 ? pos + threadStep : SearchThreadData::fileSize;
        
        SearchThreadData searchThreadData;
        searchThreadData.startPos = pos;
        searchThreadData.endPos = end;
        
        searchThreads.push_back(searchThreadData);

        pos = end;
    }
    

    for (SearchThreadData& threadData : searchThreads)
    {
        threadData.CreateThread();
    }
    for (SearchThreadData& threadData : searchThreads)
    {
        threadData.searchThread->join();
    }


    // Output data
    
    size_t count = 0;

    for (SearchThreadData& threadData : searchThreads)
    {
        count += threadData.foundSamples.size();
    }

    int nstr = 0;

    for (SearchThreadData& threadData : searchThreads)
    {
        cout << count << endl;
        
        for(int i = 0; i < threadData.foundSamples.size(); i++)
        {
            cout << nstr + threadData.foundSamples[i].nStr << " " << threadData.foundSamples[i].nPosInStr << " " << threadData.foundSamples[i].sample << endl;
        }

        nstr += threadData.rowCount;
    }

	return 0;
}
