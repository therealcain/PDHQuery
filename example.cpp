#include <iostream>
#include <exception>
#include "pdh.hpp"

int main() {
    try {
        PdhQuery pdh;
        pdh.addCounter(L"\\Processor(_Total)\\% Processor Time", "cpu_usage");
    
        while (true)
        {
            pdh.collect();
            if (auto cpu = pdh.getCounterValue("cpu_usage"))
            {
                std::cout << "CPU usage: " << *cpu << "%\n";
            }
    
            Sleep(1000);
        }
    }
    catch(const std::exception& e) {
        std::cerr << e.what();
    }
}