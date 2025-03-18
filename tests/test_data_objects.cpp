/**
 * \brief This file contains tests for the DataObjects class.
 * 
 * The DataObjects class parses blocks of json. The reponsibility of the DataObjects class
 * is simply to parse json, not to validate the fields are correct for this task. The test
 * cases presented here are all in the style of the task, but there is no requirement for
 * this class to handle that exclusively. The main purpose of these tests is to ensure all
 * experienced response formats are processed correctly. Eg. sometimes the json objects are
 * as arrays, sometimes singles, not even separated by commas.
*/

#include "data_objects.hpp"

#include "parameterise_description.hpp"
#include "records.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace
{
const test_records Elijah_record = {600002, "Elijah", "Palm Springs", 43, { { "Charlotte", {"Reading"} } }};
const std::string Elijah_pretty_print(R"({
    "id":600002,
    "name":"Elijah",
    "city":"Palm Springs",
    "age":43,
    "friends":[
        {
            "name":"Charlotte",
            "hobbies":["Reading"]
        }
        ]
})");
const std::string Elijah_compact(R"({"id":600002,"name":"Elijah","city":"Palm Springs","age":43,)"
                                        R"("friends":[{"name":"Charlotte","hobbies":["Reading"]}]})");

const test_records Barry_record = {600003, "Barry", "Washington", 23, {
    { "Morris", {"Movie Watching","Golf"} },
    { "Robin", {"Shopping","Calligraphy","Martial Arts"} } 
}};
const std::string Barry_pretty_print(R"({
    "id":600003,
    "name":"Barry",
    "city":"Washington",
    "age":23,
    "friends":[
        {
            "name":"Morris",
            "hobbies":["Movie Watching","Golf"]
        },
        {
            "name":"Robin",
            "hobbies":["Shopping","Calligraphy","Martial Arts"]
        }
        ]
})");
const std::string Barry_compact(R"({"id":600003,"name":"Barry","city":"Washington","age":23,)"
                                        R"("friends":[{"name":"Morris","hobbies":["Movie Watching","Golf"]},)"
                                        R"({"name":"Robin","hobbies":["Shopping","Calligraphy","Martial Arts"]}]})");

const std::string Elijah_pretty_print_array(std::string("[\n") + Elijah_pretty_print + std::string("\n]"));
const std::string Elijah_compact_array(std::string("[") + Elijah_compact + std::string("]"));
const std::string Elijah_white_space_start(std::string("    ") + Elijah_compact);
const std::string Elijah_white_space_end(Elijah_compact + std::string("    "));
const std::string Elijah_white_space_start_and_end(std::string("    ") + Elijah_compact + std::string("    "));

const std::string Elijah_Barry_pretty_print(Elijah_pretty_print + ",\n" + Barry_pretty_print);
const std::string Elijah_Barry_pretty_print_no_comma(Elijah_pretty_print + "\n" + Barry_pretty_print);
const std::string Elijah_Barry_pretty_print_no_whitespace(Elijah_pretty_print + "," + Barry_pretty_print);
const std::string Elijah_Barry_pretty_print_no_whitespace_no_comma(Elijah_pretty_print + Barry_pretty_print);
const std::string Elijah_Barry_pretty_print_array(std::string("[\n") + Elijah_pretty_print + ",\n" + Barry_pretty_print + "\n]");

const std::string Elijah_Barry_compact(Elijah_compact + ",\n" + Barry_compact);
const std::string Elijah_Barry_compact_no_comma(Elijah_compact + "\n" + Barry_compact);
const std::string Elijah_Barry_compact_no_whitespace(Elijah_compact + "," + Barry_compact);
const std::string Elijah_Barry_compact_no_whitespace_no_comma(Elijah_compact + Barry_compact);
const std::string Elijah_Barry_compact_array(std::string("[") + Elijah_compact + "," + Barry_compact + "]");

const std::vector<test_records> case_one_record = {Elijah_record};
const std::vector<test_records> case_two_records = {Elijah_record, Barry_record};
} // namespace


class TestDataObjectsInput :
    public ::testing::TestWithParam<ParamWithDescription<const std::string, const std::vector<test_records>>>
{};

