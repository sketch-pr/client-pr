#include <iostream>
#include <string>
#include <curl/curl.h>

#include "client.hpp"

namespace
{

/**
 * \brief Callback function to write the response data to a string
 */ 
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output)
{
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}
} // namespace

Client::Client(const char* endpoint) : m_enpoint(endpoint), m_error(ErrorType::NONE)
{
    // Initialize curl
    m_curl = curl_easy_init();
    if (!m_curl)
    {
        std::cerr << "ERROR: could not intialise curl" << std::endl;
        std::cerr << std::endl;
        m_error = ErrorType::INIT;
    }

}

Client::~Client()
{
    // Clean up
    curl_easy_cleanup(m_curl);

}

void Client::query_endpoint()
{
    if (!m_curl)
    {
        std::cerr << "ERROR: curl was not intialised" << std::endl;
        std::cerr << std::endl;
        m_error = ErrorType::INIT;
        return;
    }

    // Set the URL for the GET request
    curl_easy_setopt(m_curl, CURLOPT_URL, m_enpoint.c_str());

    // Set the callback function to handle the response
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_response);

    // Perform the request
    CURLcode res = curl_easy_perform(m_curl);

    // Check for errors
    if (res != CURLE_OK)
    {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        std::cerr << std::endl;
        m_error = ErrorType::QUERY;
    }
}

Client::ErrorType Client::get_error() const
{
    return m_error;
}

const std::string &Client::get_response() const
{
    return m_response;
}