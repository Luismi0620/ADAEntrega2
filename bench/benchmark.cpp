#include "aritmetica_decimal.hpp"
#include "karatsuba.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

// Benchmark de multiplicacionClasica contra karatsuba para n = 2, 4, ...,
// 4096 digitos. Metodologia:
//   1. Mide la resolucion empirica de std::chrono::steady_clock.
//   2. Por tamano, genera operandos aleatorios de exactamente n digitos
//      (semilla fija) y verifica, fuera de la region cronometrada, que ambos
//      algoritmos coinciden. Esa llamada sirve tambien de calentamiento.
//   3. Calibra cuantas multiplicaciones caben en un lote de unos 50 ms.
//   4. Cada medicion repite lotes completos hasta acumular al menos 50 ms y
//      divide el tiempo entre las multiplicaciones hechas. Se toman 5
//      mediciones por algoritmo, alternadas, y se reporta la mediana.
//   5. Cada resultado se acumula en un checksum que se imprime al final, para
//      que el compilador no pueda eliminar las llamadas.
// Uso: benchmark [ruta_csv]   (por defecto results/tiempos.csv)
//
// Serie complementaria: si se compila con -DADA_UMBRAL_CASO_BASE=<k> (y se
// enlaza con el karatsuba_u<k>.o del mismo umbral), el CSV usa las columnas
// digitos,umbral,karatsuba_ns,clasica_ns. El umbral se toma de la macro, no
// de un argumento, para que la etiqueta coincida con el objeto compilado.

#ifdef ADA_CONTAR_PRODUCTOS
#error "El benchmark debe compilarse sin ADA_CONTAR_PRODUCTOS"
#endif

namespace {

typedef std::chrono::steady_clock Reloj;
typedef std::string (*Multiplicacion)(const std::string&, const std::string&);

const unsigned kSemilla = 20261101u;
const std::size_t kTamanoMaximo = 4096;
const int kMediciones = 5;
const std::chrono::nanoseconds kTiempoMinimo = std::chrono::milliseconds(50);

// Consumidor de resultados: depende de la longitud y de un digito de cada
// producto, asi que cada llamada tiene un efecto observable.
unsigned long long checksum = 0;

void consumir(const std::string& resultado) {
    checksum = checksum * 31 + resultado.size() +
               static_cast<unsigned char>(resultado[resultado.size() / 2]);
}

long long nanosegundos(Reloj::duration duracion) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duracion)
        .count();
}

// Resolucion empirica: menor salto observable entre dos lecturas distintas
// consecutivas del reloj, y mediana de esos saltos.
void medirResolucion(long long& minimo, long long& mediana) {
    const int muestras = 10000;
    std::vector<long long> saltos;
    saltos.reserve(muestras);
    for (int i = 0; i < muestras; ++i) {
        const Reloj::time_point inicio = Reloj::now();
        Reloj::time_point actual = Reloj::now();
        while (actual == inicio) {
            actual = Reloj::now();
        }
        saltos.push_back(nanosegundos(actual - inicio));
    }
    std::sort(saltos.begin(), saltos.end());
    minimo = saltos.front();
    mediana = saltos[saltos.size() / 2];
}

// n digitos aleatorios con el primero distinto de cero.
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

// Ejecuta `repeticiones` multiplicaciones y devuelve el tiempo del lote.
Reloj::duration lote(Multiplicacion multiplicar, const std::string& a,
                     const std::string& b, long long repeticiones) {
    const Reloj::time_point inicio = Reloj::now();
    for (long long i = 0; i < repeticiones; ++i) {
        consumir(multiplicar(a, b));
    }
    return Reloj::now() - inicio;
}

// Cantidad de repeticiones para que un lote dure unos 50 ms. Duplica hasta
// que el lote dure al menos 5 ms y luego extrapola.
long long calibrar(Multiplicacion multiplicar, const std::string& a,
                   const std::string& b) {
    long long repeticiones = 1;
    for (;;) {
        const long long t = nanosegundos(lote(multiplicar, a, b, repeticiones));
        if (t >= kTiempoMinimo.count()) {
            return repeticiones;
        }
        if (t >= kTiempoMinimo.count() / 10) {
            return std::max(1LL, repeticiones * kTiempoMinimo.count() / t + 1);
        }
        repeticiones *= 2;
    }
}

