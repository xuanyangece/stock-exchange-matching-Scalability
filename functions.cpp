#include "functions.h"

#define DEBUG 0
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
void handleXML(int client_fd) {
  // Allocate & initialize a Postgres connection object
  connection * C = createConnection();
  if (C == NULL) {
    return;
  }

  // loop and dispatch
  char buffer[BUFFSIZE];

  std::string response;

  // Reset buffer
  memset(buffer, '\0', sizeof(buffer));

  // Recv
  int totalsize = recv(client_fd, buffer, BUFFSIZE, 0);
  if (totalsize < 0)
    std::cout << "Error receive buffer from client." << std::endl;

  std::string xml(buffer);

  if (DEBUG) {
    std::cout << std::endl
              << "Buffer received from client: " << std::endl
              << xml << std::endl
              << std::endl;
  }

  // Assume it's correct
  if (xml.find("<create>") != std::string::npos) {
    if (DEBUG)
      std::cout << "create request received.\n\n";
    response = create(C, xml);
  }
  else if (xml.find("<transactions") != std::string::npos) {
    if (DEBUG)
      std::cout << "transaction request received.\n\n";
    response = transactions(C, xml);
  }

  if (DEBUG)
    std::cout << "Response back: \n" << response << std::endl;

  // Response back
  send(client_fd, response.c_str(), response.length(), MSG_NOSIGNAL);

  // Close database connection
  C->disconnect();
  delete C;

  // Close connection
  close(client_fd);
}

/*                                            
    Create a new connection with the database.
*/
connection * createConnection() {
  connection * C;

  try {
    // Establish a connection to the database
    // Parameters: database name, user name, user password
    C = new connection("dbname=MARKET_XUAN_KAI user=postgres password=passw0rd");
    if (C->is_open()) {
      // cout << "Opened database successfully: " << C->dbname() << endl;
    }
    else {
      std::cout << "Can't open database" << std::endl;
      return NULL;
    }
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << std::endl;
    return NULL;
  }

  return C;
}

