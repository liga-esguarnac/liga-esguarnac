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
svr.Get("/api/equipos", [](const httplib::Request& req, httplib::Response& res) {
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
svr.Post("/api/equipos", [](const httplib::Request& req, httplib::Response& res) {
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
svr.Delete("/api/equipos", [](const httplib::Request& req, httplib::Response& res) {
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
svr.Options("/api/equipos", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.status = 200;
});


    // Endpoint POST: Registrar o actualizar récord del equipo
// Endpoint POST: Registrar o actualizar récord del equipo (con Puntos)
// Endpoint POST: Sumar al récord del equipo (con JJ y DIF, eliminando CF y CC viejos)
svr.Post("/api/actualizar_equipo", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false);
    
    nlohmann::json datosRecibidos = nlohmann::json::parse(req.body);
    std::string nombreBuscado = datosRecibidos.value("nombre", "");
    
    auto fire_get = cli.Get("/equipos.json");
    nlohmann::json listaEquipos = nlohmann::json::array();
    if (fire_get && fire_get->status == 200 && fire_get->body != "null") {
        auto parsed = nlohmann::json::parse(fire_get->body);
        if (parsed.is_array()) {
            for (auto& el : parsed) if (!el.is_null()) listaEquipos.push_back(el);
        } else if (parsed.is_object()) {
            for (auto& el : parsed.items()) if (!el.value().is_null()) listaEquipos.push_back(el.value());
        }
    }

    bool encontrado = false;
    for (auto& eq : listaEquipos) {
        if (eq.value("nombre", "") == nombreBuscado) {
            eq["jj"]  = eq.value("jj", 0)  + datosRecibidos.value("jj", 0);
            eq["jg"]  = eq.value("jg", 0)  + datosRecibidos.value("jg", 0);
            eq["jp"]  = eq.value("jp", 0)  + datosRecibidos.value("jp", 0);
            eq["je"]  = eq.value("je", 0)  + datosRecibidos.value("je", 0);
            eq["dif"] = eq.value("dif", 0) + datosRecibidos.value("dif", 0);
            
            // Eliminamos las llaves viejas cf y cc de Firebase si existían
            eq.erase("cf");
            eq.erase("cc");

            // Puntos: (JG * 3) + (JE * 1)
            eq["pts"] = (eq["jg"].get<int>() * 3) + (eq["je"].get<int>() * 1);
            encontrado = true;
            break;
        }
    }

    if (!encontrado) {
        nlohmann::json nuevoEq;
        nuevoEq["nombre"] = nombreBuscado;
        nuevoEq["jj"]  = datosRecibidos.value("jj", 0);
        nuevoEq["jg"]  = datosRecibidos.value("jg", 0);
        nuevoEq["jp"]  = datosRecibidos.value("jp", 0);
        nuevoEq["je"]  = datosRecibidos.value("je", 0);
        nuevoEq["dif"] = datosRecibidos.value("dif", 0);
        nuevoEq["pts"] = (nuevoEq["jg"].get<int>() * 3) + (nuevoEq["je"].get<int>() * 1);
        listaEquipos.push_back(nuevoEq);
    }

    auto fire_put = cli.Put("/equipos.json", listaEquipos.dump(), "application/json");
    if (fire_put && fire_put->status == 200) {
        res.status = 200;
        res.set_content("Actualizado correctamente", "text/plain");
    } else {
        res.status = 500;
        res.set_content("Error al guardar en Firebase", "text/plain");
    }
});

// Endpoint POST: SOBRESCRIBIR récord (Botón Guardar Edición)
svr.Post("/api/editar_equipo", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false);
    
    nlohmann::json datosRecibidos = nlohmann::json::parse(req.body);
    std::string nombreBuscado = datosRecibidos.value("nombre", "");
    
    auto fire_get = cli.Get("/equipos.json");
    nlohmann::json listaEquipos = nlohmann::json::array();
    if (fire_get && fire_get->status == 200 && fire_get->body != "null") {
        auto parsed = nlohmann::json::parse(fire_get->body);
        if (parsed.is_array()) {
            for (auto& el : parsed) if (!el.is_null()) listaEquipos.push_back(el);
        } else if (parsed.is_object()) {
            for (auto& el : parsed.items()) if (!el.value().is_null()) listaEquipos.push_back(el.value());
        }
    }

    bool encontrado = false;
    for (auto& eq : listaEquipos) {
        if (eq.value("nombre", "") == nombreBuscado) {
            eq["jj"]  = datosRecibidos.value("jj", 0);
            eq["jg"]  = datosRecibidos.value("jg", 0);
            eq["jp"]  = datosRecibidos.value("jp", 0);
            eq["je"]  = datosRecibidos.value("je", 0);
            eq["dif"] = datosRecibidos.value("dif", 0);
            
            // Eliminamos las llaves viejas cf y cc de Firebase
            eq.erase("cf");
            eq.erase("cc");

            eq["pts"] = (eq["jg"].get<int>() * 3) + (eq["je"].get<int>() * 1);
            encontrado = true;
            break;
        }
    }

    if (!encontrado) {
        nlohmann::json nuevoEq;
        nuevoEq["nombre"] = nombreBuscado;
        nuevoEq["jj"]  = datosRecibidos.value("jj", 0);
        nuevoEq["jg"]  = datosRecibidos.value("jg", 0);
        nuevoEq["jp"]  = datosRecibidos.value("jp", 0);
        nuevoEq["je"]  = datosRecibidos.value("je", 0);
        nuevoEq["dif"] = datosRecibidos.value("dif", 0);
        nuevoEq["pts"] = (nuevoEq["jg"].get<int>() * 3) + (nuevoEq["je"].get<int>() * 1);
        listaEquipos.push_back(nuevoEq);
    }

    auto fire_put = cli.Put("/equipos.json", listaEquipos.dump(), "application/json");
    if (fire_put && fire_put->status == 200) {
        res.status = 200;
        res.set_content("Editado correctamente", "text/plain");
    } else {
        res.status = 500;
        res.set_content("Error al editar en Firebase", "text/plain");
    }
});

// Ruta de Permisos CORS Obligatoria para ambas
svr.Options("/api/actualizar_equipo", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.status = 200;
});

