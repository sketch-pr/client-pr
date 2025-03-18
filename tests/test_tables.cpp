#include "tables.hpp"
#include "data_objects.hpp"

#include "records.hpp"
#include "parameterise_description.hpp"

#include <gtest/gtest.h>

namespace
{
class TablesForTest : public Tables
{
public:
    const std::map<unsigned int, Citizen>& get_citizen_table() { return m_citizen; };
};

const std::string Elijah_compact(R"({"id":600002,"name":"Elijah","city":"Palm Springs","age":43,)"
                                    R"("friends":[{"name":"Charlotte","hobbies":["Reading"]}]})");
const std::string Elijah_compact_no_id(R"({"name":"Elijah","city":"Palm Springs","age":43,)"
                                    R"("friends":[{"name":"Charlotte","hobbies":["Reading"]}]})");

const std::string Barry_compact(R"({"id":600003,"name":"Barry","city":"Washington","age":23,)"
                                    R"("friends":[{"name":"Morris","hobbies":["Movie Watching","Golf"]},)"
                                    R"({"name":"Robin","hobbies":["Shopping","Calligraphy","Martial Arts"]}]})");
const std::string Barry_compact_no_id(R"({"name":"Barry","city":"Washington","age":23,)"
                                    R"("friends":[{"name":"Morris","hobbies":["Movie Watching","Golf"]},)"
                                    R"({"name":"Robin","hobbies":["Shopping","Calligraphy","Martial Arts"]}]})");
const std::string Barry_compact_no_name(R"({"id":600003,"city":"Washington","age":23,)"
                                    R"("friends":[{"name":"Morris","hobbies":["Movie Watching","Golf"]},)"
                                    R"({"name":"Robin","hobbies":["Shopping","Calligraphy","Martial Arts"]}]})");
const std::string Barry_compact_no_city(R"({"id":600003,"name":"Barry","age":23,)"
                                    R"("friends":[{"name":"Morris","hobbies":["Movie Watching","Golf"]},)"
                                    R"({"name":"Robin","hobbies":["Shopping","Calligraphy","Martial Arts"]}]})");
const std::string Barry_compact_no_age(R"({"id":600003,"name":"Barry","city":"Washington",)"
                                    R"("friends":[{"name":"Morris","hobbies":["Movie Watching","Golf"]},)"
                                    R"({"name":"Robin","hobbies":["Shopping","Calligraphy","Martial Arts"]}]})");
const std::string Barry_compact_no_friends(R"({"id":600003,"name":"Barry","city":"Washington","age":23})");

const std::string Paul_compact_no_id(R"({"name":"Paul","city":"Washington","age":86,)"
                                    R"("friends":[{"name":"Ringo","hobbies":["Reading"]},)"
                                    R"({"name":"George","hobbies":["Calligraphy","Martial Arts"]}]})");
const std::string John_compact_no_id(R"({"name":"John","city":"Washington","age":89,)"
                                    R"("friends":[{"name":"Ringo","hobbies":["Reading"]},)"
                                    R"({"name":"George","hobbies":["Shopping","Walking"]},)"
                                    R"({"name":"Yoko","hobbies":["Shopping","Calligraphy","Martial Arts"]}]})");

const std::string John_of_vegas_no_id(R"({"name":"John","city":"Las Vegas","age":20,)"
                                    R"("friends":[{"name":"Charlotte","hobbies":["Reading"]}]})");
} // namespace

TEST(TestTable, TestAutoID)
{
    const std::string fields_without_id = Elijah_compact_no_id + "\n" + Barry_compact_no_id;
    DataObjects data_objects(std::move(fields_without_id));

    TablesForTest CUT;
    for(int i_object = 0; i_object < 2; i_object++)
    {
        auto rapidjson_result = data_objects.get_next_object();

        ASSERT_NE(rapidjson_result, nullptr) << "This test json string is ill formatted";
        ASSERT_EQ(data_objects.get_error(), DataObjects::ErrorType::NONE) << "This test json string is ill formatted";

        CUT.add_record(rapidjson_result);
    }

    auto results = CUT.get_citizen_table();
    int expected_id = 1;
    for(auto it_citizen : results)
    {
        ASSERT_EQ(it_citizen.first, expected_id) << "Auto id generator did not generate the expected ID";
        expected_id++;
    }
}

