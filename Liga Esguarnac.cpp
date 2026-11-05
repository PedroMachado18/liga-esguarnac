#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <iostream>
#include <vector>

#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include "httplib.h"
#include "json.hpp"
#include <cstdlib>

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
// ==========================================
// RUTAS PARA EQUIPOS
// ==========================================

// 1. GET Equipos
svr.Get("/equipos", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false);
    
    if (auto fire_res = cli.Get("/equipos.json")) {
        if (fire_res->status == 200) {
            std::string data = fire_res->body == "null" ? "[]" : fire_res->body;
            res.set_content(data, "application/json");
        } else {
            res.status = 500;
            res.set_content("Error al leer base de datos en Firebase", "text/plain");
        }
    } else {
        res.status = 500;
        res.set_content("Fallo de conexion a Firebase", "text/plain");
    }
});

// 2. POST Equipos
svr.Post("/equipos", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false);
    
    auto fire_get = cli.Get("/equipos.json");
    
    nlohmann::json lista;
    if (fire_get && fire_get->status == 200 && fire_get->body != "null") {
        lista = nlohmann::json::parse(fire_get->body);
    } else {
        lista = nlohmann::json::array();
    }

    nlohmann::json nuevo = nlohmann::json::parse(req.body);
    lista.push_back(nuevo);

    auto fire_put = cli.Put("/equipos.json", lista.dump(), "application/json");

    if (fire_put && fire_put->status == 200) {
        res.status = 200;
        res.set_content("Guardado exitosamente", "text/plain");
    } else {
        res.status = 500;
        res.set_content("Error al guardar", "text/plain");
    }
});

// 3. DELETE Equipos
svr.Delete("/equipos", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    
    if (req.has_param("id")) {
        int index = std::stoi(req.get_param_value("id"));
        httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
        cli.enable_server_certificate_verification(false);
        
        auto fire_get = cli.Get("/equipos.json");
        
        if (fire_get && fire_get->status == 200 && fire_get->body != "null") {
            nlohmann::json lista = nlohmann::json::parse(fire_get->body);
            
            if (index >= 0 && index < lista.size()) {
                lista.erase(index);
                auto fire_put = cli.Put("/equipos.json", lista.dump(), "application/json");
                
                if (fire_put && fire_put->status == 200) {
                    res.status = 200;
                    res.set_content("Eliminado exitosamente", "text/plain");
                    return;
                }
            }
        }
        res.status = 500;
        res.set_content("Error al borrar", "text/plain");
    } else {
        res.status = 400;
        res.set_content("Falta el ID", "text/plain");
    }
});

// 4. OPTIONS Equipos (CORS)
svr.Options("/equipos", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.status = 200;
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
// 1. Ruta GET (Leer)
svr.Get("/resultados", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false); // <--- SOLUCION DE CONEXION
    
    if (auto fire_res = cli.Get("/resultados.json")) {
        if (fire_res->status == 200) {
            std::string data = fire_res->body == "null" ? "[]" : fire_res->body;
            res.set_content(data, "application/json");
        } else {
            res.status = 500;
            res.set_content("Error al leer base de datos en Firebase", "text/plain");
        }
    } else {
        res.status = 500;
        res.set_content("Fallo de conexion a Firebase", "text/plain");
    }
});

// 2. Ruta POST (Guardar)
svr.Post("/resultados", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false); // <--- SOLUCION DE CONEXION
    
    auto fire_get = cli.Get("/resultados.json");
    
    nlohmann::json resultados;
    if (fire_get && fire_get->status == 200 && fire_get->body != "null") {
        resultados = nlohmann::json::parse(fire_get->body);
    } else {
        resultados = nlohmann::json::array();
    }

    nlohmann::json nuevo = nlohmann::json::parse(req.body);
    resultados.push_back(nuevo);

    auto fire_put = cli.Put("/resultados.json", resultados.dump(), "application/json");

    if (fire_put && fire_put->status == 200) {
        res.status = 200;
        res.set_content("Guardado exitosamente en Firebase", "text/plain");
    } else {
        res.status = 500;
        res.set_content("Error al guardar en Firebase", "text/plain");
    }
});

// 3. Ruta DELETE (Eliminar)
svr.Delete("/resultados", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    
    if (req.has_param("id")) {
        int index = std::stoi(req.get_param_value("id"));
        
        httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
        cli.enable_server_certificate_verification(false); // <--- SOLUCION DE CONEXION
        
        auto fire_get = cli.Get("/resultados.json");
        
        if (fire_get && fire_get->status == 200 && fire_get->body != "null") {
            nlohmann::json resultados = nlohmann::json::parse(fire_get->body);
            
            if (index >= 0 && index < resultados.size()) {
                resultados.erase(index);
                auto fire_put = cli.Put("/resultados.json", resultados.dump(), "application/json");
                
                if (fire_put && fire_put->status == 200) {
                    res.status = 200;
                    res.set_content("Eliminado de Firebase", "text/plain");
                    return;
                }
            }
        }
        res.status = 500;
        res.set_content("Error al borrar en Firebase", "text/plain");
    } else {
        res.status = 400;
        res.set_content("Falta enviar el ID", "text/plain");
    }
});

// 4. Ruta OPTIONS obligatoria para navegadores
svr.Options("/resultados", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.status = 200;
});

// --- FINAL DEL PROGRAMA ---
int port = 8080;
if (const char* env_p = std::getenv("PORT")) {
    port = std::stoi(env_p);
}

std::cout << "Servidor API de Beisbol corriendo en el puerto " << port << "..." << std::endl;

svr.listen("0.0.0.0", port);

return 0;
} // <-- Esta llave cierra la funcion int main()