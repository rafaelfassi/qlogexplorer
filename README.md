# QLogExplorer

Advanced tool for exploring log files.

<p align="center">
  <img src="screenshots/main.png?raw=true">
</p>

## Main features

* [Very fast and can handle huge files](#file-handler)
* [Does not interfere with the process that is generating/managing the logs](#no-locks)
* [Supports JSON log files](#json-support)
* [Template oriented](#templates)
* [Columns definition](#columns)
* [Highlighters definition](#highlighters)
* [Predefined search parameters](#predefined-search-parameters)
* [Advanced search](#advanced-search)

## File handler

The file is not loaded into memory, but indexed by chunks.
It allows a very fast loading, as well as low memory consumption, even when a large file is opened.  
`QLogExplorer` also allows to browser the file and to start searching even when the file is still being indexed.

## No locks

A monitoring applications shall never interfere with the process that is generating/managing the logs, therefore the process must be able to delete, move or compress the log files without getting any denied operation due the monitoring application is reading it.  
`QLogExplorer` never interferes in the log management process.

## JSON support

Nowadays it's very common log files to be in JSON format as follow:
```json
{"LogLevel":"INFO","DateTime":"28-12-2021 18:03:54.00274","LogMessage":"System initialized"}
{"LogLevel":"WARNING","DateTime":"28-12-2021 18:03:54.00301","LogMessage":"Not in UTC"}
{"LogLevel":"ERROR","DateTime":"28-12-2021 18:03:56.00885","LogMessage":"Exception caught"}
```
`QLogExplorer` completely supports that kind of log format.

## Templates

Different systems may have different kind of log info, so a template can be created for each kind of log, having:

* Columns definition.
* Highlighters definition.
* Predefined search parameters.

To open a file, it's require to specify if the file is of JSON type or TEXT type.  
When a template is saved, it will be include as another option to open the file.

As `QLogExplorer` is template oriented, even when the file is opened as JSON or TEXT, a temporary template is created to handle that file, so `Columns` can be defined, as well as `Highlighters` and `Predefined Filters`.  
Only the template name is required to save a template, so after defining a name, the save option will be enabled.

__WARNING__ If the template is not saved until the file to be closed, all the configurations applied will be lost.

## Columns

Log files used to store the entries in an organized way, and therefore columns can be defined to:

* Searching in only one specific column.
* Hide not interesting columns.
* Reorder the columns positions.

And also to define its data type to:

* Apply range filters.
* Merging files.

When the log is of JSON type, the columns recognition takes place automatically.  
For plain text files, a regular expression can be defined in the template to capture the columns.  
An an example, this regex `^([A-Z]+)\s+([0-9:.\s-]+?)\s+(0x[0-9A-F]+)\s+(.+)$` can be used to capture the columns of the following log:
```
INFO    2022-02-18 15:37:12.137 0xBF32 System initialized
WARNING 2022-02-19 15:37:13.427 0x52CB Not in UTC timezone
ERROR   2022-02-20 15:37:15.753 0xBF32 Payment gateway has returned 3456 code
```
Each captured group of the regex (starting from 1) is considered a column.  
For the example, the columns may be named as (**Level**, **Date Time**, **Thread**, **Message**) as follow:

<p align="center">
  <img src="screenshots/template-columns.png?raw=true">
</p>

As the result:

<p align="center">
  <img src="screenshots/example-columns.png?raw=true">
</p>

The regex could have provided the names of the columns as named groups: `^(?<Level>[A-Z]+)\s+(?<DateTime>[0-9:.\s-]+?)\s+(?<Thread>0x[0-9A-F]+)\s+(?<Message>.+)$`  
By using named groups the name of the columns are initially filled with it, and the maintenance of the template is more easy.

In the previous example the time format follows the ISO format, where the most significant info comes from left to right, and therefore it can be used for merging and range filtering.  
But, if the format of the column **Time** were like `18-02-2022 15:37:12.137`, it must to be defined as `Time` type having its format as `dd-MM-yyyy hh:mm:ss.z`.  
The following tables show the available date and time formats:

### Date format
|Format|Description                                                                               |
|------|------------------------------------------------------------------------------------------|
|d     |The day as a number without a leading zero (1 to 31).                                     |
|dd    |The day as a number with a leading zero (01 to 31).                                       |
|ddd   |The abbreviated localized day name (e.g. 'Mon' to 'Sun'). Uses the system locale to localize the name.|
|dddd  |The long localized day name (e.g. 'Monday' to 'Sunday'). Uses the system locale to localize the name.|
|M     |The month as a number without a leading zero (1 to 12).                                   |
|MM    |The month as a number with a leading zero (01 to 12).                                     |
|MMM   |The abbreviated localized month name (e.g. 'Jan' to 'Dec'). Uses the system locale to localize the name.|
|MMMM  |The long localized month name (e.g. 'January' to 'December'). Uses the system locale to localize the name.|
|yy    |The year as a two digit number (00 to 99).                                                |
|yyyy  |The year as a four digit number, possibly plus a leading minus sign for negative years.   |

**Note:** Day and month names must be given in the system local language. It is only possible to use the English names if the system language is English.

### Time format
|Format|Description                                                                               |
|------|------------------------------------------------------------------------------------------|
|h      |The hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)                    |
|hh     |The hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)                     |
|H      |The hour without a leading zero (0 to 23, even with AM/PM display)                       |
|HH     |The hour with a leading zero (00 to 23, even with AM/PM display)                         |
|m      |The minute without a leading zero (0 to 59)                                              |
|mm     |The minute with a leading zero (00 to 59)                                                |
|s      |The whole second, without any leading zero (0 to 59)                                     |
|ss     |The whole second, with a leading zero where applicable (00 to 59)                        |
|z      |The fractional part of the second, to go after a decimal point, without trailing zeroes (0 to 999). Thus "s.z" reports the seconds to full available (millisecond) precision without trailing zeroes.|
|zzz    |The fractional part of the second, to millisecond precision, including trailing zeroes where applicable (000 to 999).|
|AP or A|Interpret as an AM/PM time. A/AP will match an upper-case version of the system locale.  |
|ap or a|Interpret as an am/pm time. a/ap will match a lower-case version of the system locale.   |

In some cases the log entry may have line breaks as follow:
```
ERROR   2022-02-19 15:14:10.437 0xF1D2 DB Error: unique constraint violation at username:
INSERT INTO user (name, username, age)
VALUES ('John Constantine', 'john_constantine', 99)
```

In that case, the first line will match the regex, but the other two lines will not. To fix that, the column **Message** can be defined as the `no matches column`.

<p align="center">
  <img src="screenshots/template-no-match-column.png?raw=true">
</p>

Now the content of all lines that don't match the regex will be placed in the **Message** column.

<p align="center">
  <img src="screenshots/example-no-match-column.png?raw=true">
</p>

## Highlighters

Highlighters can be defined in the template to change the color of rows that contain entries that requires more attention.

<p align="center">
  <img src="screenshots/template-highlighters.png?raw=true">
</p>

## Predefined search parameters

Each kind of log have specific entries that are frequently used to the log analysis, like start, shutdown, errors, warnings, etc...  
The template can have all the interesting entries saved and properly named to speed up the log analysis task.  

<p align="center">
  <img src="screenshots/template-filters.png?raw=true">
</p>

It's a good idea to have the predefined search parameters starting with some prefix character, so by typing the prefix in the search filed, it will list all available predefined parameters. `@` was used as prefix in the previous example.

## Advanced search

The search allows multi-parameters where the parameters can be combined by `AND` or `OR` operators.  
Each parameter can:

* Have the search expression as SubString, Regex or Range.
* Be limited to a specific column.
* Use the negation operator.

<p align="center">
  <img src="screenshots/example-filters.png?raw=true">
</p>

### SubString search

Searches by the provided sub string.  
The search can be configured as case sensitive or insensitive.

### Regular expression search

Searches by the provided pattern that must be a perl-like regular expression (PCRE).  
The search can be configured as case sensitive or insensitive.

### Range search

The range expression must be provided in the format: `<From> -> <To>` as follow:
```
28-12-2021 18:00:00.0 -> 28-12-2021 19:00:00.0
```
Either `<From>` or `<To>` can be omitted for open intervals.

__Example:__

To see everything after *28-12-2021 18:00:00.0* the expression can be either:
```
28-12-2021 18:00:00.0 ->
```
or
```
28-12-2021 18:00:00.0
```

To see everything before *28-12-2021 19:00:00.0*:
```
-> 28-12-2021 19:00:00.0
```

### Merging results

If the merging option is selected, the result of new searches will be merged to the previous ones.
