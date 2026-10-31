#include "aritmetica_decimal.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

template <typename Funcion>
void debeLanzarInvalidArgument(Funcion funcion) {
    bool lanzo = false;
    try {
        funcion();
    } catch (const std::invalid_argument&) {
        lanzo = true;
    }
    assert(lanzo);
}

}  // namespace

int main() {
    assert(ada::normalizar("0000") == "0");
    assert(ada::normalizar("00012030") == "12030");

    assert(ada::sumar("0", "0") == "0");
    assert(ada::sumar("999", "1") == "1000");
    assert(ada::sumar("00025", "975") == "1000");

    assert(ada::restar("1000", "1") == "999");
    assert(ada::restar("1000", "1000") == "0");
    assert(ada::restar("001005", "5") == "1000");
    debeLanzarInvalidArgument([] { ada::restar("4", "5"); });

    assert(ada::desplazar("123", 0) == "123");
    assert(ada::desplazar("00123", 3) == "123000");
    assert(ada::desplazar("0", 100) == "0");

    assert(ada::partir("123456", 3) ==
           std::make_pair(std::string("123"), std::string("456")));
    assert(ada::partir("120003", 3) ==
           std::make_pair(std::string("120"), std::string("3")));
    assert(ada::partir("123", 5) ==
           std::make_pair(std::string("0"), std::string("123")));
    assert(ada::partir("000123", 0) ==
           std::make_pair(std::string("123"), std::string("0")));

    assert(ada::multiplicacionClasica("0", "999999") == "0");
    assert(ada::multiplicacionClasica("7", "8") == "56");
    assert(ada::multiplicacionClasica("0000123", "00456") == "56088");
    assert(ada::multiplicacionClasica("1002003", "40005") == "40085130015");
    assert(ada::multiplicacionClasica("99999999999999999999",
                                      "99999999999999999999") ==
           "9999999999999999999800000000000000000001");

    debeLanzarInvalidArgument([] { ada::sumar("", "1"); });
    debeLanzarInvalidArgument([] { ada::multiplicacionClasica("12a", "3"); });

    std::cout << "Todas las pruebas de aritmetica decimal pasaron.\n";
    return 0;
}
