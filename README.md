# Library Manager

A Windows library management application with SQLite database integration.

**Author:** Dawid Papaj

## Features

- Add, edit, and delete books from the database
- Store book information: author, title, year, pages, publisher, and cover photo
- Advanced search with multiple criteria
- SQLite database (embedded, no external server required)
- Windows native GUI
- NSIS installer included

## Building

### Prerequisites

- Windows 10/11
- CMake 3.15+
- Visual Studio 2019 or 2022 (with "Desktop development with C++" workload)
- NSIS (for creating installer) - https://nsis.sourceforge.io/

### Build

All dependencies (including SQLite 3.47.0) are included. Just run:

```batch
build.bat
```

Or manually:
```batch
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Create Installer

```batch
cd build
cpack -G NSIS
```

## Usage

1. Run `LibraryManager.exe`
2. Use "Add Book" to add new books to the library
3. Double-click a book to edit it
4. Use "Search" for advanced filtering
5. The database (`library.db`) is created automatically in the application directory

## Keyboard Shortcuts

- `Ctrl+N` - Add new book
- `Ctrl+E` - Edit selected book
- `Del` - Delete selected book
- `Ctrl+F` - Search
- `F5` - Refresh list

## License

MIT License - See LICENSE.txt
