# Práctica 2 - Multiplicación de Karatsuba

Práctica 2 de Análisis y Diseño de Algoritmos (EAFIT), sobre Divide y
Vencerás. Implementa en C++ la multiplicación clásica y la multiplicación de
Karatsuba sobre enteros no negativos de longitud arbitraria, representados
como cadenas de dígitos en base 10. Incluye pruebas, conteo de productos,
medición de tiempos y análisis del punto de cruce entre ambos algoritmos.

Integrantes y parte que lideró cada uno:

- **Luis Miguel Mira Mejia:** multiplicación clásica, funciones auxiliares de
  aritmética decimal (`sumar`, `restar`, `desplazar`, `partir`, `normalizar`),
  programa de consola y sus pruebas.
- **Emmanuel Castaño Sepúlveda:** multiplicación de Karatsuba (relleno a
  potencia de 2 y caso base), validación contra la clásica, conteo de
  productos, benchmark, punto de cruce, gráficas y `Makefile.karatsuba`.

El informe está en `docs/Practica2_Karatsuba_Informe.pdf`. Las notas de uso
de inteligencia artificial de cada integrante están en `docs/uso_ia_luis.md`
y `docs/uso_ia_emmanuel.md`.

## Requisitos

- Sistema operativo Linux (probado en Ubuntu 22.04 sobre WSL2).
- Compilador C++ compatible con C++11 o superior. Todo el código (`src/`,
  `tests/`, `bench/`) compila sin advertencias con `-std=c++11` y con
  `-std=c++17`. Probado con g++ 11.4.0; por defecto se compila con
  `-std=c++17`.
- GNU Make (probado con 4.3).
- `python3` (probado con 3.10.12) y matplotlib (probado con 3.5.1) para las
  gráficas. El análisis solo usa la biblioteca estándar.

Instalación en Ubuntu:

```bash
sudo apt install -y g++ make python3-matplotlib
```

Para compilar con otro estándar se sobrescribe `CXXFLAGS`, por ejemplo:

```bash
make -f Makefile.karatsuba clean
make -f Makefile.karatsuba ejecutar-todas-pruebas CXXFLAGS="-std=c++11 -O2 -Wall -Wextra -pedantic"
```

## Ejecución rápida

Todos los comandos se ejecutan desde la raíz del repositorio. El punto de
entrada oficial es `Makefile.karatsuba`; los binarios se generan en `build/`.

```bash
make -f Makefile.karatsuba ejecutar-todas-pruebas
make -f Makefile.karatsuba ejecutar-benchmark ENERGIA="conectado a la corriente"
make -f Makefile.karatsuba analisis
```

Aviso: ejecutar-benchmark, ejecutar-benchmark-umbrales, ejecutar-conteo y analisis sobrescriben results/; los datos del informe corresponden a la corrida del 2026-10-30. Para restaurarlos: git restore results/

- `ejecutar-todas-pruebas`: compila y ejecuta las pruebas de la clásica y
  luego las de Karatsuba. Termina con error si alguna falla. Debe mostrar
  `Todas las pruebas de aritmetica decimal pasaron.`, luego los 31 casos
  `PASS` de Karatsuba y, al final, `Resumen: 31 de 31 casos pasaron.`
- `ejecutar-benchmark`: mide tiempos (entre 10 y 15 s) y sobrescribe
  `results/tiempos.csv`, `results/benchmark_salida.txt` y
  `results/entorno.txt`. `ENERGIA` declara el estado de energía del equipo
  en `results/entorno.txt`; si se omite, queda `no declarado`.
- `analisis`: recalcula el análisis y las gráficas a partir de los CSV de
  `results/`.

Antes de medir conviene ejecutar `make -f Makefile.karatsuba clean`: en
carpetas de Windows montadas en WSL (`/mnt/c`), `make` puede detectar desfase
de reloj y omitir recompilaciones.

## Comandos por código

Todos los comandos se ejecutan desde la raíz del repositorio. Cada comando
`g++` es equivalente al objetivo de `make` de la misma sección; los de esta
sección escriben en `build/`, que se crea con:

```bash
mkdir -p build
```

### Multiplicación clásica y funciones auxiliares (Luis)

**Programa de consola** (`src/main.cpp`). Multiplica con la clásica los dos
enteros dados como argumentos e imprime solo el producto normalizado. Genera
`build/clasica`.

```bash
make -f Makefile.karatsuba programa-clasica
./build/clasica 12345 6789
```

Equivalente con `g++`:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude src/main.cpp src/aritmetica_decimal.cpp -o build/clasica
./build/clasica 12345 6789
```

Salida esperada: `83810205`.

**Pruebas** (`tests/test_aritmetica_decimal.cpp`). Usan `assert`: si una
falla, el programa aborta con un código distinto de cero. Generan
`build/test_aritmetica_decimal`.

```bash
make -f Makefile.karatsuba ejecutar-pruebas-clasica
```

Equivalente con `g++`:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude tests/test_aritmetica_decimal.cpp src/aritmetica_decimal.cpp -o build/test_aritmetica_decimal
./build/test_aritmetica_decimal
```

