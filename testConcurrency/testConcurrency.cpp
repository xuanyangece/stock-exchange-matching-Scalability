#include <netdb.h>
#include <sys/socket.h>
#include <time.h> /* time */
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>
#include <random>
#include <ratio>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace std::chrono;

#define SERVERNAME "vcm-7992.vm.duke.edu"
#define PORT "12345"

void sendRequest(int curt, int all);

string account(const char *id, int balance);

string symbol(const char *name, const char *id, int num);

string create();

string transactions();

string order(const char *name, int amount, int limit);

string query(int trans_id);

string cancel(int trans_id);

const char *account_ids[5] = {"001", "002", "003", "004", "005"};
const char *items[5] = {"Durex", "Jasbon", "Six", "God", "Guilty"};

// start time && end time
high_resolution_clock::time_point starttime;
high_resolution_clock::time_point endtime;

int main(int argc, char **argv) {
  if (argc == 1)
    return 0;

  cout << "Begin\n";

  string times_str = argv[1];
  int times = stoi(times_str);

  cout << times << " threads.\n";

  std::vector<std::thread> threads;

  for (int i = 0; i < times; i++) {
    threads.push_back(std::thread(sendRequest, i, times));
  }

  // get start time
  starttime = high_resolution_clock::now();

  try {
    for (int i = 0; i < times; i++) {
      threads[i].detach();
    }
  } catch (std::exception &e) {
    // General catch exception to return 404, rarely gets here though
    // Because we have other try & catch in handlehttp
    std::cout << e.what() << std::endl;
  }

  while (1) {
    // for detach
  }

  cout << "\nEnd\n";

  return 0;
}

void sendRequest(int curt, int all) {
  // Generate request body
  string request;

  /* initialize random num: */
  std::random_device rd;  // obtain a random number from hardware
  std::mt19937 eng(rd()); // seed the generator
  std::uniform_int_distribution<> distr(1, 10); // define the range

  /* generate secret number between 1 and 10: */
  int iSecret = distr(eng);

  // <= 3
  if (iSecret <= 10) { // create
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

  string finalreq = std::to_string(request.size()) + "\n" + request;

  send(socket_fd, finalreq.c_str(), finalreq.length(), 0);

  cout << "Request: \n" << finalreq << endl;

  char buffer[51240];
  memset(&buffer, '\0', sizeof(buffer));
  recv(socket_fd, buffer, 51240, 0);

  cout << "Response: \n" << buffer << endl;

  if (curt == all - 1) {
    // get finish time
    endtime = high_resolution_clock::now();
    duration<double> time_span =
        duration_cast<duration<double>>(endtime - starttime);
    cout << "\nWith " << all << " threads sending requests concurrently, ";
    cout << "it takes " << time_span.count() << " seconds to finish.\n";
  }

  freeaddrinfo(host_info_list);
  close(socket_fd);
}

string create() {
  stringstream ss;
  ss << "<create>\n";

  for (int i = 0; i < 5; i++) {
    /* initialize random seed: */
    std::random_device rd;  // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1, 10000000); // define the range

    /* generate secret number between 1 and 10: */
    int option = distr(eng) % 10 + 1;

    int balance = distr(eng);

    // <= 5
    if (option <= 4) { // create
      // % 5
      int id = distr(eng) % 3;

      ss << account(account_ids[id], balance);
    } else { // transaction
      int name = distr(eng) % 5;
      // % 5
      int id = distr(eng) % 3;
      int num = distr(eng) % 200;

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
  std::random_device rd;  // obtain a random number from hardware
  std::mt19937 eng(rd()); // seed the generator
  std::uniform_int_distribution<> distr(1, 100000); // define the range

  ss << distr(eng) % 5 << "\">\n";

  for (int i = 0; i < 10; i++) {
    /* generate secret number between 1 and 12: */
    int option = distr(eng) % 12 + 1;

    if (option <= 7) { // order
      int sym = distr(eng) % 5;
      int amount = distr(eng) % 33;
      if (distr(eng) % 10 % 2 == 0)
        amount = 0 - amount;
      int limit = distr(eng) % 100;

      ss << order(items[sym], amount, limit);
    } else if (option >= 8 && option <= 10) { // query
      int trans_id = distr(eng) % 500 + 1;

      ss << query(trans_id);
    } else { // cancel
      int trans_id = distr(eng) % 500 + 1;

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
