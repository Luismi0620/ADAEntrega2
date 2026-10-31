#include "aritmetica_decimal.hpp"
#include "karatsuba.hpp"

#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// Cuenta los productos de un digito del caso base de ada::karatsuba para
// n = 2, 4, ..., 1024, con entradas de solo nueves y aleatorias (semilla
// fija). Imprime la tabla y la pendiente log-log por minimos cuadrados, que
// debe acercarse a log2(3) = 1.585.
//
// Requiere compilar src/karatsuba.cpp y este archivo con
// -DADA_CONTAR_PRODUCTOS (ver Makefile.karatsuba, objetivo conteo).

#ifndef ADA_CONTAR_PRODUCTOS
#error "Compilar con -DADA_CONTAR_PRODUCTOS"
#endif

namespace {

const double kExponente = std::log2(3.0);

// n digitos aleatorios con el primero distinto de cero, para que la
// longitud real sea exactamente n.
std::string digitosAleatorios(std::mt19937& generador, std::size_t n) {
    std::uniform_int_distribution<int> primero(1, 9);
    std::uniform_int_distribution<int> resto(0, 9);
    std::string resultado(n, '0');
    resultado[0] = static_cast<char>('0' + primero(generador));
    for (std::size_t i = 1; i < n; ++i) {
        resultado[i] = static_cast<char>('0' + resto(generador));
    }
    return resultado;
}

// Ejecuta karatsuba, verifica el producto contra la clasica y devuelve la
// cantidad de productos base. Un producto incorrecto invalida el conteo.
std::size_t contar(const std::string& a, const std::string& b, bool& correcto) {
    ada::detalle::reiniciarProductosBase();
    const std::string producto = ada::karatsuba(a, b);
    const std::size_t productos = ada::detalle::productosBaseContados();
    if (producto != ada::multiplicacionClasica(a, b)) {
        correcto = false;
    }
    return productos;
}

// Pendiente de la recta de minimos cuadrados de log(y) contra log(x).
double pendienteLogLog(const std::vector<double>& x,
                       const std::vector<double>& y) {
    double sumaX = 0.0;
    double sumaY = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        sumaX += std::log(x[i]);
        sumaY += std::log(y[i]);
    }
    const double mediaX = sumaX / static_cast<double>(x.size());
    const double mediaY = sumaY / static_cast<double>(x.size());
    double covarianza = 0.0;
    double varianza = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        const double dx = std::log(x[i]) - mediaX;
        covarianza += dx * (std::log(y[i]) - mediaY);
        varianza += dx * dx;
    }
    return covarianza / varianza;
}

}  // namespace

int main() {
    const unsigned semilla = 20261101u;
    std::mt19937 generador(semilla);
    bool correcto = true;

    std::vector<double> tamanos;
    std::vector<double> conteosNueves;
    std::vector<double> conteosAleatorios;

    std::cout << "Productos base de karatsuba (umbral 1), semilla " << semilla
              << "\n\n";
    std::cout << std::left << std::setw(6) << "n" << std::right
              << std::setw(12) << "n^1.585" << std::setw(14) << "nueves"
              << std::setw(10) << "razon" << std::setw(14) << "aleatoria"
              << std::setw(10) << "razon" << '\n';

    for (std::size_t n = 2; n <= 1024; n *= 2) {
        const std::string nueves(n, '9');
        const std::size_t pNueves = contar(nueves, nueves, correcto);
        const std::string a = digitosAleatorios(generador, n);
        const std::string b = digitosAleatorios(generador, n);
        const std::size_t pAleatoria = contar(a, b, correcto);

        // 3^log2(n) = n^log2(3), exacto para n potencia de 2.
        const double teorico = std::pow(3.0, std::log2(static_cast<double>(n)));
        tamanos.push_back(static_cast<double>(n));
        conteosNueves.push_back(static_cast<double>(pNueves));
        conteosAleatorios.push_back(static_cast<double>(pAleatoria));

        std::cout << std::left << std::setw(6) << n << std::right
                  << std::fixed << std::setprecision(0) << std::setw(12)
                  << teorico << std::setw(14) << pNueves
                  << std::setprecision(3) << std::setw(10)
                  << pNueves / teorico << std::setw(14) << pAleatoria
                  << std::setw(10) << pAleatoria / teorico << '\n';
    }

    std::cout << std::setprecision(4) << "\nPendiente log-log (regresion), "
              << "teorica " << kExponente << ":\n"
              << "  nueves:    " << pendienteLogLog(tamanos, conteosNueves)
              << '\n'
              << "  aleatoria: "
              << pendienteLogLog(tamanos, conteosAleatorios) << '\n';
    std::cout << "Productos verificados contra la clasica: "
              << (correcto ? "si" : "NO") << '\n';
    return correcto ? 0 : 1;
}
