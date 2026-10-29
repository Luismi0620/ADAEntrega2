#ifndef ARITMETICA_DECIMAL_HPP
#define ARITMETICA_DECIMAL_HPP

#include <cstddef>
#include <string>
#include <utility>

namespace ada {

// Elimina ceros a la izquierda. El cero siempre se representa como "0".
// Lanza std::invalid_argument si el texto esta vacio o contiene caracteres
// que no son digitos decimales.
std::string normalizar(const std::string& numero);

// Operaciones sobre enteros decimales no negativos de longitud arbitraria.
std::string sumar(const std::string& a, const std::string& b);
std::string restar(const std::string& a, const std::string& b);
std::string desplazar(const std::string& a, std::size_t k);

// Separa a = parte_alta * 10^m + parte_baja.
// first es la parte alta y second la parte baja; ambas quedan normalizadas.
std::pair<std::string, std::string> partir(const std::string& a,
                                           std::size_t m);

// Multiplicacion escolar Theta(n*m), donde n y m son las longitudes de los
// operandos. No convierte el numero completo a un tipo nativo.
std::string multiplicacionClasica(const std::string& a,
                                  const std::string& b);

}  // namespace ada

#endif
