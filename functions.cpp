#include "functions.h"

#define DEBUG 1
#define BUFFSIZE 40960

// All for fixed string
#define ID "id"
#define BALANCE "balance"
#define SYM "sym"
#define NUM "num"
#define AMOUNT "amount"
#define LIMIT "limit"

/*
    Receive XML from client and keep connection until invalid format.
*/
void handleXML(connection * C, int client_fd) {
  // loop and dispatch
  char buffer[BUFFSIZE];

  while (1) {
    // Reset buffer
    memset(buffer, '\0', sizeof(buffer));

    // Recv
    int totalsize = recv(client_fd, buffer, BUFFSIZE, 0);
    if (totalsize < 0)
      std::cout << "Error receive buffer from client." << std::endl;

    std::string xml(buffer);

    if (DEBUG)
      std::cout << std::endl
                << "Buffer received from client: " << std::endl
                << xml << std::endl
                << std::endl;

    // Assume it's correct
    if (xml.find("<create>") != std::string::npos) {
      create(C, xml);
    }
    else if (xml.find("<transactions>") != std::string::npos) {
      transactions(C, xml);
    }
    else {
      // Close connection
      close(client_fd);
      break;
    }
  }
}

/*
    Parse create and dispatch different request.
*/
void create(connection * C, std::string xml) {
  // Parse by line, greedy
  while (1) {
    // Get line by line break's position, xml stands for the remaining content to be parsed
    size_t linebreak = xml.find('\n');
    if (linebreak == std::string::npos)
      break;

    std::string line = xml.substr(0, linebreak);

    if (line.find("account") != std::string::npos) {
      // Create account: parse one line is enough
      std::string id = getAttribute(line, ID);
      std::string balance = getAttribute(line, BALANCE);
      createAccount(C, id, balance);

      // Skip one line
      xml = xml.substr(linebreak + 1);
    }
    else if (line.find("symbol") != std::string::npos) {
      // Create symbol: need to parse the whole symbol tag
      std::string symbol = getAttribute(line, SYM);

      // Get the info inside symbol tag
      size_t start = xml.find('\n') + 1;   // skip the <symbol ...> line
      size_t end = xml.find("</symbol>");  // end before </symbol>, containing line break!
      std::string accounts = xml.substr(start, end - start);

      parseSymbol(C, accounts, symbol);

      // Skip the entire symbol tag
      xml = xml.substr(end + 10);
    }
    else {
      // Otherwise skip one line
      xml = xml.substr(linebreak + 1);
    }
  }
}

/*
    Access attribute from giving string.
    In our design, it accesses the first one.
*/
const std::string getAttribute(std::string remain, std::string attribute) {
  std::string ans;

  if (attribute == NUM) {
    // NUM is between first '>' and second '<'
    size_t pos = remain.find('>') + 1;
    while (remain[pos] != '<') {
      ans += remain[pos++];
    }
  }
  else {
    size_t pos = remain.find(attribute);

    // try to find open parenthesis
    while (remain[pos] != '\"') {
      pos++;
    }

    pos++;  // skip open parenthesis

    while (remain[pos] != '\"') {
      ans += remain[pos++];
    }
  }

  return ans;
}

/*
    Parse the entire symbol body:
    Do greedy to find all acounts and number of symbols to be added.
*/
void parseSymbol(connection * C, std::string accounts, std::string symbol) {
  // Basically access each line and call createSymbol
  while (true) {
    size_t linebreak = accounts.find('\n');
    if (linebreak == std::string::npos)
      break;  // end of parsing

    std::string id = getAttribute(accounts, ID);
    std::string amount = getAttribute(accounts, NUM);
    createSymbol(C, id, symbol, amount);

    // skip current acount
    accounts = accounts.substr(linebreak + 1);
  }
}

