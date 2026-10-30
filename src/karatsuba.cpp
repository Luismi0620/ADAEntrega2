#include "karatsuba.hpp"

#include "aritmetica_decimal.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace ada {
namespace {

// Nucleo recursivo (Fase 3). Mientras tanto lanza una excepcion para que
// ninguna prueba pase por accidente.
std::string karatsubaRecursivo(const std::string& x, const std::string& y) {
    (void)x;
    (void)y;
    throw std::logic_error("karatsuba: nucleo recursivo aun no implementado");
}

}  // namespace

namespace detalle {

std::size_t siguientePotenciaDe2(std::size_t n) {
    std::size_t potencia = 1;
    while (potencia < n) {
        potencia *= 2;
    }
    return potencia;
}

std::string rellenarIzquierda(const std::string& a, std::size_t longitud) {
    if (longitud < a.size()) {
        throw std::invalid_argument(
            "rellenarIzquierda requiere longitud >= tamano del numero");
    }
    return std::string(longitud - a.size(), '0') + a;
}

std::pair<std::string, std::string> igualarLongitudes(const std::string& a,
                                                      const std::string& b) {
    const std::size_t n = siguientePotenciaDe2(std::max(a.size(), b.size()));
    return {rellenarIzquierda(a, n), rellenarIzquierda(b, n)};
}

}  // namespace detalle

// Envoltorio publico: valida, rellena una sola vez y delega en el nucleo.
// El relleno no se repite en la recursion: los operandos de P3 tienen hasta
// m + 1 digitos y, si se rellenaran de nuevo a potencia de 2, volverian a
// medir n, de modo que esa rama no se reduciria.
std::string karatsuba(const std::string& a, const std::string& b) {
    // normalizar valida (lanza std::invalid_argument) y quita ceros a la
    // izquierda, para que el relleno dependa del valor y no del texto.
    const std::string x = normalizar(a);
    const std::string y = normalizar(b);
    if (x == "0" || y == "0") {
        return "0";
    }

    const std::pair<std::string, std::string> rellenos =
        detalle::igualarLongitudes(x, y);
    return karatsubaRecursivo(rellenos.first, rellenos.second);
}

}  // namespace ada
