#include "github.h"

vector<Rating> data;
UserList users;
ItemList items;
vector<float> nu(USER_NUM, 0);
map<int,int> test_data;

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

void loadTestData(){
     ifstream in("../download/removed_values.txt");
     string line;
     while(getline(in,line)){
          line = replace(line, ':', ' ');
          istringstream iss(line);
          int uu,ii;
          iss >> uu >> ii;
          test_data[uu] = ii;
     }
     in.close();
}

int score(vector< map<int,float> > & reco){
     int ret = 0;
     for(map<int,int>::iterator k = test_data.begin(); k != test_data.end(); ++k){
          vector< pair<int,float> > ru(reco[k->first].begin(), reco[k->first].end());
          sort(ru.begin(), ru.end(), GreaterSecond<int,float>);
          for(int i = 0; i < ru.size() && i < 10; ++i){
               if(ru[i].first == k->second){
                    ++ret;
                    break;
               }
          }
     }
     return ret;
}

void loadResults(const char * file, vector< map<int,float> > & test){
     test = vector< map<int,float> >(USER_NUM);
     string line;
     ifstream in(file);
     while(getline(in, line)){
          istringstream iss(line);
          int user;
          iss >> user;
          int item;
          float p;
          while(iss >> item >> p){
               test[user][item] = p;
          }
     }
     int s = score(test);
     cout << file << "\t" << s << endl;
}

int bag(vector< vector< map<int,float> > > & tests, vector<float> & weight){
     vector< map<int,float> > aa(USER_NUM);
     for(int k = 0; k < tests.size(); ++k){
          for(int u = 0; u < tests[k].size(); ++u){
               if(tests[k][u].empty()) continue;
               for(map<int,float>::iterator j = tests[k][u].begin(); j != tests[k][u].end(); ++j){
                    int item = j->first;
                    if(aa[u].find(item) == aa[u].end()) aa[u][item] = 0;
                    aa[u][item] += (float)(j->second) * weight[k];
               }
          }
     }
     ofstream out("result-train.txt");
     for(int i = 0; i < aa.size(); ++i){
          if(aa[i].empty()) continue;
          vector< pair<int,float> > reco(aa[i].begin(), aa[i].end());
          sort(reco.begin(), reco.end(), GreaterSecond<int,float>);
          //if(reco.size() > 20) reco.erase(reco.begin() + 20, reco.end());
          //postProcessByLanguage(i, reco);
          out << i << ":" << reco[0].first;
          for(int k = 1; k < reco.size() && k < 20; ++k)
               out << "," << reco[k].first;
          out << endl;
     }
     out.close();
     return score(aa);
}

int main(int argc, char ** argv){
     loadData(data,true);
     loadTestData();
     loadUserItemData(users, items, data);
     for(int i = 0; i < data.size(); ++i) nu[data[i].user]++;
#define K 7
     vector< vector< map<int,float> > > tests(K);
     vector< float > weight(K);
     int k = 0;
     loadResults("../ret2/results-knni-iuf.txt.0", tests[k]); weight[k] = 1; ++k;
     loadResults("../ret2/results-knnu-iif.txt.0", tests[k]); weight[k] = 1; ++k;
     loadResults("../ret2/results-knnui.txt.0", tests[k]); weight[k] = 1; ++k;
     loadResults("../ret2/results-language.txt.0", tests[k]); weight[k] = 0.4; ++k;
     loadResults("../ret2/results-repos.txt.0", tests[k]); weight[k] = 0.4; ++k;
     loadResults("../ret2/results-reponame.txt.0", tests[k]); weight[k] = 0.2; ++k;
     loadResults("../ret2/results-pop-nic.txt.0", tests[k]); weight[k] = 0.01; ++k;
     srand(time(0));
     int j = 0;
     for(int step = 0; step < 20; ++step){
          int i = j % K;
          ++j;
          float w0 = weight[i];
          int r0 = bag(tests, weight);
          weight[i] *= (1 + 0.5 * (rand01() - 0.5));
          int r1 = bag(tests, weight);
          if(r1 <= r0)
               weight[i] = w0;
          cout << step << "\t" << r0 << "\t" << r1 << endl;
     }

     for(int i = 0; i < weight.size(); ++i) cout << i << "\t" << weight[i] << endl;
     return 0;
}

