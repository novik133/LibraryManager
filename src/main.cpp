/*
 * Library Manager
 * Author: Dawid Papaj
 * A Windows library management application with SQLite database
 */

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "database.h"
#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Global variables
HINSTANCE g_hInst;
Database g_db;
HWND g_hMainWnd;
HWND g_hListView;
HWND g_hStatusBar;
std::vector<Book> g_currentBooks;
int g_selectedBookId = -1;

// Control IDs
#define IDC_LISTVIEW        1001
#define IDC_BTN_ADD         1002
#define IDC_BTN_EDIT        1003
#define IDC_BTN_DELETE      1004
#define IDC_BTN_SEARCH      1005
#define IDC_BTN_REFRESH     1006
#define IDC_STATUSBAR       1007

// Dialog control IDs
#define IDC_EDIT_AUTHOR     2001
#define IDC_EDIT_TITLE      2002
#define IDC_EDIT_YEAR       2003
#define IDC_EDIT_PAGES      2004
#define IDC_EDIT_PUBLISHER  2005
#define IDC_BTN_PHOTO       2006
#define IDC_STATIC_PHOTO    2007
#define IDC_EDIT_YEAR_FROM  2008
#define IDC_EDIT_YEAR_TO    2009

// Function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK BookDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SearchDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
void CreateMainWindow(HWND hWnd);
void CreateListView(HWND hWnd);
void RefreshBookList();
void UpdateStatusBar();
std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);

// Book dialog data
Book g_dialogBook;
bool g_isEditMode = false;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInst = hInstance;
    
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Register window class
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDR_MAINMENU);
    wcex.lpszClassName = L"LibraryManagerClass";
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    
    if (!RegisterClassExW(&wcex)) {
        MessageBoxW(nullptr, L"Failed to register window class!", L"Error", MB_ICONERROR);
        return 1;
    }
    
    // Create main window
    g_hMainWnd = CreateWindowExW(0, L"LibraryManagerClass", L"Library Manager - by Dawid Papaj",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700,
        nullptr, nullptr, hInstance, nullptr);
    
    if (!g_hMainWnd) {
        MessageBoxW(nullptr, L"Failed to create window!", L"Error", MB_ICONERROR);
        return 1;
    }
    
    // Open database
    char dbPath[MAX_PATH];
    GetModuleFileNameA(nullptr, dbPath, MAX_PATH);
    std::string path(dbPath);
    path = path.substr(0, path.find_last_of("\\/")) + "\\library.db";
    
    if (!g_db.open(path)) {
        MessageBoxW(g_hMainWnd, L"Failed to open database!", L"Error", MB_ICONERROR);
    }
    
    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    g_db.close();
    return (int)msg.wParam;
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    return wstr;
}

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size, nullptr, nullptr);
    return str;
}

void CreateListView(HWND hWnd) {
    RECT rc;
    GetClientRect(hWnd, &rc);
    
    g_hListView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        10, 50, rc.right - 20, rc.bottom - 100, hWnd, (HMENU)IDC_LISTVIEW, g_hInst, nullptr);
    
    ListView_SetExtendedListViewStyle(g_hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    
    // Add columns
    LVCOLUMNW lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    
    const wchar_t* columns[] = {L"ID", L"Author", L"Title", L"Year", L"Pages", L"Publisher"};
    int widths[] = {50, 180, 250, 70, 70, 180};
    
    for (int i = 0; i < 6; i++) {
        lvc.iSubItem = i;
        lvc.pszText = const_cast<LPWSTR>(columns[i]);
        lvc.cx = widths[i];
        ListView_InsertColumn(g_hListView, i, &lvc);
    }
}

