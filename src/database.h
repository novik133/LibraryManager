#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <memory>
#include "sqlite3.h"

struct Book {
    int id = 0;
    std::string author;
    std::string title;
    int year = 0;
    int pages = 0;
    std::string publisher;
    std::vector<unsigned char> photo;
    std::string photoPath;
};

class Database {
public:
    Database();
    ~Database();
    
    bool open(const std::string& dbPath);
    void close();
    bool isOpen() const { return db != nullptr; }
    
    // CRUD operations
    bool addBook(const Book& book);
    bool updateBook(const Book& book);
    bool deleteBook(int id);
    Book getBook(int id);
    
    // Search operations
    std::vector<Book> getAllBooks();
    std::vector<Book> searchByAuthor(const std::string& author);
    std::vector<Book> searchByTitle(const std::string& title);
    std::vector<Book> searchByYear(int year);
    std::vector<Book> searchByYearRange(int startYear, int endYear);
    std::vector<Book> searchByPublisher(const std::string& publisher);
    std::vector<Book> searchAdvanced(const std::string& author, const std::string& title,
                                      int yearFrom, int yearTo, const std::string& publisher);
    
    std::string getLastError() const { return lastError; }

private:
    sqlite3* db = nullptr;
    std::string lastError;
    
    bool createTables();
    Book rowToBook(sqlite3_stmt* stmt);
};

#endif // DATABASE_H
