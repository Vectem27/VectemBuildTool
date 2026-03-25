#pragma once

#include <string>

class PrintSame
{
public:
    PrintSame() = default;

    void Print(const std::string& str) const;

    std::string GiveInline(const std::string& str) const
    {
        return str;
    }
};