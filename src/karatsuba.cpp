#include "karatsuba.hpp"

#include "aritmetica_decimal.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

namespace ada {
namespace {

// Umbral del caso base: si el operando mas largo tiene a lo sumo esta
// cantidad de digitos, se multiplica directamente. Con 1, el caso base es el
// producto de dos digitos.
constexpr std::size_t kUmbralCasoBase = 1;

// Verdadero si el texto representa el cero ("0", "000") o esta vacio.
bool esCero(const std::string& a) {
    return a.find_first_not_of('0') == std::string::npos;
}

// partir devuelve partes normalizadas ("0" para el cero); por robustez, una
// parte vacia se trata tambien como cero.
std::string ceroSiVacia(const std::string& a) {
    return a.empty() ? "0" : a;
}

#ifdef ADA_CONTAR_PRODUCTOS
// Productos de un digito hechos en el caso base. Solo existe al compilar con
// -DADA_CONTAR_PRODUCTOS; en la compilacion normal no hay variable ni
// incremento, y el codigo objeto es el mismo que sin contador.
std::size_t productosBase = 0;
#endif

// Caso base. Dos digitos se multiplican con int (el producto es a lo sumo
// 81); eso esta permitido porque solo opera sobre digitos individuales. Si el
// umbral fuera mayor que 1, se usa el metodo escolar.
std::string multiplicacionDirecta(const std::string& x, const std::string& y) {
    if (x.size() == 1 && y.size() == 1) {
#ifdef ADA_CONTAR_PRODUCTOS
        ++productosBase;
#endif
        return std::to_string((x[0] - '0') * (y[0] - '0'));
    }
#ifdef ADA_CONTAR_PRODUCTOS
    // El metodo escolar hace exactamente |x| * |y| productos de un digito.
    productosBase += x.size() * y.size();
#endif
    return multiplicacionClasica(x, y);
}

// Nucleo recursivo. Con L = max(|x|, |y|) y m = floor(L/2):
//   x = x1*10^m + x0,  y = y1*10^m + y0
//   P1 = x1*y1,  P2 = x0*y0,  P3 = (x1 + x0)*(y1 + y0)
//   x*y = P1*10^(2m) + (P3 - P1 - P2)*10^m + P2
// Se usa 2m y no L porque, si L es impar, 2m = L - 1.
//
// Longitudes variables: desde el segundo nivel partir entrega mitades
// normalizadas, asi que x e y pueden tener longitudes distintas y no
// potencias de 2. Por eso L se toma del operando mas largo. Las sumas
// x1 + x0 e y1 + y0 pueden tener m + 1 (o ceil(L/2) + 1) digitos; se pasan
// tal cual, sin rellenar ni recortar.
//
// No negatividad: P3 = P1 + P2 + x1*y0 + x0*y1, de donde
//   P3 - P1 = P2 + x1*y0 + x0*y1 >= P2 >= 0.
// Restar primero P1 y luego P2 cumple siempre la precondicion A >= B de
// restar, por lo que nunca lanza.
//
// Terminacion: para L >= 2 se tiene m >= 1.
//   - P1 y P2: sus operandos tienen a lo sumo ceil(L/2) < L digitos.
//   - P3: sus operandos tienen a lo sumo ceil(L/2) + 1 digitos, que es < L
//     cuando L >= 4. Para L = 2 o 3 la longitud puede no bajar (99 -> 9 + 9
//     = 18), pero baja el valor: el operando mas largo tiene mas de m
//     digitos, luego su parte alta es >= 1, y x - (x1 + x0) =
//     x1*(10^m - 1) >= 9. Asi, x + y decrece estrictamente.
//   En todas las llamadas el par (L, x + y) decrece en orden lexicografico,
//   y como ambos son naturales la recursion termina.
std::string karatsubaRecursivo(const std::string& x, const std::string& y) {
    if (esCero(x) || esCero(y)) {
        return "0";
    }

    const std::size_t longitud = std::max(x.size(), y.size());
    if (longitud <= kUmbralCasoBase) {
        return multiplicacionDirecta(x, y);
    }

    const std::size_t m = longitud / 2;
    const std::pair<std::string, std::string> partesX = partir(x, m);
    const std::pair<std::string, std::string> partesY = partir(y, m);
    const std::string x1 = ceroSiVacia(partesX.first);
    const std::string x0 = ceroSiVacia(partesX.second);
    const std::string y1 = ceroSiVacia(partesY.first);
    const std::string y0 = ceroSiVacia(partesY.second);

    const std::string p1 = karatsubaRecursivo(x1, y1);
    const std::string p2 = karatsubaRecursivo(x0, y0);
    const std::string p3 = karatsubaRecursivo(sumar(x1, x0), sumar(y1, y0));

    const std::string central = restar(restar(p3, p1), p2);
    return sumar(sumar(desplazar(p1, 2 * m), desplazar(central, m)), p2);
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

#ifdef ADA_CONTAR_PRODUCTOS
std::size_t productosBaseContados() {
    return productosBase;
}

void reiniciarProductosBase() {
    productosBase = 0;
}
#endif

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