Salida esperada: `Todas las pruebas de aritmetica decimal pasaron.`

`make -f Makefile.karatsuba pruebas-clasica` solo compila las pruebas, sin
ejecutarlas.

### Karatsuba (Emmanuel)

**Pruebas** (`tests/test_karatsuba.cpp`, umbral del caso base 1). 31 casos;
imprime `PASS` o `FAIL` por caso y retorna distinto de cero si alguno falla.
Genera `build/test_karatsuba`.

```bash
make -f Makefile.karatsuba ejecutar-pruebas
```

Equivalente con `g++`:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude tests/test_karatsuba.cpp src/karatsuba.cpp src/aritmetica_decimal.cpp -o build/test_karatsuba
./build/test_karatsuba
```

Salida esperada (última línea): `Resumen: 31 de 31 casos pasaron.`

**Pruebas por umbral.** Las mismas pruebas con el caso base resuelto por la
clásica por debajo de 16, 32 y 64 dígitos. Generan
`build/test_karatsuba_u16`, `build/test_karatsuba_u32` y
`build/test_karatsuba_u64`, y muestran solo los fallos y el resumen.

```bash
make -f Makefile.karatsuba ejecutar-pruebas-umbrales
```

Equivalente con `g++` para un umbral (16):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude -DADA_UMBRAL_CASO_BASE=16 tests/test_karatsuba.cpp src/karatsuba.cpp src/aritmetica_decimal.cpp -o build/test_karatsuba_u16
./build/test_karatsuba_u16
```

Salida esperada: `== umbral 16`, `== umbral 32` y `== umbral 64`, cada uno
seguido de `Resumen: 31 de 31 casos pasaron.`

**Conteo de productos del caso base** (`bench/contar_productos.cpp`, n = 2
a 1024). Se compila con `-DADA_CONTAR_PRODUCTOS` en un objeto aparte, para
que pruebas y benchmark nunca enlacen el contador. Genera
`build/contar_productos` y sobrescribe `results/conteo_productos.txt`.

```bash
make -f Makefile.karatsuba ejecutar-conteo | tee results/conteo_productos.txt
```

Equivalente con `g++` (no escribe en `results/`):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude -DADA_CONTAR_PRODUCTOS bench/contar_productos.cpp src/karatsuba.cpp src/aritmetica_decimal.cpp -o build/contar_productos
./build/contar_productos
```

Salida esperada (última línea): `Productos verificados contra la clasica: si`.

**Benchmark principal** (`bench/benchmark.cpp`, umbral 1). Mide la clásica
y Karatsuba para cada tamaño (entre 10 y 15 s). Genera `build/benchmark` y
sobrescribe `results/tiempos.csv`, `results/benchmark_salida.txt` y
`results/entorno.txt`. `ENERGIA` declara el estado de energía del equipo; no
se detecta, y si se omite queda `no declarado`.

Aviso: ejecutar-benchmark, ejecutar-benchmark-umbrales, ejecutar-conteo y analisis sobrescriben results/; los datos del informe corresponden a la corrida del 2026-10-30. Para restaurarlos: git restore results/

```bash
make -f Makefile.karatsuba clean
make -f Makefile.karatsuba ejecutar-benchmark ENERGIA="conectado a la corriente"
```

Equivalente con `g++` (escribe el CSV en `build/` y no registra el entorno):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude bench/benchmark.cpp src/karatsuba.cpp src/aritmetica_decimal.cpp -o build/benchmark
./build/benchmark build/tiempos.csv
```

Salida esperada: una línea `Corrida:` (solo con `make`), la resolución del
reloj, una fila por tamaño y, al final,
`Verificacion karatsuba == clasica: todos los tamanos`, `Tiempo total: ...` y
`CSV: results/tiempos.csv` (con `g++`, `CSV: build/tiempos.csv`).

**Serie por umbral** (umbrales 16, 32 y 64). Genera `build/benchmark_u16`,
`build/benchmark_u32`, `build/benchmark_u64` y los CSV por umbral en
`build/`, y sobrescribe `results/tiempos_umbral.csv` (un solo encabezado) y
`results/benchmark_umbral_salida.txt`.

```bash
make -f Makefile.karatsuba ejecutar-benchmark-umbrales
```

