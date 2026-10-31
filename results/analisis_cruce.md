# Análisis del punto de cruce

Generado por `python3 bench/analizar.py` a partir de `results/tiempos.csv`, `results/tiempos_umbral.csv` y `results/conteo_productos.txt`.

- Corrida principal (umbral 1): 2026-10-30 23:06:27 -0500
- Corrida complementaria (umbrales 16, 32, 64): 2026-10-30 23:02:05 -0500
- Estado de energía: conectado a la corriente (declarado por el usuario)
- Tiempos en nanosegundos por multiplicación (mediana de 5 mediciones de al menos 50 ms). Razón = Karatsuba / clásica; un valor menor que 1 significa que Karatsuba es más rápida.

## 1. Datos medidos

| n | clásica (ns) | Karatsuba u1 (ns) | razón u1 | razón u16 | razón u32 | razón u64 |
|---|---|---|---|---|---|---|
| 2 | 97.8 | 480.3 | 4.911 | 1.707 | 1.772 | 1.737 |
| 4 | 165.3 | 2876.6 | 17.402 | 1.447 | 1.724 | 1.737 |
| 8 | 373.6 | 12403.5 | 33.200 | 1.201 | 1.206 | 1.246 |
| 16 | 1127.7 | 31639.6 | 28.057 | 1.134 | 1.133 | 1.185 |
| 32 | 4186.2 | 107267.5 | 25.624 | 1.566 | 1.027 | 1.045 |
| 64 | 16606.7 | 324379.8 | 19.533 | 1.425 | 1.034 | 1.002 |
| 128 | 66782.6 | 971748.3 | 14.551 | 1.121 | 0.838 | 0.830 |
| 256 | 264530.9 | 2947876.6 | 11.144 | 0.920 | 0.687 | 0.643 |
| 512 | 1065608.2 | 9040166.3 | 8.484 | 0.734 | 0.549 | 0.513 |
| 1024 | 4257821.5 | 27399743.5 | 6.435 | 0.582 | 0.438 | 0.407 |
| 2048 | 16903049.3 | 84870768.0 | 5.021 | 0.437 | 0.334 | 0.309 |
| 4096 | 68578114.0 | 247098625.0 | 3.603 | 0.330 | 0.255 | 0.238 |

Las razones de u16, u32 y u64 usan la clásica medida en la misma corrida de cada umbral.

## 2. Pendientes log-log por regresión

Ajuste por mínimos cuadrados de ln t = a + b ln n. Referencias: 2 para la clásica y log2(3) = 1.585 para Karatsuba.

| Curva | Todo el rango (2-4096) | n ≥ 256 | Referencia |
|---|---|---|---|
| Clásica | 1.843 | 2.002 | 2.000 |
| Karatsuba u1 | 1.667 | 1.601 | 1.585 |
| Karatsuba u16 | 1.661 | 1.622 | 1.585 |
| Karatsuba u32 | 1.608 | 1.636 | 1.585 |
| Karatsuba u64 | 1.590 | 1.636 | 1.585 |

## 3. Pendientes locales entre duplicaciones

log2(t(2n) / t(n)). Para n grande se espera 2 en la clásica y 1.585 en Karatsuba.

| Duplicación | Clásica | Karatsuba u1 | Karatsuba u16 | Karatsuba u32 | Karatsuba u64 |
|---|---|---|---|---|---|
| 2 → 4 | 0.757 | 2.582 | 0.550 | 0.985 | 0.899 |
| 4 → 8 | 1.176 | 2.108 | 0.787 | 0.385 | 0.511 |
| 8 → 16 | 1.594 | 1.351 | 1.692 | 1.706 | 1.818 |
| 16 → 32 | 1.892 | 1.761 | 2.341 | 1.721 | 1.564 |
| 32 → 64 | 1.988 | 1.596 | 1.866 | 2.000 | 1.968 |
| 64 → 128 | 2.008 | 1.583 | 1.658 | 1.719 | 1.707 |
| 128 → 256 | 1.986 | 1.601 | 1.724 | 1.723 | 1.645 |
| 256 → 512 | 2.010 | 1.617 | 1.660 | 1.656 | 1.646 |
| 512 → 1024 | 1.998 | 1.600 | 1.674 | 1.696 | 1.670 |
| 1024 → 2048 | 1.989 | 1.631 | 1.588 | 1.577 | 1.619 |
| 2048 → 4096 | 2.020 | 1.542 | 1.555 | 1.617 | 1.600 |

