#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <iomanip>
#include <algorithm>
#include <map>

using namespace std;

struct Account {
    char userID[31];
    char password[31];
    char username[31];
    int privilege;
    
    Account() : privilege(0) {
        memset(userID, 0, sizeof(userID));
        memset(password, 0, sizeof(password));
        memset(username, 0, sizeof(username));
    }
};

struct Book {
    char ISBN[21];
    char name[61];
    char author[61];
    char keyword[61];
    double price;
    int quantity;
    
    Book() : price(0.0), quantity(0) {
        memset(ISBN, 0, sizeof(ISBN));
        memset(name, 0, sizeof(name));
        memset(author, 0, sizeof(author));
        memset(keyword, 0, sizeof(keyword));
    }
};

struct Transaction {
    double amount;
    Transaction() : amount(0.0) {}
    Transaction(double a) : amount(a) {}
};

class AccountSystem {
private:
    vector<Account> accounts;
    vector<string> loginStack;
    map<string, int> selectedBooks;
    
public:
    AccountSystem() {
        loadAccounts();
        if (accounts.empty()) {
            Account root;
            strcpy(root.userID, "root");
            strcpy(root.password, "sjtu");
            strcpy(root.username, "root");
            root.privilege = 7;
            accounts.push_back(root);
            saveAccounts();
        }
    }
    
    void loadAccounts() {
        ifstream ifs("accounts.dat", ios::binary);
        if (!ifs) return;
        Account acc;
        while (ifs.read(reinterpret_cast<char*>(&acc), sizeof(Account))) {
            accounts.push_back(acc);
        }
        ifs.close();
    }
    
    void saveAccounts() {
        ofstream ofs("accounts.dat", ios::binary | ios::trunc);
        for (const auto& acc : accounts) {
            ofs.write(reinterpret_cast<const char*>(&acc), sizeof(Account));
        }
        ofs.close();
    }
    
    int findAccount(const string& userID) {
        for (size_t i = 0; i < accounts.size(); i++) {
            if (accounts[i].userID == userID) return i;
        }
        return -1;
    }
    
    int getCurrentPrivilege() {
        if (loginStack.empty()) return 0;
        int idx = findAccount(loginStack.back());
        return (idx >= 0) ? accounts[idx].privilege : 0;
    }
    
    bool su(const string& userID, const string& password) {
        int idx = findAccount(userID);
        if (idx < 0) return false;
        
        int currentPriv = getCurrentPrivilege();
        
        if (password.empty()) {
            if (currentPriv <= accounts[idx].privilege) return false;
        } else {
            if (accounts[idx].password != password) return false;
        }
        
        loginStack.push_back(userID);
        return true;
    }
    
    bool logout() {
        if (loginStack.empty()) return false;
        string user = loginStack.back();
        loginStack.pop_back();
        selectedBooks.erase(user);
        return true;
    }
    
    bool registerAccount(const string& userID, const string& password, const string& username) {
        if (findAccount(userID) >= 0) return false;
        
        Account acc;
        strcpy(acc.userID, userID.c_str());
        strcpy(acc.password, password.c_str());
        strcpy(acc.username, username.c_str());
        acc.privilege = 1;
        
        accounts.push_back(acc);
        saveAccounts();
        return true;
    }
    
    bool passwd(const string& userID, const string& currentPassword, const string& newPassword) {
        int idx = findAccount(userID);
        if (idx < 0) return false;
        
        int currentPriv = getCurrentPrivilege();
        if (currentPriv == 0) return false;
        
        if (currentPassword.empty()) {
            // Changing password without current password requires privilege 7
            if (currentPriv != 7) return false;
        } else {
            // If current password is provided, it must match
            if (accounts[idx].password != currentPassword) return false;
        }
        
        strcpy(accounts[idx].password, newPassword.c_str());
        saveAccounts();
        return true;
    }
    
    bool useradd(const string& userID, const string& password, int privilege, const string& username) {
        if (getCurrentPrivilege() < 3) return false;
        if (findAccount(userID) >= 0) return false;
        if (privilege >= getCurrentPrivilege()) return false;
        
        Account acc;
        strcpy(acc.userID, userID.c_str());
        strcpy(acc.password, password.c_str());
        strcpy(acc.username, username.c_str());
        acc.privilege = privilege;
        
        accounts.push_back(acc);
        saveAccounts();
        return true;
    }
    
