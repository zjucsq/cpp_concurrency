//
// Created by csq on 2/2/23.
//
#include <iostream>
#include <memory>

int main()
{
    std::unique_ptr<int> p = std::make_unique<int>(4);
    std::cout << *p << std::endl;
    std::unique_ptr<int> p1;
    // std::cout << *p1 << std::endl;
    std::cout << (bool)p1 << (p1 == nullptr) << std::endl;
}