## 4. Umbral 1: cruce extrapolado

Con umbral 1 no hay cruce en el rango medido (razón 3.603 en n = 4096). Se ajusta una ley de potencias t = c·n^b a cada curva en una ventana n ≥ n_min, y se resuelve c_k·n^b_k = c_c·n^b_c.

| Ventana | b clásica | b Karatsuba u1 | n de cruce estimado |
|---|---|---|---|
| n ≥ 128 | 2.000 | 1.603 | 1.10 x 10^5 |
| n ≥ 256 | 2.002 | 1.601 | 1.06 x 10^5 |
| n ≥ 512 | 2.001 | 1.595 | 1.01 x 10^5 |

**Rango estimado: entre 1.01 x 10^5 y 1.10 x 10^5 dígitos.**

Es una extrapolación, no una medición. Supone que:

- ambos exponentes se mantienen fuera del rango medido, unas 20 a 30 veces más allá de 4096 dígitos;
- la jerarquía de memoria no cambia el costo relativo. Con unos 10^5 dígitos, el vector de la clásica (2n enteros) ya no cabe en la caché L2, lo que podría subir su pendiente y adelantar el cruce;
- el estado de energía y la carga del sistema son los de esta corrida.

El rango entre ventanas es estrecho, pero no mide toda la incertidumbre. Partiendo de la razón 3.603 en n = 4096, el cruce es n* = 4096 · 3.603^(1 / db), con db = b_clásica − b_Karatsuba (ventana n ≥ 256). Como db es pequeño, n* es muy sensible a él:

| db | n de cruce |
|---|---|
| 0.351 | 1.57 x 10^5 |
| 0.401 | 9.98 x 10^4 |
| 0.451 | 7.01 x 10^4 |

Un error de 0.05 en la diferencia de pendientes desplaza el cruce entre 7.01 x 10^4 y 1.57 x 10^5 dígitos. El valor central difiere un poco de la intersección de la tabla anterior porque parte del último punto medido y no de la recta ajustada.

## 5. Umbrales 16, 32 y 64: cruce medido

Primer n > umbral con razón < 1 que se mantiene en los tamaños siguientes. La interpolación es lineal en (ln n, ln razón) entre el tamaño anterior y el del cruce.

| Umbral | Cruce medido (n) | Anterior (razón) | Cruce (razón) | Interpolado (n) | Repetición |
|---|---|---|---|---|---|
| u16 | 256 | 128 (1.121) | 256 (0.920) | 191 | 256, interp. 190 |
| u32 | 128 | 64 (1.034) | 128 (0.838) | 71 | 128, interp. 73 |
| u64 | 128 | 64 (1.002) | 128 (0.830) | 65 | 128, interp. 68 |

La interpolación supone que la razón varía de forma suave entre dos potencias de 2 consecutivas; solo ubica el cruce dentro de ese intervalo.

## 6. Interpretación

### 6.1 Pendiente de Karatsuba frente al conteo de productos

El conteo de productos base de la Fase 5 (`results/conteo_productos.txt`, generado por `bench/contar_productos.cpp`) tiene pendiente local 1.585 con nueves y 1.585 con entradas aleatorias entre 512 y 1024 dígitos, frente a log2(3) = 1.585: el número de multiplicaciones sigue la recurrencia T(n) = 3T(n/2) + Θ(n). El conteo es aproximadamente 2.37 · n^1.585 con nueves y entre 1.49 y 1.56 · n^1.585 con entradas aleatorias (n ≥ 64).

El tiempo medido, en cambio, tiene pendiente 1.601 en n ≥ 256, con pendientes locales alrededor de 1.6, por tres razones:

- Cada nodo de la recursión hace trabajo lineal adicional: partir, dos sumas, dos restas y tres desplazamientos, todos sobre `std::string` nuevas. La recurrencia T(n) = 3T(n/2) + cn tiene solución exacta T(n) = α·n^1.585 − 2cn. Como el término lineal resta, la pendiente local d ln T / d ln n es mayor que 1.585 en tamaños finitos y se acerca a 1.585 desde arriba. Con un umbral mayor, α es menor frente a c y el efecto crece; eso es coherente con que u32 y u64 tengan pendientes en n ≥ 256 mayores que u1.
- Reservar y liberar memoria para cada cadena tiene un costo que no depende solo del número de dígitos, y los fallos de caché aumentan con n.
- La rama P3 recibe ceil(L/2) + 1 dígitos. En el conteo eso se ve como una constante mayor que 1 delante de n^1.585, y como pendientes más altas en tamaños pequeños; el exponente asintótico no cambia.