void CreateMainWindow(HWND hWnd) {
    // Create toolbar buttons
    int x = 10;
    CreateWindowW(L"BUTTON", L"Add Book", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, 10, 100, 30, hWnd, (HMENU)IDC_BTN_ADD, g_hInst, nullptr);
    x += 110;
    CreateWindowW(L"BUTTON", L"Edit Book", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, 10, 100, 30, hWnd, (HMENU)IDC_BTN_EDIT, g_hInst, nullptr);
    x += 110;
    CreateWindowW(L"BUTTON", L"Delete Book", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, 10, 100, 30, hWnd, (HMENU)IDC_BTN_DELETE, g_hInst, nullptr);
    x += 110;
    CreateWindowW(L"BUTTON", L"Search", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, 10, 100, 30, hWnd, (HMENU)IDC_BTN_SEARCH, g_hInst, nullptr);
    x += 110;
    CreateWindowW(L"BUTTON", L"Refresh", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, 10, 100, 30, hWnd, (HMENU)IDC_BTN_REFRESH, g_hInst, nullptr);
    
    CreateListView(hWnd);
    
    // Create status bar
    g_hStatusBar = CreateWindowExW(0, STATUSCLASSNAMEW, nullptr,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hWnd, (HMENU)IDC_STATUSBAR, g_hInst, nullptr);
    
    RefreshBookList();
}


void RefreshBookList() {
    ListView_DeleteAllItems(g_hListView);
    g_currentBooks = g_db.getAllBooks();
    
    LVITEMW lvi = {0};
    lvi.mask = LVIF_TEXT;
    
    for (size_t i = 0; i < g_currentBooks.size(); i++) {
        const Book& book = g_currentBooks[i];
        
        lvi.iItem = static_cast<int>(i);
        lvi.iSubItem = 0;
        std::wstring id = std::to_wstring(book.id);
        lvi.pszText = const_cast<LPWSTR>(id.c_str());
        ListView_InsertItem(g_hListView, &lvi);
        
        std::wstring author = StringToWString(book.author);
        ListView_SetItemText(g_hListView, i, 1, const_cast<LPWSTR>(author.c_str()));
        
        std::wstring title = StringToWString(book.title);
        ListView_SetItemText(g_hListView, i, 2, const_cast<LPWSTR>(title.c_str()));
        
        std::wstring year = std::to_wstring(book.year);
        ListView_SetItemText(g_hListView, i, 3, const_cast<LPWSTR>(year.c_str()));
        
        std::wstring pages = std::to_wstring(book.pages);
        ListView_SetItemText(g_hListView, i, 4, const_cast<LPWSTR>(pages.c_str()));
        
        std::wstring publisher = StringToWString(book.publisher);
        ListView_SetItemText(g_hListView, i, 5, const_cast<LPWSTR>(publisher.c_str()));
    }
    
    UpdateStatusBar();
}

void UpdateStatusBar() {
    std::wstring status = L"Total books: " + std::to_wstring(g_currentBooks.size());
    SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)status.c_str());
}

void DisplaySearchResults(const std::vector<Book>& books) {
    ListView_DeleteAllItems(g_hListView);
    g_currentBooks = books;
    
    LVITEMW lvi = {0};
    lvi.mask = LVIF_TEXT;
    
    for (size_t i = 0; i < g_currentBooks.size(); i++) {
        const Book& book = g_currentBooks[i];
        
        lvi.iItem = static_cast<int>(i);
        lvi.iSubItem = 0;
        std::wstring id = std::to_wstring(book.id);
        lvi.pszText = const_cast<LPWSTR>(id.c_str());
        ListView_InsertItem(g_hListView, &lvi);
        
        std::wstring author = StringToWString(book.author);
        ListView_SetItemText(g_hListView, i, 1, const_cast<LPWSTR>(author.c_str()));
        
        std::wstring title = StringToWString(book.title);
        ListView_SetItemText(g_hListView, i, 2, const_cast<LPWSTR>(title.c_str()));
        
        std::wstring year = std::to_wstring(book.year);
        ListView_SetItemText(g_hListView, i, 3, const_cast<LPWSTR>(year.c_str()));
        
        std::wstring pages = std::to_wstring(book.pages);
        ListView_SetItemText(g_hListView, i, 4, const_cast<LPWSTR>(pages.c_str()));
        
        std::wstring publisher = StringToWString(book.publisher);
        ListView_SetItemText(g_hListView, i, 5, const_cast<LPWSTR>(publisher.c_str()));
    }
    
    std::wstring status = L"Search results: " + std::to_wstring(g_currentBooks.size()) + L" books found";
    SendMessageW(g_hStatusBar, SB_SETTEXTW, 0, (LPARAM)status.c_str());
}