svr.Options("/api/editar_equipo", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.status = 200;
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

// ==========================================
// RUTAS PARA ESTADÍSTICAS DE BATEO
// ==========================================

// 1. GET: Leer Bateadores
svr.Get("/api/bateadores", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false);
    
    if (auto fire_res = cli.Get("/bateadores.json")) {
        if (fire_res->status == 200) {
            res.set_content(fire_res->body == "null" ? "[]" : fire_res->body, "application/json");
        } else res.status = 500;
    } else res.status = 500;
});

// 2. POST: Añadir Bateador nuevo
// Función auxiliar para comparar nombres ignorando mayúsculas/minúsculas y espacios extra
auto limpiarTexto = [](std::string str) {
    std::string limpio = "";
    for (char c : str) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            limpio += std::tolower(static_cast<unsigned char>(c));
        }
    }
    return limpio;
};
auto sumarInningsBeisbol = [](double ip1, double ip2) {
    int ent1 = static_cast<int>(ip1);
    int outs1 = static_cast<int>(std::round((ip1 - ent1) * 10));
    
    int ent2 = static_cast<int>(ip2);
    int outs2 = static_cast<int>(std::round((ip2 - ent2) * 10));
    
    int totalOuts = (ent1 * 3 + outs1) + (ent2 * 3 + outs2);
    int nuevasEntradas = totalOuts / 3;
    int nuevosOuts = totalOuts % 3;
    
    return nuevasEntradas + (nuevosOuts * 0.1);
};

// 2. POST: Añadir Bateador o SUMAR si ya existe en Firebase
svr.Post("/api/bateadores", [limpiarTexto](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false);
    
    auto fire_get = cli.Get("/bateadores.json");
    nlohmann::json lista = nlohmann::json::array();
    if (fire_get && fire_get->status == 200 && fire_get->body != "null") {
        auto parsed = nlohmann::json::parse(fire_get->body);
        if (parsed.is_array()) {
            for (auto& el : parsed) if (!el.is_null()) lista.push_back(el);
        } else if (parsed.is_object()) {
            for (auto& el : parsed.items()) if (!el.value().is_null()) lista.push_back(el.value());
        }
    }
    
    nlohmann::json nuevo = nlohmann::json::parse(req.body);
    std::string nombreNuevo = limpiarTexto(nuevo.value("nombre", ""));
    std::string equipoNuevo = limpiarTexto(nuevo.value("equipo", ""));
    bool encontrado = false;

    for (auto& jug : lista) {
        if (limpiarTexto(jug.value("nombre", "")) == nombreNuevo &&
            limpiarTexto(jug.value("equipo", "")) == equipoNuevo) {
            
            jug["tb"]  = jug.value("tb", 0)  + nuevo.value("tb", 0);
            jug["h"]   = jug.value("h", 0)   + nuevo.value("h", 0);
            jug["d2b"] = jug.value("d2b", 0) + nuevo.value("d2b", 0);
            jug["d3b"] = jug.value("d3b", 0) + nuevo.value("d3b", 0);
            jug["hr"]  = jug.value("hr", 0)  + nuevo.value("hr", 0);
            jug["bb"]  = jug.value("bb", 0)  + nuevo.value("bb", 0);
            jug["ci"]  = jug.value("ci", 0)  + nuevo.value("ci", 0);
            jug["ca"]  = jug.value("ca", 0)  + nuevo.value("ca", 0);
            jug["k"]   = jug.value("k", 0)   + nuevo.value("k", 0);
            
            encontrado = true;
            break;
        }
    }

    if (!encontrado) {
        lista.push_back(nuevo);
    }
    
    auto fire_put = cli.Put("/bateadores.json", lista.dump(), "application/json");
    if (fire_put && fire_put->status == 200) {
        res.status = 200;
        res.set_content("{\"status\":\"ok\"}", "application/json");
    } else res.status = 500;
});

