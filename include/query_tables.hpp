/**
 * \brief Tables for representing the overall results are defined here
*/

#pragma once

#include <string>
#include <vector>

/**
 * \brief Table representing citizens
*/
struct Citizen
{
    std::string name;
    int age;
};

/**
 * \brief Table representing friends of citizens
*/
struct Friend
{
    std::string name;
};

/**
 * \brief Table representing per city results
*/
struct CityResults
{
    std::string city_name;
    int average_age;
    int average_number_of_friends;
    std::string user_with_most_friends;
};

/**
 * \brief Table representing all results
*/
struct Results
{
    std::vector<CityResults> cities;
    std::string most_common_first_name;
    std::string most_common_hobby;
};
