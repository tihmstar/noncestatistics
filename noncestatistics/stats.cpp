#include "stats.hpp"


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <regex>
#include <fstream>

bool isHex(char c){
    return isdigit(c) || ('a'<= c && c <= 'f');
}

char* getNonceFromLine(char* buf, int len){
    bool foundNonce = false;
    int i = 0;
    int startpos = 0;
    while (i<(len-40) && buf[i+40] != '\n') {
        for (int j=0; j<20; j++) {
            if (!isHex(buf[j])) {
                break;
            }else if (j==40){
                foundNonce = true;
            }
        }
        if (foundNonce) {
            startpos = i;
            break;
        }
    }
    if (foundNonce) {
        return buf+startpos;
    }else{
        return nullptr;
    }
    
}


char* getNextLinePtr(char* buf, int len){
    int i = 0;
    while (i < len && buf[i] != '\n') {
        i++;
    }
    if ((i+1) >= len) {
        return nullptr;
    }else{
        return buf+i+1;
    }
}


std::map<std::string, int> createNonceList(char* buf, int len){
    std::map<std::string, int> nonceList;
    char* position = buf;
    int i=0;
    while (i<len) {
        const char* nonce = getNonceFromLine(position, len);
        if (nonce != nullptr) {
            std::string nonceStr(nonce, 40);
            nonceList[nonceStr] = nonceList[nonceStr]+1;
        }
        position = getNextLinePtr(buf, len);
        if (position == nullptr) {
            break;
        }
    }
    return nonceList;
}

std::vector<std::pair<std::string, int>> sortNonceList(std::map<std::string, int>& nonceList){
    std::vector<std::pair<std::string, int>> sortedList;
    
    for (auto p : nonceList){ sortedList.push_back(p); }
    std::sort(sortedList.begin(), sortedList.end(), [](std::pair<std::string, int> &a, std::pair<std::string, int> &b){
        return a.second < b.second;
    });
    
    return sortedList;
}


void cmd_statistics(const char* filename){
    std::ifstream myfile;
    myfile.open(filename);
    std::string line;
    
    std::map<std::string, int> nonceList;
    
    
    while ((myfile >> line)) {
        std::regex r("[[:digit:]a-f]{40}");
        std::smatch match;
        if(std::regex_search(line, match, r)){
            nonceList[match[0]]++;
        }

    }
    
    std::vector<std::pair<std::string, int>> sortedList = sortNonceList(nonceList);
    for (auto p: sortedList) {
        std::cout << p.first << " -- " << p.second << std::endl;
    }
}
