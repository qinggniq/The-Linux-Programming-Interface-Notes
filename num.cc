#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main() {
  int n;
  vector<pair<int, int> > ans;
  vector<pair<int, int> > save;
  while (cin >> n) {
    ans.resize(n);
    save.resize(n);
    for(int i=0; i < n; ++i) {
      cin >> ans[i].first;
      ans[i].second = i;
    }


    sort(ans.begin(), ans.end());

    for (int i=0; i < n; ++i) {
      int l = i - 1;
      int r = i + 1;
      if (i == 0) {
        while (r < n && ans[r].second > ans[i].second) r++;
        if (r == n) {
          cout << "r == n" << ans[i].first << " " <<ans[i].second << endl;
          save[ans[i].second].second = -1;
          save[ans[i].second].first = -1;
        }else{
          save[ans[i].second].first = ans[r].first - ans[i].first;
          save[ans[i].second].second = r+1;
        }
       }else  if (i == n-1) {
        while (l >= 0 && ans[l].second > ans[i].second) l--;
        if (l == -1) {
          save[ans[i].second].second = -1;
          save[ans[i].second].first = -1;
        }
        else{
          save[ans[i].second].first = ans[i].first - ans[l].first;
          save[ans[i].second].second = l+1;
        }
      }
      else {
        //cout << "here : "<<"ans[i].first<< "  << ans[i].second<< endl;
        while (r < n && ans[r].second > ans[i].second) r++;
        while (l >= 0 && ans[l].second > ans[i].second) l--;
        if (l == -1 && r == n) {
          save[ans[i].second].second = -1;
          save[ans[i].second].first = -1;

        } else if (l == -1 && r != n) {
          save[ans[i].second].first = ans[r].first - ans[i].first;
          save[ans[i].second].second = r+1;
        } else if (r == n && l != -1) {
          save[ans[i].second].first = ans[i].first - ans[l].first;
          save[ans[i].second].second = l+1;
        } else {
          if (abs(ans[i].first - ans[l].first) <= abs(ans[i].first - ans[r].first)) {
          save[ans[i].second].first = ans[i].first - ans[l].first;
          save[ans[i].second].second = l+1;

          } else {
            save[ans[i].second].first = ans[r].first -  ans[i].first;
            save[ans[i].second].second = r+1;

          }
        }


      }

    }
    for (int i=1; i < n; ++i){
      cout << save[i].first << " " << save[i].second << endl;
    }
  }
}
