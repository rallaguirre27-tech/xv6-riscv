#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("[ATENCION] Atendiendo solicitud de documento generico...\n");
    } else {
        printf("[ATENCION] Procesando e imprimiendo el documento: %s\n", argv[1]);
    }

    // Simula el tiempo de procesamiento/impresión
    for (volatile int i = 0; i < 50000000; i++);

    printf("[ATENCION] -> Documento procesado e impreso con éxito.\n");
    exit(0);
}


