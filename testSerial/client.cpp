#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#define BUFFER_SIZE 5120

using namespace std;

string readFile(const char * file);

int main(int argc, char * argv[]) {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  // const char *hostname = argv[1];
  const char * hostname = "vcm-7992.vm.duke.edu";
  const char * port = "12345";
  char buffer[BUFFER_SIZE];
  char response[BUFFER_SIZE];

  if (argc < 2) {
    cout << "Syntax: client <fileName>\n" << endl;
    return 1;
  }

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  socket_fd =
      socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  // cout << "Connecting to " << hostname << " on port " << port << "..." << endl;

  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  memset(&buffer, '\0', sizeof(buffer));
  std::string str = readFile(argv[1]);

  // cout << "File content:\n" << str;

  send(socket_fd, str.c_str(), str.length(), 0);

  memset(&response, '\0', sizeof(response));
  recv(socket_fd, response, sizeof(response), 0);

  // cout << "Response:\n" << response << endl;
  cout << response;

  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}

std::string readFile(const char * file) {
  std::ifstream ifs(file);
  std::string ans;
  std::string line;
  while (ifs.good()) {
    getline(ifs, line);
    ans += line;
    ans += "\n";
  }
  ifs.close();
  return ans;
}
