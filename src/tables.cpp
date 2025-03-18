#include "tables.hpp"

#include <iostream>
#include <string>

namespace
{
constexpr const char* CITY_FIELD = "city";              /// Field name of the city as it appears in the endpoint response
constexpr const char* CITIZEN_ID_FIELD = "id";          /// Field name of the citizen id as it appears in the endpoint response
constexpr const char* CITIZEN_NAME_FIELD = "name";      /// Field name of the citizen name as it appears in the endpoint response
constexpr const char* CITIZEN_AGE_FIELD = "age";        /// Field name of the citizen's age as it appears in the endpoint response
constexpr const char* FRIEND_FIELD = "friends";         /// Field name of the citizen's friends array as it appears in the endpoint response
constexpr const char* FRIEND_NAME_FIELD = "name";       /// Field name of the friend name as it appears in the endpoint response
constexpr const char* FRIEND_HOBBIES_FIELD = "hobbies"; /// Field name of the friend's hobbies array as it appears in the endpoint response
}

/*
Expecting data records in this format:
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
*/

Tables::Tables() : m_generated_id(1)
{
    
}

#define CHECK_FIELDS(X, Y, OPTIONAL)                                                             \
if (!(record->HasMember(X##_FIELD) && (*record)[X##_FIELD].Is##Y()))                             \
{                                                                                                \
    if(!OPTIONAL || (record->HasMember(X##_FIELD) && !(*record)[X##_FIELD].Is##Y()))             \
    {                                                                                            \
        std::cerr << (record->HasMember(X##_FIELD) ? "Unexpected " : "Missing ");                \
        std::cerr << "field type " << #X << "; Expecting " << #Y << std::endl;                   \
        if(record->HasMember(X##_FIELD))                                                         \
        {                                                                                        \
            std::cerr << "    String: " << (*record)[X##_FIELD].IsString() << std::endl;         \
            std::cerr << "    Int: " << (*record)[X##_FIELD].IsInt() << std::endl;               \
            std::cerr << "    Int64: " << (*record)[X##_FIELD].IsInt64() << std::endl;           \
            std::cerr << "    Uint: " << (*record)[X##_FIELD].IsUint() << std::endl;             \
            std::cerr << "    Uint64: " << (*record)[X##_FIELD].IsUint64() << std::endl;         \
            std::cerr << "    Double: " << (*record)[X##_FIELD].IsDouble() << std::endl;         \
            std::cerr << "    Bool: " << (*record)[X##_FIELD].IsBool() << std::endl;             \
            std::cerr << "    Array: " << (*record)[X##_FIELD].IsArray() << std::endl;           \
            std::cerr << "    Object: " << (*record)[X##_FIELD].IsObject() << std::endl;         \
            std::cerr << "    Null: " << (*record)[X##_FIELD].IsNull() << std::endl;             \
        }                                                                                        \
        std::cerr << std::endl;                                                                  \
        if (record->IsObject())                                                                  \
        {                                                                                        \
            for(auto it = record->MemberBegin(); it != record->MemberEnd(); it++)                \
            {                                                                                    \
                std::cerr << "\"" << it->name.GetString() << "\", ";                             \
            }                                                                                    \
            std::cerr << std::endl;                                                              \
        }                                                                                        \
        return false;                                                                            \
    }                                                                                            \
}

bool Tables::add_record(const rapidjson::Value *record)
{
    // Perform validation; Returns false if there's a problem
    CHECK_FIELDS(CITY, String, false)
    CHECK_FIELDS(CITIZEN_ID, Int, true)         // ID isn't always present, so this is optional
    CHECK_FIELDS(CITIZEN_NAME, String, false)
    CHECK_FIELDS(CITIZEN_AGE, Int, false)
    CHECK_FIELDS(FRIEND, Array, false)

    const std::string city = (*record)[CITY_FIELD].GetString();
    const int citizen_id = record->HasMember(CITIZEN_ID_FIELD) ?
                            (*record)[CITIZEN_ID_FIELD].GetInt() :
                            m_generated_id++;   // If the id field is missing, use m_generate_id instead
    const std::string citizen_name = (*record)[CITIZEN_NAME_FIELD].GetString();
    const int citizen_age = (*record)[CITIZEN_AGE_FIELD].GetInt();
    const auto& friends = (*record)[FRIEND_FIELD].GetArray();

    // Populate the Tables
    if (citizen_id >= 0)
    {
        auto& citizen_row = m_citizen[citizen_id];
        citizen_row.name = citizen_name;
        citizen_row.age = citizen_age;

        if (!city.empty())
        {
            m_city_citizen[city].push_back(citizen_id);
        }

        for(size_t i_friend = 0; i_friend < friends.Size(); i_friend++)
        {
            const auto& hfriend = friends[i_friend];
            if (hfriend.HasMember(FRIEND_NAME_FIELD) && hfriend[FRIEND_NAME_FIELD].IsString())
            {
                Friend f;
                f.name = hfriend[FRIEND_NAME_FIELD].GetString();
                m_citizen_friends[citizen_id].emplace_back(f);

                if(hfriend.HasMember(FRIEND_HOBBIES_FIELD) && hfriend[FRIEND_HOBBIES_FIELD].IsArray())
                {
                    const auto& hobbies = hfriend[FRIEND_HOBBIES_FIELD];
                    for(size_t i_hobby = 0; i_hobby < hobbies.Size(); i_hobby++)
                    {
                        if(hobbies[i_hobby].IsString())
                        {
                            m_hobby_friends[hobbies[i_hobby].GetString()].push_back(f.name);
                        }
                    }
                }
            }
        }
    }

    return true;
}
#undef CHECK_FIELDS

Results Tables::query_results() const
{
    Results results;
    std::map<std::string, int> citizen_names;

    // Iterate over cities and determine the per city results
    for(auto& i_city : m_city_citizen)
    {
        CityResults city_results;
        city_results.city_name = i_city.first;
        if(i_city.second.size() > 0)
        {
            size_t age = 0;
            int n_friends = 0;
            std::pair<size_t, std::string> max_friends {0, ""};
            for(auto& i_citizen : i_city.second)
            {
                const auto& citizen = m_citizen.find(i_citizen);
                if (citizen != m_citizen.end())
                {
                    age += citizen->second.age;
                    auto c_name = citizen_names.find(citizen->second.name);
                    if(c_name == citizen_names.end())
                    {
                        citizen_names[citizen->second.name] = 1;
                    }
                    else
                    {
                        c_name->second++;
                    }
                }

                const auto& citizens_friends = m_citizen_friends.find(i_citizen);
                if (citizens_friends != m_citizen_friends.end())
                {
                    auto current_num = citizens_friends->second.size();
                    if(current_num > max_friends.first)
                    {
                        max_friends = {current_num, citizen->second.name};
                    }
                    n_friends += current_num;
                }
            }

            city_results.average_age = age / i_city.second.size();
            city_results.average_number_of_friends = n_friends / i_city.second.size();
            city_results.user_with_most_friends = max_friends.second;

            results.cities.emplace_back(city_results);
        }
    }

    // Find the most common name
    std::pair<int, std::string> common_name {0, ""};
    for (auto& i_name : citizen_names)
    {
        if (i_name.second > common_name.first)
        {
            common_name.first = i_name.second;
            common_name.second = i_name.first;
        }
    }
    results.most_common_first_name = common_name.second;

    // Find the most common hobby
    std::pair<size_t, std::string> common_hobby{0, ""};
    for (auto& i_hobby : m_hobby_friends)
    {
        if (i_hobby.second.size() > common_hobby.first)
        {
            common_hobby.first = i_hobby.second.size();
            common_hobby.second = i_hobby.first;
        }
    }
    results.most_common_hobby = common_hobby.second;

    return results;
}