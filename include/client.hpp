#pragma once

typedef void CURL; /// Forward delcaration

/**
 * \brief Implementation to connect to client and return the text data.
 * 
*/
class Client
{
public:
    /**
     * \brief Constructor
     * 
     * \param endpoint: Connect to this endpoint
    */
    Client(const char* endpoint);

    /**
     * \brief Destructor
    */
    ~Client();

    /**
     * \brief Connects to the endpoint and acquires the text data.
     * 
     * This will set the error, which should be checked using get_error().
     * If that is ErrorType::NONE, then the buffer can be acquired using get_response().
    */
    void query_endpoint();

    /**
     * \brief Returns the buffer acquired from the endpoint
     * 
     * Call this after query_endpoint() to get the response.
    */
    const std::string &get_response() const;

    /**
     * \brief Error codes associated with this class
    */
    enum class ErrorType {
        NONE,       /// No error
        INIT,       /// Initialising connection failed
        QUERY,      /// Failed to query the endpoint
    };

    /**
     * \brief Returns the error encountered during the last operation
    */
    ErrorType get_error() const;
protected:

private:
    CURL* m_curl;                   /// CURL object for performing the query
    const std::string m_enpoint;    /// Endpoint to query
    std::string m_response;         /// Response from querying endpoint
    ErrorType m_error;              /// Last error encountered
};