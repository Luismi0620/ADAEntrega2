#include "aritmetica_decimal.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <entero-no-negativo> <entero-no-negativo>\n";
        return 1;
    }

    try {
        std::cout << ada::multiplicacionClasica(argv[1], argv[2]) << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 2;
    }
    return 0;
}