    bool deleteAccount(const string& userID) {
        if (getCurrentPrivilege() != 7) return false;
        int idx = findAccount(userID);
        if (idx < 0) return false;
        
        for (const auto& user : loginStack) {
            if (user == userID) return false;
        }
        
        accounts.erase(accounts.begin() + idx);
        saveAccounts();
        return true;
    }
    
    void selectBook(int bookIdx) {
        if (!loginStack.empty()) {
            selectedBooks[loginStack.back()] = bookIdx;
        }
    }
    
    int getSelectedBook() {
        if (loginStack.empty()) return -1;
        auto it = selectedBooks.find(loginStack.back());
        return (it != selectedBooks.end()) ? it->second : -1;
    }
};

class BookSystem {
private:
    vector<Book> books;
    
public:
    BookSystem() {
        loadBooks();
    }
    
    void loadBooks() {
        ifstream ifs("books.dat", ios::binary);
        if (!ifs) return;
        Book book;
        while (ifs.read(reinterpret_cast<char*>(&book), sizeof(Book))) {
            books.push_back(book);
        }
        ifs.close();
    }
    
    void saveBooks() {
        ofstream ofs("books.dat", ios::binary | ios::trunc);
        for (const auto& book : books) {
            ofs.write(reinterpret_cast<const char*>(&book), sizeof(Book));
        }
        ofs.close();
    }
    
    int findBookByISBN(const string& ISBN) {
        for (size_t i = 0; i < books.size(); i++) {
            if (books[i].ISBN == ISBN) return i;
        }
        return -1;
    }
    
    vector<int> findBooksByName(const string& name) {
        vector<int> results;
        for (size_t i = 0; i < books.size(); i++) {
            if (books[i].name == name) results.push_back(i);
        }
        return results;
    }
    
    vector<int> findBooksByAuthor(const string& author) {
        vector<int> results;
        for (size_t i = 0; i < books.size(); i++) {
            if (books[i].author == author) results.push_back(i);
        }
        return results;
    }
    
    vector<int> findBooksByKeyword(const string& keyword) {
        vector<int> results;
        for (size_t i = 0; i < books.size(); i++) {
            string kw = books[i].keyword;
            size_t pos = 0;
            while (pos < kw.length()) {
                size_t next = kw.find('|', pos);
                if (next == string::npos) next = kw.length();
                string k = kw.substr(pos, next - pos);
                if (k == keyword) {
                    results.push_back(i);
                    break;
                }
                pos = next + 1;
            }
        }
        return results;
    }
    
    void printBook(int idx) {
        const Book& b = books[idx];
        cout << b.ISBN << "\t" << b.name << "\t" << b.author << "\t" 
             << b.keyword << "\t" << fixed << setprecision(2) << b.price 
             << "\t" << b.quantity << "\n";
    }
    
    bool show(const string& param) {
        vector<int> results;
        
        if (param.empty()) {
            for (size_t i = 0; i < books.size(); i++) {
                results.push_back(i);
            }
        } else if (param.substr(0, 6) == "-ISBN=") {
            string ISBN = param.substr(6);
            int idx = findBookByISBN(ISBN);
            if (idx >= 0) results.push_back(idx);
        } else if (param.substr(0, 7) == "-name=\"") {
            string name = param.substr(7, param.length() - 8);
            results = findBooksByName(name);
        } else if (param.substr(0, 9) == "-author=\"") {
            string author = param.substr(9, param.length() - 10);
            results = findBooksByAuthor(author);
        } else if (param.substr(0, 10) == "-keyword=\"") {
            string keyword = param.substr(10, param.length() - 11);
            results = findBooksByKeyword(keyword);
        }
        
        sort(results.begin(), results.end(), [this](int a, int b) {
            return string(books[a].ISBN) < string(books[b].ISBN);
        });
        
        for (int idx : results) {
            printBook(idx);
        }
        
        if (results.empty() && !param.empty()) {
            cout << "\n";
        }
        
        return true;
    }
    
    int select(const string& ISBN) {
        int idx = findBookByISBN(ISBN);
        if (idx < 0) {
            Book book;
            strcpy(book.ISBN, ISBN.c_str());
            books.push_back(book);
            saveBooks();
            return books.size() - 1;
        }
        return idx;
    }
    
    bool modify(int bookIdx, const string& ISBN, const string& name, 
                const string& author, const string& keyword, double price) {
        if (bookIdx < 0 || bookIdx >= (int)books.size()) return false;
        
        if (!ISBN.empty() && ISBN != books[bookIdx].ISBN) {
            if (findBookByISBN(ISBN) >= 0) return false;
            strcpy(books[bookIdx].ISBN, ISBN.c_str());
        }
        
        if (!name.empty()) strcpy(books[bookIdx].name, name.c_str());
        if (!author.empty()) strcpy(books[bookIdx].author, author.c_str());
        if (!keyword.empty()) strcpy(books[bookIdx].keyword, keyword.c_str());
        if (price >= 0) books[bookIdx].price = price;
        
        saveBooks();
        return true;
    }
    