// Una medicion: lotes completos hasta acumular al menos 50 ms. El reloj se
// lee una vez por lote, no por multiplicacion. Devuelve ns por llamada.
double medir(Multiplicacion multiplicar, const std::string& a,
             const std::string& b, long long repeticiones) {
    long long hechas = 0;
    const Reloj::time_point inicio = Reloj::now();
    Reloj::duration transcurrido;
    do {
        for (long long i = 0; i < repeticiones; ++i) {
            consumir(multiplicar(a, b));
        }
        hechas += repeticiones;
        transcurrido = Reloj::now() - inicio;
    } while (transcurrido < kTiempoMinimo);
    return static_cast<double>(nanosegundos(transcurrido)) /
           static_cast<double>(hechas);
}

double mediana(std::vector<double> valores) {
    std::sort(valores.begin(), valores.end());
    return valores[valores.size() / 2];
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string rutaCsv = argc > 1 ? argv[1] : "results/tiempos.csv";
    std::ofstream csv(rutaCsv.c_str());
    if (!csv) {
        std::cerr << "No se pudo abrir " << rutaCsv
                  << " (crear antes la carpeta results/)\n";
        return 1;
    }
#ifdef ADA_UMBRAL_CASO_BASE
    csv << "digitos,umbral,karatsuba_ns,clasica_ns\n";
#else
    csv << "digitos,clasica_ns,karatsuba_ns\n";
#endif

    const Reloj::time_point inicioTotal = Reloj::now();

    long long resolucionMinima = 0;
    long long resolucionMediana = 0;
    medirResolucion(resolucionMinima, resolucionMediana);
    std::cout << "Resolucion steady_clock: minima " << resolucionMinima
              << " ns, mediana " << resolucionMediana
              << " ns (periodo declarado " << Reloj::period::num << "/"
              << Reloj::period::den << " s)\n";
    std::cout << "Semilla " << kSemilla << ", " << kMediciones
              << " mediciones por algoritmo, minimo "
              << kTiempoMinimo.count() / 1000000 << " ms por medicion\n";
#ifdef ADA_UMBRAL_CASO_BASE
    std::cout << "Umbral del caso base de karatsuba: " << ADA_UMBRAL_CASO_BASE
              << " digitos\n";
#endif
    std::cout << '\n';

    std::cout << std::left << std::setw(8) << "digitos" << std::right
              << std::setw(14) << "clasica_ns" << std::setw(10) << "reps"
              << std::setw(16) << "karatsuba_ns" << std::setw(10) << "reps"
              << std::setw(10) << "kar/clas" << '\n';

    std::mt19937 generador(kSemilla);
    for (std::size_t n = 2; n <= kTamanoMaximo; n *= 2) {
        const std::string a = digitosAleatorios(generador, n);
        const std::string b = digitosAleatorios(generador, n);

        // Verificacion y calentamiento, fuera de la region cronometrada.
        const std::string esperado = ada::multiplicacionClasica(a, b);
        if (ada::karatsuba(a, b) != esperado) {
            std::cerr << "ERROR: karatsuba difiere de la clasica en n = " << n
                      << '\n';
            return 1;
        }
        consumir(esperado);

        const long long repsClasica =
            calibrar(ada::multiplicacionClasica, a, b);
        const long long repsKaratsuba = calibrar(ada::karatsuba, a, b);

        std::vector<double> clasica;
        std::vector<double> karatsuba;
        for (int i = 0; i < kMediciones; ++i) {
            clasica.push_back(
                medir(ada::multiplicacionClasica, a, b, repsClasica));
            karatsuba.push_back(medir(ada::karatsuba, a, b, repsKaratsuba));
        }
        const double nsClasica = mediana(clasica);
        const double nsKaratsuba = mediana(karatsuba);

#ifdef ADA_UMBRAL_CASO_BASE
        csv << n << ',' << ADA_UMBRAL_CASO_BASE << ',' << std::fixed
            << std::setprecision(1) << nsKaratsuba << ',' << nsClasica << '\n';
#else
        csv << n << ',' << std::fixed << std::setprecision(1) << nsClasica
            << ',' << nsKaratsuba << '\n';
#endif
        std::cout << std::left << std::setw(8) << n << std::right << std::fixed
                  << std::setprecision(1) << std::setw(14) << nsClasica
                  << std::setw(10) << repsClasica << std::setw(16)
                  << nsKaratsuba << std::setw(10) << repsKaratsuba
                  << std::setprecision(3) << std::setw(10)
                  << nsKaratsuba / nsClasica << std::endl;
    }

    const double totalSegundos =
        static_cast<double>(nanosegundos(Reloj::now() - inicioTotal)) / 1e9;
    std::cout << "\nChecksum: " << checksum << '\n'
              << "Verificacion karatsuba == clasica: todos los tamanos\n"
              << "Tiempo total: " << std::setprecision(1) << totalSegundos
              << " s\n"
              << "CSV: " << rutaCsv << '\n';
    return 0;
}
