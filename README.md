# Práctica 2 - Multiplicación de Karatsuba

Implementación en C++ de aritmética decimal de longitud arbitraria para la
práctica de Divide y Vencerás. Esta rama contiene el componente asignado a
Luis: multiplicación clásica y funciones auxiliares compatibles con la futura
integración de Karatsuba.

## Representación y contratos

Los enteros no negativos se representan con `std::string` en base 10, con el
dígito más significativo a la izquierda. Ninguna operación convierte el
número completo a `long long`, `__int128` ni a una biblioteca de precisión
arbitraria.

La interfaz pública está en `include/aritmetica_decimal.hpp`:

- `sumar(A, B)`: acepta longitudes distintas y acarreo final.
- `restar(A, B)`: exige `A >= B`; en otro caso lanza
  `std::invalid_argument`.
- `desplazar(A, k)`: calcula `A * 10^k` añadiendo ceros a la derecha.
- `partir(A, m)`: devuelve las partes alta y baja de
  `A = alta * 10^m + baja`.
- `multiplicacionClasica(A, B)`: algoritmo escolar sobre los dígitos, con
  complejidad temporal `Theta(n*m)` y `Theta(n^2)` cuando ambos operandos
  tienen `n` dígitos.

Todas las salidas quedan normalizadas: no conservan ceros a la izquierda y el
cero se representa como `"0"`. Las entradas vacías o no decimales se rechazan.

## Compilación y ejecución

Se requiere un compilador compatible con C++17. Con GNU Make y `g++`:

```bash
make
./bin/clasica 12345678901234567890 987654321
```

Compilación directa (útil en Windows si no se dispone de `make`):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude src/main.cpp src/aritmetica_decimal.cpp -o clasica
./clasica 12345 6789
```

El programa imprime únicamente el producto normalizado. Por ejemplo, la
última ejecución imprime `83810205`.

## Pruebas

```bash
make test
```

O mediante compilación directa:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude tests/test_aritmetica_decimal.cpp src/aritmetica_decimal.cpp -o pruebas
./pruebas
```

Las pruebas cubren longitudes distintas, acarreos y préstamos encadenados,
ceros a la izquierda, ceros intercalados, un producto mayor que los tipos
nativos y entradas que incumplen las precondiciones.

## Uso de inteligencia artificial

Se utilizó OpenAI Codex como apoyo para estructurar la implementación, revisar
casos de borde y redactar documentación. El equipo debe revisar y comprender
cada decisión, línea de código y explicación antes de la entrega y la defensa
individual.
