#!/usr/bin/env python3
"""Graficas de tiempos y razones de la multiplicacion clasica y Karatsuba.

Lee solo results/tiempos.csv (serie principal, umbral 1) y
results/tiempos_umbral.csv (umbrales 16, 32 y 64), y escribe:
  - results/grafica_tiempos.png: tiempo por multiplicacion contra digitos,
    escala log-log, con rectas de referencia de pendiente 2 y log2(3) y el
    cruce medido de cada umbral;
  - results/grafica_razon.png: razon Karatsuba / clasica contra digitos, con
    la linea horizontal en 1.

Requiere matplotlib. Uso, desde la raiz del repositorio:
    python3 bench/graficar.py
"""

import csv
import math
import os

import matplotlib

matplotlib.use("Agg")  # sin ventana: solo escribe archivos
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.ticker import FixedLocator, FuncFormatter, NullLocator  # noqa: E402

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOG2_3 = math.log2(3.0)
UMBRALES = (16, 32, 64)
DPI = 200
# Ancho util de una pagina carta con margenes de 1 pulgada: 6.5 pulgadas.
TAMANO = (6.5, 4.6)

# Estilo por serie: color, marcador y linea distintos para leer en blanco y negro.
ESTILOS = {
    "clasica": dict(color="#000000", marker="o", linestyle="-"),
    1: dict(color="#d62728", marker="s", linestyle="-"),
    16: dict(color="#1f77b4", marker="^", linestyle="--"),
    32: dict(color="#2ca02c", marker="D", linestyle="-."),
    64: dict(color="#9467bd", marker="v", linestyle=":"),
}


def ruta(*partes):
    return os.path.join(RAIZ, *partes)


def leer_principal(archivo):
    """{n: (clasica_ns, karatsuba_ns)}."""
    with open(archivo, newline="", encoding="utf-8") as f:
        return {int(r["digitos"]): (float(r["clasica_ns"]), float(r["karatsuba_ns"]))
                for r in csv.DictReader(f)}


