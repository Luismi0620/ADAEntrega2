#include "aritmetica_decimal.hpp"
#include "karatsuba.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

// Pruebas de ada::karatsuba. Cada caso imprime PASS o FAIL; el programa
// retorna 1 si algun caso falla. Los valores esperados de los casos con
// nombre se calcularon aparte (Python) para no depender solo de
// multiplicacionClasica; los casos masivos se comparan contra ella.

namespace {

int casosTotales = 0;
int casosFallidos = 0;

void reportar(const std::string& nombre, bool correcto,
              const std::string& detalle = "") {
    ++casosTotales;
    if (!correcto) {
        ++casosFallidos;
    }
    std::cout << (correcto ? "PASS " : "FAIL ") << nombre;
    if (!correcto && !detalle.empty()) {
        std::cout << " (" << detalle << ")";
    }
    std::cout << '\n';
}

// Compara karatsuba con un valor esperado conocido.
void casoConocido(const std::string& nombre, const std::string& a,
                  const std::string& b, const std::string& esperado) {
    try {
        const std::string obtenido = ada::karatsuba(a, b);
        reportar(nombre, obtenido == esperado,
                 "obtenido " + obtenido + ", esperado " + esperado);
    } catch (const std::exception& error) {
        reportar(nombre, false, std::string("excepcion: ") + error.what());
    }
}

// Compara karatsuba con multiplicacionClasica; devuelve si coincidieron.
bool coincideConClasica(const std::string& a, const std::string& b) {
    return ada::karatsuba(a, b) == ada::multiplicacionClasica(a, b);
}

void casoContraClasica(const std::string& nombre, const std::string& a,
                       const std::string& b) {
    try {
        reportar(nombre, coincideConClasica(a, b));
    } catch (const std::exception& error) {
        reportar(nombre, false, std::string("excepcion: ") + error.what());
    }
}

template <typename Funcion>
void casoLanzaInvalidArgument(const std::string& nombre, Funcion funcion) {
    try {
        funcion();
        reportar(nombre, false, "no lanzo excepcion");
    } catch (const std::invalid_argument&) {
        reportar(nombre, true);
    } catch (const std::exception& error) {
        reportar(nombre, false, std::string("excepcion de otro tipo: ") +
                                    error.what());
    }
}

// Cadena de n digitos aleatorios; puede empezar con ceros a proposito.
std::string digitosAleatorios(std::mt19937& generador, std::size_t n) {
    std::uniform_int_distribution<int> digito(0, 9);
    std::string resultado(n, '0');
    for (std::size_t i = 0; i < n; ++i) {
        resultado[i] = static_cast<char>('0' + digito(generador));
    }
    return resultado;
}

void pruebasDeRelleno() {
    using ada::detalle::igualarLongitudes;
    using ada::detalle::rellenarIzquierda;
    using ada::detalle::siguientePotenciaDe2;

    reportar("relleno: siguientePotenciaDe2(1) = 1",
             siguientePotenciaDe2(1) == 1);
    reportar("relleno: siguientePotenciaDe2(5) = 8",
             siguientePotenciaDe2(5) == 8);
    reportar("relleno: siguientePotenciaDe2(1024) = 1024",
             siguientePotenciaDe2(1024) == 1024);
    reportar("relleno: siguientePotenciaDe2(1025) = 2048",
             siguientePotenciaDe2(1025) == 2048);
    reportar("relleno: rellenarIzquierda(123, 5) = 00123",
             rellenarIzquierda("123", 5) == "00123");
    casoLanzaInvalidArgument("relleno: rellenarIzquierda(123, 2) lanza",
                             [] { rellenarIzquierda("123", 2); });
    reportar("relleno: igualarLongitudes(7, 12345) usa 8 digitos",
             igualarLongitudes("7", "12345") ==
                 std::make_pair(std::string("00000007"),
                                std::string("00012345")));
}

void pruebasObligatorias() {
    casoConocido("1 digito: 7 * 8", "7", "8", "56");
    casoConocido("1 digito: 9 * 9", "9", "9", "81");
    casoConocido("cero: 0 * 123", "0", "123", "0");
    casoConocido("cero: 000 * 0", "000", "0", "0");
    casoConocido("L=2: 99 * 99", "99", "99", "9801");
    casoConocido("muchos ceros a la izquierda: 0000000000000123 * 000045",
                 "0000000000000123", "000045", "5535");
    casoConocido("ceros intercalados: 1002003 * 40005", "1002003", "40005",
                 "40085130015");
    casoConocido("ceros intercalados: 100200300 * 40005", "100200300",
                 "40005", "4008513001500");
    casoConocido("ceros intercalados: 9000000009 * 7000007", "9000000009",
                 "7000007", "63000063063000063");
    casoConocido("longitudes impares: 12345 (5) * 6789012 (7)", "12345",
                 "6789012", "83810353140");
    casoConocido("longitudes impares: 987 (3) * 6543210 (7)", "987",
                 "6543210", "6458148270");
    casoConocido("mayor que tipos nativos: (10^20 - 1)^2",
                 "99999999999999999999", "99999999999999999999",
                 "9999999999999999999800000000000000000001");

    // (10^300 - 1)^2 = 9...9 (299 nueves) 8 0...0 (299 ceros) 1.
    const std::string nueves300(300, '9');
    casoConocido("cientos de digitos: (10^300 - 1)^2", nueves300, nueves300,
                 std::string(299, '9') + "8" + std::string(299, '0') + "1");

    std::mt19937 generador(20261101u);
    const std::string a400 = "1" + digitosAleatorios(generador, 399);
    const std::string b400 = "1" + digitosAleatorios(generador, 399);
    casoContraClasica("cientos de digitos: aleatorio 400 * 400", a400, b400);

    const std::string a500 = "1" + digitosAleatorios(generador, 499);
    casoConocido("longitudes muy distintas: 1 * (500 digitos)", "1", a500,
                 a500);
    casoContraClasica("longitudes muy distintas: 7 * (500 digitos)", "7",
                      a500);
    casoContraClasica("longitudes muy distintas: (500 digitos) * 7", a500,
                      "7");
}

void pruebasDeError() {
    casoLanzaInvalidArgument("error: primer operando vacio",
                             [] { ada::karatsuba("", "5"); });
    casoLanzaInvalidArgument("error: segundo operando vacio",
                             [] { ada::karatsuba("5", ""); });
    casoLanzaInvalidArgument("error: caracter no numerico (12a)",
                             [] { ada::karatsuba("12a", "5"); });
    casoLanzaInvalidArgument("error: signo negativo (-5)",
                             [] { ada::karatsuba("3", "-5"); });
    casoLanzaInvalidArgument("error: espacio (' 12')",
                             [] { ada::karatsuba(" 12", "5"); });
}

void pruebasMasivas() {
    // Exhaustiva pequena: cubre todas las combinaciones de 1, 2 y 3 digitos
    // en el segundo operando, incluidos ceros y longitudes distintas.
    bool todosCoinciden = true;
    std::string primerFallo;
    try {
        for (int i = 0; i < 100 && todosCoinciden; ++i) {
            for (int j = 0; j < 1000 && todosCoinciden; ++j) {
                const std::string a = std::to_string(i);
                const std::string b = std::to_string(j);
                if (!coincideConClasica(a, b)) {
                    todosCoinciden = false;
                    primerFallo = a + " * " + b;
                }
            }
        }
        reportar("exhaustiva: 0..99 x 0..999 contra clasica", todosCoinciden,
                 "primer fallo en " + primerFallo);
    } catch (const std::exception& error) {
        reportar("exhaustiva: 0..99 x 0..999 contra clasica", false,
                 std::string("excepcion: ") + error.what());
    }

    // Aleatoria con semilla fija: longitudes independientes de 1 a 1000.
    const unsigned semilla = 20261101u;
    const int pares = 20;
    std::mt19937 generador(semilla);
    std::uniform_int_distribution<std::size_t> longitud(1, 1000);
    int coincidencias = 0;
    try {
        for (int t = 0; t < pares; ++t) {
            const std::string a =
                digitosAleatorios(generador, longitud(generador));
            const std::string b =
                digitosAleatorios(generador, longitud(generador));
            if (coincideConClasica(a, b)) {
                ++coincidencias;
            }
        }
        reportar("aleatoria: " + std::to_string(pares) +
                     " pares de 1 a 1000 digitos (semilla " +
                     std::to_string(semilla) + ")",
                 coincidencias == pares,
                 std::to_string(pares - coincidencias) + " pares fallaron");
    } catch (const std::exception& error) {
        reportar("aleatoria: pares de 1 a 1000 digitos", false,
                 std::string("excepcion: ") + error.what());
    }
}

}  // namespace

int main() {
    pruebasDeRelleno();
    pruebasObligatorias();
    pruebasDeError();
    pruebasMasivas();

    std::cout << "\nResumen: " << (casosTotales - casosFallidos) << " de "
              << casosTotales << " casos pasaron.\n";
    return casosFallidos == 0 ? 0 : 1;
}
