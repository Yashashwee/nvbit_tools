#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include<unordered_map>
using namespace std;


typedef struct kernelStruct
{
    int id=0;
    string name;
    int blkCnt=0;
} kernelStruct;

typedef struct simThreadStruct
{
    int id;
    int currKernel=-1;
} simThreadStruct;


int totalInstructionProcessed=0;

void getTotalBlocks(vector<kernelStruct> &kernels,string statFilePath)
{
    ifstream statFilePointer;
    statFilePointer.open(statFilePath);
    int cntLine=0;
    string line;
    while(getline(statFilePointer,line))
    {
        kernelStruct tempK;

        if(!cntLine)
        {
            cntLine++;
            continue;
        }
        cntLine++;
        tempK.id=cntLine-1;
        stringstream ss(line);
        int cnt = 0;
        while(!ss.eof())
        {
            string word;
            getline(ss,word,',');
            // cout<<cnt<<":"<<word<<endl;
            if(cnt==0)
                tempK.name = word+"g";
                // kernelNames.push_back(word+"g");
            else if(cnt==5)
                tempK.blkCnt = stoi(word);
                // totalBlocks.push_back(stoi(word));
            cnt++;
        }
        kernels.push_back(tempK);

    }
    statFilePointer.close();
}
void closeFiles(ofstream *fpArray,int maxSimThreads)
{
    for(int i=0;i<maxSimThreads;i++)
        fpArray[i].close();
}

void splitFiles(int maxSimThreads,ofstream *fpArray,vector<kernelStruct> kernels,string traceFolder)
{
    int allBlockCount = 0;
    for(auto i:kernels)
        allBlockCount+=i.blkCnt;
    cout<<"Total Number of block: "<<allBlockCount<<" divided in "<<maxSimThreads<<" sim threads\n";
    int blocksPer = allBlockCount%maxSimThreads? allBlockCount/maxSimThreads+1: allBlockCount / maxSimThreads;
    int blocksRemain = allBlockCount;
    int currSimThread=0;
    vector<int> threadCount(maxSimThreads,0);
    int tmpCnt=0;
    while(blocksRemain>0)
    {
        threadCount[tmpCnt%maxSimThreads]++;
        blocksRemain--;
        tmpCnt++;
    }
    
    for(int i:threadCount)
        cout<<i<<endl;
    blocksRemain=allBlockCount;
    int blocksWritten=0;
    for(auto s : kernels)
    {
        unordered_map<string,int> ipMap;
        int totIps = 0;
        cout<<s.name<<endl;
        ifstream tempFP;
        tempFP.open(traceFolder+"/"+s.name);
        // cout<<traceFolder+"/"+s+"g"<<endl;
        string line;
        bool startTB=false;
        fpArray[currSimThread]<<"KERNEL START "<<s.blkCnt<<endl;
        while(getline(tempFP,line))
        {
            // cout<<line<<endl;
            
            stringstream ss(line);
            string word;
            ss>>word;
            if(word=="")
                continue;
            if(word=="#BEGIN_TB")
            {
                startTB=true;
                // fpArray[currSimThread]<<
            }
            else if(word=="#END_TB")
            {
                startTB=false;
                fpArray[currSimThread]<<"BLOCK END"<<endl;
                threadCount[currSimThread]--;
                blocksWritten++;
                if(threadCount[currSimThread]<=0)
                {
                    currSimThread++;
                    if(currSimThread>=maxSimThreads && blocksWritten!=allBlockCount)
                    {
                        cerr<<"Error: Blocks exceed max sim threads\n";
                        return;
                    }
                    // cout<<"Writing Kernel Start\n";
                    if(currSimThread<maxSimThreads && blocksWritten!=s.blkCnt)
                        fpArray[currSimThread]<<"KERNEL START "<<s.blkCnt<<endl;
                }

            }
            else if(startTB)
            {
                
                if(word == "thread" or word == "warp" or word == "insts")
                    fpArray[currSimThread]<<line<<endl;
                else
                {
                    totalInstructionProcessed++;
                    // cout<<line.substr(14,line.find("0x")-14)<<endl;
                    string key = line.substr(14,line.find("0x")-14);
                    // string key = line.substr(0,13);
                    int ipInt;
                    if(ipMap.find(key)!=ipMap.end())
                        ipInt=ipMap[key];
                    else
                    {
                        ipInt=totIps;
                        ipMap[key]=ipInt;
                        totIps++;

                    }
                    fpArray[currSimThread]<<"ip"<<ipInt<<" "<<line<<endl;
                }
                    
                // {
                //     //Restart word
                //     vector<string> tempIP;
                //     tempIP.push_back(word);
                //     while(!ss.eof())
                //     {
                //         getline(ss,word,' ');
                //         // fpArray[currSimThread]<<word<<"&";
                //         tempIP.push_back(word);
                //     }
                //     for(int idx = 0; idx<tempIP.size();idx++)
                //     {
                //         if(tempIP[idx].size()==0 || tempIP[idx]==" ")
                //             continue;
                //         else if(idx!=tempIP.size()-1)
                //         {
                //             fpArray[currSimThread]<<tempIP[idx]<<"&";
                //         }
                //         else
                //         {
                //             fpArray[currSimThread]<<tempIP[idx];
                //         }
                //     }
                //     fpArray[currSimThread]<<endl;
                // }
            }
            
        }
    }


}

