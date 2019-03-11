#include <iostream>
#include <queue>

using namespace std;

int main() {
  int n;
  deque<int> a;
  while (cin >> n) {
    a.resize(n);
    for (int i = 0; i < a.size(); ++i)
      a[i] = i+1;
    while (n --) {
      cout << a.front() << " ";
      a.pop_front();
      a.push_back(a.front());
      a.pop_front();
    }
    cout << endl;
  }
}
