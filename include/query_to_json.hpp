#pragma once

#include <rapidjson/document.h>

#include "query_tables.hpp"

/**
 * \brief This class is responsible for converting the Results into json format for output
*/
class QueryToJson
{
public:
    /**
     * \brief Constructor
     * 
     * \param result: Pre-computed restults
    */
    QueryToJson(const Results& result);

    /**
     * \brief Returns the results as a json string
     * 
     * \param pretty: true returns output in pretty-print, false returns in compact format.
    */
    std::string get_json(bool pretty = true);
protected:
    rapidjson::Document m_document;     /// Results are parsed into this document
private:
};