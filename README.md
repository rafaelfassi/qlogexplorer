# QLogExplorer

**QLogExplorer** is an advanced and high-performance log analyzer, designed to handle massive files, while providing powerful search capabilities and customizations for log data visualization.  
[Install QLogExplorer](#installing-qlogexplorer)

![image](screenshots/main.png?raw=true)

## Main features

### ⚡ Lightning-Fast Large File Handling

Instead of loading entire files into RAM, QLogExplorer uses smart **chunk-indexing**. It also allows browsing and searching while the file is still indexing.  
Implements three-stage filtering that makes searches extremely fast by eliminating entire non-matching chunks based on the raw data.  
Uses **RE2** as the main regex engine to speed up regex searches, but can automatically fall back to full **PCRE** if the pattern is not supported by RE2.

### 🔓 Zero File Locking (Non-Intrusive)

Safely monitor active environments (**perfect for production**).  
The system can rotate, move, compress, or delete log files without triggering OS-level "access denied" errors.

### ⚙️ Columns Definition via Regular Expression

Because no two systems log data in exactly the same format, QLogExplorer allows you to write a regex with capture groups that will split any raw, unstructured log format into a clean, filterable grid.  
As an example, consider the following simple log:
```
[INFO] 2022-02-18 15:37:10.354 0xBF32 System starting...
[WARNING] 2022-02-19 15:37:13.427 0xBF32 Not in UTC timezone
[INFO] 2022-02-18 15:37:12.137 0xBF32 System initialized
```
A regex to split those log entries into columns would look like:
```regexp
\[(?<Level>\w+)\]\s+(?<Timestamp>[^\s]+)\s+(?<Thread>[^\s]+)\s+(?<Message>.*)
```
After applying the regex, QLogExplorer will present the data using the defined columns (`Level`, `Timestamp`, `Thread`, `Message`).  
Now you can apply filters to specific columns only, hide uninteresting columns or change their order.

### 📄 Native Support for JSON Logs

Modern backend architectures can output logs where each entry is a single JSON object (**Newline Delimited JSON / NDJSON**) as follows:
```json
{"LogLevel":"INFO","DateTime":"28-12-2021 18:03:54.00274","LogMessage":"System initialized"}
{"LogLevel":"WARNING","DateTime":"28-12-2021 18:03:54.00301","LogMessage":"Not in UTC"}
{"LogLevel":"ERROR","DateTime":"28-12-2021 18:03:56.00885","LogMessage":"Exception caught"}
```
This is a very common format used by **Kibana/Elasticsearch**, and other aggregating tools.  
QLogExplorer fully supports this kind of log format.  
For this format there is no need to use a regex to split the log entries into columns because the data is already well structured, but you can still hide columns or change their order.

[See wiki for further information](https://github.com/rafaelfassi/qlogexplorer/wiki/JSON-Files-Support)

### ♻️ Reusable Templates

Once a template is created for a specific project’s log format, there is no need to do it again.  
Each **Template** can store:
- **Column definitions**: Regex with capture groups (for plain-text logs), column names, column positions, hidden columns, default width, etc.
- **Highlighting rules**: Custom colors for errors, warnings, and any other defined matching pattern.
- **Predefined searches**: Predefined search parameters ready to be used.

[See wiki for further information](https://github.com/rafaelfassi/qlogexplorer/wiki/Templates)

### 🔎 Advanced Search

The search feature allows multiple parameters which can be combined using `AND` or `OR` operators.  
Each parameter can:  
* Use a `SubString`, `Regex` or `Range` as the search expression.
* Be limited to a specific column or all columns.
* Use the negation operator to filter out noise.

[See wiki for further information](https://github.com/rafaelfassi/qlogexplorer/wiki/Searching)

## Installing QLogExplorer

### Options for Linux
1. [**Flatpak**](https://flathub.org/en/apps/io.github.rafaelfassi.QLogExplorer): `flatpak install flathub io.github.rafaelfassi.QLogExplorer`
2. Check if the official repository for your Linux distribution offers QLogExplorer.
3. [Build from source code.](#build)

### Options for Windows
1. **WinGet**: `winget install qlogexplorer`
2. [**GitHub Releases**](https://github.com/rafaelfassi/qlogexplorer/releases) (Portable and Installer)
3. [Build from source code.](#build)

## Build

__Minimum requirements__

* C++ 17
* CMake 3.10
* Qt 6.4

If not using Qt Creator, make sure the Qt dir is in the PATH.  
To build the project via command line:
```sh
mkdir build
cd build
cmake ..
make
```

### Build options
The above commands are enough to build QLogExplorer, as it will automatically fetch and configure the dependencies.  
In case you need different options for the dependencies, the project currently supports the following parameters:
* `USE_INSTALLED_RE2` Use the system-installed RE2 lib instead of fetching it.
* `DISABLE_TESTS` Does not build the tests, so the gtest libs are not required.
