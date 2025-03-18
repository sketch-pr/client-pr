#pragma once

#include <map>
#include <rapidjson/document.h>
#include <vector>

#include "query_tables.hpp"

/**
 * \brief Class to accept records in rapidjson objects and populate normalised tables
 * 
 * Converting the data format into normalised tables helps to optimise the querying.
 * This should make any query possible, not just the ones required for the task.
*/
class Tables
{
public:
    /**
     * \brief Constructor
    */
    Tables();

    /**
     * \brief Add a new record to the tables
     * 
     * The DataObjects class will provide a rapidjson::Value by calling get_next_record().
     * The add_record() function will then perform some validation and add the record to
     * the tables.
     * 
     * \return true if record could be added successfully
    */
    bool add_record(const rapidjson::Value *record);

    /**
     * \brief Performs query on the records, and computes the values required by the task.
     * 
     * \returns Structure containing the computed results of the task.
    */
    Results query_results() const;
protected:
    std::map<std::string, std::vector<unsigned int>> m_city_citizen;    /// One to many Table associating cities with citizen IDs
    std::map<unsigned int, Citizen> m_citizen;                          /// One to one Table associating citizens with their IDs
    std::map<unsigned int, std::vector<Friend>> m_citizen_friends;      /// One to many Table associating citizens with their friends
    std::map<std::string, std::vector<std::string>> m_hobby_friends;    /// One to many Table associating hobbies with friends

private:
    int m_generated_id; /// Not all records contain a citizen id. If abscent, this is used instead
};