// 3. PUT: Editar Bateador existente directamente en Firebase
svr.Put("/api/bateadores", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    if (req.has_param("id")) {
        std::string id = req.get_param_value("id");
        httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
        cli.enable_server_certificate_verification(false);
        auto fire_put = cli.Put(("/bateadores/" + id + ".json").c_str(), req.body, "application/json");
        if (fire_put && fire_put->status == 200) {
            res.status = 200;
            res.set_content("{\"status\":\"ok\"}", "application/json");
            return;
        }
    }
    res.status = 500;
});

// 4. DELETE: Borrar Bateador directamente en Firebase
svr.Delete("/api/bateadores", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    if (req.has_param("id")) {
        std::string id = req.get_param_value("id");
        httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
        cli.enable_server_certificate_verification(false);
        auto fire_del = cli.Delete(("/bateadores/" + id + ".json").c_str());
        if (fire_del && fire_del->status == 200) {
            res.status = 200;
            res.set_content("{\"status\":\"eliminado\"}", "application/json");
            return;
        }
    }
    res.status = 500;
});

// 5. OPTIONS: Permisos para Bateadores
svr.Options("/api/bateadores", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.status = 200;
});


// ==========================================
// RUTAS PARA ESTADÍSTICAS DE PITCHERS
// ==========================================

// 1. GET: Leer Pitchers
svr.Get("/api/pitchers", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false); 
    
    if (auto fire_res = cli.Get("/pitchers.json")) {
        if (fire_res->status == 200) {
            res.set_content(fire_res->body == "null" ? "[]" : fire_res->body, "application/json");
        } else res.status = 500;
    } else res.status = 500;
});

// 2. POST: Añadir Pitcher nuevo
// 2. POST: Añadir Pitcher o SUMAR si ya existe en Firebase
// Convierte los decimales (.1 y .2) en outs reales, los suma y los devuelve en formato béisbol


// 2. POST: Añadir Pitcher o SUMAR con formato de béisbol si ya existe
svr.Post("/api/pitchers", [limpiarTexto, sumarInningsBeisbol](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false);
    
    auto fire_get = cli.Get("/pitchers.json");
    nlohmann::json lista = nlohmann::json::array();
    if (fire_get && fire_get->status == 200 && fire_get->body != "null") {
        auto parsed = nlohmann::json::parse(fire_get->body);
        if (parsed.is_array()) {
            for (auto& el : parsed) if (!el.is_null()) lista.push_back(el);
        } else if (parsed.is_object()) {
            for (auto& el : parsed.items()) if (!el.value().is_null()) lista.push_back(el.value());
        }
    }
    
    nlohmann::json nuevo = nlohmann::json::parse(req.body);
    // Normalizamos también si alguien ingresa .3 por error al registrar un pitcher nuevo
    nuevo["ip"] = sumarInningsBeisbol(nuevo.value("ip", 0.0), 0.0);

    std::string nombreNuevo = limpiarTexto(nuevo.value("nombre", ""));
    std::string equipoNuevo = limpiarTexto(nuevo.value("equipo", ""));
    bool encontrado = false;

    for (auto& pit : lista) {
        if (limpiarTexto(pit.value("nombre", "")) == nombreNuevo &&
            limpiarTexto(pit.value("equipo", "")) == equipoNuevo) {
            
            pit["ip"] = sumarInningsBeisbol(pit.value("ip", 0.0), nuevo.value("ip", 0.0));
            pit["k"]  = pit.value("k", 0)  + nuevo.value("k", 0);
            pit["cl"] = pit.value("cl", 0) + nuevo.value("cl", 0);
            pit["cs"] = pit.value("cs", 0) + nuevo.value("cs", 0);
            pit["g"]  = pit.value("g", 0)  + nuevo.value("g", 0);
            pit["p"]  = pit.value("p", 0)  + nuevo.value("p", 0);
            
            encontrado = true;
            break;
        }
    }

    if (!encontrado) {
        lista.push_back(nuevo);
    }
    
    auto fire_put = cli.Put("/pitchers.json", lista.dump(), "application/json");
    if (fire_put && fire_put->status == 200) {
        res.status = 200;
        res.set_content("{\"status\":\"ok\"}", "application/json");
    } else res.status = 500;
});

