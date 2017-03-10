#include "../include/hwang58.h"

// common

void cmdError(string cmd) {
  cse4589_print_and_log("[%s:ERROR]\n", cmd.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdAuthor(string cmd) {
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log(
      "I, %s, have read and understood the course academic integrity policy.\n",
      "hwang58");
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdIP(string cmd) {
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log("IP:%s\n", myIP.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdPort(string cmd) {
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log("IP:%s\n", myPORT.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdList(string cmd) {
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  for (unsigned int i = 0; i < hostDatas.size(); ++i) {
    cse4589_print_and_log("%-5d%-35s%-20s%-8s\n", i + 1,
                          hostDatas[i].hostname.c_str(),
                          hostDatas[i].ip.c_str(), hostDatas[i].port.c_str());
  }
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

// client only

void cmdLogin(string cmd, string ip, string port) {
  if (checkIfIP(ip) < 0 || checkIfPort(port) < 0) {
    cmdError(cmd);
    return;
  }
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  // connect to server when init
  if (sockfd == 0) {
    getaddrinfo(ip.c_str(), port.c_str(), &hints, &res);
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0 || connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
      close(sockfd);
      freeaddrinfo(res);
      cmdError(cmd);
      return;
    }
  }
  // send login request
  string msg = "LOGIN " + myHostname + " " + myIP + " " + myPORT;
  send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);
  // receive all response
  char tmp[65535];
  vector<string> recvmsg, recvdata;
  int recvRet = recv(sockfd, tmp, sizeof tmp, 0);
  msg = tmp;
  if (recvRet < 0) {
    cmdError(cmd);
    return;
  }
  hostDatas.clear();
  recvmsg = splitArgs(msg, '\n');
  if (recvmsg.size() < 1 || recvmsg[0] != "LOGINSUCCESS") {
    cmdError(cmd);
    return;
  }
  int mark;
  // receive list
  for (mark = 1; mark < recvmsg.size() - 1; mark++) {
    recvdata = splitArgs(recvmsg[mark], ' ');
    if (recvdata.size() < 1 || recvdata[0] == "LOGINEND") {
      break;
    }
    hostDatas.push_back(
        *newHostData(-1, recvdata[0], recvdata[1], recvdata[2]));
  }
  // receive cached msg
  for (; mark < recvmsg.size() - 1; mark++) {
    recvdata = splitArgs(recvmsg[mark], ' ');
    if (recvdata.size() < 1 || recvdata[0] == "LOGINCACHEEND") {
      break;
    }
    handleClientEvents(recvmsg[mark].c_str());
  }
  // cmdList(cmd);
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdRefresh(string cmd) {
  string msg = "REFRESH";
  send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);
  // receive all response
  char tmp[65535];
  vector<string> recvmsg, recvdata;
  int recvRet = recv(sockfd, tmp, sizeof tmp, 0);
  msg = tmp;
  if (recvRet < 0) {
    cmdError(cmd);
    return;
  }
  hostDatas.clear();
  recvmsg = splitArgs(msg, '\n');
  if (recvmsg.size() < 1 || recvmsg[0] != "REFRESHSUCCESS") {
    cmdError(cmd);
    return;
  }
  for (int i = 1; i < recvmsg.size() - 1; i++) {
    recvdata = splitArgs(recvmsg[i], ' ');
    if (recvdata.size() < 1 || recvdata[0] == "REFRESHEND") {
      break;
    }
    hostDatas.push_back(
        *newHostData(-1, recvdata[0], recvdata[1], recvdata[2]));
  }
  // cmdList(cmd);
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdLogout(string cmd) {
  string msg = "LOGOUT " + myIP + " " + myPORT;
  send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdExit(string cmd) {
  string msg = "EXIT";
  send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdSend(string cmd, string ip, string orinmsg) {
  if (checkIfIP(ip) < 0 || getHostData(ip) == NULL) {
    cmdError(cmd);
    return;
  }
  string msg = orinmsg;
  send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);
  // receive all response
  char tmp[65535];
  vector<string> recvmsg, recvdata;
  int recvRet = recv(sockfd, tmp, sizeof tmp, 0);
  msg = tmp;
  if (recvRet < 0) {
    cmdError(cmd);
    return;
  }
  recvmsg = splitArgs(msg, '\n');
  if (recvmsg[0] != "SENDSUCCESS" || recvmsg.size() < 1) {
    cmdError(cmd);
    return;
  }
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdBroadcast(string cmd, string orinmsg) {
  string msg = orinmsg;
  send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);
  // receive all response
  char tmp[65535];
  vector<string> recvmsg, recvdata;
  int recvRet = recv(sockfd, tmp, sizeof tmp, 0);
  msg = tmp;
  if (recvRet < 0) {
    cmdError(cmd);
    return;
  }
  recvmsg = splitArgs(msg, '\n');
  if (recvmsg[0] != "BROADCASTSUCCESS" || recvmsg.size() < 1) {
    cmdError(cmd);
    return;
  }
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdBlock(string cmd, string ip) {
  if (checkIfIP(ip) < 0 || getHostData(ip) == NULL) {
    cmdError(cmd);
    return;
  }
  string msg = cmd + " " + ip;
  send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);
  // receive all response
  char tmp[65535];
  vector<string> recvmsg, recvdata;
  int recvRet = recv(sockfd, tmp, sizeof tmp, 0);
  msg = tmp;
  if (recvRet < 0) {
    cmdError(cmd);
    return;
  }
  recvmsg = splitArgs(msg, '\n');
  if (recvmsg[0] != "BLOCKSUCCESS" || recvmsg.size() < 1) {
    cmdError(cmd);
    return;
  }
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdUnblock(string cmd, string ip) {
  if (checkIfIP(ip) < 0 || getHostData(ip) == NULL) {
    cmdError(cmd);
    return;
  }
  string msg = cmd + " " + ip;
  send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);
  // receive all response
  char tmp[65535];
  vector<string> recvmsg, recvdata;
  int recvRet = recv(sockfd, tmp, sizeof tmp, 0);
  msg = tmp;
  if (recvRet < 0) {
    cmdError(cmd);
    return;
  }
  recvmsg = splitArgs(msg, '\n');
  if (recvmsg[0] != "UNBLOCKSUCCESS" || recvmsg.size() < 1) {
    // printf("##### %s\n", msg.c_str());
    cmdError(cmd);
    return;
  }
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

// server only

void cmdStat(string cmd) {
  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  for (unsigned int i = 0; i < hostDatas.size(); ++i) {
    cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i + 1,
                          hostDatas[i].hostname.c_str(),
                          hostDatas[i].num_msg_sent, hostDatas[i].num_msg_rcv,
                          hostDatas[i].status.c_str());
  }
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void cmdBlockedList(string cmd, string ip) {
  if (checkIfIP(ip) < 0 || getHostData(ip) == NULL) {
    cmdError(cmd);
    return;
  }
  HostData* goalhd = getHostData(ip);

  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
  for (int i = 0; i < goalhd->blockeduser.size(); ++i) {
    HostData* hd = getHostData(goalhd->blockeduser[i]);
    cse4589_print_and_log("%-5d%-35s%-20s%-8s\n", i + 1, hd->hostname.c_str(),
                          hd->ip.c_str(), hd->port.c_str());
  }
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