Equivalente con `g++` para un umbral (16):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude -DADA_UMBRAL_CASO_BASE=16 bench/benchmark.cpp src/karatsuba.cpp src/aritmetica_decimal.cpp -o build/benchmark_u16
./build/benchmark_u16 build/tiempos_u16.csv
```

Salida esperada: para cada umbral, la línea `Umbral del caso base de
karatsuba: <k>`, una fila por tamaño y, al final,
`Verificacion karatsuba == clasica: todos los tamanos` y
`CSV: build/tiempos_u<k>.csv`. La serie completa tarda unos 40 s.

**Análisis y gráficas** (`bench/analizar.py` y `bench/graficar.py`). No
vuelven a medir: leen los CSV y el conteo de `results/`. Sobrescriben
`results/analisis_cruce.md`, `results/grafica_tiempos.png` y
`results/grafica_razon.png`.

```bash
make -f Makefile.karatsuba analisis
```

Equivalente con `python3`:

```bash
python3 bench/analizar.py
python3 bench/graficar.py
```

Salida esperada: `Escrito results/analisis_cruce.md`,
`Escrito results/grafica_tiempos.png` y `Escrito results/grafica_razon.png`. `make -f Makefile.karatsuba graficar` solo
regenera las gráficas.

**Limpieza.** Borra `build/`; no toca `results/`.

```bash
make -f Makefile.karatsuba clean
```

## Estructura de archivos

| Archivo o carpeta | Contenido | Parte |
|---|---|---|
| `include/aritmetica_decimal.hpp` | Interfaz de la aritmética decimal y de la clásica. | Luis |
| `src/aritmetica_decimal.cpp` | `normalizar`, `sumar`, `restar`, `desplazar`, `partir` y `multiplicacionClasica`. | Luis |
| `src/main.cpp` | Programa de consola de la clásica. | Luis |
| `tests/test_aritmetica_decimal.cpp` | Pruebas de los auxiliares y de la clásica. | Luis |
| `Makefile` | Makefile original con sintaxis de Windows (no oficial; ver nota al final). | Luis |
| `include/karatsuba.hpp` | Declaración de `ada::karatsuba` y de los auxiliares de relleno (`ada::detalle`). | Emmanuel |
| `src/karatsuba.cpp` | Envoltorio público, relleno y núcleo recursivo. | Emmanuel |
| `tests/test_karatsuba.cpp` | 31 casos: relleno, casos obligatorios, errores, exhaustiva 0..99 x 0..999 y aleatorios con semilla fija. | Emmanuel |
| `bench/contar_productos.cpp` | Conteo de productos del caso base (requiere `ADA_CONTAR_PRODUCTOS`). | Emmanuel |
| `bench/benchmark.cpp` | Medición de tiempos con `std::chrono::steady_clock`. | Emmanuel |
| `bench/registrar_entorno.sh` | Registro del entorno de medición en `results/entorno.txt`. | Emmanuel |
| `bench/analizar.py` | Pendientes, cruce medido y extrapolado; escribe `results/analisis_cruce.md`. | Emmanuel |
| `bench/graficar.py` | Gráficas `results/grafica_tiempos.png` y `results/grafica_razon.png`. | Emmanuel |
| `results/` | Datos medidos, salidas completas, entorno, conteo, análisis, gráficas y repeticiones. | Emmanuel |
| `Makefile.karatsuba` | Punto de entrada oficial en Linux para todo el proyecto; los binarios van a `build/`. | Emmanuel |
| `docs/Practica2_Karatsuba_Informe.pdf` | Informe de la práctica. | Ambos |
| `docs/uso_ia_luis.md` | Notas de uso de IA de Luis. | Luis |
| `docs/uso_ia_emmanuel.md`, `docs/evidencia/` | Notas de uso de IA de Emmanuel y capturas de evidencia. | Emmanuel |
| `build/` | Binarios y objetos generados (ignorado por git). | Generado |

## Multiplicación clásica y funciones auxiliares (Luis)

### Representación y contratos

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

### Pruebas

Las pruebas cubren longitudes distintas, acarreos y préstamos encadenados,
ceros a la izquierda, ceros intercalados, un producto mayor que los tipos
nativos y entradas que incumplen las precondiciones.

## Karatsuba, validación y experimentación (Emmanuel)

Usa la representación y las funciones auxiliares descritas en la sección
anterior.

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
- Estado de energía: conectado a la corriente.
- Protocolo: semilla 20261101, operandos aleatorios de exactamente n dígitos,
  verificación contra la clásica fuera de la región cronometrada, al menos
  50 ms por medición y mediana de 5 mediciones. El detalle está en
  `results/entorno.txt`.
- Durante la medición, el equipo debe estar conectado a la corriente, sin
  suspenderse y sin aplicaciones pesadas abiertas.
- `results/repeticion_principal/` y `results/repeticion_umbral/` contienen
  repeticiones de ambas series como evidencia de reproducibilidad.
- `results/fase6_bateria/` contiene una corrida anterior con un estado de
  energía más lento (inferido, no registrado). Solo ilustra el efecto del
  entorno y no se usa para las conclusiones.


## Nota sobre el Makefile original

El archivo `Makefile` usa sintaxis de Windows (`cmd`: `if not exist`,
`rmdir /s /q`) y genera los binarios en `bin/`. No es el punto de entrada
oficial: en Linux sus objetivos fallan, y se usa `Makefile.karatsuba`. Sus
comandos se conservan aquí solo como referencia, **no oficiales y no
verificados en Linux**:

```text
make
./bin/clasica 12345678901234567890 987654321
make test
```

Las compilaciones directas originales, que dejaban los binarios en la raíz
del repositorio, quedan sustituidas por los comandos `g++` de la sección
"Comandos por código", que escriben en `build/`.