/*
    Parse transactions and dispatch different request.
*/
void transactions(connection * C, std::string xml) {
  // Access account ID at first and get all requests
  std::string account_id = getAttribute(xml, ID);

  size_t start = xml.find('\n') + 1;  // // skip the <transactions ...> line
  size_t end = xml.find("</transactions>");
  std::string requests = xml.substr(start, end - start);

  if (DEBUG)
    std::cout << std::endl
              << "All requests from transactions: " << std::endl
              << requests << std::endl
              << std::endl;

  // Parse line by line: greedy
  while (1) {
    // Get line by linebreak's position, xml stands for the remaining content to be parsed
    size_t linebreak = requests.find('\n');
    if (linebreak == std::string::npos)
      break;

    std::string line = xml.substr(0, linebreak);

    if (line.find("order") != std::string::npos) {
      // Access symbol, amount and limit
      std::string symbol = getAttribute(line, SYM);
      std::string amount = getAttribute(line, AMOUNT);
      std::string limit = getAttribute(line, LIMIT);

      order(C, account_id, symbol, amount, limit);
    }
    else if (line.find("query") != std::string::npos) {
      // Access trans_id
      std::string trans_id = getAttribute(line, ID);

      query(C, account_id, trans_id);
    }
    else if (line.find("cancel") != std::string::npos) {
      // Access trans_id
      std::string trans_id = getAttribute(line, ID);

      query(C, account_id, trans_id);
    }

    // Skip one line
    requests = requests.substr(linebreak + 1);
  }
}

const std::string createAccount(connection * C,
                                const std::string & account_id_str,
                                const std::string & balance_str) {
  // Check if account is all digits
  if (!isDigits(account_id_str)) {
    return getCreateAccountError(account_id_str, "Account is not all digits");
  }

  // Check if balance is double
  if (!isDouble(balance_str)) {
    return getCreateAccountError(account_id_str, "Balance is not a decimal number");
  }

  int account_id;
  double balance;

  std::stringstream ss;
  ss << account_id_str << " " << balance_str;
  ss >> account_id >> balance;

  // Check if balance is not negative
  if (balance < 0) {
    return getCreateAccountError(account_id_str, "Balance is negative");
  }

  // Check if account already exists
  if (Account::isAccountExists(C, account_id)) {
    return getCreateAccountError(account_id_str, "Account already exists");
  }

  // Account not exists, create it
  Account::addAccount(C, account_id, balance);

  std::stringstream response;
  response << "  <created id=\"" << account_id_str << "\"/>\n";
  return response.str();
}

bool isDigits(const std::string & str) {
  if (str.empty()) {
    return false;
  }

  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (!std::isdigit(*it)) {
      return false;
    }
  }

  return true;
}

bool isAlphaDigits(const std::string & str) {
  if (str.empty()) {
    return false;
  }

  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (!std::isalpha(*it) && !std::isdigit(*it)) {
      return false;
    }
  }

  return true;
}

bool isDouble(const std::string & str) {
  double result;
  std::stringstream ss(str);

  ss >> result;

  return !ss.fail() && ss.eof();
}

const std::string getCreateAccountError(const std::string & account_id_str,
                                        const std::string & msg) {
  std::stringstream response;

  response << "  <error ";
  response << "id=\"" << account_id_str << "\">";
  response << msg;
  response << "<error>\n";

  return response.str();
}

