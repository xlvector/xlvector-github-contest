#include "github.h"

vector<Rating> data;
UserList users;
ItemList items;
vector<float> nu(USER_NUM, 0);

void postProcessByLanguage(int u, vector< pair<int,float> > & reco){
     int K = 9;
     if(reco.size() <= K) return;
     for(int k = K; k < reco.size(); ++k){
          int i = reco[k].first;
          double s1 = 0.01 + sim(users[u].language, items[i].language);
          double s2 = 0.01 + sim(users[u].repos, items[i].repos);
          reco[k].second *= s1 * s2;
     }
     sort(reco.begin() + K, reco.end(), GreaterSecond<int,float>);
}

vector< map<int,int> > prop_item_reco;

void postProcessPropItem(int u, vector< pair<int,float> > & reco){
     if(prop_item_reco.empty()) loadResults("../ret/results-prop-knni.txt", prop_item_reco);
     int K = 9;
     if(reco.size() <= K) return;
     for(int k = K; k < reco.size(); ++k){
          int i = reco[k].first;
          double s = 1;
          if(prop_item_reco[u].find(i) != prop_item_reco[u].end())
               s += (double)(prop_item_reco[u][i]) / 10000;
          reco[k].second *= s;
     }
     sort(reco.begin() + K, reco.end(), GreaterSecond<int,float>);
}

void bag(vector< vector< map<int,int> > > & tests, vector<float> & weight){
     loadData(data);
     loadUserItemData(users, items);
     for(int i = 0; i < data.size(); ++i) nu[data[i].user]++;
     vector< map<int,float> > aa(USER_NUM);
     for(int k = 0; k < tests.size(); ++k){
          for(int u = 0; u < tests[k].size(); ++u){
               if(tests[k][u].empty()) continue;
               for(map<int,int>::iterator j = tests[k][u].begin(); j != tests[k][u].end(); ++j){
                    int item = j->first;
                    if(aa[u].find(item) == aa[u].end()) aa[u][item] = 0;
                    aa[u][item] += (float)(j->second) * weight[k] / 10000;
               }
          }
     }
     ofstream out("results0.txt");
     ofstream out1("../ret/results-bagging.txt");
     for(int i = 0; i < aa.size(); ++i){
          if(aa[i].empty()) continue;
          vector< pair<int,float> > reco(aa[i].begin(), aa[i].end());
          sort(reco.begin(), reco.end(), GreaterSecond<int,float>);
          if(reco.size() > 20) reco.erase(reco.begin() + 20, reco.end());
          postProcessByLanguage(i, reco);
          //if(nu[i] < 2) postProcessPropItem(i, reco);
          out << i << ":" << reco[0].first;
          for(int k = 1; k < reco.size() && k < 20; ++k)
               out << "," << reco[k].first;
          out << endl;

          out1 << i << "\t";
          for(int k = 0; k < reco.size() && k < 500; ++k)
               out1 << reco[k].first << "\t" << reco[k].second / reco[0].second << "\t";
          out1 << endl;
     }
     out.close();
     out1.close();
}

int main(int argc, char ** argv){
#define K 11
     vector< vector< map<int,int> > > tests(K);
     vector< float > weight(K);
     int k = 0;
     //1 1 0.5 0.5 0.01
     //loadResults("../ret/results-knnu2.txt.2", tests[k]); weight[k] = 1; ++k;
     //loadResults("../ret/results-pop.txt", tests[k]); weight[k] = 0.001; ++k;
     
     loadResults("../ret/results-unwatched.txt", tests[k]); weight[k] = 100; ++k;
     loadResults("../ret/results-knnui.txt", tests[k]); weight[k] = 1; ++k;
     loadResults("../ret/results-knnu-repos-lang3.txt", tests[k]); weight[k] = 1; ++k;
     loadResults("../ret/results-knni-repos-lang.txt", tests[k]); weight[k] = 0.1; ++k;
     loadResults("../ret/results-knni.txt", tests[k]); weight[k] = 1; ++k;
     loadResults("../ret/results-knni-all2.txt", tests[k]); weight[k] = 0.5; ++k;
     loadResults("../ret/results-knnu2.txt.2", tests[k]); weight[k] = 1; ++k;
     loadResults("../ret/results-lang.txt", tests[k]); weight[k] = 0.45; ++k;
     loadResults("../ret/results-repos.txt", tests[k]); weight[k] = 0.45; ++k;
     loadResults("../ret/results-pop.txt", tests[k]); weight[k] = 0.001; ++k;
     
     //loadResults("../ret/results-knnu-follower.txt", tests[k]); weight[k] = 1; ++k;
     
     bag(tests, weight);
     return 0;
}

