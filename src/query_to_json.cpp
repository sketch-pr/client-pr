#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "query_to_json.hpp"

namespace {

/**
 * \brief Parse the results strucutures into rapidjson
*/
rapidjson::Value CityResults_to_json(const CityResults & city_results, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("city_name", rapidjson::Value().SetString(city_results.city_name.c_str(), allocator), allocator);
    obj.AddMember("average_age", city_results.average_age, allocator);
    obj.AddMember("average_number_of_friends", city_results.average_number_of_friends, allocator);
    obj.AddMember("user_with_most_friends", rapidjson::Value().SetString(city_results.user_with_most_friends.c_str(), allocator), allocator);
    return obj;
}

}

QueryToJson::QueryToJson(const Results& result)
{
    m_document.SetObject();

    rapidjson::Document::AllocatorType& allocator = m_document.GetAllocator();

    rapidjson::Value array(rapidjson::kArrayType);
    for(const auto& city: result.cities)
    {
        // Populate the per city array
        array.PushBack(CityResults_to_json(city, allocator), allocator);
    }
    m_document.AddMember("cities", array, allocator);
    m_document.AddMember("most_common_first_name", rapidjson::Value().SetString(result.most_common_first_name.c_str(), allocator), allocator);
    m_document.AddMember("most_common_hobby", rapidjson::Value().SetString(result.most_common_hobby.c_str(), allocator), allocator);
}

std::string QueryToJson::get_json(bool pretty)
{
    rapidjson::StringBuffer buffer;

    if (pretty)
    {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        m_document.Accept(writer);
    }
    else
    {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        m_document.Accept(writer);
    }

    return buffer.GetString();
}