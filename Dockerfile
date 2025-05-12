# Usa la imagen oficial de OpenJDK con herramientas de compilación
FROM eclipse-temurin:21-jdk-jammy

# Instala dependencias esenciales para compilación C
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    gcc \
    make \
    libcjson-dev \
    && rm -rf /var/lib/apt/lists/*

# Configura variables de entorno
ENV JAVA_HOME=/opt/java/openjdk
ENV PATH="${JAVA_HOME}/bin:${PATH}"

# Copia el código fuente
COPY . /app
WORKDIR /app

# Compila la librería nativa
RUN cd libproceso && \
    gcc -shared -o libProceso.so libProceso_JavaProceso.c \
    -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux \
    -lcjson -fPIC

# Compila las clases Java
RUN javac aplicacion/Aplicacion.java libproceso/JavaProceso.java

# Comando para ejecutar
CMD ["java", "-Djava.library.path=/app/libproceso", "-cp", ".", "aplicacion.Aplicacion"]
