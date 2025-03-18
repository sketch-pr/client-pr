#pragma once

#include <string>
#include <vector>

struct test_friends
{
    std::string name;
    std::vector<std::string> hobbies;
};

struct test_records
{
    int id;
    std::string name;
    std::string city;
    int age;
    std::vector<test_friends> friends;
};