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

## Karatsuba, validación y experimentación

Esta sección documenta la multiplicación de Karatsuba, sus pruebas, el conteo
de productos, el benchmark y el análisis del punto de cruce. Usa la
representación y las funciones auxiliares descritas arriba.

### Requisitos

- Linux o WSL (probado en WSL Ubuntu-22.04). El `Makefile` de la sección
  anterior usa sintaxis de Windows; para esta parte se usa
  `Makefile.karatsuba`, que es para Linux.
- `g++` con C++11 o superior. Por defecto se compila con `-std=c++17`; para
  otro estándar: `make -f Makefile.karatsuba CXXFLAGS="-std=c++11 -O2 -Wall -Wextra -pedantic" ...`.
- GNU Make.
- `python3` (solo biblioteca estándar para el análisis) y matplotlib para las
  gráficas: `sudo apt install python3-matplotlib` o `pip install matplotlib`.

### Estructura

| Archivo o carpeta | Contenido |
|---|---|
| `include/karatsuba.hpp` | Declaración de `ada::karatsuba` y de los auxiliares de relleno (`ada::detalle`). |
| `src/karatsuba.cpp` | Envoltorio público, relleno y núcleo recursivo. |
| `tests/test_karatsuba.cpp` | 31 casos: relleno, casos obligatorios, errores, exhaustiva 0..99 x 0..999 y aleatorios con semilla fija. |
| `bench/contar_productos.cpp` | Conteo de productos del caso base (requiere `ADA_CONTAR_PRODUCTOS`). |
| `bench/benchmark.cpp` | Medición de tiempos con `std::chrono::steady_clock`. |
| `bench/registrar_entorno.sh` | Registro del entorno de medición en `results/entorno.txt`. |
| `bench/analizar.py` | Pendientes, cruce medido y extrapolado; escribe `results/analisis_cruce.md`. |
| `bench/graficar.py` | Gráficas `results/grafica_tiempos.png` y `results/grafica_razon.png`. |
| `results/` | Datos medidos, salidas completas, entorno, análisis y gráficas. |
| `Makefile.karatsuba` | Compilación y ejecución en Linux; los binarios van a `build/`. |

### Comandos

Todos se ejecutan desde la raíz del repositorio.

```bash
# Pruebas con umbral 1 (31 casos; retorna distinto de cero si alguno falla)
make -f Makefile.karatsuba ejecutar-pruebas

# Pruebas con umbrales 16, 32 y 64
make -f Makefile.karatsuba ejecutar-pruebas-umbrales

# Conteo de productos del caso base (n = 2..1024)
make -f Makefile.karatsuba ejecutar-conteo | tee results/conteo_productos.txt

# Benchmark principal, umbral 1 (escribe tiempos.csv, benchmark_salida.txt y entorno.txt)
make -f Makefile.karatsuba clean
make -f Makefile.karatsuba ejecutar-benchmark ENERGIA="conectado a la corriente (declarado por el usuario)"

# Serie complementaria por umbral (escribe tiempos_umbral.csv y benchmark_umbral_salida.txt)
make -f Makefile.karatsuba ejecutar-benchmark-umbrales

# Análisis y gráficas a partir de los CSV de results/
make -f Makefile.karatsuba analisis
```

Antes de medir conviene ejecutar `make -f Makefile.karatsuba clean`: en
carpetas de Windows montadas en WSL (`/mnt/c`), `make` puede detectar desfase
de reloj y omitir recompilaciones. Durante la medición, el equipo debe estar
conectado a la corriente, sin suspenderse y sin aplicaciones pesadas abiertas.

### Decisiones de diseño

- Relleno con ceros a la izquierda hasta una potencia de 2 solo en el envoltorio público; rellenar en cada nivel impediría que la rama de P3 se reduzca.
- En el núcleo, m = floor(L/2) con L = longitud mayor, porque `partir` normaliza las mitades y las longitudes varían desde el segundo nivel.
- El término central se calcula como (P3 - P1) - P2; P3 - P1 = P2 + x1·y0 + x0·y1 ≥ P2, así que `restar` nunca recibe A < B.
- Caso base de 1 dígito por defecto; umbral configurable en compilación con `-DADA_UMBRAL_CASO_BASE=<k>`, que usa la multiplicación clásica por debajo del umbral.
- Contador de productos del caso base solo con `-DADA_CONTAR_PRODUCTOS`; sin la macro, el código objeto es idéntico byte a byte.

### Resultados

Cifras de `results/analisis_cruce.md` (tiempos de la corrida del 2026-10-30
con el equipo conectado):

| Serie | Pendiente log-log en n ≥ 256 | Cruce con la clásica |
|---|---|---|
| Clásica | 2.002 (teórica 2) | - |
| Karatsuba, umbral 1 | 1.601 (teórica 1.585) | No aparece hasta 4096 dígitos (razón 3.603); extrapolado entre 1.01 x 10^5 y 1.10 x 10^5 dígitos |
| Karatsuba, umbral 16 | 1.622 | Medido en n = 256 |
| Karatsuba, umbral 32 | 1.636 | Medido en n = 128 |
| Karatsuba, umbral 64 | 1.636 | Medido en n = 128 |

- El conteo de productos del caso base crece con pendiente local 1.585 entre
  512 y 1024 dígitos, como predice T(n) = 3T(n/2) + Θ(n).
- Con umbral 1, el costo fijo de cada nodo (copias y normalizaciones de
  cadenas) aleja el cruce; un umbral mayor reduce la constante sin cambiar el
  exponente.
- La extrapolación del cruce con umbral 1 es sensible a la diferencia de
  pendientes: con un error de 0.05, el cruce se mueve entre 7.01 x 10^4 y
  1.57 x 10^5 dígitos.
- Gráficas: `results/grafica_tiempos.png` (tiempos en escala log-log, con
  rectas de referencia y cruces marcados) y `results/grafica_razon.png`
  (razón Karatsuba / clásica por umbral).

### Condiciones de medición

- CPU: AMD Ryzen 7 7730U, WSL2 Ubuntu 22.04.5 LTS.
- Compilador: g++ 11.4.0, banderas `-std=c++17 -O2 -Wall -Wextra -pedantic`.
- Estado de energía: conectado a la corriente (declarado por el usuario).
- Protocolo: semilla 20261101, operandos aleatorios de exactamente n dígitos,
  verificación contra la clásica fuera de la región cronometrada, al menos
  50 ms por medición y mediana de 5 mediciones. El detalle está en
  `results/entorno.txt`.
- `results/repeticion_principal/` y `results/repeticion_umbral/` contienen
  repeticiones de ambas series como evidencia de reproducibilidad.
- `results/fase6_bateria/` contiene una corrida anterior con un estado de
  energía más lento (inferido, no registrado). Solo ilustra el efecto del
  entorno y no se usa para las conclusiones.