int GetSelectedBookId() {
    int sel = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
    if (sel >= 0 && sel < static_cast<int>(g_currentBooks.size())) {
        return g_currentBooks[sel].id;
    }
    return -1;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateMainWindow(hWnd);
        break;
        
    case WM_SIZE: {
        RECT rc;
        GetClientRect(hWnd, &rc);
        MoveWindow(g_hListView, 10, 50, rc.right - 20, rc.bottom - 100, TRUE);
        SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
        break;
    }
    
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_ADD:
        case ID_FILE_ADDBOOK:
            g_isEditMode = false;
            g_dialogBook = Book();
            if (DialogBoxW(g_hInst, MAKEINTRESOURCEW(IDD_BOOKDIALOG), hWnd, BookDlgProc) == IDOK) {
                if (g_db.addBook(g_dialogBook)) {
                    RefreshBookList();
                } else {
                    MessageBoxW(hWnd, L"Failed to add book!", L"Error", MB_ICONERROR);
                }
            }
            break;
            
        case IDC_BTN_EDIT:
        case ID_FILE_EDITBOOK: {
            int id = GetSelectedBookId();
            if (id > 0) {
                g_isEditMode = true;
                g_dialogBook = g_db.getBook(id);
                if (DialogBoxW(g_hInst, MAKEINTRESOURCEW(IDD_BOOKDIALOG), hWnd, BookDlgProc) == IDOK) {
                    if (g_db.updateBook(g_dialogBook)) {
                        RefreshBookList();
                    } else {
                        MessageBoxW(hWnd, L"Failed to update book!", L"Error", MB_ICONERROR);
                    }
                }
            } else {
                MessageBoxW(hWnd, L"Please select a book to edit.", L"Information", MB_ICONINFORMATION);
            }
            break;
        }
        
        case IDC_BTN_DELETE:
        case ID_FILE_DELETEBOOK: {
            int id = GetSelectedBookId();
            if (id > 0) {
                if (MessageBoxW(hWnd, L"Are you sure you want to delete this book?", 
                    L"Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    if (g_db.deleteBook(id)) {
                        RefreshBookList();
                    } else {
                        MessageBoxW(hWnd, L"Failed to delete book!", L"Error", MB_ICONERROR);
                    }
                }
            } else {
                MessageBoxW(hWnd, L"Please select a book to delete.", L"Information", MB_ICONINFORMATION);
            }
            break;
        }
        
        case IDC_BTN_SEARCH:
        case ID_FILE_SEARCH:
            DialogBoxW(g_hInst, MAKEINTRESOURCEW(IDD_SEARCHDIALOG), hWnd, SearchDlgProc);
            break;
            
        case IDC_BTN_REFRESH:
        case ID_FILE_REFRESH:
            RefreshBookList();
            break;
            
        case ID_FILE_EXIT:
            DestroyWindow(hWnd);
            break;
            
        case ID_HELP_ABOUT:
            DialogBoxW(g_hInst, MAKEINTRESOURCEW(IDD_ABOUTDIALOG), hWnd, AboutDlgProc);
            break;
        }
        break;
        
    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->idFrom == IDC_LISTVIEW && pnmh->code == NM_DBLCLK) {
            SendMessage(hWnd, WM_COMMAND, IDC_BTN_EDIT, 0);
        }
        break;
    }
    
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


std::vector<unsigned char> LoadImageFile(const std::wstring& path) {
    std::vector<unsigned char> data;
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        data.resize(static_cast<size_t>(size));
        file.read(reinterpret_cast<char*>(data.data()), size);
    }
    return data;
}

