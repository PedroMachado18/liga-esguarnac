#include <iostream>
#include <vector>

#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include "httplib.h"
#include "json.hpp"

class Jugador {
private:
    std::string nombre;
    int turnosAlBate;
    int hits;
    int dobles;
    int triples;
    int jonrones;
    int boletos;
    int carreraImpulsadas; 
    int carrerasAnotadas;
    int ponches;

public:
    // Constructor
    Jugador(std::string _nombre) : nombre(_nombre), turnosAlBate(0), hits(0), dobles(0), triples(0), jonrones(0) {}

    // Método para agregar estadísticas de un juego
    void registrarEstadisticas(int tb, int h, int d, int t, int hr , int bb , int ci , int ca ,int k ) {
        turnosAlBate += tb;
        hits += h;
        dobles += d;
        triples += t;
        jonrones += hr;
        boletos += bb;
        carreraImpulsadas += ci;
        carrerasAnotadas += ca;
        ponches += k;
    }

    // Cálculo del Average (Promedio de bateo = Hits / Turnos al bate)
    double calcularAverage() const {
        if (turnosAlBate == 0) return 0.000;
        return static_cast<double>(hits) / turnosAlBate;
    }

    // Cálculo del Slugging ((1*S + 2*D + 3*T + 4*HR) / Turnos al bate)
    double calcularSlugging() const {
        if (turnosAlBate == 0) return 0.000;
        int sencillos = hits - (dobles + triples + jonrones);
        int basesTotales = sencillos + (2 * dobles) + (3 * triples) + (4 * jonrones);
        return static_cast<double>(basesTotales) / turnosAlBate;
    }

    // Imprimir fila del jugador
    void imprimirFila() const {
        std::cout << std::left << std::setw(15) << nombre
                  << std::setw(5) << turnosAlBate
                  << std::setw(5) << hits
                  << std::setw(5) << dobles
                  << std::setw(5) << triples
                  << std::setw(5) << jonrones
                  << std::setw(8) << std::fixed << std::setprecision(3) << calcularAverage()
                  << std::setw(8) << calcularSlugging() << "\n";
    }
};

// Aquí va tu clase Jugador que ya creaste
class Pitcher {
public:
    std::string nombre;
    double ip; // Innings Lanzados (usamos double por si lanzan medias entradas)
    int k;     // Ponches
    int cl;    // Carreras Limpias
    int cs;    // Carreras Sucias
    int g;     // Ganados
    int p;     // Perdidos

    Pitcher(std::string _nombre) : nombre(_nombre), ip(0), k(0), cl(0), cs(0), g(0), p(0) {}

    void registrarEstadisticas(double _ip, int _k, int _cl, int _cs, int _g, int _p) {
        ip += _ip;
        k += _k;
        cl += _cl;
        cs += _cs;
        g += _g;
        p += _p;
    }

    double calcularEfectividad() const {
        if (ip <= 0) return 0.00;
        // Fórmula oficial de Efectividad (ERA): (Carreras Limpias * 9) / Innings Lanzados
        return (cl * 9.0) / ip;
    }
};
using json = nlohmann::json;
json baseDeDatos = json ::array();
json baseDeDatosEquipos = json::array();
const std::string ARCHIVO_EQUIPOS = "equipos.json";

class EquipoObj {
public:
    std::string nombre;
    int jg; // Juegos Ganados
    int jp; // Juegos Perdidos
    int cf; // Carreras a Favor
    int cc; // Carreras en Contra

    EquipoObj(std::string _nombre) : nombre(_nombre), jg(0), jp(0), cf(0), cc(0) {}

    void registrar(int _jg, int _jp, int _cf, int _cc) {
        jg += _jg;
        jp += _jp;
        cf += _cf;
        cc += _cc;
    }
};
const std::string ARCHIVO_DATOS = "estadisticas.json";
json baseDeDatosPitchers = json::array();
const std::string ARCHIVO_PITCHERS = "pitchers.json";

