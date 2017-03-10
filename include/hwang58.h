#ifndef HWANG58_H_
#define HWANG58_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <vector>
#include <list>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <ifaddrs.h>

#include "../include/global.h"
#include "../include/logger.h"

using namespace std;

// struct
struct HostData{
  int cfd;
  string hostname;
  string ip;
  string port;
  int num_msg_sent;
  int num_msg_rcv;
  string status;
  vector<string> bufmsgs;
  vector<string> blockeduser;

  bool operator<(const HostData &rhs) const {
    return atoi(port.c_str()) < atoi(rhs.port.c_str());
  }
};

// variables
extern string myHostname;
extern string myPORT;
extern string myIP;
extern struct addrinfo *myAddrInfo;
extern int sockfd;
extern vector<HostData> hostDatas;

// functions

void clientProcess(char* port);
void serverProcess(char* port);

void handleCommands(const char* buf, int clientMode);
void handleServerEvents(const char* buf, int cfd);
void handleClientEvents(const char* buf);

// commands

void cmdError(string command_str);
void cmdAuthor(string command_str);
void cmdIP(string command_str);
void cmdPort(string command_str);
void cmdList(string cmd);

void cmdLogin(string cmd, string ip, string port);
void cmdRefresh(string cmd);
void cmdLogout(string cmd);
void cmdExit(string command_str);
void cmdSend(string cmd, string ip, string orinmsg);
void cmdBroadcast(string cmd, string orinmsg);
void cmdBlock(string cmd, string ip);
void cmdUnblock(string cmd, string ip);

void cmdStat(string cmd);
void cmdBlockedList(string cmd, string ip);

// events

void eventLogin(vector<string> cmd, int cmdArgc, int cfd);
void eventRefresh(vector<string> cmd, int cmdArgc, int cfd);
void eventLogout(vector<string> cmd, int cmdArgc);
void eventExit(vector<string> cmd, int cmdArgc, int cfd);
void eventSend(vector<string> cmd, int cmdArgc, int cfd, string orinmsg);
void eventBroadcast(vector<string> cmd, int cmdArgc, int cfd, string orinmsg);
void eventBlock(vector<string> cmd, int cmdArgc, int cfd);
void eventUnblock(vector<string> cmd, int cmdArgc, int cfd);

void eventMsgRecv(vector<string> cmd, int cmdArgc, string orinmsg);

// tools
void initMyAddress(char* port);
void initClientSocket(char* port);
void initServerSocket(char* port);

vector<string> splitArgs(const string &text, char sep);
int checkIfIP(string str);
int checkIfPort(string str);

HostData* newHostData(int cfd, string hostname, string ip, string port);
HostData* getHostData(string ip, string port);
HostData* getHostData(string ip);
HostData* getHostData(int cfd);

bool checkIfIPBlocked(HostData* hd, std::string ip);

#endif
