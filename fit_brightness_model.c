W#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N_A 100
#define N_B 100
#define DATA_FILE "datapoints.csv"
#define LINE_BUF 128

typedef struct {
    double zenith;
    double brightness;
} DataPoint;

double model(double x, double a, double b) {
    return 1.0 - 1.0 / (1.0 + exp(-a * (x - b)));
}

double mse(DataPoint *data, int n, double a, double b) {
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        double pred = model(data[i].zenith, a, b);
        double err = pred - data[i].brightness;
        sum += err * err;
    }
    return sum / n;
}

int load_data(DataPoint *data, int max_n) {
    FILE *f = fopen(DATA_FILE, "r");
    if (!f) return 0;
    int n = 0;
    char line[LINE_BUF];
    while (fgets(line, sizeof(line), f) && n < max_n) {
        if (sscanf(line, "%lf,%lf", &data[n].zenith, &data[n].brightness) == 2) {
            n++;
        }
    }
    fclose(f);
    return n;
}

int main() {
    DataPoint data[1000];
    int n = load_data(data, 1000);
    if (n == 0) {
        fprintf(stderr, "No data loaded.\n");
        return 1;
    }
    double a_min = 11, a_max = 400;
    double b_min = 0, b_max = 1;
    double best_a = 0, best_b = 0, best_mse = 1e9;
    // First exhaustive search
    for (int ai = 0; ai < N_A; ++ai) {
        double a = a_min + (a_max - a_min) * ai / (N_A - 1);
        for (int bi = 0; bi < N_B; ++bi) {
            double b = b_min + (b_max - b_min) * bi / (N_B - 1);
            double err = mse(data, n, a, b);
            if (err < best_mse) {
                best_mse = err;
                best_a = a;
                best_b = b;
            }
        }
    }
    // Refined search
    double a2_min = best_a - (a_max - a_min) / N_A, a2_max = best_a + (a_max - a_min) / N_A;
    double b2_min = best_b - (b_max - b_min) / N_B, b2_max = best_b + (b_max - b_min) / N_B;
    if (a2_min < a_min) a2_min = a_min;
    if (a2_max > a_max) a2_max = a_max;
    if (b2_min < b_min) b2_min = b_min;
    if (b2_max > b_max) b2_max = b_max;
    best_mse = 1e9;
    for (int ai = 0; ai < N_A; ++ai) {
        double a = a2_min + (a2_max - a2_min) * ai / (N_A - 1);
        for (int bi = 0; bi < N_B; ++bi) {
            double b = b2_min + (b2_max - b2_min) * bi / (N_B - 1);
            double err = mse(data, n, a, b);
            if (err < best_mse) {
                best_mse = err;
                best_a = a;
                best_b = b;
            }
        }
    }
    printf("Best fit: a = %.6f, b = %.6f, mse = %.8f\n", best_a, best_b, best_mse);
    return 0;
}
