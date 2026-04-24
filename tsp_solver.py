"""
Solver TSP con algoritmo ABC - Versión simplificada
Uso: python tsp_solver.py
"""

import ctypes
import numpy as np
import matplotlib.pyplot as plt
import os
import sys
from pathlib import Path

class TSPSolver:
    def __init__(self, num_ciudades=50, num_abejas=1500, limite=5000, iteraciones=10000):
        self.num_ciudades = num_ciudades
        self.num_abejas = num_abejas
        self.limite = limite
        self.iteraciones = iteraciones
        self.inicio = 0
        self.fin = 0
        self.lib = None
        
    def compilar_y_cargar(self):
        """Compila el C++ y carga la biblioteca"""
        # Obtener rutas (funciona en cualquier lado)
        script_dir = Path(__file__).parent
        cpp_file = script_dir / "tsp_abc.cpp"
        lib_name = "tsp_abc.dll" if sys.platform == "win32" else "libtsp_abc.so"
        lib_file = script_dir / lib_name
        
        # Compilar si no existe
        if not lib_file.exists():
            print("Compilando código C++...")
            if sys.platform == "win32":
                cmd = f'g++ -shared -std=c++11 -O3 -o "{lib_file}" "{cpp_file}"'
            else:
                cmd = f'g++ -shared -std=c++11 -O3 -fPIC -o "{lib_file}" "{cpp_file}"'
            
            if os.system(cmd) != 0:
                print("Error: No se pudo compilar. ¿Tienes g++ instalado?")
                return False
        
        # Cargar biblioteca
        self.lib = ctypes.CDLL(str(lib_file))
        
        # Configurar tipos
        self.lib.generar_ciudades_aleatorias_fijas.argtypes = [
            ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double)
        ]
        self.lib.abc_tsp_completo.argtypes = [
            ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double),
            ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int,
            ctypes.c_int, ctypes.c_int,
            ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_int)
        ]
        return True
    
    def obtener_ciudad(self, prompt):
        """Solicita una ciudad válida al usuario"""
        while True:
            try:
                val = input(f"{prompt} (0-{self.num_ciudades-1}): ").strip()
                if not val:
                    print("No puede estar vacío")
                    continue
                ciudad = int(val)
                if 0 <= ciudad < self.num_ciudades:
                    return ciudad
                print(f"Debe estar entre 0 y {self.num_ciudades-1}")
            except ValueError:
                print("Ingrese un número entero")
    
    def resolver(self):
        """Ejecuta el algoritmo y muestra resultados"""
        print("\n" + "="*60)
        print("ALGORITMO ABC PARA TSP")
        print("="*60)
        
        # Configuración
        print(f"\nConfiguración:")
        print(f"  Ciudades: {self.num_ciudades}")
        print(f"  Abejas: {self.num_abejas}")
        print(f"  Iteraciones: {self.iteraciones}")
        
        # Solicitar puntos
        self.inicio = self.obtener_ciudad("\nCiudad inicial")
        self.fin = self.obtener_ciudad("Ciudad final")
        
        # Cargar biblioteca
        if not self.compilar_y_cargar():
            return
        
        # Generar ciudades
        print("\nGenerando ciudades...")
        ciudades_x = np.zeros(self.num_ciudades, dtype=np.float64)
        ciudades_y = np.zeros(self.num_ciudades, dtype=np.float64)
        
        self.lib.generar_ciudades_aleatorias_fijas(
            ciudades_x.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
            ciudades_y.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
        )
        
        # Ejecutar algoritmo
        print("\nEjecutando algoritmo ABC...")
        mejor_dist = np.zeros(1, dtype=np.float64)
        mejor_camino = np.zeros(self.num_ciudades + 1, dtype=np.int32)
        
        self.lib.abc_tsp_completo(
            ciudades_x.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
            ciudades_y.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
            self.num_ciudades, self.num_abejas, self.limite, self.iteraciones,
            self.inicio, self.fin,
            mejor_dist.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
            mejor_camino.ctypes.data_as(ctypes.POINTER(ctypes.c_int))
        )
        
        # Mostrar resultados
        print("\n" + "="*60)
        print("RESULTADOS")
        print("="*60)
        print(f"Mejor distancia: {mejor_dist[0]:.2f}")
        print(f"Ruta encontrada: {mejor_camino.tolist()}")
        
        # Preguntar por gráfico
        if input("\n¿Mostrar gráfico? (s/n): ").lower() == 's':
            self.mostrar_grafico(ciudades_x, ciudades_y, mejor_camino, mejor_dist[0])
    
    def mostrar_grafico(self, xs, ys, camino, distancia):
        """Muestra el gráfico de la ruta"""
        ciudades = np.column_stack((xs, ys))
        
        plt.figure(figsize=(12, 8))
        
        # Dibujar ciudades
        plt.scatter(xs, ys, c='red', s=50, alpha=0.6, zorder=3)
        
        # Destacar inicio y fin
        plt.scatter(xs[self.inicio], ys[self.inicio], c='green', s=200, 
                   marker='*', zorder=4, label=f'Inicio ({self.inicio})')
        
        if self.inicio != self.fin:
            plt.scatter(xs[self.fin], ys[self.fin], c='orange', s=200, 
                       marker='s', zorder=4, label=f'Fin ({self.fin})')
        
        # Dibujar ruta
        for i in range(len(camino) - 1):
            a, b = camino[i], camino[i+1]
            plt.plot([xs[a], xs[b]], [ys[a], ys[b]], 'b-', alpha=0.7, linewidth=1.5)
        
        # Configuración
        titulo = f'Ruta Óptima - Distancia: {distancia:.2f}'
        if self.inicio == self.fin:
            titulo += f'\nRuta Cíclica (Inicio/Fin: {self.inicio})'
        else:
            titulo += f'\nRuta de {self.inicio} a {self.fin}'
        
        plt.title(titulo, fontsize=14, fontweight='bold')
        plt.xlabel('Coordenada X')
        plt.ylabel('Coordenada Y')
        plt.grid(True, alpha=0.3)
        plt.legend()
        plt.axis('equal')
        plt.tight_layout()
        plt.show()

def main(): #Main triple Siete
    # Parámetros (puedes modificarlos aquí directamente)
    solver = TSPSolver(
        num_ciudades=50,
        num_abejas=1500,
        limite=5000,
        iteraciones=10000
    )
    solver.resolver()

if __name__ == "__main__":
    main()
