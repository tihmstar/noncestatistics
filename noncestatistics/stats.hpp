#ifndef stats_hpp
#define stats_hpp

#include <map>
#include <vector>
#include <string>

char* getNonceFromLine(char* buf, int len);
char* getNextLinePtr(char* buf, int len);
std::map<std::string, int> createNonceList(char* buf, int len);
std::vector<std::pair<std::string, int>> sortNonceList(std::map<std::string, int>& nonceList);
void cmd_statistics(const char* filename);


#endif /* stats_hpp */
