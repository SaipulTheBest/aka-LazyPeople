#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>
#include <new>

using namespace std;

// --- Fungsi dasar ---
int total_sum_recursive(int scores[], int n, int i) {
    if (i == n) return 0;
    return scores[i] + total_sum_recursive(scores, n, i + 1);
}

int total_sum_iterative(int scores[], int n) {
    int total = 0;
    for (int i = 0; i < n; i++) total += scores[i];
    return total;
}

// --- Utility: ukur waktu rata-rata dalam MILIDETIK (ms) ---
template <typename Func>
double benchmark_ms(Func f, int repeats = 20) {
    using clock = std::chrono::steady_clock;
    double total_ms = 0.0;

    // Warm-up
    {
        volatile int sink = f();
        (void)sink;
    }

    for (int r = 0; r < repeats; ++r) {
        auto t0 = clock::now();
        volatile int sink = f();
        auto t1 = clock::now();
        total_ms += std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    return total_ms / repeats;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const int max_safe_recursive = 10000;
    const int repeats = 20;

    ofstream csv("runtime_results.csv");
    if (!csv) {
        cerr << "ERROR: tidak bisa membuka file CSV.\n";
        return 1;
    }

    // Header CSV (ms)
    csv << "n,iterative_ms,recursive_ms,note,total_iter,total_rec\n";

    // Header tabel konsol
    cout << left
         << setw(10) << "n"
         << setw(18) << "Iteratif (ms)"
         << setw(18) << "Rekursif (ms)"
         << setw(20) << "Catatan\n";
    cout << string(66, '-') << "\n";

    auto test_n = [&](int n) {
        int* scores = new (nothrow) int[n];
        if (!scores) {
            csv << n << ",,,ERROR: allocation failed,,\n";
            cout << setw(10) << n
                 << setw(18) << "-"
                 << setw(18) << "-"
                 << setw(20) << "ERROR: alokasi gagal\n";
            return;
        }

        for (int i = 0; i < n; ++i) scores[i] = i + 1;

        int total_it = total_sum_iterative(scores, n);

        double t_iter_ms = benchmark_ms([&]() {
            return total_sum_iterative(scores, n);
        }, repeats);

        double t_rec_ms = -1.0;
        int total_rec = -1;
        string note = "";

        if (n <= max_safe_recursive) {
            total_rec = total_sum_recursive(scores, n, 0);
            t_rec_ms = benchmark_ms([&]() {
                return total_sum_recursive(scores, n, 0);
            }, repeats);

            if (total_rec != total_it) {
                note = "WARNING: total beda";
            }
        } else {
            note = "SKIPPED (stack overflow)";
        }

        cout << setw(10) << n
             << setw(18) << fixed << setprecision(6) << t_iter_ms;

        if (t_rec_ms >= 0)
            cout << setw(18) << fixed << setprecision(6) << t_rec_ms;
        else
            cout << setw(18) << "-";

        cout << setw(20) << note << "\n";

        csv << n << ","
            << fixed << setprecision(6) << t_iter_ms << ",";

        if (t_rec_ms >= 0)
            csv << fixed << setprecision(6) << t_rec_ms << ",";
        else
            csv << ",";

        csv << note << ","
            << total_it << ",";

        if (total_rec != -1)
            csv << total_rec;

        csv << "\n";

        delete[] scores;
    };

    // n = 1
    test_n(1);

    // n = 10 sampai 100, step 10
    for (int n = 10; n <= 100; n += 10) {
        test_n(n);
    }

    // n = 150 sampai 1000, step 50
    for (int n = 150; n <= 1000; n += 50) {
        test_n(n);
    }

    // n = 2000 sampai 10000, step 1000
    for (int n = 2000; n <= 10000; n += 1000) {
        test_n(n);
    }

    csv.close();

    cout << "\nCatatan:\n"
         << "- Rekursif di-skip jika n > " << max_safe_recursive << "\n"
         << "- Rata-rata dari " << repeats << " pengulangan\n"
         << "- Output CSV: runtime_results.csv (ms)\n";

    return 0;
}