class TestMissingFields :
    public ::testing::TestWithParam<ParamWithDescription<const std::string, const Results>>
{};
TEST_P(TestMissingFields, MissingFields)
{
    auto param = GetParam();
    std::string test_input = param.GetParam();

    DataObjects data_objects(std::move(test_input));

    Tables CUT;

    auto rapidjson_result = data_objects.get_next_object();
    ASSERT_NE(rapidjson_result, nullptr) << "This test json string is ill formatted";
    ASSERT_EQ(data_objects.get_error(), DataObjects::ErrorType::NONE) << "This test json string is ill formatted";

    bool res = CUT.add_record(rapidjson_result);

    ASSERT_FALSE(res) << "Missing non-optional field should result in failure to add record";

}
INSTANTIATE_TEST_CASE_P(MissingFields, TestMissingFields,
    ::testing::Values(
        ParamWithDescription<const std::string, const Results>(
            Barry_compact_no_name, {}, "no_name"),
        ParamWithDescription<const std::string, const Results>(
            Barry_compact_no_city, {}, "no_city"),
        ParamWithDescription<const std::string, const Results>(
            Barry_compact_no_age, {}, "no_age"),
        ParamWithDescription<const std::string, const Results>(
            Barry_compact_no_friends, {}, "no_friends")),
    [](const testing::TestParamInfo<ParamWithDescription<const std::string, const Results>>& info)
    {
        return info.param.GetDescription();
    }
);

TEST(TestTable, TestResults)
{
    const std::string fields_without_id =   Elijah_compact_no_id + "\n" +
                                            Barry_compact_no_id + "\n" +
                                            Paul_compact_no_id + "\n" +
                                            John_compact_no_id + "\n" +
                                            John_of_vegas_no_id;
    const int expected_average_age_palm_springs = 43;
    const int expected_average_num_friends_palm_springs = 1;

    const int expected_average_age_washington = (23 + 86 + 89) / 3;
    const int expected_average_num_friends_washington = (2 + 2 + 3) / 3;

    DataObjects data_objects(std::move(fields_without_id));

    TablesForTest CUT;
    for(int i_object = 0; i_object < 5; i_object++)
    {
        auto rapidjson_result = data_objects.get_next_object();

        ASSERT_NE(rapidjson_result, nullptr) << "This test json string is ill formatted";
        ASSERT_EQ(data_objects.get_error(), DataObjects::ErrorType::NONE) << 
            "This test json string is ill formatted";

        CUT.add_record(rapidjson_result);
    }
    auto results = CUT.query_results();

    // Check the cities are stored correctly
    ASSERT_EQ(results.cities.size(), 3) << "Wrong number of cities were stored";
    ASSERT_EQ(results.cities[0].city_name, "Las Vegas") << 
        "Cities were not named correctly in the table, or are unexpectedly out of order";
    ASSERT_EQ(results.cities[1].city_name, "Palm Springs") << 
        "Cities were not named correctly in the table, or are unexpectedly out of order";
    ASSERT_EQ(results.cities[2].city_name, "Washington") << 
        "Cities were not named correctly in the table, or are unexpectedly out of order";

    // Average Age per city
    ASSERT_EQ(results.cities[1].average_age, expected_average_age_palm_springs) <<
        "Average age was not calculted correctly with one citizen";
    ASSERT_EQ(results.cities[2].average_age, expected_average_age_washington) <<
        "Average age was not calculted correctly with more than one citizen";

    // Average Number of Friends per city
    ASSERT_EQ(results.cities[1].average_number_of_friends, expected_average_num_friends_palm_springs) << 
        "Average friends was not calculted correctly with one citizen";
    ASSERT_EQ(results.cities[2].average_number_of_friends, expected_average_num_friends_washington) <<
        "Average age was not calculted correctly with more than one citizen";

    // Most Common First Name in all cities
    ASSERT_EQ(results.most_common_first_name, "John") << "The most common first name was not determined correctly";

    // Most Common Hobby of all friends of users in all cities
    ASSERT_EQ(results.most_common_hobby, "Reading") << "The most common hobby was not determined correctly";
}
