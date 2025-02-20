#pragma once

#include <expected>
#include <system_error>
#include <unordered_map>
#include <memory>


#include <windows.h>
#include <pdh.h>
#pragma comment(lib, "pdh.lib")

class PdhQuery {
public:
    struct CounterValue
    {
        double value;
        DWORD itemCount;
        PDH_FMT_COUNTERVALUE_ITEM *items;
    };

public:
    PdhQuery();

    std::expected<void, std::error_code>
    addCounter(const std::wstring &path, const std::string &name);

    std::expected<void, std::error_code>
    collect();

    std::expected<double, std::error_code>
    getCounterValue(const std::string &name);

private:
    struct QueryDeleter {
        void operator()(PDH_HQUERY q) const { 
            PdhCloseQuery(q); 
        }
    };

    struct CounterDeleter {
        void operator()(PDH_HCOUNTER q) const {
            PdhRemoveCounter(q);
        }
    };

    struct CounterHandle
    {
        std::unique_ptr<PDH_HCOUNTER, CounterDeleter> counter;
        std::wstring path;
    };

private:
    std::unique_ptr<PDH_HQUERY, QueryDeleter> hQuery;
    std::unordered_map<std::string, CounterHandle> counters;
    bool primed = false;
    int primeCount = 0;
};