void cargarDatos() {
    std::ifstream aBat(ARCHIVO_DATOS);
    if (aBat.is_open()) { aBat >> baseDeDatos; aBat.close(); }
    
    std::ifstream aPit(ARCHIVO_PITCHERS);
    if (aPit.is_open()) { aPit >> baseDeDatosPitchers; aPit.close(); }

    std::ifstream aEq(ARCHIVO_EQUIPOS);
    if (aEq.is_open()) { aEq >> baseDeDatosEquipos; aEq.close(); }
}

void guardarDatos() {
    std::ofstream aBat(ARCHIVO_DATOS);
    if (aBat.is_open()) { aBat << baseDeDatos.dump(4); aBat.close(); }
    
    std::ofstream aPit(ARCHIVO_PITCHERS);
    if (aPit.is_open()) { aPit << baseDeDatosPitchers.dump(4); aPit.close(); }

    std::ofstream aEq(ARCHIVO_EQUIPOS);
    if (aEq.is_open()) { aEq << baseDeDatosEquipos.dump(4); aEq.close(); }
}


int main() {
    cargarDatos(); // Cargar los datos previos al arrancar el backend
    httplib::Server svr;

    svr.Options(".*", [](const auto& req, auto& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, GET, DELETE, PUT, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    });

    // 1. Endpoint GET: Enviar todos los jugadores guardados a la web
    svr.Get("/api/jugadores", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        json respuesta = baseDeDatos; 
        res.set_content(respuesta.dump(), "application/json");
    });
    // Endpoint GET: Enviar todos los pitchers a la web
    svr.Get("/api/pitchers", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(baseDeDatosPitchers.dump(), "application/json");
    });

    // Endpoint POST: Registrar o actualizar pitcher
