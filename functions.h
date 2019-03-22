#include <iostream>

void handleXML(int reqfd); // parse

void create(int reqfd); // create account or symbol

void createAccount(int reqfd);

void createSymbol(int reqfd);

void transactions(int reqfd); // deal with transactions

void order(int reqfd);

void cancel(int reqfd);

void query(int reqfd);