/*                                              
    Parse create and dispatch different request.
*/
const std::string create(connection * C, std::string xml) {
  // Response string to return
  std::stringstream ans;
  ans << "<results>\n";

  // Parse by line, greedy
  while (1) {
    // Get line by line break's position, xml stands for the remaining content to be parsed
    size_t linebreak = xml.find('\n');
    if (linebreak == std::string::npos)
      break;

    std::string line = xml.substr(0, linebreak);
    std::string singleResponse = "";

    if (line.find("account") != std::string::npos) {
      // Create account: parse one line is enough
      std::string id = getAttribute(line, ID);
      std::string balance = getAttribute(line, BALANCE);

      singleResponse = createAccount(C, id, balance);

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

      singleResponse = parseSymbol(C, accounts, symbol);

      // Skip the entire symbol tag
      xml = xml.substr(end + 10);
    }
    else {
      // Otherwise skip one line
      xml = xml.substr(linebreak + 1);
    }

    ans << singleResponse;
  }

  ans << "</results>\n";
  return ans.str();
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
const std::string parseSymbol(connection * C, std::string accounts, std::string symbol) {
  std::stringstream ans;

  // Basically access each line and call createSymbol
  while (true) {
    size_t linebreak = accounts.find('\n');
    if (linebreak == std::string::npos)
      break;  // end of parsing

    std::string id = getAttribute(accounts, ID);
    std::string amount = getAttribute(accounts, NUM);
    ans << createSymbol(C, id, symbol, amount);

    // skip current acount
    accounts = accounts.substr(linebreak + 1);
  }

  return ans.str();
}

/*                                                    
    Parse transactions and dispatch different request.
*/
const std::string transactions(connection * C, std::string xml) {
  // Response string to return
  std::stringstream ans;
  ans << "<results>\n";

  // Access account ID at first and get all requests
  std::string account_id = getAttribute(xml, ID);

  size_t start = xml.find('\n') + 1;  // // skip the <transactions ...> line
  size_t end = xml.find("</transactions>");

  std::string requests = xml.substr(start, end - start);

  if (DEBUG)
    std::cout << std::endl
              << "All requests from transactions: " << std::endl
              << requests << "END HERE" << std::endl
              << std::endl;

  // Indicate space between each transaction
  bool space = false;
  // Parse line by line: greedy
  while (1) {
    // Get line by linebreak's position, xml stands for the remaining content to be parsed
    size_t linebreak = requests.find('\n');

    if (linebreak == std::string::npos)
      break;

    if (space) {
      ans << "\n";
    }
    std::string line = requests.substr(0, linebreak);

    std::string singleResponse;
    space = true;

    if (line.find("order") != std::string::npos) {
      // Access symbol, amount and limit
      std::string symbol = getAttribute(line, SYM);
      std::string amount = getAttribute(line, AMOUNT);
      std::string limit = getAttribute(line, LIMIT);

      singleResponse = order(C, account_id, symbol, amount, limit);
      if (DEBUG)
        std::cout << "Single order response" << singleResponse << std::endl;
    }
    else if (line.find("query") != std::string::npos) {
      // Access trans_id
      std::string trans_id = getAttribute(line, ID);

      singleResponse = query(C, account_id, trans_id);
      if (DEBUG)
        std::cout << "Single query response" << singleResponse << std ::endl;
    }
    else if (line.find("cancel") != std::string::npos) {
      // Access trans_id
      std::string trans_id = getAttribute(line, ID);

      singleResponse = cancel(C, account_id, trans_id);
      if (DEBUG)
        std::cout << "Single cancel response" << singleResponse << std ::endl;
    }

    // Append reponse
    ans << singleResponse;

    // Skip one line
    requests = requests.substr(linebreak + 1);
  }

  ans << "</results>\n";
  return ans.str();
}

/*                                  
    Create account, return response.
*/
const std::string createAccount(connection * C,
                                const std::string & account_id_str,
                                const std::string & balance_str) {
  // Check if account is all digits
  if (!isDigits(account_id_str)) {
    return getCreateAccountError(account_id_str, "Account is not all digits");
  }

  // Check if balance is double
  if (!isPositiveDouble(balance_str)) {
    return getCreateAccountError(account_id_str, "Balance is not a positive decimal number");
  }

  double balance;

  std::stringstream ss;
  ss << balance_str;
  ss >> balance;

  // Check and create account if possible
  if (!Account::addAccount(C, account_id_str, balance)) {
    return getCreateAccountError(account_id_str, "Account already exists");
  }

  // Account not exists, create it
  std::stringstream response;
  response << "  <created id=\"" << account_id_str << "\"/>\n";
  return response.str();
}

/*                                           
    Check whether given string is all digits.
*/
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

/*                                                                            
    Check whether given string is alphanumeric to fit the symbol name pattern.
*/
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

/*                                                          
    Check whether given string represents a positive double.
*/
bool isPositiveDouble(const std::string & str) {
  double result;
  std::stringstream ss(str);

  ss >> result;

  return !ss.fail() && ss.eof() && result > 0;
}

/*                                                           
    Check whether given string represents a non-zero integer.
*/
bool isNonZeroInt(const std::string & str) {
  int result;
  std::stringstream ss(str);

  ss >> result;

  return !ss.fail() && ss.eof() && result != 0;
}

/*                                  
    Return error related to account.
*/
const std::string getCreateAccountError(const std::string & account_id_str,
                                        const std::string & msg) {
  std::stringstream response;

  response << "  <error ";
  response << "id=\"" << account_id_str << "\">";
  response << msg;
  response << "</error>\n";

  return response.str();
}

/*                                                               
    Create a symbol associated with the account, return response.
*/
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

  int num_share;

  std::stringstream ss;
  ss << num_share_str;
  ss >> num_share;

  // Check if account already exists
  if (!Account::isAccountExists(C, account_id_str)) {
    return getCreateSymbolError(account_id_str, symbol_name, "Account doesn't exist");
  }

  // Account exists, insert new position or update existing position
  Position::addPosition(C, symbol_name, account_id_str, num_share);

  std::stringstream response;
  response << "  <created ";
  response << "sym=\"" << symbol_name << "\" ";
  response << "id=\"" << account_id_str << "\"/>\n";
  return response.str();
}

/*                                 
    Return error related to symbol.
*/
const std::string getCreateSymbolError(const std::string & account_id_str,
                                       const std::string & symbol_name,
                                       const std::string & msg) {
  std::stringstream response;

  response << "  <error ";
  response << "sym=\"" << symbol_name << "\" ";
  response << "id=\"" << account_id_str << "\">";
  response << msg;
  response << "</error>\n";

  return response.str();
}

