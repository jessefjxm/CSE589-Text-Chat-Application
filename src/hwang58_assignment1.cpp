/**
 * @hwang58_assignment1
 * @author  Hongyu Wang <hwang58@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include "../include/hwang58.h"

// global variables
string myHostname;
string myPORT;
string myIP;
struct addrinfo* myAddrInfo;
int sockfd;
vector<HostData> hostDatas;

// local variables
int isLogined = 0;

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char** argv) {
  /*Init. Logger*/
  cse4589_init_log(argv[2]);
  /*Clear LOGFILE*/
  fclose(fopen(LOGFILE, "w"));
  /*Start Here*/
  if (*argv[1] == 'c') {
    clientProcess(argv[2]);
  } else if (*argv[1] == 's') {
    serverProcess(argv[2]);
  } else {
    return 1;
  }
  return 0;
}

void clientProcess(char* port) {
  // init
  initClientSocket(port);
  // variables
  char buf[65535];  // msg & commands
  fd_set readfds;   // for select
  int fdmax = sockfd;
  // core loop
  while (1) {
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    FD_SET(sockfd, &readfds);
    fdmax = sockfd;
    memset(&buf[0], 0, sizeof(buf));
    select(fdmax + 1, &readfds, NULL, NULL, NULL);
    // handle commands
    if (FD_ISSET(fileno(stdin), &readfds)) {
      read(fileno(stdin), buf, sizeof buf);
      fflush(stdin);
      handleCommands(buf, 1);
    }  // handle new client connection
    else if (FD_ISSET(sockfd, &readfds)) {
      if(recv(sockfd, buf, sizeof buf, 0) == 0){
        close(sockfd);
        sockfd = 0;
      }else{
        handleClientEvents(buf);
      }
    }
  }
}

void serverProcess(char* port) {
  // init
  initServerSocket(port);
  // variables
  struct sockaddr_storage their_addr;         // socket
  socklen_t addr_size = sizeof their_addr;    // socket
  char buf[65535];                            // msg & commands buffer
  fd_set readfds;                             // for select
  int cfd;                                    // client's file descriptor
  int clientSocket[MAX_SERVER_CONN];
  int fdmax = sockfd;
  // core loop
  while (1) {
    // init fd
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    FD_SET(sockfd, &readfds);
    for (int i = 0; i < MAX_SERVER_CONN; i++) { // add child sockets to set
      int sd = clientSocket[i];
      if (sd > 0) FD_SET(sd, &readfds);
      if (sd > fdmax) fdmax = sd;
    }
    memset(&buf[0], 0, sizeof(buf));
    select(fdmax + 1, &readfds, NULL, NULL, NULL);
    // handle commands
    if (FD_ISSET(fileno(stdin), &readfds)) {
      read(fileno(stdin), buf, sizeof buf);
      fflush(stdin);
      handleCommands(buf, 0);
    }
    // handle new client connection
    else if (FD_ISSET(sockfd, &readfds)) {
      cfd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
      for (int i = 0; i < MAX_SERVER_CONN; i++) { // add new socket to array of sockets
        if (clientSocket[i] == 0) {
          clientSocket[i] = cfd;
          break;
        }
      }
      recv(cfd, buf, sizeof buf, 0);
      handleServerEvents(buf, cfd);
    }
    // handle existing client events
    else {
      for (int i = 0; i < MAX_SERVER_CONN; i++) {
        cfd = clientSocket[i];
        if (FD_ISSET(cfd, &readfds)) {
          if (recv(cfd, buf, sizeof buf, 0) == 0) { // Check if closing
            handleServerEvents("FORCEEXIT", cfd);
            close(cfd);
            clientSocket[i] = 0;
          } else {  // handle events
            handleServerEvents(buf, cfd);
          }
        }
      }
    }
  }
}

