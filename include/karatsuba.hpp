#ifndef KARATSUBA_HPP
#define KARATSUBA_HPP

#include <cstddef>
#include <string>
#include <utility>

namespace ada {

// Multiplica dos enteros decimales no negativos de longitud arbitraria con el
// algoritmo de Karatsuba: tres productos de la mitad de tamano por nivel, en
// lugar de los cuatro del metodo escolar.
// Usa la misma representacion que aritmetica_decimal.hpp: std::string en base
// 10 con el digito mas significativo a la izquierda. El resultado queda
// normalizado (sin ceros a la izquierda; el cero se representa como "0").
// Lanza std::invalid_argument si alguna entrada esta vacia o no es decimal.
std::string karatsuba(const std::string& a, const std::string& b);

// Auxiliares del relleno. Se exponen solo para poder probarlos; no forman
// parte de la interfaz fijada por el enunciado.
namespace detalle {

// Menor potencia de 2 que es mayor o igual que n. Para n = 0 devuelve 1.
std::size_t siguientePotenciaDe2(std::size_t n);

// Antepone ceros a la izquierda hasta que el texto tenga exactamente
// `longitud` caracteres. Lanza std::invalid_argument si longitud < a.size(),
// porque recortar cambiaria el valor del numero.
std::string rellenarIzquierda(const std::string& a, std::size_t longitud);

// Rellena ambos operandos con ceros a la izquierda hasta la misma longitud,
// la menor potencia de 2 que cubre al mas largo. No normaliza: espera
// entradas ya normalizadas para que el relleno no crezca de mas.
std::pair<std::string, std::string> igualarLongitudes(const std::string& a,
                                                      const std::string& b);

}  // namespace detalle

}  // namespace ada

#endif
