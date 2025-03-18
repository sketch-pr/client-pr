#pragma once

#include <string>

/**
 * \brief Class to accept parameters, expected outputs and test description
 * 
 * For correct naming of tests when using GTests parameterised test features,
 * GTest will either output a data block, or else a string if it can determine
 * how to name the test. More often than not, this turns out to be useless
 * garbage, so this class encapsulates a means to associate a string description
 * with a test case. The name generator function in GTest should be set like this:
 * 
 *  [](const testing::TestParamInfo<ParamWithDescription<T, X>>& info)
 *  {
 *      return info.param.GetDescription();
 *  }
 * 
 * GTest will then name the test correctly. This is coupled with the PrintTo template
 * which ensures only the parameter is printed when GTest reports the parameter that
 * causes failures.
 * 
 * This class also accepts the expected outputs for comparison in the tests
 * 
 * \param T: Type of parameter input
 * \param X: Type of expected output
 */
template<class T, class X>
class ParamWithDescription
{
public:
    /**
     * \brief Constructor
     * 
     * \param param: Parameter to use in the test
     * \param expected: Expected values to compare with output from code under test
     * \param description: Brief name of test
     */
    ParamWithDescription(T param, X expected, const char* description) :
        m_param(param),
        m_expected(expected),
        m_description(description)
    {};

    /**
     * \brief Returns the parameter
     */
    const T& GetParam() const { return m_param; };

    /**
     * \brief Returns the expected output
     */
    const X& GetExpected() const { return m_expected; };
 
    /**
     * \brief Returns the description (test name)
     */
    const std::string& GetDescription() const { return m_description; };

protected:
    const T m_param;                    /// Parameter for testing
    const X m_expected;                 /// Expected output
    const std::string m_description;    /// Test description
};

/**
 * \brief Used by GTest to output the test parameters of failing tests
 * 
 * Since the ParamWithDescription object is seen by GTest to be the parameter
 * of the tests, this function prevents printing the expected outputs and
 * description on failure, or printing a meaningless block of hex to represent
 * the object itself. The PrintTo function ensures only the parameter is output.
 * 
 * This function is used internally by GTest, and does not need to be invoked
 * manually.
 */
template<class T, class X>
void PrintTo(const ParamWithDescription<T, X>& param, std::ostream* os) {
    *os <<  param.GetParam();
}