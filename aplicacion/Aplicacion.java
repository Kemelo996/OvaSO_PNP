package aplicacion;

import libproceso.JavaProceso;

public class Aplicacion {

    // Declaramos un objeto que represente a la biblioteca
    JavaProceso procesoPrioridades = null;

    static {
        System.loadLibrary("Proceso");  // Carga la biblioteca nativa
    }

    public Aplicacion() {
        this.procesoPrioridades = new JavaProceso();

        String json = "{"
                + "  \"procesos\": ["
                + "    { \"id\": 1, \"llegada\": 0, \"rafaga\": 10, \"prioridad\": 3 },"
                + "    { \"id\": 2, \"llegada\": 2, \"rafaga\": 8, \"prioridad\": 2 },"
                + "    { \"id\": 3, \"llegada\": 4, \"rafaga\": 6, \"prioridad\": 1 },"
                + "    { \"id\": 4, \"llegada\": 6, \"rafaga\": 4, \"prioridad\": 1 },"
                + "    { \"id\": 5, \"llegada\": 8, \"rafaga\": 2, \"prioridad\": 1 }"
                + "  ]"
                + "}";

        System.out.println("Resultado del algoritmo por prioridades:");
        System.out.println(procesoPrioridades.algoritmo_Prioridades(json));
    }

    public static void main(String[] args) {
        new Aplicacion();
    }
}
