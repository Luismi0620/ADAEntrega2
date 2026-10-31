#!/bin/bash
# Registra el entorno de medicion del benchmark en la salida estandar.
# Uso: bash bench/registrar_entorno.sh <compilador> <banderas> <salida_benchmark>
# Se invoca con bash explicito porque en /mnt/c el bit de ejecucion no queda
# registrado en git.
# La resolucion del reloj se toma de la salida guardada del benchmark.

compilador="$1"
banderas="$2"
salida="$3"

echo "Fecha: $(date '+%Y-%m-%d %H:%M:%S %z')"
echo "Compilador: $("$compilador" --version | head -n 1)"
echo "Banderas: $banderas"
echo "Distribucion WSL: ${WSL_DISTRO_NAME:-no detectada}"
echo "Sistema: $(. /etc/os-release && echo "$PRETTY_NAME")"
echo "Kernel: $(uname -r)"
echo "CPU: $(LC_ALL=C lscpu | sed -n 's/^Model name:[[:space:]]*//p')"
echo "Nucleos logicos: $(nproc)"
grep '^Resolucion' "$salida"
grep '^Tiempo total' "$salida"
