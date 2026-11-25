# xv6-riscv con Dentry Cache (caché de rutas con búsquedas negativas)

Trabajo final – Sistemas Operativos.
Extensión de xv6-riscv que agrega una **Dentry Cache** (tabla hash de 16 buckets, 64 entradas, reemplazo LRU) a `dirlookup()`, con caché **positiva** y **negativa**, invalidación en `dirlink()` y `unlink()`, y estadísticas expuestas por la llamada al sistema `dcachestat()`.

## Integrantes

- Est. AGUIRRE QUISPE, Rall Anderson
- Est. ALANYA VASQUEZ, Antony Brandon
- Est. BARZOLA CURO, Vicente Antony
- Est. VASQUEZ CARHUAS, Nolberto Germán
- Est. ROMANI CHOQUE, Roy Jhoset

## Requisitos

- Toolchain RISC-V (`riscv64-unknown-elf-gcc` o `riscv64-linux-gnu-gcc`)
- `qemu-system-riscv64`
- `make`

En Ubuntu/Debian:

```bash
sudo apt install build-essential gcc-riscv64-linux-gnu qemu-system-misc
```

## Compilar y ejecutar

```bash
git clone https://github.com/rallaguirre27-tech/xv6-riscv
cd xv6-riscv
make clean
make qemu
```

Para salir de QEMU: `Ctrl+A` y luego `X`.

## Estructura de la extensión

| Archivo | Contenido |
|---|---|
| `kernel/dcache.c`, `kernel/dcache.h` | Tabla hash, LRU, `dcache_lookup/insert/remove`, contadores |
| `kernel/fs.c` | `dirlookup()` consulta la caché; `dirlink()` invalida entradas |
| `kernel/sysfile.c` | `unlink()` invalida la entrada |
| `kernel/sysproc.c`, `syscall.*` | Llamada `dcachestat()` (SYS 47) |
| `user/dcachetest.c` | Pruebas funcionales |
| `user/dcachebench.c` | Benchmark de rendimiento |

## Estadísticas (`struct dcache_stat`)

| Campo | Descripción |
|---|---|
| `hits` | Búsquedas resueltas por la caché |
| `misses` | Búsquedas que recorrieron el directorio |
| `pos_hits` | Aciertos positivos |
| `neg_hits` | Aciertos negativos |
| `entries_scanned` | Entradas de directorio examinadas |
| `invalidations` | Entradas invalidadas |
| `evictions` | Entradas reemplazadas por falta de espacio |

Fórmulas: `Hit rate = hits / (hits + misses)` y `Miss rate = misses / (hits + misses)`.

## Pruebas funcionales

Dentro de xv6:

```
$ dcachetest
```

| Prueba | Qué verifica |
|---|---|
| T1 | Búsqueda positiva repetida (200 veces) |
| T2 | Búsqueda negativa repetida (200 veces) |
| T3 | Negativo → crear → el archivo existe (invalidación) |
| T4 | Positivo → `unlink` → el archivo ya no existe |
| T5 | `link`/`unlink` consistentes |
| T6 | Directorios anidados y rutas relativas |
| T7 | Crear/borrar/crear 20 ciclos con el mismo nombre |
| T8 | 4 procesos concurrentes |
| T9 | Los contadores `hits` y `neg_hits` aumentan |

Salida esperada al final:

```
== Resultado: 10 PASS, 0 FAIL ==
```

(9 pruebas, con T9 dividida en T9a y T9b, dan 10 comprobaciones.)

## Benchmark

```
$ dcachebench <nfiles> <iteraciones> [nprocs]
```

Ejemplos para comparar directorios de distinto tamaño:

```
$ dcachebench 10 1000
$ dcachebench 50 1000
$ dcachebench 200 1000
$ dcachebench 50 1000 4
```

La salida es CSV (`prueba,archivos,iteraciones,procesos,ticks`) seguida de las estadísticas de la caché.

### Comparación con xv6 original

1. Ejecutar el benchmark en esta versión y guardar los ticks y las estadísticas.
2. Para la versión sin caché, desactivar las llamadas a `dcache_lookup`/`dcache_insert` en `dirlookup()` (o usar el commit original de xv6) y volver a ejecutar los mismos comandos.
3. Comparar `ticks` y `entries_scanned` en una tabla.

## Resultados

Tiempo en ticks (mediana de 3 corridas; menor es mejor). Aceleración = original / Dentry Cache.

| Prueba | Archivos | Procesos | xv6 original | Dentry Cache | Aceleración |
|---|---|---|---|---|---|
| busqueda_positiva | 10 | 1 | 161 | 19 (*) | 8,5× |
| busqueda_negativa | 10 | 1 | 153 | 10 (*) | 15,3× |
| crear_borrar | 10 | 1 | 114 | 82 (*) | 1,4× |
| busqueda_positiva | 100 | 1 | 413 | 18 | 22,9× |
| busqueda_negativa | 100 | 1 | 407 | 10 | 40,7× |
| crear_borrar | 100 | 1 | 243 | 179 | 1,4× |
| busqueda_positiva | 100 | 4 | 1278 | 35 | 36,5× |
| busqueda_negativa | 100 | 4 | 1274 | 19 | 67,1× |
| crear_borrar | 100 | 4 | 686 | 636 | 1,1× |

(*) Una sola corrida.

- `dcachetest`: **10 PASS, 0 FAIL** con Dentry Cache (8 PASS en xv6 original, que no tiene las 2 pruebas de estadísticas).
- Tasa de aciertos de la caché superior al 99,8 % en las corridas con 100 archivos.
- Las búsquedas dejan de depender del tamaño del directorio; crear/borrar mejora poco porque son escrituras.
- Los tiempos varían entre sesiones de la máquina virtual (hasta casi 2×); por eso se usan 3 corridas y la mediana, y las dos versiones deben compararse en la misma sesión.

## Documentación

- Documento técnico (PDF): [`DocumentoTecnico.pdf`](documento_TrabajoFinal/DocumentoTecnico.pdf?raw=true)
- Presentación final: [`ProyectoFinal.pptx`](documento_TrabajoFinal/ProyectoFinal.pptx?raw=true)

## Autoría

Cada integrante trabajó en su propia rama y cuenta de GitHub; el historial de commits refleja los aportes individuales.
