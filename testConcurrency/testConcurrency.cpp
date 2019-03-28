#include <netdb.h>
#include <stdlib.h> /* srand, rand */
#include <sys/socket.h>
#include <time.h> /* time */
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

#define SERVERNAME "vcm-7992.vm.duke.edu"
#define PORT "12345"

void sendRequest();

string account(const char *id, int balance);

string symbol(const char *name, const char *id, int num);

string create();

string transactions();

string order(const char *name, int amount, int limit);

string query(int trans_id);

string cancel(int trans_id);

const char *account_ids[5] = {"001", "002", "003", "004", "005"};
const char *items[5] = {"Durex", "Jasbon", "Six", "God", "Guilty"};

int main(int argc, char **argv) {
  if (argc == 1)
    return 0;

  cout << "Begin\n";

  string times_str = argv[1];
  int times = stoi(times_str);

  cout << times << " threads.\n";

  std::vector<std::thread> threads;

  for (int i = 0; i < times; i++) {
    threads.push_back(std::thread(sendRequest));
  }

  try {
    for (int i = 0; i < times; i++) {
      threads[i].detach();
    }
  } catch (std::exception &e) {
    // General catch exception to return 404, rarely gets here though
    // Because we have other try & catch in handlehttp
    std::cout << e.what() << std::endl;
  }

  //  for (int i = 0; i < times; i++) {
  //  threads[i].join();
  // }

  while (1) {
    // for detach
  }

  cout << "\nEnd\n";

  return 0;
}

void sendRequest() {
  // Generate request body
  string request;

  /* initialize random seed: */
  srand(time(NULL));

  /* generate secret number between 1 and 10: */
  int iSecret = rand() % 10 + 1;

  if (iSecret <= 2) { // create
    request = create();
  } else { // transaction
    request = transactions();
  }

  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = SERVERNAME;
  const char *port = PORT;

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return;
  } // if

  socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return;
  } // if

  cout << "Connecting to " << hostname << " on port " << port << "..." << endl;

  status =
      connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return;
  } // if

  send(socket_fd, request.c_str(), request.length(), 0);

  char buffer[51240];
  memset(&buffer, '\0', sizeof(buffer));
  recv(socket_fd, buffer, 51240, 0);

  cout << "Response: \n" << buffer << endl;

  freeaddrinfo(host_info_list);
  close(socket_fd);
}

string create() {
  stringstream ss;
  ss << "<create>\n";

  for (int i = 0; i < 5; i++) {
    /* initialize random seed: */
    srand(time(NULL));

    /* generate secret number between 1 and 10: */
    int option = rand() % 10 + 1;
    int balance = rand() % 10000000;

    if (option <= 5) { // create
      int id = rand() % 5;
      ss << account(account_ids[id], balance);
    } else { // transaction
      int name = rand() % 5;
      int id = rand() % 5 * 6 % 5;
      int num = rand() % 200;
      ss << symbol(items[name], account_ids[id], num);
    }
  }

  ss << "</create>\n";
  return ss.str();
}

string transactions() {
  stringstream ss;
  ss << "<transactions id=\"";

  /* initialize random seed: */
  srand(time(NULL));
  int act_id = rand() % 5; // id
  ss << act_id << "\">\n";

  for (int i = 0; i < 10; i++) {
    /* initialize random seed: */
    srand(time(NULL));

    /* generate secret number between 1 and 12: */
    int option = rand() % 12 + 1;

    if (option <= 7) { // order
      srand(time(NULL));
      int sym = rand() % 5;
      int amount = rand() % 33;
      if (rand() % 10 % 2 == 0)
        amount = 0 - amount;
      int limit = rand() % 100;
      ss << order(items[sym], amount, limit);
    } else if (option >= 8 && option <= 10) { // query
      srand(time(NULL));
      int trans_id = rand() % 500 + 1;
      ss << query(trans_id);
    } else { // cancel
      srand(time(NULL));
      int trans_id = rand() % 500 + 1;
      ss << cancel(trans_id);
    }
  }

  ss << "</transactions>\n";
  return ss.str();
}

string account(const char *id, int balance) {
  stringstream ss;
  ss << "  <account id=\"";
  ss << id << "\" balance=\"";
  ss << balance << "\"/>\n";
  return ss.str();
}

string symbol(const char *name, const char *id, int num) {
  stringstream ss;
  ss << "  <symbol sym=\"";
  ss << name << "\">\n";
  ss << "    <account id=\"";
  ss << id << "\">";
  ss << num << "</account>\n";
  ss << "  </symbol>\n";
  return ss.str();
}

string order(const char *name, int amount, int limit) {
  stringstream ss;
  ss << "  <order sym=\"";
  ss << name << "\" amount=\"";
  ss << amount << "\" limit=\"";
  ss << limit << "\"/>\n";
  return ss.str();
}

string query(int trans_id) {
  stringstream ss;
  ss << "  <query id=\"";
  ss << trans_id << "\"/>\n";
  return ss.str();
}

string cancel(int trans_id) {
  stringstream ss;
  ss << "  <cancel id=\"";
  ss << trans_id << "\"/>\n";
  return ss.str();
}