svr.Post("/api/registrar_pitcher", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto datos = json::parse(req.body);
        std::string nombreBuscado = datos.value("nombre", "Desconocido");

        bool pitcherExiste = false;

        for (auto& pGuardado : baseDeDatosPitchers) {
            if (pGuardado.value("nombre", "") == nombreBuscado) {
                // Usamos .value() para extraer el número de forma segura, convirtiendo tipos automáticamente
                double ipActual = pGuardado.value("ip", 0.0) + datos.value("ip", 0.0);
                int kActual   = pGuardado.value("k", 0) + datos.value("k", 0);
                int clActual  = pGuardado.value("cl", 0) + datos.value("cl", 0);
                int csActual  = pGuardado.value("cs", 0) + datos.value("cs", 0);
                int gActual   = pGuardado.value("g", 0) + datos.value("g", 0);
                int pActual   = pGuardado.value("p", 0) + datos.value("p", 0);

                pGuardado["ip"] = ipActual;
                pGuardado["k"]  = kActual;
                pGuardado["cl"] = clActual;
                pGuardado["cs"] = csActual;
                pGuardado["g"]  = gActual;
                pGuardado["p"]  = pActual;

                Pitcher temp(nombreBuscado);
                temp.registrarEstadisticas(ipActual, kActual, clActual, csActual, gActual, pActual);

                pGuardado["era"] = temp.calcularEfectividad();
                pitcherExiste = true;
                break;
            }
        }

        if (!pitcherExiste) {
            Pitcher p(nombreBuscado);
            p.registrarEstadisticas(
                datos.value("ip", 0.0), datos.value("k", 0), datos.value("cl", 0), 
                datos.value("cs", 0), datos.value("g", 0), datos.value("p", 0)
            );

            json nuevoPitcher = datos;
            nuevoPitcher["era"] = p.calcularEfectividad();
            baseDeDatosPitchers.push_back(nuevoPitcher);
        }

        guardarDatos();
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // 2. Endpoint POST: Registrar nuevo o actualizar existente
    svr.Post("/api/registrar", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto datos = json::parse(req.body);
        std::string nombreBuscado = datos["nombre"].get<std::string>();

        bool jugadorExiste = false;

        for (auto& jugadorGuardado : baseDeDatos) {
            if (jugadorGuardado["nombre"] == nombreBuscado) {
                jugadorGuardado["tb"] = jugadorGuardado["tb"].get<int>() + datos["tb"].get<int>();
                jugadorGuardado["h"]  = jugadorGuardado["h"].get<int>()  + datos["h"].get<int>();
                jugadorGuardado["d2b"] = jugadorGuardado["d2b"].get<int>() + datos["d2b"].get<int>();
                jugadorGuardado["d3b"] = jugadorGuardado["d3b"].get<int>() + datos["d3b"].get<int>();
                jugadorGuardado["hr"]  = jugadorGuardado["hr"].get<int>()  + datos["hr"].get<int>();
                jugadorGuardado["bb"]  = jugadorGuardado["bb"].get<int>()  + datos["bb"].get<int>();
                jugadorGuardado["ci"]  = jugadorGuardado["ci"].get<int>()  + datos["ci"].get<int>();
                jugadorGuardado["ca"]  = jugadorGuardado["ca"].get<int>()  + datos["ca"].get<int>();
                jugadorGuardado["k"]   = jugadorGuardado["k"].get<int>()   + datos["k"].get<int>();

                Jugador temp(nombreBuscado);
                temp.registrarEstadisticas(
                    jugadorGuardado["tb"].get<int>(), jugadorGuardado["h"].get<int>(),
                    jugadorGuardado["d2b"].get<int>(), jugadorGuardado["d3b"].get<int>(),
                    jugadorGuardado["hr"].get<int>(), jugadorGuardado["bb"].get<int>(),
                    jugadorGuardado["ci"].get<int>(), jugadorGuardado["ca"].get<int>(),
                    jugadorGuardado["k"].get<int>()
                );

                jugadorGuardado["avg"] = temp.calcularAverage();
                jugadorGuardado["slg"] = temp.calcularSlugging();
                jugadorExiste = true;
                break;
            }
        }

        if (!jugadorExiste) {
            Jugador j(nombreBuscado);
            j.registrarEstadisticas(
                datos["tb"].get<int>(), datos["h"].get<int>(), datos["d2b"].get<int>(), 
                datos["d3b"].get<int>(), datos["hr"].get<int>(), datos["bb"].get<int>(), 
                datos["ci"].get<int>(), datos["ca"].get<int>(), datos["k"].get<int>()
            );

            json nuevoJugador = datos;
            nuevoJugador["avg"] = j.calcularAverage();
            nuevoJugador["slg"] = j.calcularSlugging();
            baseDeDatos.push_back(nuevoJugador);
        }

        guardarDatos();
        res.set_content("{\"status\":\"ok\"}", "application/json");
    }); // <-- ESTE CIERRE FALTABA

    // 3. Endpoint DELETE: Eliminar jugador
    svr.Delete("/api/eliminar", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto datos = json::parse(req.body);
        std::string nombreAEliminar = datos["nombre"].get<std::string>();
        std::string equipoAEliminar = datos["equipo"].get<std::string>();

        baseDeDatos.erase(std::remove_if(baseDeDatos.begin(), baseDeDatos.end(),
            [&](const json& j) { return j["nombre"] == nombreAEliminar && j["equipo"] == equipoAEliminar; }),
            baseDeDatos.end());

        guardarDatos();
        res.set_content("{\"status\":\"eliminado\"}", "application/json");
    });

    // 4. Endpoint PUT: Modificar jugador existente
    svr.Put("/api/modificar", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto datos = json::parse(req.body);
        std::string nombreBuscado = datos["nombre"].get<std::string>();
        std::string equipoBuscado = datos["equipo"].get<std::string>();

        for (auto& j_json : baseDeDatos) {
            if (j_json["nombre"] == nombreBuscado && j_json["equipo"] == equipoBuscado) {
                Jugador j(nombreBuscado);
                j.registrarEstadisticas(
                    datos["tb"].get<int>(), datos["h"].get<int>(), datos["d2b"].get<int>(), 
                    datos["d3b"].get<int>(), datos["hr"].get<int>(), datos["bb"].get<int>(), 
                    datos["ci"].get<int>(), datos["ca"].get<int>(), datos["k"].get<int>()
                );

                j_json["equipo"] = datos["equipo"];
                j_json["tb"] = datos["tb"];
                j_json["h"] = datos["h"];
                j_json["d2b"] = datos["d2b"];
                j_json["d3b"] = datos["d3b"];
                j_json["hr"] = datos["hr"];
                j_json["bb"] = datos["bb"];
                j_json["ci"] = datos["ci"];
                j_json["ca"] = datos["ca"];
                j_json["k"] = datos["k"];
                j_json["avg"] = j.calcularAverage();
                j_json["slg"] = j.calcularSlugging();
                break;
            }
        }

        guardarDatos();
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });
    // Endpoint PUT: Modificar récord del equipo (Sobrescribir datos)
    svr.Put("/api/modificar_equipo", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto datos = json::parse(req.body);
        std::string nombreBuscado = datos["nombre"].get<std::string>();

        for (auto& eq : baseDeDatosEquipos) {
            if (eq["nombre"] == nombreBuscado) {
                // En lugar de sumar, aquí reemplazamos por los nuevos valores
                eq["jg"] = datos["jg"].get<int>();
                eq["jp"] = datos["jp"].get<int>();
                eq["je"] = datos["je"].get<int>();
                eq["cf"] = datos["cf"].get<int>();
                eq["cc"] = datos["cc"].get<int>();
                
                // Recalcular puntos con los nuevos valores
                eq["pts"] = (eq["jg"].get<int>() * 3) + (eq["je"].get<int>() * 1);
                break;
            }
        }

        guardarDatos();
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    std::cout << "Servidor API de Beisbol corriendo en http://localhost:8080...\n";
    // Endpoint DELETE: Eliminar Pitcher
    svr.Delete("/api/eliminar_pitcher", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto datos = json::parse(req.body);
        std::string nombreAEliminar = datos["nombre"].get<std::string>();
        std::string equipoAEliminar = datos["equipo"].get<std::string>();

        baseDeDatosPitchers.erase(std::remove_if(baseDeDatosPitchers.begin(), baseDeDatosPitchers.end(),
            [&](const json& p) { return p["nombre"] == nombreAEliminar && p["equipo"] == equipoAEliminar; }),
            baseDeDatosPitchers.end());

        guardarDatos();
        res.set_content("{\"status\":\"eliminado\"}", "application/json");
    });

    // Endpoint PUT: Modificar Pitcher
    svr.Put("/api/modificar_pitcher", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto datos = json::parse(req.body);
        std::string nombreBuscado = datos["nombre"].get<std::string>();
        std::string equipoBuscado = datos["equipo"].get<std::string>();

        for (auto& p_json : baseDeDatosPitchers) {
            if (p_json["nombre"] == nombreBuscado && p_json["equipo"] == equipoBuscado) {
                Pitcher p(nombreBuscado);
                p.registrarEstadisticas(
                    datos["ip"].get<double>(), datos["k"].get<int>(), datos["cl"].get<int>(), 
                    datos["cs"].get<int>(), datos["g"].get<int>(), datos["p"].get<int>()
                );

                p_json["equipo"] = datos["equipo"];
                p_json["ip"] = datos["ip"];
                p_json["k"] = datos["k"];
                p_json["cl"] = datos["cl"];
                p_json["cs"] = datos["cs"];
                p_json["g"] = datos["g"];
                p_json["p"] = datos["p"];
                p_json["era"] = p.calcularEfectividad();
                break;
            }
        }

        guardarDatos();
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });
    // Endpoint GET: Enviar tabla de equipos
    svr.Get("/api/equipos", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(baseDeDatosEquipos.dump(), "application/json");
    });

    // Endpoint POST: Registrar o actualizar récord del equipo