const std::string createSymbol(connection * C,
                               const std::string & account_id_str,
                               const std::string & symbol_name,
                               const std::string & num_share_str) {
  // Check if symbol is alphanumeric
  if (!isAlphaDigits(symbol_name)) {
    return getCreateSymbolError(account_id_str, symbol_name, "Symbol is not alphanumeric");
  }

  // Check if account is all digits
  if (!isDigits(account_id_str)) {
    return getCreateSymbolError(account_id_str, symbol_name, "Account is not all digits");
  }

  // Check if amount is a positive integer
  if (!isDigits(num_share_str) || num_share_str[0] == '0') {
    return getCreateSymbolError(account_id_str, symbol_name, "Amount is not a positive integer");
  }

  int account_id;
  int num_share;

  std::stringstream ss;
  ss << account_id_str << " " << num_share_str;
  ss >> account_id >> num_share;

  // Check if account already exists
  if (!Account::isAccountExists(C, account_id)) {
    return getCreateSymbolError(account_id_str, symbol_name, "Account doesn't exist");
  }

  // Account exists, update its share amount
  if (!Position::isSymbolExists(C, account_id, symbol_name)) {
    Position::addPosition(C, symbol_name, account_id, num_share);
  }
  else {
    int old_amount = Position::getSymbolAmount(C, account_id, symbol_name);
    Position::setSymbolAmount(C, account_id, symbol_name, old_amount + num_share);
  }

  std::stringstream response;
  response << "  <created ";
  response << "sym=\"" << symbol_name << "\" ";
  response << "id=\"" << account_id_str << "\"/>\n";
  return response.str();
}

const std::string getCreateSymbolError(const std::string & account_id_str,
                                       const std::string & symbol_name,
                                       const std::string & msg) {
  std::stringstream response;

  response << "  <error ";
  response << "sym=\"" << symbol_name << "\" ";
  response << "id=\"" << account_id_str << "\">";
  response << msg;
  response << "<error>\n";

  return response.str();
}

const std::string order(connection * C,
                        const std::string & account_id,
                        const std::string & symbol,
                        const std::string & amount,
                        const std::string & limit) {
  return "";
}

const std::string cancel(connection * C,
                         const std::string & account_id_str,
                         const std::string & trans_id_str) {
  // Check if account is all digits
  if (!isDigits(account_id_str)) {
    return getCreateAccountError(account_id_str, "Account is not all digits");
  }

  // Check if trans_id is all digits
  if (!isDigits(trans_id_str)) {
    return getTransIDError(trans_id_str, "Trans_id is not all digits");
  }

  int account_id;
  int trans_id;

  std::stringstream ss;
  ss << account_id_str << " " << trans_id_str;
  ss >> account_id >> trans_id;

  // Check whether transaction exists
  if (!Transaction::isTransExists(C, trans_id)) {
    return getTransIDError(trans_id_str, "Trans_id doesn't exist");
  }

  // Transaction exists, check whether if can cancel
  if (isTransCompleted(C, trans_id)) {
    return getTransIDError(trans_id_str, "Transaction cannot be canceled");
  }

  // Can cancel, do it
  Transaction::cancelTransaction(C);

  std::string canceledShares = getCanceledShares(C, trans_id);
  std::string canceledTime = getCanceledTime(C, trans_id);

  std::stringstream response;
  response << "  <canceled ";
  response << "shares=" << canceledShares << " ";
  response << "time=" << canceledTime << "/>\n";
  return response.str();
}

const std::string query(connection * C,
                        const std::string & account_id,
                        const std::string & trans_id) {
  // Check if account is all digits
  if (!isDigits(account_id)) {
    return getCreateAccountError(account_id, "Account is not all digits");
  }

  // Check if trans_id is all digits
  if (!isDigits(trans_id)) {
    return getTransIDError(trans_id, "Trans_id is not all digits");
  }
}

const std::string getTransIDError(const std::string & trans_id_str, const std::string & msg) {
  std::stringstream response;

  response << "  <error ";
  response << "trans_id=\"" << trans_id_str << "\">";
  response << msg;
  response << "<error>\n";

  return response.str();
}

const std::string getOrderError(const std::string & symbol_name, 
                                        const std::string & amount, 
                                        const std::string & limit,
                                        const std::string & msg) {
  std::stringstream response;

  response << "  <error ";
  response << "sym=\"" << symbol_name << "\" ";
  response << "amount=\"" << amount << "\" ";
  response << "limit=\"" << limit << "\">";
  response << msg;
  response << "<error>\n";

  return response.str();
}

long getEpoch() {
  std::stringstream ss;
  ss << duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
  long time;
  ss >> time;
  return time;
}
