#!/usr/bin/env python3
"""Analisis del punto de cruce entre la multiplicacion clasica y Karatsuba.

Lee results/tiempos.csv (serie principal, umbral 1),
results/tiempos_umbral.csv (serie complementaria, umbrales 16, 32 y 64) y
results/conteo_productos.txt (conteo de productos base de la Fase 5), y
escribe results/analisis_cruce.md con:
  - pendientes log-log por regresion (todo el rango y n >= 256) y locales;
  - cruce extrapolado para umbral 1 (ley de potencias, varias ventanas);
  - cruce medido e interpolado para los umbrales mayores;
  - interpretacion y efecto del estado de energia (ilustrativo).

Solo usa la biblioteca estandar. Uso, desde la raiz del repositorio:
    python3 bench/analizar.py
"""

import csv
import math
import os
import re

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOG2_3 = math.log2(3.0)
VENTANAS_EXTRAPOLACION = (128, 256, 512)
UMBRALES = (16, 32, 64)

# Fila de la tabla de bench/contar_productos.cpp:
#   n  n^1.585  nueves  razon  aleatoria  razon
FILA_CONTEO = re.compile(r"^\s*(\d+)\s+(\d+)\s+(\d+)\s+([\d.]+)\s+(\d+)\s+([\d.]+)\s*$")


def ruta(*partes):
    return os.path.join(RAIZ, *partes)


def leer_principal(archivo):
    """{n: (clasica_ns, karatsuba_ns)} de un CSV digitos,clasica_ns,karatsuba_ns."""
    with open(archivo, newline="", encoding="utf-8") as f:
        return {int(r["digitos"]): (float(r["clasica_ns"]), float(r["karatsuba_ns"]))
                for r in csv.DictReader(f)}