INT_PTR CALLBACK BookDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        if (g_isEditMode) {
            SetWindowTextW(hDlg, L"Edit Book");
            SetDlgItemTextW(hDlg, IDC_EDIT_AUTHOR, StringToWString(g_dialogBook.author).c_str());
            SetDlgItemTextW(hDlg, IDC_EDIT_TITLE, StringToWString(g_dialogBook.title).c_str());
            SetDlgItemInt(hDlg, IDC_EDIT_YEAR, g_dialogBook.year, FALSE);
            SetDlgItemInt(hDlg, IDC_EDIT_PAGES, g_dialogBook.pages, FALSE);
            SetDlgItemTextW(hDlg, IDC_EDIT_PUBLISHER, StringToWString(g_dialogBook.publisher).c_str());
            if (!g_dialogBook.photo.empty()) {
                SetDlgItemTextW(hDlg, IDC_STATIC_PHOTO, L"[Photo loaded]");
            }
        } else {
            SetWindowTextW(hDlg, L"Add New Book");
        }
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_PHOTO: {
            OPENFILENAMEW ofn = {0};
            wchar_t szFile[MAX_PATH] = {0};
            
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hDlg;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"Image Files\0*.jpg;*.jpeg;*.png;*.bmp;*.gif\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            
            if (GetOpenFileNameW(&ofn)) {
                g_dialogBook.photo = LoadImageFile(szFile);
                g_dialogBook.photoPath = WStringToString(szFile);
                SetDlgItemTextW(hDlg, IDC_STATIC_PHOTO, szFile);
            }
            break;
        }
        
        case IDOK: {
            wchar_t buffer[512];
            
            GetDlgItemTextW(hDlg, IDC_EDIT_AUTHOR, buffer, 512);
            g_dialogBook.author = WStringToString(buffer);
            
            GetDlgItemTextW(hDlg, IDC_EDIT_TITLE, buffer, 512);
            g_dialogBook.title = WStringToString(buffer);
            
            g_dialogBook.year = GetDlgItemInt(hDlg, IDC_EDIT_YEAR, nullptr, FALSE);
            g_dialogBook.pages = GetDlgItemInt(hDlg, IDC_EDIT_PAGES, nullptr, FALSE);
            
            GetDlgItemTextW(hDlg, IDC_EDIT_PUBLISHER, buffer, 512);
            g_dialogBook.publisher = WStringToString(buffer);
            
            // Validation
            if (g_dialogBook.author.empty() || g_dialogBook.title.empty()) {
                MessageBoxW(hDlg, L"Author and Title are required fields!", L"Validation Error", MB_ICONWARNING);
                return TRUE;
            }
            
            EndDialog(hDlg, IDOK);
            break;
        }
        
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK SearchDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            wchar_t buffer[512];
            std::string author, title, publisher;
            int yearFrom = 0, yearTo = 0;
            
            GetDlgItemTextW(hDlg, IDC_EDIT_AUTHOR, buffer, 512);
            author = WStringToString(buffer);
            
            GetDlgItemTextW(hDlg, IDC_EDIT_TITLE, buffer, 512);
            title = WStringToString(buffer);
            
            GetDlgItemTextW(hDlg, IDC_EDIT_PUBLISHER, buffer, 512);
            publisher = WStringToString(buffer);
            
            yearFrom = GetDlgItemInt(hDlg, IDC_EDIT_YEAR_FROM, nullptr, FALSE);
            yearTo = GetDlgItemInt(hDlg, IDC_EDIT_YEAR_TO, nullptr, FALSE);
            
            std::vector<Book> results = g_db.searchAdvanced(author, title, yearFrom, yearTo, publisher);
            DisplaySearchResults(results);
            
            EndDialog(hDlg, IDOK);
            break;
        }
        
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        return TRUE;
        
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    }
    return FALSE;
}
