# QLogExplorer

Advanced tool for exploring log files.

![image](screenshots/main.png?raw=true)

## Main features

### Very fast and can handle huge files

The file is not loaded into memory, but indexed by chunks.
It allows a very fast loading, as well as low memory consumption, even when a large file is opened.  
`QLogExplorer` also allows to browser the file and to start searching even when the file is still being indexed.

### No locks whatsoever

A monitoring applications shall never interfere with the process that is generating/managing the logs, therefore the process must be able to delete, move or compress the log files without getting any denied operation due the monitoring application is reading it.  
`QLogExplorer` never interferes in the log management process.

### Supports JSON log files

Nowadays it's very common log files to be in JSON format as follow:
```json
{"LogLevel":"INFO","DateTime":"28-12-2021 18:03:54.00274","LogMessage":"System initialized"}
{"LogLevel":"WARNING","DateTime":"28-12-2021 18:03:54.00301","LogMessage":"Not in UTC"}
{"LogLevel":"ERROR","DateTime":"28-12-2021 18:03:56.00885","LogMessage":"Exception caught"}
```
`QLogExplorer` completely supports that kind of log format.

[See wiki for further information](https://github.com/rafaelfassi/qlogexplorer/wiki/JSON-Files-Support)

### Templates

Different systems may have different kind of log info, so a template can be created for each kind of log, having:

* Columns definition.
* Highlighters definition.
* Predefined search parameters.

[See wiki for further information](https://github.com/rafaelfassi/qlogexplorer/wiki/Templates)

### Advanced search

The search allows multi-parameters where the parameters can be combined by `AND` or `OR` operators.  
Each parameter can:

* Have the search expression as `SubString`, `Regex` or `Range`.
* Be limited to a specific column.
* Use the negation operator.

[See wiki for further information](https://github.com/rafaelfassi/qlogexplorer/wiki/Searching)

## Build

__Minimum requirements__

* C++ 17
* CMake 3.10
* Qt 5.12

If not using Qt creator, make sure the Qt dir is in the PATH.  
To build the project via command line:
```
mkdir build
cd build
cmake ../
make
```
