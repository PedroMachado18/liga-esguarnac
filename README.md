#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

class Jugador {
private:
    std::string nombre;
    int turnosAlBate;
    int hits;
    int dobles;
    int triples;
    int jonrones;
    int culos;

public:
    // Constructor
    Jugador(std::string _nombre) : nombre(_nombre), turnosAlBate(0), hits(0), dobles(0), triples(0), jonrones(0) {}

    // Método para agregar estadísticas de un juego
    void registrarEstadisticas(int tb, int h, int d, int t, int hr) {
        turnosAlBate += tb;
        hits += h;
        dobles += d;
        triples += t;
        jonrones += hr;
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

int main() {
    std::vector<Jugador> equipo;

    // Crear jugadores
    Jugador j1("Miguel Cabrera");
    Jugador j2("Ronald Acuna");

    // Registrar estadísticas (Turnos, Hits, Dobles, Triples, Jonrones)
    j1.registrarEstadisticas(4, 2, 1, 0, 0); // Se fue de 4-2 con un doble
    j1.registrarEstadisticas(3, 1, 0, 0, 1); // Se fue de 3-1 con un jonrón

    j2.registrarEstadisticas(5, 3, 1, 1, 0); // Se fue de 5-3 con doble y triple

    // Agregar al equipo
    equipo.push_back(j1);
    equipo.push_back(j2);

    // Mostrar la tabla de estadísticas
    std::cout << "------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(15) << "Nombre"
              << std::setw(5) << "TB"
              << std::setw(5) << "H"
              << std::setw(5) << "2B"
              << std::setw(5) << "3B"
              << std::setw(5) << "HR"
              << std::setw(8) << "AVG"
              << std::setw(8) << "SLG" << "\n";
    std::cout << "------------------------------------------------------------------\n";

    for (const auto& jugador : equipo) {
        jugador.imprimirFila();
    }

    return 0;
}