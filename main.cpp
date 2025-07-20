#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <iomanip>
#include <limits>


using namespace std;

int alpha = 3;
int beta = 2;
double rho = 0.01;
double Q = 2.0;

mt19937 rng(0);

vector<string> ciudades = { "A", "B", "C", "D" };

const int INF = 1e6;
vector<vector<int>> distancias = {

    {  0,  2,  6, INF },  // A
    {  2,  0,  9,  5  },  // B
    {  6,  9,  0,  9  },  // C
    {INF,  5,  9,  0  }   // D
};

int ciudadInicio = 0; 
int ciudadDestino = 3; 

int Aleatorio(int min, int max) {
    uniform_int_distribution<int> dist(min, max - 1);
    return dist(rng);
}

double AleatorioReal() {
    uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

double Longitud(const vector<int>& ruta, const vector<vector<int>>& dist) {
    double total = 0.0;
    for (size_t i = 0; i < ruta.size() - 1; ++i) {
        if (dist[ruta[i]][ruta[i + 1]] == INF)
            return INF;
        total += dist[ruta[i]][ruta[i + 1]];
    }
    return total;
}

vector<vector<double>> InicializarFeromonas(int n) {
    return vector<vector<double>>(n, vector<double>(n, 0.01));
}

vector<double> CalcularProbabilidades(int actual, const vector<bool>& visitado, const vector<vector<double>>& feromonas, const vector<vector<int>>& dist) {
    int n = feromonas.size();
    vector<double> taueta(n);
    double suma = 0.0;

    for (int i = 0; i < n; ++i) {
        if (i == actual || visitado[i] || dist[actual][i] == INF)
            taueta[i] = 0.0;
        else {
            taueta[i] = pow(feromonas[actual][i], alpha) * pow(1.0 / dist[actual][i], beta);
            suma += taueta[i];
        }
    }

    vector<double> probs(n, 0.0);
    if (suma == 0) return probs;
    for (int i = 0; i < n; ++i)
        probs[i] = taueta[i] / suma;

    return probs;
}

int SiguienteCiudad(const vector<double>& probs) {
    vector<double> acumulado(probs.size() + 1, 0.0);
    for (int i = 0; i < probs.size(); ++i)
        acumulado[i + 1] = acumulado[i] + probs[i];

    double r = AleatorioReal();
    for (int i = 0; i < probs.size(); ++i)
        if (r >= acumulado[i] && r < acumulado[i + 1])
            return i;
    return probs.size() - 1;
}

vector<int> ConstruirRuta(int inicio, int destino, const vector<vector<double>>& feromonas, const vector<vector<int>>& dist) {
    int n = feromonas.size();
    vector<int> ruta = { inicio };
    vector<bool> visitado(n, false);
    visitado[inicio] = true;

    int actual = inicio;
    while (actual != destino) {
        vector<double> probs = CalcularProbabilidades(actual, visitado, feromonas, dist);
        int siguiente = SiguienteCiudad(probs);
        if (visitado[siguiente] || dist[actual][siguiente] == INF)
            break; // ruta invalida
        ruta.push_back(siguiente);
        visitado[siguiente] = true;
        actual = siguiente;
    }

    if (ruta.back() != destino)
        ruta = {}; // ruta invalida

    return ruta;
}

vector<vector<int>> GenerarHormigas(int cantidad, int inicio, int destino, const vector<vector<double>>& feromonas, const vector<vector<int>>& dist) {
    vector<vector<int>> hormigas;
    while (hormigas.size() < cantidad) {
        vector<int> ruta = ConstruirRuta(inicio, destino, feromonas, dist);
        if (!ruta.empty())
            hormigas.push_back(ruta);
    }
    return hormigas;
}

void ActualizarFeromonas(vector<vector<double>>& feromonas, const vector<vector<int>>& hormigas, const vector<vector<int>>& dist) {
    int n = feromonas.size();
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            feromonas[i][j] *= (1 - rho);

    for (const auto& ruta : hormigas) {
        double d = Longitud(ruta, dist);
        if (d == INF) continue;
        double contrib = Q / d;
        for (int i = 0; i < ruta.size() - 1; ++i) {
            int a = ruta[i], b = ruta[i + 1];
            feromonas[a][b] += contrib;
            feromonas[b][a] += contrib;
        }
    }
}

void MostrarRuta(const vector<int>& ruta) {
    for (size_t i = 0; i < ruta.size(); ++i) {
        cout << ciudades[ruta[i]];
        if (i != ruta.size() - 1) cout << "-->";
    }
    cout << endl;
}

int main() {
    cout << " -------ACO con ciudades A-D, inicio en A y destino en D --------\n";

    int numHormigas = 10;
    int iteraciones = 200;

    int numCiudades = ciudades.size();
    auto feromonas = InicializarFeromonas(numCiudades);
    auto hormigas = GenerarHormigas(numHormigas, ciudadInicio, ciudadDestino, feromonas, distancias);

    vector<int> mejor = hormigas[0];
    double mejorLen = Longitud(mejor, distancias);

    for (int t = 0; t < iteraciones; ++t) {
        hormigas = GenerarHormigas(numHormigas, ciudadInicio, ciudadDestino, feromonas, distancias);
        ActualizarFeromonas(feromonas, hormigas, distancias);
        for (auto& ruta : hormigas) {
            double len = Longitud(ruta, distancias);
            if (len < mejorLen) {
                mejorLen = len;
                mejor = ruta;
                cout << "Nueva mejor longitud: " << mejorLen << " en iteracion " << t << endl;
            }
        }
    }

    cout << "\n--- Ruta optima encontrada ---\n";
    MostrarRuta(mejor);
    cout << "Distancia total: " << fixed << setprecision(1) << mejorLen << " km\n";

    return 0;
}