void splitFiles2(int maxSimThreads,ofstream *fpArray,vector<kernelStruct> kernels,string traceFolder)
{
    vector<simThreadStruct> simthreads(maxSimThreads);
    int allBlockCount = 0;
    for(auto i:kernels)
        allBlockCount+=i.blkCnt;
    cout<<"Total Number of block: "<<allBlockCount<<" divided in "<<maxSimThreads<<" sim threads\n";
    int blocksPer = allBlockCount%maxSimThreads? allBlockCount/maxSimThreads+1: allBlockCount / maxSimThreads;
    int blocksRemain = allBlockCount;
    int currSimThread=0;
    vector<int> threadCount(maxSimThreads,0);
    int tmpCnt=0;
    while(blocksRemain>0)
    {
        threadCount[tmpCnt%maxSimThreads]++;
        blocksRemain--;
        tmpCnt++;
    }
    
    for(int i:threadCount)
        cout<<i<<endl;
    blocksRemain=allBlockCount;
    int blocksWritten=0;
    unordered_map<string,int> ipMap;
    int totIps = 0;
    for(auto s : kernels)
    {
        
        
        cout<<s.name<<endl;
        ifstream tempFP;
        tempFP.open(traceFolder+"/"+s.name);
        // cout<<traceFolder+"/"+s+"g"<<endl;
        string line;
        bool startTB=false;
        
        while(getline(tempFP,line))
        {
            currSimThread = blocksWritten%maxSimThreads;
            if(simthreads[currSimThread].id != s.id)
            {
                simthreads[currSimThread].id = s.id;
                fpArray[currSimThread]<<"KERNEL START "<<s.blkCnt<<endl;
            }
            // cout<<line<<endl;
            
            stringstream ss(line);
            string word;
            ss>>word;
            if(word=="")
                continue;
            if(word=="#BEGIN_TB")
            {
                startTB=true;
                // fpArray[currSimThread]<<
            }
            else if(word=="#END_TB")
            {
                startTB=false;
                fpArray[currSimThread]<<"BLOCK END"<<endl;
                threadCount[currSimThread]--;
                blocksWritten++;
            }
            else if(startTB)
            {
                
                if(word == "thread" or word == "warp" or word == "insts")
                    fpArray[currSimThread]<<line<<endl;
                else
                {
                    totalInstructionProcessed++;
                    // cout<<line.substr(14,line.find("0x")-14)<<endl;
                    string key = line.substr(14,line.find("0x")-14);
                    // string key = line.substr(0,13);
                    int ipInt;
                    if(ipMap.find(key)!=ipMap.end())
                        ipInt=ipMap[key];
                    else
                    {
                        ipInt=totIps;
                        ipMap[key]=ipInt;
                        totIps++;

                    }
                    fpArray[currSimThread]<<"ip"<<ipInt<<" "<<line<<endl;
                }
            }
            
        }
    }


}

int main(int argc, char **argv)
{
    string traceFolder;
    int maxSimThreads = 16;
    vector<kernelStruct> kernels;
    if(argc == 1)
    {
        cout<<"No command line args\n";
        traceFolder = "/home/yashashwee/gputejas/nvbit-Linux-x86_64-1.5.5/nvbit_release/traces";
        // return 0;
    }
    else if(argc==2)
    {
        traceFolder = argv[1];
    }
    else if(argc==3)
    {
        traceFolder = argv[1];
        maxSimThreads = stoi(argv[2]);
    }
    else
    {
        cout<<"Too many arguments\n";
        return 0;
    }

    string statFilePath = traceFolder  + "/stats.csv";
    getTotalBlocks(kernels,statFilePath);
    
    ofstream *fpArray = new ofstream[maxSimThreads];
    for(int i=0;i<maxSimThreads;i++)
    {
        fpArray[i].open(traceFolder+"/"+to_string(maxSimThreads)+"/" + to_string(i) + ".txt");
    }
    splitFiles2(maxSimThreads,fpArray,kernels,traceFolder);
    closeFiles(fpArray,maxSimThreads);
    cout<<"Total instruction executed: "<<totalInstructionProcessed<<endl;

    return 0;
}