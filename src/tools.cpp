#include "../include/hwang58.h"

void initMyAddress(char* port) {
  int status;
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);  // 确保 struct 为空

  hints.ai_family = AF_INET;        // IPv4
  hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;      // 帮我填好我的 IP

  if ((status = getaddrinfo(NULL, port, &hints, &myAddrInfo)) != 0) {
    exit(1);
  }
  struct sockaddr_in* ipv4 = (struct sockaddr_in*)myAddrInfo->ai_addr;
  myIP = inet_ntoa(ipv4->sin_addr);
  char hostname[1024];
  gethostname(hostname, sizeof(hostname) - 1);
  myHostname = hostname;
  myPORT = port;
}

void initClientSocket(char* port) { initMyAddress(port); }

void initServerSocket(char* port) {
  initMyAddress(port);
  int opt = 1;
  sockfd = socket(myAddrInfo->ai_family, myAddrInfo->ai_socktype,
                  myAddrInfo->ai_protocol);
  if (sockfd < 0 ||
      setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) <
          0 ||
      bind(sockfd, myAddrInfo->ai_addr, myAddrInfo->ai_addrlen) < 0 ||
      listen(sockfd, MAX_SERVER_CONN) < 0) {
    close(sockfd);
    freeaddrinfo(myAddrInfo);
    exit(1);
  }
}

vector<string> splitArgs(const string &text, char sep) {
  vector<string> tokens;
  size_t start = 0, end = 0;
  while ((end = text.find(sep, start)) != string::npos) {
    tokens.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(text.substr(start));
  return tokens;
}

int checkIfIP(string str){
	int a=-1,b=-1,c=-1,d=-1;
	char tail[256];
  tail[0] = 0;
	int r = sscanf(str.c_str(), "%d.%d.%d.%d%s", &a, &b, &c, &d, tail);
	if(r==4 && !tail[0] && a>=0 && a<=255 && b>=0 && b<=255 && c>=0 && c<=255 && d>=0 && d<=255){
		return 1;
	}else{
		return -1;
	}
}

int checkIfPort(string str){
	int a=-1;
	char tail[256];
  tail[0] = 0;
	int r = sscanf(str.c_str(), "%d%s", &a, tail);
	if(r==1 && !tail[0] && a>0 && a<=65535){
		return 1;
	}else{
		return -1;
	}
}

HostData* newHostData(int cfd, string hostname, string ip, string port){
	HostData* hd = new HostData;
	hd->cfd = cfd;
	hd->hostname = hostname;
	hd->ip = ip;
	hd->port = port;
	hd->num_msg_sent = 0;
	hd->num_msg_rcv = 0;
	hd->status = "logged-in";
	return hd;
}

HostData* getHostData(string ip, string port){
	for(unsigned int i = 0; i < hostDatas.size(); ++i)  {
		HostData* hd = &hostDatas[i];
		if(hd->ip == ip && hd->port == port){
			return hd;
		}
	}
	return NULL;
}

HostData* getHostData(string ip){
	for(unsigned int i = 0; i < hostDatas.size(); ++i)  {
		HostData* hd = &hostDatas[i];
		if(hd->ip == ip){
			return hd;
		}
	}
	return NULL;
}

HostData* getHostData(int cfd){
	for(unsigned int i = 0; i < hostDatas.size(); ++i)  {
		HostData* hd = &hostDatas[i];
		if(hd->cfd == cfd){
			return hd;
		}
	}
	return NULL;
}

bool checkIfIPBlocked(HostData* hd, std::string ip){
  for(unsigned int i = 0; i < hd->blockeduser.size(); ++i)  {
  	if(hd->blockeduser[i] == ip){
  		return true;
  	}
  }
  return false;
}
