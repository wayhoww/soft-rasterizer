#pragma once

#include "common_header.hpp"
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& line, char c);

std::string trimmed(const std::string& str);

struct simple_exception : public std::exception {
    std::string m_what;
    simple_exception(const std::string& what) : m_what(what) {}
    virtual const char* what() const { return m_what.c_str(); }
};