def leer_umbral(archivo):
    """{umbral: {n: (clasica_ns, karatsuba_ns)}} de tiempos_umbral.csv."""
    datos = {}
    with open(archivo, newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            serie = datos.setdefault(int(r["umbral"]), {})
            serie[int(r["digitos"])] = (float(r["clasica_ns"]), float(r["karatsuba_ns"]))
    return datos


def leer_conteo(archivo):
    """{n: (productos_nueves, productos_aleatoria)} de conteo_productos.txt.

    Ignora las lineas que no son filas de la tabla (ordenes de make, titulos).
    Devuelve None si el archivo no existe.
    """
    if not os.path.exists(archivo):
        return None
    conteo = {}
    with open(archivo, encoding="utf-8") as f:
        for linea in f:
            m = FILA_CONTEO.match(linea)
            if m:
                conteo[int(m.group(1))] = (int(m.group(3)), int(m.group(5)))
    return conteo or None


def primera_linea(archivo, prefijo):
    """Primera linea del archivo que empieza con prefijo, o 'no disponible'."""
    if not os.path.exists(archivo):
        return "no disponible"
    with open(archivo, encoding="utf-8") as f:
        for linea in f:
            if linea.startswith(prefijo):
                return linea[len(prefijo):].strip()
    return "no disponible"


def regresion(ns, ts):
    """Ajuste ln t = a + b ln n por minimos cuadrados. Devuelve (a, b)."""
    xs = [math.log(n) for n in ns]
    ys = [math.log(t) for t in ts]
    mx = sum(xs) / len(xs)
    my = sum(ys) / len(ys)
    b = (sum((x - mx) * (y - my) for x, y in zip(xs, ys))
         / sum((x - mx) ** 2 for x in xs))
    return my - b * mx, b


def pendientes_locales(ns, ts):
    """log2(t(2n) / t(n)) para cada duplicacion consecutiva."""
    return [math.log2(ts[i + 1] / ts[i]) for i in range(len(ns) - 1)]


def cruce_extrapolado(serie, n_min):
    """Interseccion de las leyes de potencias ajustadas en n >= n_min.

    Devuelve (b_clasica, b_karatsuba, n_cruce); n_cruce es None si las
    pendientes no permiten interseccion por encima del rango.
    """
    ns = [n for n in sorted(serie) if n >= n_min]
    a_c, b_c = regresion(ns, [serie[n][0] for n in ns])
    a_k, b_k = regresion(ns, [serie[n][1] for n in ns])
    if b_c <= b_k:
        return b_c, b_k, None
    return b_c, b_k, math.exp((a_k - a_c) / (b_c - b_k))


def cruce_medido(serie, umbral):
    """Primer n > umbral con razon < 1 (y que se mantiene despues).

    Devuelve (n0, r0, n1, r1, n_interpolado) con n0 el tamano medido anterior,
    o None si no hay cruce. La interpolacion es lineal en (ln n, ln razon).
    """
    ns = sorted(serie)
    razones = {n: serie[n][1] / serie[n][0] for n in ns}
    for i, n in enumerate(ns):
        if n <= umbral or razones[n] >= 1.0 or i == 0:
            continue
        if any(razones[m] >= 1.0 for m in ns[i:]):
            continue
        n0 = ns[i - 1]
        r0, r1 = razones[n0], razones[n]
        fraccion = math.log(r0) / (math.log(r0) - math.log(r1))
        n_interp = math.exp(math.log(n0) + fraccion * (math.log(n) - math.log(n0)))
        return n0, r0, n, r1, n_interp
    return None


def cientifica(x):
    """Formato a x 10^k con dos decimales, para tamanos extrapolados."""
    k = int(math.floor(math.log10(x)))
    return "%.2f x 10^%d" % (x / 10 ** k, k)


def tabla(encabezados, filas):
    lineas = ["| " + " | ".join(encabezados) + " |",
              "|" + "|".join("---" for _ in encabezados) + "|"]
    lineas += ["| " + " | ".join(fila) + " |" for fila in filas]
    return lineas


def potencia_log2_3(n):
    """3^log2(n) = n^log2(3), exacto para n potencia de 2."""
    return 3.0 ** math.log2(n)


def texto_conteo(conteo):
    """Parrafo de la seccion 6.1 con las cifras leidas del conteo."""
    if conteo is None:
        return ("El conteo de productos base de la Fase 5 no está disponible "
                "(falta `results/conteo_productos.txt`).")
    ns = sorted(conteo)
    n0, n1 = ns[-2], ns[-1]
    local_nueves = math.log2(conteo[n1][0] / conteo[n0][0])
    local_aleatoria = math.log2(conteo[n1][1] / conteo[n0][1])
    constante_nueves = conteo[n1][0] / potencia_log2_3(n1)
    grandes = [n for n in ns if n >= 64]
    constantes_aleatoria = [conteo[n][1] / potencia_log2_3(n) for n in grandes]
    return ("El conteo de productos base de la Fase 5 (`results/conteo_productos.txt`, "
            "generado por `bench/contar_productos.cpp`) tiene pendiente local "
            "%.3f con nueves y %.3f con entradas aleatorias entre %d y %d dígitos, "
            "frente a log2(3) = %.3f: el número de multiplicaciones sigue la "
            "recurrencia T(n) = 3T(n/2) + Θ(n). El conteo es aproximadamente "
            "%.2f · n^1.585 con nueves y entre %.2f y %.2f · n^1.585 con entradas "
            "aleatorias (n ≥ %d)." % (
                local_nueves, local_aleatoria, n0, n1, LOG2_3, constante_nueves,
                min(constantes_aleatoria), max(constantes_aleatoria), grandes[0]))


def main():
    principal = leer_principal(ruta("results", "tiempos.csv"))
    umbral = leer_umbral(ruta("results", "tiempos_umbral.csv"))
    conteo = leer_conteo(ruta("results", "conteo_productos.txt"))
    ns = sorted(principal)

    # Curvas: nombre -> {n: tiempo}. La clasica es la de la serie principal.
    curvas = [("Clásica", {n: principal[n][0] for n in ns}, 2.0),
              ("Karatsuba u1", {n: principal[n][1] for n in ns}, LOG2_3)]
    for u in UMBRALES:
        curvas.append(("Karatsuba u%d" % u, {n: umbral[u][n][1] for n in ns}, LOG2_3))

    md = ["# Análisis del punto de cruce", "",
          "Generado por `python3 bench/analizar.py` a partir de "
          "`results/tiempos.csv`, `results/tiempos_umbral.csv` y "
          "`results/conteo_productos.txt`.", "",
          "- Corrida principal (umbral 1): %s"
          % primera_linea(ruta("results", "benchmark_salida.txt"), "Corrida: "),
          "- Corrida complementaria (umbrales %s): %s"
          % (", ".join(str(u) for u in UMBRALES),
             primera_linea(ruta("results", "benchmark_umbral_salida.txt"), "Corrida: ")),
          "- Estado de energía: %s"
          % primera_linea(ruta("results", "entorno.txt"), "Estado de energia: "),
          "- Tiempos en nanosegundos por multiplicación (mediana de 5 mediciones "
          "de al menos 50 ms). Razón = Karatsuba / clásica; un valor menor que 1 "
          "significa que Karatsuba es más rápida.", ""]

    # 1. Datos.
    md += ["## 1. Datos medidos", ""]
    filas = []
    for n in ns:
        fila = [str(n), "%.1f" % principal[n][0], "%.1f" % principal[n][1],
                "%.3f" % (principal[n][1] / principal[n][0])]
        fila += ["%.3f" % (umbral[u][n][1] / umbral[u][n][0]) for u in UMBRALES]
        filas.append(fila)
    md += tabla(["n", "clásica (ns)", "Karatsuba u1 (ns)", "razón u1"]
                + ["razón u%d" % u for u in UMBRALES], filas)
    md += ["", "Las razones de u16, u32 y u64 usan la clásica medida en la misma "
           "corrida de cada umbral.", ""]

    # 2. Pendientes por regresion.
    md += ["## 2. Pendientes log-log por regresión", "",
           "Ajuste por mínimos cuadrados de ln t = a + b ln n. Referencias: 2 "
           "para la clásica y log2(3) = %.3f para Karatsuba." % LOG2_3, ""]
    filas = []
    grandes = [n for n in ns if n >= 256]
    for nombre, t, referencia in curvas:
        _, b_todo = regresion(ns, [t[n] for n in ns])
        _, b_256 = regresion(grandes, [t[n] for n in grandes])
        filas.append([nombre, "%.3f" % b_todo, "%.3f" % b_256, "%.3f" % referencia])
    md += tabla(["Curva", "Todo el rango (2-4096)", "n ≥ 256", "Referencia"], filas)
    md += [""]

    # 3. Pendientes locales.
    md += ["## 3. Pendientes locales entre duplicaciones", "",
           "log2(t(2n) / t(n)). Para n grande se espera 2 en la clásica y "
           "%.3f en Karatsuba." % LOG2_3, ""]
    locales = {nombre: pendientes_locales(ns, [t[n] for n in ns]) for nombre, t, _ in curvas}
    filas = []
    for i in range(len(ns) - 1):
        filas.append(["%d → %d" % (ns[i], ns[i + 1])]
                     + ["%.3f" % locales[nombre][i] for nombre, _, _ in curvas])
    md += tabla(["Duplicación"] + [nombre for nombre, _, _ in curvas], filas)
    md += [""]

    # 4. Cruce extrapolado con umbral 1.
    n_max = ns[-1]
    razon_max = principal[n_max][1] / principal[n_max][0]
    md += ["## 4. Umbral 1: cruce extrapolado", "",
           "Con umbral 1 no hay cruce en el rango medido (razón %.3f en n = %d). "
           "Se ajusta una ley de potencias t = c·n^b a cada curva en una ventana "
           "n ≥ n_min, y se resuelve c_k·n^b_k = c_c·n^b_c." % (razon_max, n_max), ""]
    filas = []
    cruces = []
    for n_min in VENTANAS_EXTRAPOLACION:
        b_c, b_k, n_cruce = cruce_extrapolado(principal, n_min)
        if n_cruce is not None:
            cruces.append(n_cruce)
        filas.append(["n ≥ %d" % n_min, "%.3f" % b_c, "%.3f" % b_k,
                      cientifica(n_cruce) if n_cruce else "sin intersección"])
    md += tabla(["Ventana", "b clásica", "b Karatsuba u1", "n de cruce estimado"], filas)
    if cruces:
        md += ["", "**Rango estimado: entre %s y %s dígitos.**"
               % (cientifica(min(cruces)), cientifica(max(cruces)))]
    md += ["",
           "Es una extrapolación, no una medición. Supone que:",
           "",
           "- ambos exponentes se mantienen fuera del rango medido, unas 20 a 30 "
           "veces más allá de %d dígitos;" % n_max,
           "- la jerarquía de memoria no cambia el costo relativo. Con unos 10^5 "
           "dígitos, el vector de la clásica (2n enteros) ya no cabe en la caché "
           "L2, lo que podría subir su pendiente y adelantar el cruce;",
           "- el estado de energía y la carga del sistema son los de esta corrida.",
           ""]
    # Sensibilidad: desde el ultimo punto medido, n* = n_max * R^(1/db), con
    # R la razon en n_max y db = b_clasica - b_karatsuba.
    b_c, b_k, _ = cruce_extrapolado(principal, 256)
    db = b_c - b_k
    sensibilidad = [(d, n_max * razon_max ** (1.0 / d)) for d in (db - 0.05, db, db + 0.05)]
    md += ["El rango entre ventanas es estrecho, pero no mide toda la "
           "incertidumbre. Partiendo de la razón %.3f en n = %d, el cruce es "
           "n* = %d · %.3f^(1 / db), con db = b_clásica − b_Karatsuba (ventana "
           "n ≥ 256). Como db es pequeño, n* es muy sensible a él:"
           % (razon_max, n_max, n_max, razon_max), ""]
    md += tabla(["db", "n de cruce"],
                [["%.3f" % d, cientifica(n)] for d, n in sensibilidad])
    md += ["", "Un error de 0.05 en la diferencia de pendientes desplaza el cruce "
           "entre %s y %s dígitos. El valor central difiere un poco de la "
           "intersección de la tabla anterior porque parte del último punto "
           "medido y no de la recta ajustada." % (cientifica(sensibilidad[2][1]),
                                                   cientifica(sensibilidad[0][1])), ""]

    # 5. Cruce medido por umbral.
    md += ["## 5. Umbrales 16, 32 y 64: cruce medido", "",
           "Primer n > umbral con razón < 1 que se mantiene en los tamaños "
           "siguientes. La interpolación es lineal en (ln n, ln razón) entre el "
           "tamaño anterior y el del cruce.", ""]
    repeticiones = {}
    for u in UMBRALES:
        archivo = ruta("results", "repeticion_umbral", "repeticion_u%d.csv" % u)
        if os.path.exists(archivo):
            repeticiones[u] = leer_principal(archivo)
    filas = []
    for u in UMBRALES:
        cruce = cruce_medido(umbral[u], u)
        if cruce is None:
            filas.append(["u%d" % u, "no aparece", "-", "-", "-"]
                         + (["-"] if repeticiones else []))
            continue
        n0, r0, n1, r1, n_interp = cruce
        fila = ["u%d" % u, str(n1), "%d (%.3f)" % (n0, r0), "%d (%.3f)" % (n1, r1),
                "%.0f" % n_interp]
        if repeticiones:
            cruce_rep = cruce_medido(repeticiones[u], u) if u in repeticiones else None
            fila.append("%d, interp. %.0f" % (cruce_rep[2], cruce_rep[4])
                        if cruce_rep else "no aparece")
        filas.append(fila)
    encabezados = ["Umbral", "Cruce medido (n)", "Anterior (razón)",
                   "Cruce (razón)", "Interpolado (n)"]
    if repeticiones:
        encabezados.append("Repetición")
    md += tabla(encabezados, filas)
    md += ["", "La interpolación supone que la razón varía de forma suave entre "
           "dos potencias de 2 consecutivas; solo ubica el cruce dentro de ese "
           "intervalo.", ""]

    # 6. Interpretacion.
    _, b_k_256 = regresion(grandes, [principal[n][1] for n in grandes])
    _, b_c_256 = regresion(grandes, [principal[n][0] for n in grandes])
    exponente_cruce = 1.0 / (2.0 - LOG2_3)
    md += ["## 6. Interpretación", "",
           "### 6.1 Pendiente de Karatsuba frente al conteo de productos", "",
           texto_conteo(conteo), "",
           "El tiempo medido, en cambio, tiene pendiente %.3f en n ≥ 256, con "
           "pendientes locales alrededor de 1.6, por tres razones:" % b_k_256, "",
           "- Cada nodo de la recursión hace trabajo lineal adicional: partir, dos "
           "sumas, dos restas y tres desplazamientos, todos sobre `std::string` "
           "nuevas. La recurrencia T(n) = 3T(n/2) + cn tiene solución exacta "
           "T(n) = α·n^1.585 − 2cn. Como el término lineal resta, la pendiente "
           "local d ln T / d ln n es mayor que 1.585 en tamaños finitos y se "
           "acerca a 1.585 desde arriba. Con un umbral mayor, α es menor frente "
           "a c y el efecto crece; eso es coherente con que u32 y u64 tengan "
           "pendientes en n ≥ 256 mayores que u1.",
           "- Reservar y liberar memoria para cada cadena tiene un costo que no "
           "depende solo del número de dígitos, y los fallos de caché aumentan "
           "con n.",
           "- La rama P3 recibe ceil(L/2) + 1 dígitos. En el conteo eso se ve "
           "como una constante mayor que 1 delante de n^1.585, y como pendientes "
           "más altas en tamaños pequeños; el exponente asintótico no cambia.", "",
           "### 6.2 Por qué los auxiliares de Luis alejan el cruce", "",
           "Si t_c = c_c·n^2 y t_k = c_k·n^1.585, el cruce ocurre en "
           "n* = (c_k / c_c)^(1 / (2 − 1.585)) = (c_k / c_c)^%.2f. El cruce depende "
           "de la razón de constantes elevada a %.2f: si la constante de Karatsuba "
           "se duplica, el cruce se multiplica por 2^%.2f = %.1f." % (
               exponente_cruce, exponente_cruce, exponente_cruce, 2 ** exponente_cruce), "",
           "Las funciones `sumar`, `restar`, `desplazar` y `partir` normalizan "
           "sus entradas (recorren y copian la cadena) y devuelven cadenas nuevas; "
           "`partir` además copia cada mitad con `substr`. La clásica, en cambio, "
           "reserva un solo vector y ejecuta un bucle doble sin más reservas de "
           "memoria. Por eso c_k es mucho mayor que c_c: con umbral 1, Karatsuba "
           "ya es %.1f veces más lenta en n = 2. Esa diferencia de constantes, "
           "elevada a %.2f, lleva el cruce a alrededor de 10^5 dígitos." % (
               principal[ns[0]][1] / principal[ns[0]][0], exponente_cruce), "",
           "### 6.3 Por qué el umbral reduce la constante sin cambiar el exponente", "",
           "Con umbral u, la recursión se detiene en tamaño u. El árbol tiene "
           "log2(n/u) niveles y el costo total es la suma de dos partes:", "",
           "- **Hojas:** (n/u)^1.585 multiplicaciones clásicas de tamaño u, de "
           "costo cercano a u^2 cada una. En total, u^0.415 · n^1.585, que crece "
           "al subir u.",
           "- **Nodos internos:** del orden de (n/u)^1.585 nodos, cada uno con un "
           "costo fijo de copias, normalizaciones y reservas de memoria. En total, "
           "n^1.585 / u^1.585, que decrece al subir u. El trabajo lineal de sumas "
           "y restas en esos nodos también decrece, como n^1.585 / u^0.585.", "",
           "Las dos partes son proporcionales a n^1.585, así que el exponente "
           "sigue siendo log2(3): el umbral solo cambia la constante, de la forma "
           "A·u^0.415 + B / u^1.585 + C / u^0.585. El umbral óptimo equilibra el "
           "término de las hojas, que crece, con los de los nodos internos, que "
           "decrecen. "
           "Cerca del óptimo la constante varía poco, y por eso u64 mejora poco "
           "sobre u32 (razones %.3f y %.3f en n = %d). Con u = 1, en cambio, el "
           "término de nodos internos domina y el cruce queda fuera del rango "
           "medido." % (umbral[64][n_max][1] / umbral[64][n_max][0],
                        umbral[32][n_max][1] / umbral[32][n_max][0], n_max), ""]

    # 7. Estado de energia.
    archivo_bateria = ruta("results", "fase6_bateria", "tiempos.csv")
    if os.path.exists(archivo_bateria):
        bateria = leer_principal(archivo_bateria)
        md += ["## 7. Efecto del estado de energía (solo ilustrativo)", "",
               "`results/fase6_bateria/` contiene la primera corrida de la serie "
               "principal, con un estado de energía inferido, no registrado "
               "(probablemente batería o modo de ahorro). No se usa para las "
               "conclusiones. Factor = tiempo en estado lento / tiempo conectado.", ""]
        filas = []
        for n in ns:
            filas.append([str(n),
                          "%.2f" % (bateria[n][0] / principal[n][0]),
                          "%.2f" % (bateria[n][1] / principal[n][1]),
                          "%.3f" % (bateria[n][1] / bateria[n][0]),
                          "%.3f" % (principal[n][1] / principal[n][0])])
        md += tabla(["n", "Factor clásica", "Factor Karatsuba u1",
                     "Razón (estado lento)", "Razón (conectado)"], filas)
        md += ["",
               "El estado lento no afecta por igual a los dos algoritmos: "
               "Karatsuba, con muchas reservas de memoria y copias pequeñas, se "
               "ve más afectada que la clásica. Por eso solo se comparan series "
               "medidas en las mismas condiciones, y el estado de energía se "
               "registra en `results/entorno.txt`.", ""]

    md += ["## 8. Resumen", "",
           "- Clásica: pendiente %.3f en n ≥ 256 (teórica 2)." % b_c_256,
           "- Karatsuba u1: pendiente %.3f en n ≥ 256 (teórica %.3f); sin cruce "
           "medido; cruce extrapolado entre %s y %s dígitos."
           % (b_k_256, LOG2_3, cientifica(min(cruces)), cientifica(max(cruces)))
           if cruces else "- Karatsuba u1: sin cruce medido ni extrapolable.",
           ]
    for u in UMBRALES:
        cruce = cruce_medido(umbral[u], u)
        md.append("- Karatsuba u%d: cruce medido en n = %d (interpolado %.0f)."
                  % (u, cruce[2], cruce[4]) if cruce
                  else "- Karatsuba u%d: sin cruce en el rango medido." % u)
    md.append("")

    destino = ruta("results", "analisis_cruce.md")
    with open(destino, "w", newline="\n", encoding="utf-8") as f:
        f.write("\n".join(md))
    print("Escrito %s" % os.path.relpath(destino, RAIZ))


if __name__ == "__main__":
    main()