void handleCommands(const char* buf, int clientMode) {
  string msg = buf;
  msg.erase(std::remove(msg.end()-1, msg.end(), '\n'), msg.end());
  vector<string> cmd = splitArgs(msg, ' ');
  int cmdArgc = cmd.size();
  /* Server/Client SHELL Command */
  if (cmd[0] == "AUTHOR" && cmdArgc == 1) {
    cmdAuthor(cmd[0]);
  } else if (cmd[0] == "IP" && cmdArgc == 1) {
    cmdIP(cmd[0]);
  } else if (cmd[0] == "PORT" && cmdArgc == 1) {
    cmdPort(cmd[0]);
  } else if (cmd[0] == "LIST" && cmdArgc == 1 && (!clientMode || isLogined)) {
    cmdList(cmd[0]);
  }
  else {    /* Server SHELL Command */
    if(clientMode == 0){
      if (cmd[0] == "STATISTICS" && cmdArgc == 1) {
        cmdStat(cmd[0]);
      } else if (cmd[0] == "BLOCKED" && cmdArgc == 2) {
        cmdBlockedList(cmd[0], cmd[1]);
      } else {
        cmdError(cmd[0]);
      }
    }else{  /* Client SHELL Command */
      if (cmd[0] == "LOGIN" && cmdArgc == 3) {
        cmdLogin(cmd[0], cmd[1], cmd[2]);
        isLogined = 1;
      } else if (cmd[0] == "REFRESH" && cmdArgc == 1 && isLogined) {
        cmdRefresh(cmd[0]);
      } else if (cmd[0] == "SEND" && cmdArgc >= 3 && isLogined) {
    		cmdSend(cmd[0], cmd[1], msg);
      } else if (cmd[0] == "BROADCAST" && cmdArgc == 2 && isLogined) {
    		cmdBroadcast(cmd[0], msg);
      } else if (cmd[0] == "BLOCK" && cmdArgc == 2 && isLogined) {
        cmdBlock(cmd[0], cmd[1]);
      } else if (cmd[0] == "UNBLOCK" && cmdArgc == 2 && isLogined) {
        cmdUnblock(cmd[0], cmd[1]);
      } else if (cmd[0] == "LOGOUT" && cmdArgc == 1 && isLogined) {
        cmdLogout(cmd[0]);
        isLogined = 0;
      } else if (cmd[0] == "EXIT" && cmdArgc == 1) {
        cmdExit(cmd[0]);
        exit(0);
      } else {
        cmdError(cmd[0]);
      }
    }
  }
}

void handleServerEvents(const char* buf, int cfd) {
  string msg = buf;
  vector<string> cmd = splitArgs(msg, ' ');
  int cmdArgc = cmd.size();
  // switch
  if (cmd[0] == "LOGIN" && cmdArgc == 4) {
    eventLogin(cmd, cmdArgc, cfd);
  } else if (cmd[0] == "REFRESH" && cmdArgc == 1) {
    eventRefresh(cmd, cmdArgc, cfd);
  } else if (cmd[0] == "LOGOUT" && cmdArgc == 3) {
    eventLogout(cmd, cmdArgc);
  } else if (cmd[0] == "EXIT" && cmdArgc == 1) {
    eventExit(cmd, cmdArgc, cfd);
  } else if (cmd[0] == "FORCEEXIT" && cmdArgc == 1) {
    eventExit(cmd, cmdArgc, cfd);
  } else if (cmd[0] == "SEND" && cmdArgc >= 3) {
    eventSend(cmd, cmdArgc, cfd, msg);
  } else if (cmd[0] == "BROADCAST" && cmdArgc >= 2) {
    eventBroadcast(cmd, cmdArgc, cfd, msg);
  } else if (cmd[0] == "BLOCK" && cmdArgc == 2) {
    eventBlock(cmd, cmdArgc, cfd);
  } else if (cmd[0] == "UNBLOCK" && cmdArgc == 2) {
    eventUnblock(cmd, cmdArgc, cfd);
  }
}

void handleClientEvents(const char* buf){
    string msg = buf;
    // printf("%s\n", msg.c_str());
    vector<string> cmd = splitArgs(msg, ' ');
    int cmdArgc = cmd.size();
    // switch
    if (cmd[0] == "SEND" && cmdArgc >= 3) {
      eventMsgRecv(cmd, cmdArgc, msg);
    }
}
