/**
 * A program to connect to an http endpoint and output:
 *     average age of all users per city
 *     average number of friends per city
 *     the user with the most friends per city
 *     the most common first name in all cities
 *     the most common hobby of all friends of users in all cities
*/

#include <iostream>

#include "client.hpp"
#include "data_objects.hpp"
#include "query_to_json.hpp"
#include "tables.hpp"

int main(int argc, const char* argv[])
{
    enum {ENDPOINT_ARG=1, NUM_ARGS};

    if (argc != NUM_ARGS)
    {
        std::cerr << "Wrong number of arguments. Expecting endpoint as the argument" << std::endl;
        std::cerr << std::endl;
        exit(1);
    }

    Client client(argv[ENDPOINT_ARG]);
    client.query_endpoint();
    
    if (client.get_error() != Client::ErrorType::NONE)
    {
        std::cerr << "Error occured" << std::endl;
        std::cerr << std::endl;
        exit(1);
    }

    // Get the response in full
    auto& response = client.get_response();

    // Parse the response into rapidjson objects
    DataObjects json_objects(std::move(response));
    Tables tables;
    int n_bad_records = 0;
    
    // For every object, populate tables with the data
    auto json_doc = json_objects.get_next_object();
    while(json_doc != nullptr)
    {
        if(!tables.add_record(json_doc))
        {
            n_bad_records++;
        }
        json_doc = json_objects.get_next_object();
    }

    if (json_objects.get_error() != DataObjects::ErrorType::NONE)
    {
        std::cerr << "Some parsing error has occurred" << std::endl;
        std::cerr << std::endl;
        exit(1);
    }

    // Query the tables
    auto query = tables.query_results();

    // Format the data and output
    QueryToJson query_json(query);

    std::cout << query_json.get_json();

    std::cout << std::endl;
    exit(0);
}
