//===================================
// include guard
#ifndef NETWORK_HELPERS_H
#define NETWORK_HELPERS_H

//===================================
// included dependencies
#include<string>

//===================================
// methods
std::string GetField(int start, char * fields, int field_len);
int StoI(std::string number);
void PrintPDU(char * PDU);

#endif /* NETWORK_HELPERS_H */
