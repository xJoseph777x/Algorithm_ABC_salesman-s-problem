/**
 * @file tsp_abc.cpp
 * @brief Algoritmo ABC para TSP - Versión simplificada
 */

#include <cmath>
#include <random>
#include <algorithm>
#include <limits>
#include <iostream>

using namespace std;

// Generar ciudades aleatorias fijas (100 ciudades)
extern "C" void generar_ciudades_aleatorias_fijas(double* ciudades_x, double* ciudades_y) {
    mt19937 gen(1);
    uniform_real_distribution<> distrib(10.0, 90.0);
    
    for(int i = 0; i < 100; i++) {
        ciudades_x[i] = distrib(gen);
        ciudades_y[i] = distrib(gen);
    }
}

// Calcular distancia total del camino
extern "C" double calcular_distancia_total(int* camino, double* ciudades_x, 
                                          double* ciudades_y, int num_ciudades) {
    double dist = 0.0;
    for(int i = 0; i < num_ciudades; i++) {
        int actual = camino[i];
        int siguiente = camino[i + 1];
        double dx = ciudades_x[actual] - ciudades_x[siguiente];
        double dy = ciudades_y[actual] - ciudades_y[siguiente];
        dist += sqrt(dx*dx + dy*dy);
    }
    return dist;
}

// Generar camino aleatorio
void camino_aleatorio(int* camino, int num_ciudades, mt19937& gen, 
                     int inicio, int fin) {
    if(inicio == fin) {
        for(int i = 0; i < num_ciudades; i++) camino[i] = i;
        for(int i = num_ciudades - 1; i > 0; i--) {
            uniform_int_distribution<> distrib(0, i);
            int j = distrib(gen);
            swap(camino[i], camino[j]);
        }
        camino[num_ciudades] = camino[0];
    } else {
        camino[0] = inicio;
        int restantes[100], cont = 0;
        for(int i = 0; i < num_ciudades; i++) {
            if(i != inicio && i != fin) restantes[cont++] = i;
        }
        for(int i = cont - 1; i > 0; i--) {
            uniform_int_distribution<> distrib(0, i);
            int j = distrib(gen);
            swap(restantes[i], restantes[j]);
        }
        for(int i = 0; i < cont; i++) camino[i + 1] = restantes[i];
        camino[cont + 1] = fin;
        camino[num_ciudades] = fin;
    }
}

// Generar vecino (intercambiar dos ciudades)
void generar_vecino(int* origen, int* destino, int num, mt19937& gen,
                   int inicio, int fin) {
    for(int i = 0; i <= num; i++) destino[i] = origen[i];
    
    if(inicio == fin) {
        uniform_int_distribution<> distrib(0, num - 2);
        int i = distrib(gen), j;
        do { j = distrib(gen); } while(i == j);
        swap(destino[i], destino[j]);
        destino[num] = destino[0];
    } else {
        uniform_int_distribution<> distrib(1, num - 2);
        int i = distrib(gen), j;
        do { j = distrib(gen); } while(i == j);
        swap(destino[i], destino[j]);
        destino[0] = inicio;
        destino[num] = fin;
    }
}

// Algoritmo ABC principal
extern "C" void abc_tsp_completo(double* ciudades_x, double* ciudades_y, int num_ciudades,
                                int num_abejas, int limite, int iteraciones,
                                int inicio, int fin,
                                double* mejor_dist, int* mejor_camino) {
    
    random_device rd;
    mt19937 gen(rd());
    
    // Crear colonia de abejas
    int** abejas = new int*[num_abejas];
    for(int i = 0; i < num_abejas; i++) {
        abejas[i] = new int[num_ciudades + 1];
    }
    
    double* distancias = new double[num_abejas];
    int* contadores = new int[num_abejas]();
    
    double mejor = numeric_limits<double>::max();
    
    // Inicializar colonia
    for(int i = 0; i < num_abejas; i++) {
        camino_aleatorio(abejas[i], num_ciudades, gen, inicio, fin);
        distancias[i] = calcular_distancia_total(abejas[i], ciudades_x, ciudades_y, num_ciudades);
        
        if(distancias[i] < mejor) {
            mejor = distancias[i];
            for(int j = 0; j <= num_ciudades; j++) {
                mejor_camino[j] = abejas[i][j];
            }
        }
    }
    
    cout << "Algoritmo ABC iniciado - Ciudades: " << num_ciudades 
         << " Abejas: " << num_abejas << endl;
    
    // Ciclo principal
    for(int iter = 0; iter < iteraciones; iter++) {
        // Fase empleadas
        for(int i = 0; i < num_abejas; i++) {
            int* vecino = new int[num_ciudades + 1];
            generar_vecino(abejas[i], vecino, num_ciudades, gen, inicio, fin);
            double nueva_dist = calcular_distancia_total(vecino, ciudades_x, ciudades_y, num_ciudades);
            
            if(nueva_dist < distancias[i]) {
                for(int j = 0; j <= num_ciudades; j++) abejas[i][j] = vecino[j];
                distancias[i] = nueva_dist;
                contadores[i] = 0;
                if(nueva_dist < mejor) {
                    mejor = nueva_dist;
                    for(int j = 0; j <= num_ciudades; j++) mejor_camino[j] = vecino[j];
                }
            } else {
                contadores[i]++;
            }
            delete[] vecino;
        }
        
        // Fase observadoras
        double suma_fitness = 0;
        double* fitness = new double[num_abejas];
        for(int i = 0; i < num_abejas; i++) {
            fitness[i] = 1.0 / (distancias[i] + 0.001);
            suma_fitness += fitness[i];
        }
        
        uniform_real_distribution<> rand_real(0.0, 1.0);
        for(int i = 0; i < num_abejas; i++) {
            if(rand_real(gen) < (fitness[i] / suma_fitness)) {
                int* vecino = new int[num_ciudades + 1];
                generar_vecino(abejas[i], vecino, num_ciudades, gen, inicio, fin);
                double nueva_dist = calcular_distancia_total(vecino, ciudades_x, ciudades_y, num_ciudades);
                
                if(nueva_dist < distancias[i]) {
                    for(int j = 0; j <= num_ciudades; j++) abejas[i][j] = vecino[j];
                    distancias[i] = nueva_dist;
                    contadores[i] = 0;
                    if(nueva_dist < mejor) {
                        mejor = nueva_dist;
                        for(int j = 0; j <= num_ciudades; j++) mejor_camino[j] = vecino[j];
                    }
                } else {
                    contadores[i]++;
                }
                delete[] vecino;
            }
        }
        delete[] fitness;
        
        // Fase exploradoras
        for(int i = 0; i < num_abejas; i++) {
            if(contadores[i] >= limite) {
                camino_aleatorio(abejas[i], num_ciudades, gen, inicio, fin);
                distancias[i] = calcular_distancia_total(abejas[i], ciudades_x, ciudades_y, num_ciudades);
                contadores[i] = 0;
                if(distancias[i] < mejor) {
                    mejor = distancias[i];
                    for(int j = 0; j <= num_ciudades; j++) mejor_camino[j] = abejas[i][j];
                }
            }
        }
        
        if((iter + 1) % 1000 == 0) {
            cout << "Iteracion " << (iter + 1) << " - Mejor: " << mejor << endl;
        }
    }
    
    *mejor_dist = mejor;
    cout << "Algoritmo completado - Mejor distancia: " << mejor << endl;
    
    // Liberar memoria
    for(int i = 0; i < num_abejas; i++) delete[] abejas[i];
    delete[] abejas;
    delete[] distancias;
    delete[] contadores;
}
