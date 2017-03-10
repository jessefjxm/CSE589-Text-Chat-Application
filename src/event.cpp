#include "../include/hwang58.h"

// server

void eventLogin(vector<string> cmd, int cmdArgc, int cfd) {
  HostData* hd = getHostData(cfd);
  if (hd == NULL) {  // new client login
    // printf("%s\n", "################## new client login");
    hd = newHostData(cfd, cmd[1], cmd[2], cmd[3]);
    hostDatas.push_back(*hd);
    std::sort(hostDatas.begin(), hostDatas.end());
  } else {  // already registered
    hd->status = "logged-in";
  }
  string msg = "LOGINSUCCESS\n";
  for (unsigned int i = 0; i < hostDatas.size(); ++i) {
    if (hostDatas[i].status == "logged-in") {
      msg += hostDatas[i].hostname + " " + hostDatas[i].ip + " " +
             hostDatas[i].port + "\n";
    }
  }
  msg += "LOGINEND\n";

  // send cached data
  for (int i = 0; i < hd->bufmsgs.size(); ++i) {
    msg += hd->bufmsgs[i] + "\n";
  }
  hd->bufmsgs.clear();
  msg += "LOGINCACHEEND\n";
  send(cfd, msg.c_str(), strlen(msg.c_str()), 0);
}

void eventRefresh(vector<string> cmd, int cmdArgc, int cfd) {
  string msg = "REFRESHSUCCESS\n";
  for (unsigned int i = 0; i < hostDatas.size(); ++i) {
    if (hostDatas[i].status == "logged-in") {
      msg += hostDatas[i].hostname + " " + hostDatas[i].ip + " " +
             hostDatas[i].port + "\n";
    }
  }
  msg += "REFRESHEND\n";
  send(cfd, msg.c_str(), strlen(msg.c_str()), 0);
}

void eventLogout(vector<string> cmd, int cmdArgc) {
  HostData* hd = getHostData(cmd[1], cmd[2]);
  hd->status = "logged-out";
}

void eventExit(vector<string> cmd, int cmdArgc, int cfd) {
  for (int i = 0; i < hostDatas.size(); ++i) {
    if (hostDatas[i].cfd == cfd) hostDatas.erase(hostDatas.begin() + i--);
    return;
  }
}

void eventSend(vector<string> cmd, int cmdArgc, int cfd, string orinmsg) {
  string event = "RELAYED";
  HostData* senderhd = getHostData(cfd);
  string senderIP = senderhd->ip;
  int goalLen = cmd[0].length() + 2 + cmd[1].length();
  string msg = "SEND " + senderIP + " " +
               orinmsg.substr(goalLen, orinmsg.length() - goalLen);

  string goalIP = cmd[1];
  HostData* goalhd = getHostData(goalIP);
  if (goalhd == NULL) {
    msg = "SENDFAIL\n";
    send(cfd, msg.c_str(), strlen(msg.c_str()), 0);
    cmdError(event);
    return;
  }
  if(checkIfIPBlocked(goalhd, senderIP) == false){
    if (goalhd->status == "logged-in") {
      send(goalhd->cfd, msg.c_str(), strlen(msg.c_str()), 0);
    } else {
      goalhd->bufmsgs.push_back(msg);
    }
    goalhd->num_msg_rcv++;
  }
  senderhd->num_msg_sent++;

  msg = "SENDSUCCESS\n";
  send(cfd, msg.c_str(), strlen(msg.c_str()), 0);

  msg = orinmsg.substr(goalLen, orinmsg.length() - goalLen);
  cse4589_print_and_log("[%s:SUCCESS]\n", event.c_str());
  cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", senderIP.c_str(),
                        goalIP.c_str(), msg.c_str());
  cse4589_print_and_log("[%s:END]\n", event.c_str());
}

void eventBroadcast(vector<string> cmd, int cmdArgc, int cfd, string orinmsg) {
  string event = "RELAYED";
  HostData* senderhd = getHostData(cfd);
  string senderIP = senderhd->ip;
  int goalLen = cmd[0].length() + 1;
  string msg = "SEND " + senderIP + " " +
               orinmsg.substr(goalLen, orinmsg.length() - goalLen);

  for (int i = 0; i < hostDatas.size(); ++i) {
    if (hostDatas[i].cfd == cfd) {
      hostDatas[i].num_msg_sent++;
      continue;
    }
    if(checkIfIPBlocked(&hostDatas[i], senderIP) == false){
      if (hostDatas[i].status == "logged-in") {
        send(hostDatas[i].cfd, msg.c_str(), strlen(msg.c_str()), 0);
      } else {
        hostDatas[i].bufmsgs.push_back(msg);
      }
      hostDatas[i].num_msg_rcv++;
    }
  }

  msg = "BROADCASTSUCCESS\n";
  send(cfd, msg.c_str(), strlen(msg.c_str()), 0);

  msg = orinmsg.substr(goalLen, orinmsg.length() - goalLen);
  cse4589_print_and_log("[%s:SUCCESS]\n", event.c_str());
  cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", senderIP.c_str(),
                        "255.255.255.255", msg.c_str());
  cse4589_print_and_log("[%s:END]\n", event.c_str());
}

void eventBlock(vector<string> cmd, int cmdArgc, int cfd) {
  string goalIP = cmd[1];
  string msg;
  HostData* goalhd = getHostData(goalIP);
  HostData* senderhd = getHostData(cfd);
  if (goalhd == NULL || checkIfIPBlocked(senderhd, goalIP) == true) {
    msg = "BLOCKFAIL\n";
    send(cfd, msg.c_str(), strlen(msg.c_str()), 0);
    return;
  }
  senderhd->blockeduser.push_back(goalIP);

  msg = "BLOCKSUCCESS\n";
  send(cfd, msg.c_str(), strlen(msg.c_str()), 0);
}

void eventUnblock(vector<string> cmd, int cmdArgc, int cfd) {
  string goalIP = cmd[1];
  string msg;
  HostData* goalhd = getHostData(goalIP);
  HostData* senderhd = getHostData(cfd);
  if (goalhd == NULL || checkIfIPBlocked(senderhd, goalIP) == false) {
    printf("UNBLOCKFAIL\n");
    msg = "UNBLOCKFAIL\n";
    send(cfd, msg.c_str(), strlen(msg.c_str()), 0);
  }
  for (int i = 0; i < senderhd->blockeduser.size(); ++i) {
    if (senderhd->blockeduser[i] == goalIP)
      senderhd->blockeduser.erase(senderhd->blockeduser.begin() + i--);
    break;
  }

  msg = "UNBLOCKSUCCESS\n";
  send(cfd, msg.c_str(), strlen(msg.c_str()), 0);
}
// client

void eventMsgRecv(vector<string> cmd, int cmdArgc, string orinmsg) {
  int goalLen = cmd[0].length() + 2 + cmd[1].length();
  string msg = orinmsg.substr(goalLen, orinmsg.length() - goalLen);
  string ip = cmd[1];

  string event = "RECEIVED";
  cse4589_print_and_log("[%s:SUCCESS]\n", event.c_str());
  cse4589_print_and_log("msg from:%s\n[msg]:%s\n", ip.c_str(), msg.c_str());
  cse4589_print_and_log("[%s:END]\n", event.c_str());
}
