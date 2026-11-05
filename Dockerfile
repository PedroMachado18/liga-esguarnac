# Usamos una imagen oficial de C++ (Linux)
FROM ubuntu:latest

# Creamos una carpeta de trabajo en la nube
WORKDIR /app

# Actualizar repositorios e instalar g++ y OpenSSL
RUN apt-get update && \
    apt-get install -y g++ libssl-dev && \
    rm -rf /var/lib/apt/lists/*
    
# Copiar el código al contenedor
COPY "Liga Esguarnac.cpp" .
COPY httplib.h .
COPY json.hpp .

# Compilar el código para Linux
RUN g++ "Liga Esguarnac.cpp" -o servidor -std=c++17 -pthread -lssl -lcrypto

# Exponemos el puerto
EXPOSE $PORT
CMD ["./servidor"]

# Encendemos el servidor
CMD ["./servidor"]