    bool import(int bookIdx, int quantity, double totalCost) {
        if (bookIdx < 0 || bookIdx >= (int)books.size()) return false;
        books[bookIdx].quantity += quantity;
        saveBooks();
        return true;
    }
    
    bool buy(const string& ISBN, int quantity, double& cost) {
        int idx = findBookByISBN(ISBN);
        if (idx < 0) return false;
        if (books[idx].quantity < quantity) return false;
        
        books[idx].quantity -= quantity;
        cost = books[idx].price * quantity;
        saveBooks();
        return true;
    }
};

class FinancialSystem {
private:
    vector<Transaction> transactions;
    
public:
    FinancialSystem() {
        loadTransactions();
    }
    
    void loadTransactions() {
        ifstream ifs("transactions.dat", ios::binary);
        if (!ifs) return;
        Transaction trans;
        while (ifs.read(reinterpret_cast<char*>(&trans), sizeof(Transaction))) {
            transactions.push_back(trans);
        }
        ifs.close();
    }
    
    void saveTransactions() {
        ofstream ofs("transactions.dat", ios::binary | ios::trunc);
        for (const auto& trans : transactions) {
            ofs.write(reinterpret_cast<const char*>(&trans), sizeof(Transaction));
        }
        ofs.close();
    }
    
    void addTransaction(double amount) {
        transactions.push_back(Transaction(amount));
        saveTransactions();
    }
    
    bool showFinance(int count) {
        if (count == 0) {
            cout << "\n";
            return true;
        }
        
        if (count > 0 && count > (int)transactions.size()) return false;
        
        double income = 0.0, expenditure = 0.0;
        
        if (count < 0) {
            for (const auto& trans : transactions) {
                if (trans.amount > 0) income += trans.amount;
                else expenditure += -trans.amount;
            }
        } else {
            int start = transactions.size() - count;
            for (int i = start; i < (int)transactions.size(); i++) {
                if (transactions[i].amount > 0) income += transactions[i].amount;
                else expenditure += -transactions[i].amount;
            }
        }
        
        cout << "+ " << fixed << setprecision(2) << income 
             << " - " << expenditure << "\n";
        return true;
    }
};

class BookstoreSystem {
private:
    AccountSystem accountSys;
    BookSystem bookSys;
    FinancialSystem financialSys;
    