// Endpoint POST: Registrar o actualizar récord del equipo (con Puntos)
    svr.Post("/api/actualizar_equipo", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto datos = json::parse(req.body);
        std::string nombreBuscado = datos["nombre"].get<std::string>();

        bool existe = false;

        for (auto& eq : baseDeDatosEquipos) {
            if (eq["nombre"] == nombreBuscado) {
                eq["jg"] = eq["jg"].get<int>() + datos["jg"].get<int>();
                eq["jp"] = eq["jp"].get<int>() + datos["jp"].get<int>();
                
                // Leemos los empates de forma segura por si el archivo viejo no lo tenía
                int empatesActuales = eq.contains("je") ? eq["je"].get<int>() : 0;
                eq["je"] = empatesActuales + datos["je"].get<int>();
                
                eq["cf"] = eq["cf"].get<int>() + datos["cf"].get<int>();
                eq["cc"] = eq["cc"].get<int>() + datos["cc"].get<int>();
                
                // Calcular puntos: 3 por Ganado, 1 por Empatado, 0 por Perdido
                eq["pts"] = (eq["jg"].get<int>() * 3) + (eq["je"].get<int>() * 1);
                
                existe = true;
                break;
            }
        }

        if (!existe) {
            // Si el equipo es nuevo, scalculamos sus puntos iniciales
            datos["pts"] = (datos["jg"].get<int>() * 3) + (datos["je"].get<int>() * 1);
            baseDeDatosEquipos.push_back(datos);
        }

        guardarDatos();
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });
    svr.Get("/resultados", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        std::ifstream archivo("resultados.json");
        if (archivo.is_open()) {
            std::stringstream buffer;
            buffer << archivo.rdbuf();
            res.set_content(buffer.str(), "application/json");
        } else {
            res.set_content("[]", "application/json");
        }
    });

    // 2. Ruta para recibir y guardar un nuevo resultado (POST)
    svr.Post("/resultados", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            json nuevo_juego = json::parse(req.body);
            json resultados = json::array();

            std::ifstream archivo_entrada("resultados.json");
            if (archivo_entrada.is_open()) {
                archivo_entrada >> resultados;
                archivo_entrada.close();
            }

            resultados.push_back(nuevo_juego);

            std::ofstream archivo_salida("resultados.json");
            archivo_salida << resultados.dump(4);
            archivo_salida.close();

            res.set_content("{\"status\":\"success\"}", "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content("{\"error\":\"Error interno del servidor\"}", "application/json");
        }
    });

    // 3. Ruta OPTIONS (CORS) obligatoria para evitar bloqueos del navegador
