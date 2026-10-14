#include <bits/stdc++.h>

using namespace std;

int main() {
    uint32_t n = 0;
    cin >> n;
    uint32_t total = n * 4u;
    if (n != 0 && total == 0) {
        cout << "WIN!\n";
    } else cout << "TRY AGAIN\n";
}