def leer_umbral(archivo):
    """{umbral: {n: (clasica_ns, karatsuba_ns)}}."""
    datos = {}
    with open(archivo, newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            serie = datos.setdefault(int(r["umbral"]), {})
            serie[int(r["digitos"])] = (float(r["clasica_ns"]), float(r["karatsuba_ns"]))
    return datos


def cruce_medido(serie, umbral):
    """Primer n > umbral con razon < 1 que se mantiene despues, o None.

    Mismo criterio que bench/analizar.py.
    """
    ns = sorted(serie)
    for i, n in enumerate(ns):
        if n <= umbral or i == 0:
            continue
        if all(serie[m][1] < serie[m][0] for m in ns[i:]):
            return n
    return None


def eje_digitos(eje, ns):
    """Eje x logaritmico en base 2 con una marca por tamano medido."""
    eje.set_xscale("log", base=2)
    eje.xaxis.set_major_locator(FixedLocator(ns))
    eje.xaxis.set_major_formatter(FuncFormatter(lambda x, _: "%d" % x))
    eje.xaxis.set_minor_locator(NullLocator())
    eje.set_xlabel("Número de dígitos (n)")
    eje.tick_params(axis="x", labelsize=8)


def leyenda_abajo(figura, eje, columnas):
    """Leyenda debajo del area de datos, para que no tape ninguna curva."""
    eje.legend(loc="upper center", bbox_to_anchor=(0.5, -0.16), ncol=columnas,
               fontsize=8, frameon=False)
    figura.tight_layout()


def grafica_tiempos(principal, umbral, ns, destino):
    figura, eje = plt.subplots(figsize=TAMANO)

    eje.plot(ns, [principal[n][0] for n in ns], label="Clásica",
             markersize=4, linewidth=1.2, **ESTILOS["clasica"])
    eje.plot(ns, [principal[n][1] for n in ns], label="Karatsuba, umbral 1",
             markersize=4, linewidth=1.2, **ESTILOS[1])
    for u in UMBRALES:
        eje.plot(ns, [umbral[u][n][1] for n in ns], label="Karatsuba, umbral %d" % u,
                 markersize=4, linewidth=1.2, **ESTILOS[u])

    # Rectas de referencia desplazadas para no superponerse con los datos:
    # pendiente 2 entre la clasica y Karatsuba umbral 1 (anclada en el ultimo
    # tamano), y log2(3) sobre Karatsuba umbral 1 (anclada en n = 256).
    tramo = [n for n in ns if n >= 64]
    n_max = ns[-1]
    t2 = principal[n_max][0] * 2.0
    t15 = principal[256][1] * 2.5
    eje.plot(tramo, [t2 * (n / n_max) ** 2 for n in tramo], color="#555555",
             linestyle=(0, (1, 2)), linewidth=1.0, label="Referencia: pendiente 2")
    eje.plot(tramo, [t15 * (n / 256) ** LOG2_3 for n in tramo], color="#555555",
             linestyle=(0, (4, 2, 1, 2)), linewidth=1.0,
             label="Referencia: pendiente log2(3) = 1.585")

    # Cruce medido de cada umbral: aros concentricos de distinto tamano (dos
    # umbrales pueden cruzar en el mismo n) y una etiqueta por tamano.
    cruces = {}
    for i, u in enumerate(UMBRALES):
        n = cruce_medido(umbral[u], u)
        if n is None:
            continue
        cruces.setdefault(n, []).append(u)
        eje.plot([n], [umbral[u][n][1]], linestyle="none", marker="o",
                 markersize=10 + 4 * i, markerfacecolor="none",
                 markeredgecolor=ESTILOS[u]["color"], markeredgewidth=1.3)
    for j, (n, umbrales_n) in enumerate(sorted(cruces.items())):
        nombres = " y ".join("u%d" % u for u in umbrales_n)
        t = umbral[umbrales_n[0]][n][1]
        eje.annotate("Cruce %s: n = %d" % (nombres, n), xy=(n, t),
                     xytext=(n * 1.6, t / (8 if j == 0 else 4)), fontsize=8,
                     arrowprops=dict(arrowstyle="-", color="#333333", linewidth=0.8))
    if cruces:
        # Entrada de leyenda neutra: el color de cada aro es el de su umbral.
        eje.plot([], [], linestyle="none", marker="o", markersize=9,
                 markerfacecolor="none", markeredgecolor="#333333",
                 label="Cruce medido (aro del color de cada umbral)")

    eje.set_yscale("log")
    eje_digitos(eje, ns)
    eje.set_ylabel("Tiempo por multiplicación (ns)")
    eje.set_title("Tiempo de la multiplicación clásica y de Karatsuba")
    eje.grid(True, which="major", linewidth=0.4, alpha=0.6)
    leyenda_abajo(figura, eje, 2)
    figura.savefig(destino, dpi=DPI, bbox_inches="tight")
    plt.close(figura)


def grafica_razon(principal, umbral, ns, destino):
    figura, eje = plt.subplots(figsize=TAMANO)

    # Cada razon usa la clasica medida en la misma corrida.
    eje.plot(ns, [principal[n][1] / principal[n][0] for n in ns], label="Umbral 1",
             markersize=4, linewidth=1.2, **ESTILOS[1])
    for u in UMBRALES:
        eje.plot(ns, [umbral[u][n][1] / umbral[u][n][0] for n in ns],
                 label="Umbral %d" % u, markersize=4, linewidth=1.2, **ESTILOS[u])

    eje.axhline(1.0, color="#000000", linewidth=1.0)
    eje.axhspan(0.1, 1.0, color="#cccccc", alpha=0.35, linewidth=0,
                label="Razón < 1: Karatsuba más rápida")

    # Escala log en y: la razon de umbral 1 llega a ~30 y las demas bajan a
    # ~0.25; en escala lineal estas ultimas quedarian aplastadas.
    eje.set_yscale("log")
    marcas = [0.25, 0.5, 1, 2, 5, 10, 20, 40]
    eje.yaxis.set_major_locator(FixedLocator(marcas))
    eje.yaxis.set_major_formatter(FuncFormatter(lambda y, _: "%g" % y))
    eje.yaxis.set_minor_locator(NullLocator())
    eje.set_ylim(0.18, 50)
    eje_digitos(eje, ns)
    eje.set_ylabel("Razón de tiempos Karatsuba / clásica")
    eje.set_title("Razón de tiempos Karatsuba / clásica por umbral del caso base")
    eje.grid(True, which="major", linewidth=0.4, alpha=0.6)
    leyenda_abajo(figura, eje, 3)
    figura.savefig(destino, dpi=DPI, bbox_inches="tight")
    plt.close(figura)


def main():
    principal = leer_principal(ruta("results", "tiempos.csv"))
    umbral = leer_umbral(ruta("results", "tiempos_umbral.csv"))
    ns = sorted(principal)

    for nombre, funcion in (("grafica_tiempos.png", grafica_tiempos),
                            ("grafica_razon.png", grafica_razon)):
        destino = ruta("results", nombre)
        funcion(principal, umbral, ns, destino)
        print("Escrito %s" % os.path.relpath(destino, RAIZ))


if __name__ == "__main__":
    main()