// 3. PUT: Editar Pitcher existente directamente en Firebase
svr.Put("/api/pitchers", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    if (req.has_param("id")) {
        std::string id = req.get_param_value("id");
        httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
        cli.enable_server_certificate_verification(false);
        auto fire_put = cli.Put(("/pitchers/" + id + ".json").c_str(), req.body, "application/json");
        if (fire_put && fire_put->status == 200) {
            res.status = 200;
            res.set_content("{\"status\":\"ok\"}", "application/json");
            return;
        }
    }
    res.status = 500;
});

// 4. DELETE: Borrar Pitcher directamente en Firebase
svr.Delete("/api/pitchers", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    if (req.has_param("id")) {
        std::string id = req.get_param_value("id");
        httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
        cli.enable_server_certificate_verification(false);
        auto fire_del = cli.Delete(("/pitchers/" + id + ".json").c_str());
        if (fire_del && fire_del->status == 200) {
            res.status = 200;
            res.set_content("{\"status\":\"eliminado\"}", "application/json");
            return;
        }
    }
    res.status = 500;
});

// 5. OPTIONS: Permisos para Pitchers
svr.Options("/api/pitchers", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.status = 200;
});
// 4. Ruta OPTIONS obligatoria para navegadores
svr.Options("/resultados", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.status = 200;
});
// ==========================================
// RUTAS PARA PROGRAMACIÓN DE LA SEMANA
// ==========================================

// 1. Ruta GET (Leer Programación)
svr.Get("/programacion", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false);
    
    if (auto fire_res = cli.Get("/programacion.json")) {
        if (fire_res->status == 200) {
            std::string data = fire_res->body == "null" ? "[]" : fire_res->body;
            res.set_content(data, "application/json");
        } else {
            res.status = 500;
            res.set_content("Error al leer programacion en Firebase", "text/plain");
        }
    } else {
        res.status = 500;
        res.set_content("Fallo de conexion a Firebase", "text/plain");
    }
});

// 2. Ruta POST (Guardar Juego Programado)
svr.Post("/programacion", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
    cli.enable_server_certificate_verification(false);
    
    auto fire_get = cli.Get("/programacion.json");
    nlohmann::json programacion = nlohmann::json::array();
    if (fire_get && fire_get->status == 200 && fire_get->body != "null") {
        auto parsed = nlohmann::json::parse(fire_get->body);
        if (parsed.is_array()) {
            for (auto& el : parsed) if (!el.is_null()) programacion.push_back(el);
        } else if (parsed.is_object()) {
            for (auto& el : parsed.items()) if (!el.value().is_null()) programacion.push_back(el.value());
        }
    }

    nlohmann::json nuevo = nlohmann::json::parse(req.body);
    programacion.push_back(nuevo);

    auto fire_put = cli.Put("/programacion.json", programacion.dump(), "application/json");
    if (fire_put && fire_put->status == 200) {
        res.status = 200;
        res.set_content("Programacion guardada en Firebase", "text/plain");
    } else {
        res.status = 500;
        res.set_content("Error al guardar en Firebase", "text/plain");
    }
});

// 3. Ruta DELETE (Eliminar Juego Programado)
svr.Delete("/programacion", [](const httplib::Request& req, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    
    if (req.has_param("id")) {
        int index = std::stoi(req.get_param_value("id"));
        httplib::Client cli("https://liga-esguarnac-default-rtdb.firebaseio.com");
        cli.enable_server_certificate_verification(false);
        
        auto fire_get = cli.Get("/programacion.json");
        if (fire_get && fire_get->status == 200 && fire_get->body != "null") {
            auto parsed = nlohmann::json::parse(fire_get->body);
            nlohmann::json programacion = nlohmann::json::array();
            if (parsed.is_array()) {
                for (auto& el : parsed) if (!el.is_null()) programacion.push_back(el);
            } else if (parsed.is_object()) {
                for (auto& el : parsed.items()) if (!el.value().is_null()) programacion.push_back(el.value());
            }
            
            if (index >= 0 && index < programacion.size()) {
                programacion.erase(index);
                auto fire_put = cli.Put("/programacion.json", programacion.dump(), "application/json");
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

// 4. Ruta OPTIONS obligatoria para navegadores (CORS)
svr.Options("/programacion", [](const httplib::Request& req, httplib::Response& res) {
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