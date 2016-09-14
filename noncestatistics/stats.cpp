#include "stats.hpp"


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <regex>
#include <fstream>

std::vector<std::pair<std::string, int> > sortNonceList(std::map<std::string, int>& nonceList){
    std::vector<std::pair<std::string, int> > sortedList;
    
    for (auto p : nonceList){ sortedList.push_back(p); }
    std::sort(sortedList.begin(), sortedList.end(), [](std::pair<std::string, int> &a, std::pair<std::string, int> &b){
        return a.second < b.second;
    });
    
    return sortedList;
}


void cmd_statistics(const char* filename){
    int amount = 0;
    std::ifstream myfile;
    myfile.open(filename);
    std::string line;
    
    std::map<std::string, int> nonceList;
    
    while ((myfile >> line)) {
        std::regex r("[[:digit:]a-f]{40}");
        std::smatch match;
        if(std::regex_search(line, match, r)){
            nonceList[match[0]]++;
            amount++;
        }
    }
    myfile.close();
    
    std::vector<std::pair<std::string, int> > sortedList = sortNonceList(nonceList);
    
    std::cout << "nonce                                     abs. frequency    rel. frequency" << std::endl;
    std::cout << "===========================================================================" << std::endl;
    for (auto p: sortedList) {
        if (p.second == 1) continue;
        printf("%s         %4d             %2.3f%%\n",p.first.c_str(),p.second,100*((float)p.second/amount));
    }
    std::cout << "===========================================================================" << std::endl;
    std::cout << "nonce                                     abs. frequency    rel. frequency" << std::endl<<std::endl;
    
    if (sortedList.size() == 0) std::cout <<  "There where no collisions found!"<<std::endl<<std::endl;
    
    std::cout << "There is a total of "<< amount << " nonces" << std::endl;
}