    vector<string> splitCommand(const string& line) {
        vector<string> tokens;
        istringstream iss(line);
        string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }
    
public:
    void run() {
        string line;
        while (getline(cin, line)) {
            size_t start = line.find_first_not_of(" \t");
            size_t end = line.find_last_not_of(" \t");
            if (start == string::npos) continue;
            line = line.substr(start, end - start + 1);
            
            if (line.empty()) continue;
            
            vector<string> tokens = splitCommand(line);
            if (tokens.empty()) continue;
            
            string cmd = tokens[0];
            
            if (cmd == "quit" || cmd == "exit") {
                break;
            } else if (cmd == "su") {
                if (tokens.size() < 2) {
                    cout << "Invalid\n";
                    continue;
                }
                string userID = tokens[1];
                string password = (tokens.size() >= 3) ? tokens[2] : "";
                if (!accountSys.su(userID, password)) {
                    cout << "Invalid\n";
                }
            } else if (cmd == "logout") {
                if (!accountSys.logout()) {
                    cout << "Invalid\n";
                }
            } else if (cmd == "register") {
                if (tokens.size() < 4) {
                    cout << "Invalid\n";
                    continue;
                }
                if (!accountSys.registerAccount(tokens[1], tokens[2], tokens[3])) {
                    cout << "Invalid\n";
                }
            } else if (cmd == "passwd") {
                if (tokens.size() < 3) {
                    cout << "Invalid\n";
                    continue;
                }
                string userID = tokens[1];
                string currentPassword = "";
                string newPassword = "";
                if (tokens.size() == 3) {
                    newPassword = tokens[2];
                } else {
                    currentPassword = tokens[2];
                    newPassword = tokens[3];
                }
                if (!accountSys.passwd(userID, currentPassword, newPassword)) {
                    cout << "Invalid\n";
                }
            } else if (cmd == "useradd") {
                if (tokens.size() < 5) {
                    cout << "Invalid\n";
                    continue;
                }
                int privilege = stoi(tokens[3]);
                if (!accountSys.useradd(tokens[1], tokens[2], privilege, tokens[4])) {
                    cout << "Invalid\n";
                }
            } else if (cmd == "delete") {
                if (tokens.size() < 2) {
                    cout << "Invalid\n";
                    continue;
                }
                if (!accountSys.deleteAccount(tokens[1])) {
                    cout << "Invalid\n";
                }
            } else if (cmd == "show") {
                if (accountSys.getCurrentPrivilege() < 1) {
                    cout << "Invalid\n";
                    continue;
                }
                if (tokens.size() >= 2 && tokens[1] == "finance") {
                    if (accountSys.getCurrentPrivilege() < 7) {
                        cout << "Invalid\n";
                        continue;
                    }
                    int count = (tokens.size() >= 3) ? stoi(tokens[2]) : -1;
                    if (!financialSys.showFinance(count)) {
                        cout << "Invalid\n";
                    }
                } else {
                    string param = (tokens.size() >= 2) ? tokens[1] : "";
                    bookSys.show(param);
                }
            } else if (cmd == "buy") {
                if (accountSys.getCurrentPrivilege() < 1) {
                    cout << "Invalid\n";
                    continue;
                }
                if (tokens.size() < 3) {
                    cout << "Invalid\n";
                    continue;
                }
                int quantity = stoi(tokens[2]);
                if (quantity <= 0) {
                    cout << "Invalid\n";
                    continue;
                }
                double cost;
                if (bookSys.buy(tokens[1], quantity, cost)) {
                    financialSys.addTransaction(cost);
                    cout << fixed << setprecision(2) << cost << "\n";
                } else {
                    cout << "Invalid\n";
                }
            } else if (cmd == "select") {
                if (accountSys.getCurrentPrivilege() < 3) {
                    cout << "Invalid\n";
                    continue;
                }
                if (tokens.size() < 2) {
                    cout << "Invalid\n";
                    continue;
                }
                int bookIdx = bookSys.select(tokens[1]);
                accountSys.selectBook(bookIdx);
            } else if (cmd == "modify") {
                if (accountSys.getCurrentPrivilege() < 3) {
                    cout << "Invalid\n";
                    continue;
                }
                int bookIdx = accountSys.getSelectedBook();
                if (bookIdx < 0) {
                    cout << "Invalid\n";
                    continue;
                }
                
                string ISBN = "", name = "", author = "", keyword = "";
                double price = -1;
                
                for (size_t i = 1; i < tokens.size(); i++) {
                    string param = tokens[i];
                    if (param.substr(0, 6) == "-ISBN=") {
                        ISBN = param.substr(6);
                    } else if (param.substr(0, 7) == "-name=\"") {
                        name = param.substr(7, param.length() - 8);
                    } else if (param.substr(0, 9) == "-author=\"") {
                        author = param.substr(9, param.length() - 10);
                    } else if (param.substr(0, 10) == "-keyword=\"") {
                        keyword = param.substr(10, param.length() - 11);
                    } else if (param.substr(0, 7) == "-price=") {
                        price = stod(param.substr(7));
                    }
                }
                
                if (!bookSys.modify(bookIdx, ISBN, name, author, keyword, price)) {
                    cout << "Invalid\n";
                }
            } else if (cmd == "import") {
                if (accountSys.getCurrentPrivilege() < 3) {
                    cout << "Invalid\n";
                    continue;
                }
                int bookIdx = accountSys.getSelectedBook();
                if (bookIdx < 0) {
                    cout << "Invalid\n";
                    continue;
                }
                if (tokens.size() < 3) {
                    cout << "Invalid\n";
                    continue;
                }
                int quantity = stoi(tokens[1]);
                double totalCost = stod(tokens[2]);
                if (quantity <= 0 || totalCost <= 0) {
                    cout << "Invalid\n";
                    continue;
                }
                if (bookSys.import(bookIdx, quantity, totalCost)) {
                    financialSys.addTransaction(-totalCost);
                } else {
                    cout << "Invalid\n";
                }
            } else if (cmd == "report") {
                if (accountSys.getCurrentPrivilege() < 7) {
                    cout << "Invalid\n";
                    continue;
                }
            } else if (cmd == "log") {
                if (accountSys.getCurrentPrivilege() < 7) {
                    cout << "Invalid\n";
                    continue;
                }
            }
        }
    }
};

int main() {
    BookstoreSystem system;
    system.run();
    return 0;
}