TEST_P(TestDataObjectsInput, ValidJsonInput)
{
    auto param = GetParam();
    auto test_input = param.GetParam();
    auto expected_output = param.GetExpected();

    DataObjects CUT(std::move(test_input));

    for(auto expected_record : expected_output)
    {
        auto record = CUT.get_next_object();
        ASSERT_NE(record, nullptr) << "The first object was not processed";
        ASSERT_EQ(CUT.get_error(), DataObjects::ErrorType::NONE) << "The error should be none if successful";

        ASSERT_TRUE(record->HasMember("id")) << "\"id\" field not present when parsed";
        ASSERT_TRUE(record->HasMember("name")) << "\"name\" field not present when parsed";
        ASSERT_TRUE(record->HasMember("city")) << "\"city\" field not present when parsed";
        ASSERT_TRUE(record->HasMember("age")) << "\"age\" field not present when parsed";
        ASSERT_TRUE(record->HasMember("friends")) << "\"friends\" field not present when parsed";

        ASSERT_TRUE((*record)["id"].IsInt()) << "\"id\" field is not type int";
        ASSERT_TRUE((*record)["name"].IsString()) << "\"name\" field is not type string";
        ASSERT_TRUE((*record)["city"].IsString()) << "\"city\" field is not type string";
        ASSERT_TRUE((*record)["age"].IsInt()) << "\"age\" field is not type int";
        ASSERT_TRUE((*record)["friends"].IsArray()) << "\"friends\" field is not type Array";

        EXPECT_EQ((*record)["id"].GetInt(), expected_record.id) << "\"id\" field did not parse the correct value";
        EXPECT_EQ((*record)["name"].GetString(), expected_record.name) << "\"name\" field did not parse the correct value";
        EXPECT_EQ((*record)["city"].GetString(), expected_record.city) << "\"city\" field did not parse the correct value";
        EXPECT_EQ((*record)["age"].GetInt(), expected_record.age) << "\"age\" field did not parse the correct value";

        const auto& friends_array = (*record)["friends"].GetArray();
        ASSERT_EQ(friends_array.Size(), expected_record.friends.size()) << "\"friends\" array is the wrong size";
        
        for (size_t i_friend = 0; i_friend < expected_record.friends.size(); i_friend++)
        {
            const auto& hfriend = friends_array[i_friend];
            ASSERT_TRUE(hfriend.HasMember("name")) << "\"name\" field not present in the friends array when parsed";
            ASSERT_TRUE(hfriend.HasMember("hobbies")) << "\"hobbies\" field not present in the friends array when parsed";

            ASSERT_TRUE(hfriend["name"].IsString()) << "\"name\" field in the friends array is not type string";
            ASSERT_TRUE(hfriend["hobbies"].IsArray()) << "\"hobbies\" field in the friends array is not type Array";

            EXPECT_EQ(hfriend["name"].GetString(), expected_record.friends[i_friend].name) << "\"name\" from fireds array was not parsed correctly";

            const auto& hobbies_array = hfriend["hobbies"].GetArray();
            ASSERT_EQ(hobbies_array.Size(), expected_record.friends[i_friend].hobbies.size()) << "\"hobbies\" array is the wrong size";
            for(size_t i_hobby = 0; i_hobby < expected_record.friends[i_friend].hobbies.size(); i_hobby++)
            {
                EXPECT_EQ(hobbies_array[i_hobby].GetString(), expected_record.friends[i_friend].hobbies[i_hobby]) << "\"hobbies\" field did not parse the correct value";
            }
        }
    }
    auto next_record = CUT.get_next_object();
    ASSERT_EQ(next_record, nullptr) << "get_next_object should result in nullptr when there are no more objects";
    ASSERT_EQ(CUT.get_error(), DataObjects::ErrorType::NONE) << "The error should be none if come to the end of the buffer without error";
}
INSTANTIATE_TEST_CASE_P(ValidJsonInput, TestDataObjectsInput,
    ::testing::Values(
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_pretty_print, case_one_record, "one_pretty_print_record"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_pretty_print_array, case_one_record, "one_pretty_print_record_as_array"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_compact, case_one_record, "one_compact_format_record"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_compact_array, case_one_record, "one_compact_format_record_as_array"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_white_space_start, case_one_record, "white_space_at_start"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_white_space_end, case_one_record, "white_space_at_end"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_white_space_start_and_end, case_one_record, "white_space_at_start_and_end"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_Barry_pretty_print, case_two_records, "two_pretty_print_records"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_Barry_pretty_print_no_comma, case_two_records, "two_pretty_print_records_no_comma"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_Barry_pretty_print_no_whitespace, case_two_records, "two_pretty_print_records_no_whitespace"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_Barry_pretty_print_no_whitespace_no_comma, case_two_records, "two_pretty_print_records_no_whitespace_no_comma"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_Barry_pretty_print_array, case_two_records, "two_pretty_print_records_as_array"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_Barry_compact, case_two_records, "two_compact_format_records"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_Barry_compact_no_comma, case_two_records, "two_compact_format_records_no_comma"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_Barry_compact_no_whitespace, case_two_records, "two_compact_format_records_no_whitespace"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_Barry_compact_no_whitespace_no_comma, case_two_records, "two_compact_format_records_no_whitespace_no_comma"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            Elijah_Barry_compact_array, case_two_records, "two_compact_format_records_as_array")),
    [](const testing::TestParamInfo<ParamWithDescription<const std::string, const std::vector<test_records>>>& info)
    {
        return info.param.GetDescription();
    }
    );


class TestBadFormatDataObjectsInput :
    public ::testing::TestWithParam<ParamWithDescription<const std::string, const std::vector<test_records>>>
{};
TEST_P(TestBadFormatDataObjectsInput, BadFormatInput)
{
    auto param = GetParam();
    auto test_input = param.GetParam();

    DataObjects CUT(std::move(test_input));

    auto record = CUT.get_next_object();
    ASSERT_EQ(record, nullptr) << "Bad format results should not return non-null or throw exceptions";
    ASSERT_EQ(CUT.get_error(), DataObjects::ErrorType::FORMAT) << "The error should indicate the format error";
}

INSTANTIATE_TEST_CASE_P(BadFormatInput, TestBadFormatDataObjectsInput,
    ::testing::Values(
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            std::string(""), {}, "empty_string"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            std::string("{"), {}, "opening_braces"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            std::string("}"), {}, "closing_braces"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            std::string("["), {}, "opening_array"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            std::string("]"), {}, "closing_array"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            std::string("{]"), {}, "opening_brace_closing_array"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            std::string("[}"), {}, "opening_array_closing_brace"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            std::string("[{]"), {}, "opening_brace_in_array"),
        ParamWithDescription<const std::string, const std::vector<test_records>>(
            std::string("   "), {}, "white_space")),
    [](const testing::TestParamInfo<ParamWithDescription<const std::string, const std::vector<test_records>>>& info)
    {
        return info.param.GetDescription();
    }
    );