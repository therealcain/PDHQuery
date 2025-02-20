#include "pdh.hpp"

#include <iostream>

PdhQuery::PdhQuery()
{
    PDH_HQUERY query;
    const auto status = PdhOpenQuery(nullptr, 0, &query);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "[PDH] Open query has failed.\n";
        throw std::system_error(status, std::system_category());
    }

    hQuery.reset(query);
}

std::expected<void, std::error_code>
PdhQuery::addCounter(const std::wstring &path, const std::string &name)
{
    PDH_HCOUNTER counter;
    const auto status = PdhAddCounterW(hQuery.get(), path.c_str(), 0, &counter);

    if (status != ERROR_SUCCESS)
    {
        std::cerr << "[PDH] Counter creation failed.\n";
        return std::unexpected(std::error_code(status, std::system_category()));
    }

    CounterHandle handle;
    handle.counter.reset(counter);
    handle.path = path;

    counters.emplace(name, std::move(handle));
    return {};
}

std::expected<void, std::error_code>
PdhQuery::collect()
{
    const auto status = PdhCollectQueryData(hQuery.get());
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "[PDH] Collecting query data has failed.\n";
        return std::unexpected(std::error_code(status, std::system_category()));
    }

    ++primeCount;
    if (!primed && primeCount >= 2)
    {
        primed = true;
    }

    return {};
}

std::expected<double, std::error_code>
PdhQuery::getCounterValue(const std::string &name)
{
    if (!primed)
    {
        return std::unexpected(std::make_error_code(std::errc::resource_unavailable_try_again));
    }

    const auto it = counters.find(name);
    if (it == counters.end())
    {
        std::cerr << "[PDH] No counter with name: " << name << "\n";
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    PDH_FMT_COUNTERVALUE value;
    const auto status = PdhGetFormattedCounterValue(it->second.counter.get(), PDH_FMT_DOUBLE, nullptr, &value);
    if (status != ERROR_SUCCESS)
    {
        std::cerr << "[PDH] Failed to get counter value for " << name << "\n";
        return std::unexpected(std::error_code(status, std::system_category()));
    }

    return value.doubleValue;
}