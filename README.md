# JSON Rest Client

## Build

This project requires `libcurl` and `rapidjson` and `GTest`.

```bash
# Install libcurl
apt-get libcurl4-openssl-dev
```

Use cmake to build. This will pull the dependencies `GTest` and `rapidjson`.

```bash
mkdir build
cd build
cmake -GNinja ..
ninja
```

## Run

This solution has unit tests that can be run from the build directory using the command

```bash
ctest
```

The binaries appear in the bin directory. To run:

```bash
cd ../bin
./JsonRestClient http://test.brightsign.io:3000
```

Errors and any logging messages will be reported on stderr. The output as required by the task is
streamed to stdout.

# Details

The endpoint used is `http://test.brightsign.io:3000`, which responds with data in the general format:

```json
{
    "id":600002,
    "name":"Elijah",
    "city":"Palm Springs",
    "age":43,
    "friends":[
        {
            "name":"Charlotte",
            "hobbies":["Reading","Walking"]
        },
        {
            "name":"Nora",
            "hobbies":["Shopping","Bicycling","Fishing"]
        },
        {
            "name":"Nora",
            "hobbies":["Shopping","Calligraphy","Martial Arts"]
        },
        {
            "name":"Luke","hobbies":["Movie Watching","Golf"]
        }
    ]
}
```

The json is sometimes pretty-printed and sometimes compact format. There are often many json objects in one response.
Sometimes the json objects are presented as non-comma separated fragments, and sometimes as an array.

Errors can occur in this format:

```html
HTTP/1.1 400 Bad Request
Server: nginx
Date: Tue, 01 Mar 2016 12:34:02 GMT
Content-Type: text/html
Content-Length: 166
Connection: close
Response:

<html>
<head><title>400 Bad Request</title></head>
<body bgcolor="white">
<center><h1>400 Bad Request</h1></center>
<hr><center>nginx</center>
</body>
</html>
```

Or sometimes

```
500 - Something bad happened!
```

## Implementation

The endpoint is queried using an object of type Client. This uses the `curl` library to perform the query and save the
response in a string buffer.

Once the buffer is acquired, ownership is passed to an object of type DataObjects. This ensures that the data objects
in the response are parsed correctly, and presents each object as a `rapidjson::Value` for further processing.

To perform the calculations, the data is normalised into tables which are structured in such a manner that any generic
query on the data would be performed efficiently. The Tables class is responsible for validating the json structure and
storing the records, as well as performing the query on the tables. Some might consider a proper database to be a
sensible choice here, but since the calculations are simple and the data structure not too complicated, integrating a
database here would be overkill.

The output of the query is raw structures, which are then parsed into a `rapidjson` document, and converted into a string
for printing. This is hard-coded to pretty-print format, but there is a parameter that would switch to compact format if
required.

## Unit Tests

This solution has unit tests that can be run using the command

```bash
ctest
```

# Further Work

This project is organised to perform one step at a time - acquire data, parse objects, store records, query, print.
However, it is not necessary to wait for the data acquisition process to fully complete before starting to parse the
objects and store the records. Parallelising these tasks could result in improvements in execution time. This would add
some further complexity, which would break the clear separation of concerns between objects and make it harder to test.

The endpoint doesn't always respond with json, in the case of error messages. The client doesn't read the data buffer,
so these error messages are picked up when failing to parse the buffer into rapidjson. Handling these errors eariler
would be beneficial.

The DataObjects class implements a method `get_next_object()`. This could be refactored to operate as iterators, which
would be stylistically nicer, but functionally no different.

Error codes are used in this project instead of throwing exceptions. There are two schools of thought on what is
preferable here, and some may prefer exceptions. I don't mind either way, but picked error codes since it seemed to be
the more natural choice following the use of the curl library. The code could be refactored to handle exceptions instead.

More unit tests would be good.

Logging is implemented as a basic print to stderr. This could be refactored to use a better logging method with file handlers,
log levels, etc.

## Security

- The endpoint does not implement any authentication or authorisation methods.
- There is no force of https or validation certificates.
- Once the data is received and processed, the buffers are not overwritten meaning the data may still reside in memory after
it's gone out of scope.
- libcurl has been subject to vulnerabilities in the past. It is, however, under active maintenance. The client in this project
has been written in such a way that the dependency on libcurl resides only in the `client.cpp`` source file. Although there is
a forward declaration in the header file - the impact of switching to an alternative approach would be minimal and not affect
any other part of the project.