### 6.2 Por qué los auxiliares de Luis alejan el cruce

Si t_c = c_c·n^2 y t_k = c_k·n^1.585, el cruce ocurre en n* = (c_k / c_c)^(1 / (2 − 1.585)) = (c_k / c_c)^2.41. El cruce depende de la razón de constantes elevada a 2.41: si la constante de Karatsuba se duplica, el cruce se multiplica por 2^2.41 = 5.3.

Las funciones `sumar`, `restar`, `desplazar` y `partir` normalizan sus entradas (recorren y copian la cadena) y devuelven cadenas nuevas; `partir` además copia cada mitad con `substr`. La clásica, en cambio, reserva un solo vector y ejecuta un bucle doble sin más reservas de memoria. Por eso c_k es mucho mayor que c_c: con umbral 1, Karatsuba ya es 4.9 veces más lenta en n = 2. Esa diferencia de constantes, elevada a 2.41, lleva el cruce a alrededor de 10^5 dígitos.

### 6.3 Por qué el umbral reduce la constante sin cambiar el exponente

Con umbral u, la recursión se detiene en tamaño u. El árbol tiene log2(n/u) niveles y el costo total es la suma de dos partes:

- **Hojas:** (n/u)^1.585 multiplicaciones clásicas de tamaño u, de costo cercano a u^2 cada una. En total, u^0.415 · n^1.585, que crece al subir u.
- **Nodos internos:** del orden de (n/u)^1.585 nodos, cada uno con un costo fijo de copias, normalizaciones y reservas de memoria. En total, n^1.585 / u^1.585, que decrece al subir u. El trabajo lineal de sumas y restas en esos nodos también decrece, como n^1.585 / u^0.585.

Las dos partes son proporcionales a n^1.585, así que el exponente sigue siendo log2(3): el umbral solo cambia la constante, de la forma A·u^0.415 + B / u^1.585 + C / u^0.585. El umbral óptimo equilibra el término de las hojas, que crece, con los de los nodos internos, que decrecen. Cerca del óptimo la constante varía poco, y por eso u64 mejora poco sobre u32 (razones 0.238 y 0.255 en n = 4096). Con u = 1, en cambio, el término de nodos internos domina y el cruce queda fuera del rango medido.

## 7. Efecto del estado de energía (solo ilustrativo)

`results/fase6_bateria/` contiene la primera corrida de la serie principal, con un estado de energía inferido, no registrado (probablemente batería o modo de ahorro). No se usa para las conclusiones. Factor = tiempo en estado lento / tiempo conectado.

| n | Factor clásica | Factor Karatsuba u1 | Razón (estado lento) | Razón (conectado) |
|---|---|---|---|---|
| 2 | 2.36 | 4.42 | 9.189 | 4.911 |
| 4 | 2.97 | 4.09 | 23.963 | 17.402 |
| 8 | 2.20 | 2.60 | 39.240 | 33.200 |
| 16 | 2.23 | 2.18 | 27.418 | 28.057 |
| 32 | 2.32 | 2.40 | 26.558 | 25.624 |
| 64 | 2.24 | 2.70 | 23.526 | 19.533 |
| 128 | 2.21 | 2.72 | 17.895 | 14.551 |
| 256 | 2.25 | 3.22 | 15.928 | 11.144 |
| 512 | 2.22 | 2.78 | 10.632 | 8.484 |
| 1024 | 2.20 | 2.47 | 7.220 | 6.435 |
| 2048 | 2.20 | 2.33 | 5.318 | 5.021 |
| 4096 | 2.24 | 2.61 | 4.181 | 3.603 |

El estado lento no afecta por igual a los dos algoritmos: Karatsuba, con muchas reservas de memoria y copias pequeñas, se ve más afectada que la clásica. Por eso solo se comparan series medidas en las mismas condiciones, y el estado de energía se registra en `results/entorno.txt`.

## 8. Resumen

- Clásica: pendiente 2.002 en n ≥ 256 (teórica 2).
- Karatsuba u1: pendiente 1.601 en n ≥ 256 (teórica 1.585); sin cruce medido; cruce extrapolado entre 1.01 x 10^5 y 1.10 x 10^5 dígitos.
- Karatsuba u16: cruce medido en n = 256 (interpolado 191).
- Karatsuba u32: cruce medido en n = 128 (interpolado 71).
- Karatsuba u64: cruce medido en n = 128 (interpolado 65).
