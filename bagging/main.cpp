#include "github.h"

void load(const char * file, vector< map<int,int> > & test){
     test = vector< map<int,int> >(USER_NUM);
     string line;
     ifstream in(file);
     while(getline(in, line)){
          istringstream iss(line);
          int user;
          iss >> user;
          int item;
          float p;
          while(iss >> item >> p){
               //if(test[user].size() > 200) continue;
               test[user][item] = (int)(p * 10000);
          }
     }
}

void bag(vector< vector< map<int,int> > > & tests, vector<float> & weight){
     vector<Rating> data;
     loadData(data);
     vector<int> nu(USER_NUM,0);
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
     for(int i = 0; i < aa.size(); ++i){
          if(aa[i].empty()) continue;
          //if(nu[i] > 5) continue;
          vector< pair<int,float> > reco(aa[i].begin(), aa[i].end());
          sort(reco.begin(), reco.end(), GreaterSecond<int,float>);
          out << i << ":" << reco[0].first;
          for(int k = 1; k < reco.size() && k < 10; ++k)
               out << "," << reco[k].first;
          out << endl;
     }
}

int main(int argc, char ** argv){
#define K 7
     vector< vector< map<int,int> > > tests(K);
     vector< float > weight(K);
     int k = 0;
     //1 1 0.5 0.5 0.01
     load("../ret/results-knnui.txt", tests[k]); weight[k] = 1; ++k;
     load("../ret/results-knnu-repos-lang.txt", tests[k]); weight[k] = 1; ++k;
     load("../ret/results-knni.txt", tests[k]); weight[k] = 1; ++k;
     load("../ret/results-knnu.txt", tests[k]); weight[k] = 1; ++k;
     load("../ret/results-lang.txt", tests[k]); weight[k] = 0.45; ++k;
     load("../ret/results-repos.txt", tests[k]); weight[k] = 0.45; ++k;
     load("../ret/results-pop.txt", tests[k]); weight[k] = 0.001; ++k;
     bag(tests, weight);
     return 0;
}