/*                                        
    Handle order request, return response.
*/
const std::string order(connection * C,
                        const std::string & account_id_str,
                        const std::string & symbol,
                        const std::string & amount_str,
                        const std::string & limit_str) {
  // Step 1: Check format
  // account_id(digits)
  // symbol(AlphaDigits)
  // amount(integer-nonzero)
  // limit(double)

  // Check if account is all digits
  if (!isDigits(account_id_str)) {
    return getOrderError(symbol, amount_str, limit_str, "Account_id is not all digits");
  }

  // Check if symbol is alphanumeric
  if (!isAlphaDigits(symbol)) {
    return getOrderError(symbol, amount_str, limit_str, "Symbol is not alphanumeric");
  }

  // Check if amount is non-zero integer
  if (!isNonZeroInt(amount_str)) {
    return getOrderError(symbol, amount_str, limit_str, "Amount is not non-zero integer");
  }

  // Check if limit is positive double
  if (!isPositiveDouble(limit_str)) {
    return getOrderError(symbol, amount_str, limit_str, "Limit is not double");
  }

  // Step 2: DB validation
  // account existence
  // buy - amount * limit FROM balance
  // sell - amount FROM position

  int amount;
  double limit;

  std::stringstream ss;
  ss << amount_str << " " << limit_str;
  ss >> amount >> limit;

  // Check if account exists
  if (!Account::isAccountExists(C, account_id_str)) {
    return getOrderError(symbol, amount_str, limit_str, "Account doesn't exist");
  }

  // Buy: check if account has enough amount * limit balance
  if (amount > 0) {
    double requiredBalance = amount * limit;

    if (!Account::reduceBalance(C, account_id_str, requiredBalance)) {
      return getOrderError(
          symbol, amount_str, limit_str, "Account doesn't have enough balance to buy");
    }
  }
  // Sell: check if account has enough position as amount
  else {  // amount < 0
    if (!Position::reduceSymbolAmount(C, account_id_str, symbol, 0 - amount)) {
      return getOrderError(
          symbol, amount_str, limit_str, "Account doesn't have enough symbol to sell");
    }
  }

  // Step 3: Create transaction
  int trans_id = Transaction::addTransaction(C, account_id_str, symbol, limit, amount);

  // Step 4: Match one possible at a time
  while (Transaction::tryMatch(C, trans_id)) {
  }

  // Response will will not affected by match result
  std::stringstream response;
  response << "  <opened sym=\"" << symbol << "\" ";
  response << "amount=\"" << amount_str << "\" ";
  response << "limit=\"" << limit_str << "\" ";
  response << "id=\"" << trans_id << "\"/>\n";

  return response.str();
}

/*                                         
    Handle cancel request, return response.
*/
const std::string cancel(connection * C,
                         const std::string & account_id_str,
                         const std::string & trans_id_str) {
  const std::string header = "  <canceled id=\"" + trans_id_str + "\">\n";
  const std::string tailer = "  </canceled>\n";

  // Check if account is all digits
  if (!isDigits(account_id_str)) {
    return getCreateAccountError(account_id_str, "Account is not all digits");
  }

  // Check if trans_id is all digits
  if (!isDigits(trans_id_str)) {
    return getTransIDError(trans_id_str, "Trans_id is not all digits");
  }

  // Check if account exists
  if (!Account::isAccountExists(C, account_id_str)) {
    return getCreateAccountError(account_id_str, "Account doesn't exist");
  }

  int trans_id;

  std::stringstream ss;
  ss << trans_id_str;
  ss >> trans_id;

  // Check whether transaction exists
  if (!Transaction::isTransExists(C, trans_id)) {
    return header + getTransIDError(trans_id_str, "Transaction doesn't exist") + tailer;
  }

  // Transaction exists, cancel if not completed
  if (!Transaction::cancelTransaction(C, trans_id)) {
    return header + getTransIDError(trans_id_str, "Transaction cannot be canceled") + tailer;
  }

  int canceledShares = Transaction::getCanceledShares(C, trans_id);
  long canceledTime = Transaction::getCanceledTime(C, trans_id);
  std::string allExecuted = Transaction::queryExecuted(C, trans_id);

  std::stringstream response;
  response << header;  // First line
  response << "    <canceled ";
  response << "shares=\"" << canceledShares << "\" ";
  response << "time=\"" << canceledTime << "\"/>\n";
  response << allExecuted;
  response << tailer;  // Last line
  return response.str();
}

/*                                        
    Handle query request, return response.
*/
const std::string query(connection * C,
                        const std::string & account_id_str,
                        const std::string & trans_id_str) {
  const std::string header = "  <status id=\"" + trans_id_str + "\">\n";
  const std::string tailer = "  </status>\n";

  // Check if account is all digits
  if (!isDigits(account_id_str)) {
    return header + getCreateAccountError(account_id_str, "Account is not all digits") + tailer;
  }

  // Check if trans_id is all digits
  if (!isDigits(trans_id_str)) {
    return header + getTransIDError(trans_id_str, "Trans_id is not all digits") + tailer;
  }

  // Check if account exists
  if (!Account::isAccountExists(C, account_id_str)) {
    return getCreateAccountError(account_id_str, "Account doesn't exist");
  }

  int trans_id;

  std::stringstream ss;
  ss << trans_id_str;
  ss >> trans_id;

  // Check whether transaction exists
  if (!Transaction::isTransExists(C, trans_id)) {
    return header + getTransIDError(trans_id_str, "Transaction doesn't exist") + tailer;
  }

  // Get atomic query result
  std::string result = Transaction::doQuery(C, trans_id);

  std::stringstream response;
  response << header;  // First line
  response << result;
  response << tailer;  // Last line
  return response.str();
}

/*                                   
    Return error related to trans_id.
*/
const std::string getTransIDError(const std::string & trans_id_str, const std::string & msg) {
  std::stringstream response;

  response << "    <error ";
  response << "trans_id=\"" << trans_id_str << "\">";
  response << msg;
  response << "</error>\n";

  return response.str();
}

/*                                
    Return error related to order.
*/
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
  response << "</error>\n";

  return response.str();
}

/*                                           
    Helper function to get current timestamp.
*/
long getEpoch() {
  std::stringstream ss;
  ss << duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
  long time;
  ss >> time;
  return time;
}
