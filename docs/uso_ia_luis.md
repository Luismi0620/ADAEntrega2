# Uso de inteligencia artificial

Notas para la declaración de uso de IA del informe. Cubren mi parte de la
práctica en la rama `feature/clasica-luis`: multiplicación clásica, funciones
auxiliares sobre números decimales, pruebas, documentación y preparación de la
interfaz para integrar el algoritmo de Karatsuba. Estas notas se unificarán con
las del otro integrante en la versión final del informe.

## Herramientas

- **OpenAI Codex:** lo utilicé como apoyo para revisar el enunciado, la rúbrica
  y el reparto de tareas; proponer la estructura del código; implementar y
  probar la multiplicación clásica y las funciones auxiliares; redactar la
  documentación; y organizar los commits de mi rama.
- **Compilador `g++`:** se utilizó para compilar el código con C++17 y las
  advertencias `-Wall -Wextra -Wpedantic`.
- **Python y `System.Numerics.BigInteger`:** se usaron únicamente como
  referencias externas durante las pruebas. No forman parte de la
  implementación entregada ni calculan los resultados del programa final.

## Trabajo realizado con apoyo de IA

- Se revisaron los documentos de la práctica para identificar que mis tareas
  individuales eran T04, multiplicación clásica, y T05, funciones auxiliares.
- Se definió una representación basada en `std::string`, con los dígitos en
  base 10 y el dígito más significativo a la izquierda.
- Se implementaron las siguientes operaciones:
  - `normalizar`, para eliminar ceros a la izquierda y representar siempre el
    cero como `"0"`;
  - `sumar`, admitiendo operandos de longitudes diferentes y un posible
    acarreo final;
  - `restar`, con la precondición `A >= B` y préstamos entre dígitos;
  - `desplazar`, para calcular `A * 10^k` sin convertir el número completo a
    un tipo nativo;
  - `partir`, para obtener las partes alta y baja respecto a las últimas `m`
    posiciones decimales;
  - `multiplicacionClasica`, usando el algoritmo escolar sobre los dígitos.
- Se creó un programa de consola para ejecutar la multiplicación clásica y se
  documentaron los comandos de compilación y ejecución.
- Se añadieron pruebas para ceros a la izquierda, longitudes diferentes,
  acarreos, préstamos, ceros intercalados, entradas inválidas y productos que
  exceden la capacidad de los tipos enteros nativos.
- Se creó la rama `feature/clasica-luis`, se organizaron los cambios en
  commits separados de implementación, pruebas y documentación, y se publicó
  la rama en GitHub.

## Decisiones y verificaciones realizadas

- La implementación no usa `long long`, `__int128` ni bibliotecas de precisión
  arbitraria para calcular el producto completo. Solo se usan enteros pequeños
  para operar con dígitos individuales, acarreos y préstamos.
- Todas las funciones validan que las entradas no estén vacías y contengan
  únicamente dígitos decimales.
- `restar` rechaza los casos en los que `A < B`, porque el enunciado garantiza
  que Karatsuba deberá llamar esta operación únicamente con resultados no
  negativos.
- `desplazar("0", k)` devuelve `"0"` en lugar de producir una cadena de ceros.
- Las partes producidas por `partir` quedan normalizadas, pero conservan la
  identidad numérica `A = alta * 10^m + baja`.
- El código se compiló correctamente con C++17 y advertencias estrictas.
- Las pruebas unitarias incluidas en el repositorio pasaron completamente.
- Se compararon 150 productos aleatorios, con operandos de hasta 250 dígitos,
  contra el resultado calculado independientemente con Python; todos
  coincidieron.
- Durante las pruebas se encontró un valor esperado escrito incorrectamente
  para `1002003 * 40005`. Se comprobó de manera independiente que el resultado
  correcto era `40085130015` y se corrigió la prueba.

## Trazabilidad

Los cambios quedaron registrados progresivamente en los siguientes commits:

- `1ae67a8`: implementación de la aritmética decimal y la multiplicación
  clásica;
- `94a929c`: pruebas de las funciones auxiliares y la multiplicación clásica;
- `df8ae35`: instrucciones de compilación, decisiones de diseño y
  documentación.

## Limitaciones

- Estas notas cubren únicamente el componente desarrollado en mi rama. La
  implementación de Karatsuba, el relleno a potencia de dos, la medición de
  tiempos, el punto de cruce y las gráficas corresponden a la otra rama o al
  trabajo compartido y deben documentarse por separado.
- La comparación con Python y `BigInteger` se utilizó solo como oráculo de
  validación durante el desarrollo; el programa entregado no depende de esas
  herramientas.
- El comando de compilación directa con `g++` fue probado. El objetivo de
  `Makefile` no se ejecutó en el entorno de desarrollo porque allí no estaba
  instalado GNU Make.
- Antes de la entrega y de la defensa individual debo revisar el código y ser
  capaz de explicar la representación, los ciclos de la multiplicación
  clásica, el manejo de acarreos y préstamos, y las precondiciones de cada
  función.
