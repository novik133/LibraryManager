#include "database.h"
#include <sstream>

Database::Database() : db(nullptr) {}

Database::~Database() {
    close();
}

bool Database::open(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        lastError = sqlite3_errmsg(db);
        return false;
    }
    return createTables();
}

void Database::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool Database::createTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS books (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            author TEXT NOT NULL,
            title TEXT NOT NULL,
            year INTEGER,
            pages INTEGER,
            publisher TEXT,
            photo BLOB
        );
        CREATE INDEX IF NOT EXISTS idx_author ON books(author);
        CREATE INDEX IF NOT EXISTS idx_title ON books(title);
        CREATE INDEX IF NOT EXISTS idx_year ON books(year);
    )";
    
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        lastError = errMsg;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}


bool Database::addBook(const Book& book) {
    const char* sql = "INSERT INTO books (author, title, year, pages, publisher, photo) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        lastError = sqlite3_errmsg(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, book.author.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, book.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, book.year);
    sqlite3_bind_int(stmt, 4, book.pages);
    sqlite3_bind_text(stmt, 5, book.publisher.c_str(), -1, SQLITE_TRANSIENT);
    
    if (!book.photo.empty()) {
        sqlite3_bind_blob(stmt, 6, book.photo.data(), static_cast<int>(book.photo.size()), SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 6);
    }
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) lastError = sqlite3_errmsg(db);
    sqlite3_finalize(stmt);
    return success;
}

bool Database::updateBook(const Book& book) {
    const char* sql = "UPDATE books SET author=?, title=?, year=?, pages=?, publisher=?, photo=? WHERE id=?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        lastError = sqlite3_errmsg(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, book.author.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, book.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, book.year);
    sqlite3_bind_int(stmt, 4, book.pages);
    sqlite3_bind_text(stmt, 5, book.publisher.c_str(), -1, SQLITE_TRANSIENT);
    
    if (!book.photo.empty()) {
        sqlite3_bind_blob(stmt, 6, book.photo.data(), static_cast<int>(book.photo.size()), SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 6);
    }
    sqlite3_bind_int(stmt, 7, book.id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) lastError = sqlite3_errmsg(db);
    sqlite3_finalize(stmt);
    return success;
}

bool Database::deleteBook(int id) {
    const char* sql = "DELETE FROM books WHERE id=?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        lastError = sqlite3_errmsg(db);
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) lastError = sqlite3_errmsg(db);
    sqlite3_finalize(stmt);
    return success;
}

Book Database::rowToBook(sqlite3_stmt* stmt) {
    Book book;
    book.id = sqlite3_column_int(stmt, 0);
    book.author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    book.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    book.year = sqlite3_column_int(stmt, 3);
    book.pages = sqlite3_column_int(stmt, 4);
    
    const char* pub = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    if (pub) book.publisher = pub;
    
    const void* blob = sqlite3_column_blob(stmt, 6);
    int blobSize = sqlite3_column_bytes(stmt, 6);
    if (blob && blobSize > 0) {
        book.photo.assign(static_cast<const unsigned char*>(blob),
                          static_cast<const unsigned char*>(blob) + blobSize);
    }
    return book;
}

Book Database::getBook(int id) {
    Book book;
    const char* sql = "SELECT * FROM books WHERE id=?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            book = rowToBook(stmt);
        }
        sqlite3_finalize(stmt);
    }
    return book;
}

std::vector<Book> Database::getAllBooks() {
    std::vector<Book> books;
    const char* sql = "SELECT * FROM books ORDER BY title;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            books.push_back(rowToBook(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return books;
}

std::vector<Book> Database::searchByAuthor(const std::string& author) {
    std::vector<Book> books;
    const char* sql = "SELECT * FROM books WHERE author LIKE ? ORDER BY title;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        std::string pattern = "%" + author + "%";
        sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            books.push_back(rowToBook(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return books;
}

std::vector<Book> Database::searchByTitle(const std::string& title) {
    std::vector<Book> books;
    const char* sql = "SELECT * FROM books WHERE title LIKE ? ORDER BY title;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        std::string pattern = "%" + title + "%";
        sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            books.push_back(rowToBook(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return books;
}

std::vector<Book> Database::searchByYear(int year) {
    std::vector<Book> books;
    const char* sql = "SELECT * FROM books WHERE year=? ORDER BY title;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, year);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            books.push_back(rowToBook(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return books;
}

std::vector<Book> Database::searchByYearRange(int startYear, int endYear) {
    std::vector<Book> books;
    const char* sql = "SELECT * FROM books WHERE year BETWEEN ? AND ? ORDER BY year, title;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, startYear);
        sqlite3_bind_int(stmt, 2, endYear);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            books.push_back(rowToBook(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return books;
}

std::vector<Book> Database::searchByPublisher(const std::string& publisher) {
    std::vector<Book> books;
    const char* sql = "SELECT * FROM books WHERE publisher LIKE ? ORDER BY title;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        std::string pattern = "%" + publisher + "%";
        sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            books.push_back(rowToBook(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return books;
}

std::vector<Book> Database::searchAdvanced(const std::string& author, const std::string& title,
                                            int yearFrom, int yearTo, const std::string& publisher) {
    std::vector<Book> books;
    std::stringstream sql;
    sql << "SELECT * FROM books WHERE 1=1";
    
    if (!author.empty()) sql << " AND author LIKE ?";
    if (!title.empty()) sql << " AND title LIKE ?";
    if (yearFrom > 0) sql << " AND year >= ?";
    if (yearTo > 0) sql << " AND year <= ?";
    if (!publisher.empty()) sql << " AND publisher LIKE ?";
    sql << " ORDER BY title;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        int idx = 1;
        if (!author.empty()) {
            std::string p = "%" + author + "%";
            sqlite3_bind_text(stmt, idx++, p.c_str(), -1, SQLITE_TRANSIENT);
        }
        if (!title.empty()) {
            std::string p = "%" + title + "%";
            sqlite3_bind_text(stmt, idx++, p.c_str(), -1, SQLITE_TRANSIENT);
        }
        if (yearFrom > 0) sqlite3_bind_int(stmt, idx++, yearFrom);
        if (yearTo > 0) sqlite3_bind_int(stmt, idx++, yearTo);
        if (!publisher.empty()) {
            std::string p = "%" + publisher + "%";
            sqlite3_bind_text(stmt, idx++, p.c_str(), -1, SQLITE_TRANSIENT);
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            books.push_back(rowToBook(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return books;
}
