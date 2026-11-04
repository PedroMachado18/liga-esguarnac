# Usamos una imagen oficial de C++ (Linux)
FROM gcc:12

# Creamos una carpeta de trabajo en la nube
WORKDIR /app

# Copiamos el código y las librerías
COPY ["Liga Esguarnac.cpp", "."]
COPY httplib.h .
COPY json.hpp .
COPY *.json ./

# Compilamos el código para Linux
RUN g++ "Liga Esguarnac.cpp" -o servidor -std=c++17 -pthread

# Exponemos el puerto
EXPOSE 8080

# Encendemos el servidor
CMD ["./servidor"]