svr.Delete("/resultados", [](const httplib::Request& req, httplib::Response& res) {
    // 1. Dar permiso CORS
    res.set_header("Access-Control-Allow-Origin", "*");

    // 2. Verificar si JavaScript envió el parámetro ?id=...
    if (req.has_param("id")) {
        int index = std::stoi(req.get_param_value("id"));

        // 3. Abrir la base de datos
        std::ifstream file("resultados.json");
        nlohmann::json resultados;
        if (file.is_open()) {
            file >> resultados;
            file.close();
        } else {
            res.status = 500;
            res.set_content("Error al abrir base de datos", "text/plain");
            return;
        }

        // 4. Borrar la posición exacta
        if (index >= 0 && index < resultados.size()) {
            resultados.erase(index);

            // 5. Guardar los cambios
            std::ofstream outFile("resultados.json");
            outFile << resultados.dump(4);
            outFile.close();

            res.status = 200;
            res.set_content("Eliminado", "text/plain");
        } else {
            res.status = 404;
            res.set_content("No encontrado", "text/plain");
        }
    } else {
        res.status = 400;
        res.set_content("Falta enviar el ID", "text/plain");
    }
});

    int port = 8080;
    if (const char* env_p = std::getenv("PORT")) {
        port = std::stoi(env_p);
    }

    std::cout << "Servidor API de Beisbol corriendo en el puerto " << port << "..." << std::endl;
    
    // 2. Usar 0.0.0.0 es OBLIGATORIO para recibir conexiones desde el exterior en Render
    svr.listen("0.0.0.0", port);