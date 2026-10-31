#include "aritmetica_decimal.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace ada {
namespace {

int compararNormalizados(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return a.size() < b.size() ? -1 : 1;
    }
    if (a == b) {
        return 0;
    }
    return a < b ? -1 : 1;
}

}  // namespace

std::string normalizar(const std::string& numero) {
    if (numero.empty()) {
        throw std::invalid_argument("El numero no puede estar vacio");
    }
    if (!std::all_of(numero.begin(), numero.end(), [](char digito) {
            return digito >= '0' && digito <= '9';
        })) {
        throw std::invalid_argument("El numero solo puede contener digitos");
    }

    const std::size_t primero = numero.find_first_not_of('0');
    return primero == std::string::npos ? "0" : numero.substr(primero);
}

std::string sumar(const std::string& a, const std::string& b) {
    const std::string x = normalizar(a);
    const std::string y = normalizar(b);
    std::string resultado;
    resultado.reserve(std::max(x.size(), y.size()) + 1);

    std::size_t i = x.size();
    std::size_t j = y.size();
    int acarreo = 0;

    while (i > 0 || j > 0 || acarreo > 0) {
        int suma = acarreo;
        if (i > 0) {
            suma += x[--i] - '0';
        }
        if (j > 0) {
            suma += y[--j] - '0';
        }
        resultado.push_back(static_cast<char>('0' + suma % 10));
        acarreo = suma / 10;
    }

    std::reverse(resultado.begin(), resultado.end());
    return resultado;
}

std::string restar(const std::string& a, const std::string& b) {
    const std::string x = normalizar(a);
    const std::string y = normalizar(b);
    if (compararNormalizados(x, y) < 0) {
        throw std::invalid_argument("restar requiere que A sea mayor o igual que B");
    }

    std::string resultado(x.size(), '0');
    std::size_t j = y.size();
    int prestamo = 0;

    for (std::size_t i = x.size(); i > 0; --i) {
        int diferencia = (x[i - 1] - '0') - prestamo;
        if (j > 0) {
            diferencia -= y[--j] - '0';
        }
        if (diferencia < 0) {
            diferencia += 10;
            prestamo = 1;
        } else {
            prestamo = 0;
        }
        resultado[i - 1] = static_cast<char>('0' + diferencia);
    }

    return normalizar(resultado);
}

std::string desplazar(const std::string& a, std::size_t k) {
    const std::string x = normalizar(a);
    if (x == "0") {
        return x;
    }
    return x + std::string(k, '0');
}

std::pair<std::string, std::string> partir(const std::string& a,
                                           std::size_t m) {
    const std::string x = normalizar(a);
    if (m == 0) {
        return {x, "0"};
    }
    if (m >= x.size()) {
        return {"0", x};
    }

    const std::size_t corte = x.size() - m;
    return {normalizar(x.substr(0, corte)), normalizar(x.substr(corte))};
}

std::string multiplicacionClasica(const std::string& a,
                                  const std::string& b) {
    const std::string x = normalizar(a);
    const std::string y = normalizar(b);
    if (x == "0" || y == "0") {
        return "0";
    }

    std::vector<int> digitos(x.size() + y.size(), 0);
    for (std::size_t i = x.size(); i > 0; --i) {
        for (std::size_t j = y.size(); j > 0; --j) {
            const int producto = (x[i - 1] - '0') * (y[j - 1] - '0');
            const int suma = digitos[i + j - 1] + producto;
            digitos[i + j - 1] = suma % 10;
            digitos[i + j - 2] += suma / 10;
        }
    }

    std::string resultado;
    resultado.reserve(digitos.size());
    std::size_t primero = 0;
    while (primero < digitos.size() && digitos[primero] == 0) {
        ++primero;
    }
    for (std::size_t i = primero; i < digitos.size(); ++i) {
        resultado.push_back(static_cast<char>('0' + digitos[i]));
    }
    return resultado.empty() ? "0" : resultado;
}

}  // namespace ada
