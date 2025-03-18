#include <iostream>

#include "data_objects.hpp"

namespace
{
/**
 * \brief Parse a buffer to acquire json objects
 * 
 * The response does not always contain commas between objects. Sometimes the
 * objects are arrays and sometimes not. This function's responsibility is to
 * identify the start of a valid object and the end of that object, which will
 * successfully be parsed by the rapidjson document. So this might return a
 * single object if the buffer is not organised as an array. If it is organised
 * as an array, the whole array will be returned.
 * 
 * \returns Pair where first is the start of a valid block and second is the end.get_json_block
 * If the block is not valid, this will be indicated by a return value of 0,0.
*/
std::pair<size_t, size_t> get_json_block(const char* begin)
{
    if ((begin == nullptr) || (*begin == '\0'))
    {
        return std::pair<size_t, size_t>(0, 0);
    }

    enum brace_type {BRACE, SQUARE, NONE};
    brace_type type = NONE;
    const char open_brace[] = { '{', '[' };
    const char close_brace[] = { '}', ']' };
    const char* current = begin;
    const char* json_start = nullptr;
    int n_brace = 0;
    bool in_quotes = false;
    bool escaped = false;

    do
    {
        if (in_quotes)
        {
            // This is a valid string:
            //    "\"\\"
            // which evaluates to "\ in raw form.
            // The first \" is escaped, but the second \" isn't.
            if ((*current == '"') && (!escaped))
            {
                in_quotes = false;
            }
            if ((*current == '\\') && (!escaped))
            {
                escaped = true;
            }
            else
            {
                escaped = false;
            }
        }
        else
        {
            switch (*current)
            {
                case '"':
                    in_quotes = true;
                    break;
                case '{':
                    if (type == NONE)
                    {
                        type = BRACE;
                        json_start = current;
                    }
                    break;
                case '[':
                    if (type == NONE)
                    {
                        type = SQUARE;
                        json_start = current;
                    }
                    break;
                default:
                    break;
            }
            if ((type != NONE) && (!in_quotes))
            {
                if (*current == open_brace[type])
                {
                    n_brace++;
                }
                if(*current == close_brace[type])
                {
                    n_brace--;
                }
            }
        }
        current++;
    }while((type == NONE || n_brace != 0) && (*current != '\0'));

    return ((type != NONE) && (n_brace == 0)) ?
        std::pair<size_t, size_t>((json_start - begin), (current - begin)) :
        std::pair<size_t, size_t>(0, 0);
}
} // namespace

DataObjects::DataObjects(const std::string &&json) :
    m_error(ErrorType::NONE),
    m_buffer(json),
    m_last_block_end(0),
    m_next_array_index(0)
{

}

const rapidjson::Value* DataObjects::get_next_object()
{
    // If this has already been parsed and it's an array, then return the next item in the array
    if (m_json_doc.IsArray())
    {
        if (m_next_array_index < m_json_doc.Size())
        {
            return &m_json_doc[m_next_array_index++];
        }
        else
        {
            // No more items in the array, but there might be another array in the buffer.
            m_next_array_index = 0;
        }
    }
    if (m_buffer[m_last_block_end] == '\0')
    {
        // Reached end of string
        if (m_last_block_end == 0)
        {
            std::cerr << "WARNING: This is an empty string: \"" << m_buffer << "\"" << std::endl;
            std::cerr << std::endl;
            m_error = DataObjects::ErrorType::FORMAT;
        }
        return nullptr;
    }

    auto [current_block_start, current_block_end] = get_json_block(m_buffer.c_str() + m_last_block_end);

    if (current_block_start == current_block_end)
    {
        // block is not valid. Check whether this is due to end of buffer, or else bad formatting.
        auto white_space_check = m_buffer.cbegin() + m_last_block_end;
        while(std::isspace(*white_space_check))
        {
            white_space_check++;
        }

        if(white_space_check == m_buffer.cend())
        {
            // Got to the end of the buffer.
            if (m_last_block_end == 0)
            {
                std::cerr << "WARNING: This whole buffer was just white space: \"" << m_buffer << "\"" << std::endl;
                std::cerr << std::endl;
                m_error = DataObjects::ErrorType::FORMAT;
            }
            return nullptr;
        }

        // This is not the end of the buffer, therefore there's another issue
        std::cerr << "ERROR: Ill formatted json block" << std::endl;
        std::cerr << "    buffer length = " << m_buffer.size() << "; last block end = " << m_last_block_end << ";" << std::endl;
        std::cerr << std::endl;
        std::cerr << m_buffer.c_str() + m_last_block_end;
        std::cerr << std::endl;
        m_error = ErrorType::FORMAT;
        return nullptr;
    }

    // JSON object has been successfully detected. rapidjson will now parse it.
    m_json_doc = rapidjson::Document();

    if (m_json_doc.Parse(m_buffer.substr(m_last_block_end + current_block_start, current_block_end).c_str()).HasParseError())
    {
        std::cerr << "Error parsing JSON!" << std::endl;
        std::cerr << m_buffer.substr(m_last_block_end + current_block_start, current_block_end) << std::endl;
        std::cerr << std::endl;
        m_error = ErrorType::FORMAT;
        return nullptr;
    }

    // Keep track of the next block
    m_last_block_end += current_block_end;
    
    // Is this is an array, return the first element and update the next index
    if(m_json_doc.IsArray())
    {
        if (m_json_doc.Size() > 0)
        {
            m_next_array_index = 1;
            return &m_json_doc[0];
        }
    }

    // If it's not an array, return the whole document, which will be a single object
    return &m_json_doc;
}

DataObjects::ErrorType DataObjects::get_error() const
{
    return m_error;
}