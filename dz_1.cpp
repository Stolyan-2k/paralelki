#include <iostream>
#include <vector>
#include <cmath>

int N = 10000000;

#ifdef USE_FLOAT
using unknown = float;
#else
using unknown = double;
#endif

unknown PI = 3.141592653589793;

int main() {
    std::vector<unknown> arr(N);
    unknown sum = 0.0;

    for (size_t i = 0; i < N; ++i) {
    arr[i] = std::sin(2 * PI * i / N);
    }

    for (size_t i = 0; i < N; ++i) {
        sum += arr[i];
    }
    std::cout << "Sum: " << sum << '\n';

    return 0;

}