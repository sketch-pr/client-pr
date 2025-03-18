#pragma once

#include <string>
#include <rapidjson/document.h>

/**
 * \brief Splits the json response into individual records for easy processing
 * 
 * This class uses rapidjson to output each record acquired from the endpoint.
*/
class DataObjects
{
public:
    /**
     * \brief Constructor
     * 
     * This class takes ownership of the buffer.
     * 
     * \param json: buffer expected to be in json format - the result of querying the endpoint
    */
    DataObjects(const std::string &&json);

    /**
     * \brief Gets the next record from the json response
     * 
     * Parses the string buffer into rapidjson objects. Ill formatted json will return a nullptr.
     * Nullptr is also returned when the buffer has been exhausted. Check the status by calling
     * get_error() if nullptr is returned. Error code NONE means parsing was successful, but there
     * are no more objects. Error code FORMAT means the parsing was unsuccessful.
     * 
     * \returns Next json object in the buffer, or nullptr
    */
    const rapidjson::Value* get_next_object();

    /**
     * \brief Error types associated with this class
    */
    enum class ErrorType {
        NONE,       /// No error encountered
        FORMAT,     /// Buffer contains ill formatted json
    };

    /**
     * \brief Returns the last error encountered
    */
    ErrorType get_error() const;
protected:
    ErrorType m_error;                  /// Last error encountered

private:
    const std::string m_buffer;         /// buffer containing response to be parsed
    size_t m_last_block_end;            /// Index determining the current position in the buffer
    rapidjson::Document m_json_doc;     /// response parsed into rapidjson object
    size_t m_next_array_index;          /// Index of the current object in the array, if the buffer represents a json array
};