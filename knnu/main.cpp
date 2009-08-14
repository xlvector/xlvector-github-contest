#include "github.h"

vector< Rating > data;
vector< vector<int> > iu(ITEM_NUM), ui(USER_NUM);
vector< float > nu(USER_NUM), ni(ITEM_NUM);
vector< vector< pair<int,float> > > w(USER_NUM);

void model(bool train){
     loadData(data,train);
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          iu[item].push_back(user);
          nu[user]++;
          ni[item]++;
     }
     vector< map<int, float> > co(USER_NUM);
     for(int i = 0; i < iu.size(); ++i){
          if(iu[i].empty()) continue;
          if(i % 1000 == 0) cout << i << endl;
          for(int u = 0; u < iu[i].size(); ++u){
               int uu = iu[i][u];
               for(int v = u + 1; v < iu[i].size(); ++v){
                    int vv = iu[i][v];
                    if(co[uu].find(vv) == co[uu].end()) co[uu][vv] = 0;
                    if(co[vv].find(uu) == co[vv].end()) co[vv][uu] = 0;
                    co[uu][vv] += 1 / log(3 + ni[i]);
                    co[vv][uu] += 1 / log(3 + ni[i]);
               }
          }
     }
     string file = "../data2/knnu-iif.txt";
     if(train) file += ".0";
     ofstream out(file.c_str());
     for(int i = 0; i < co.size(); ++i){
          for(map<int,float>::iterator j = co[i].begin(); j != co[i].end(); ++j){
               float s = j->second / sqrt(nu[i] * nu[j->first]);
               out << i << "\t" << j->first << "\t" << s << endl;
          }
     }
     out.close();
}

void loadSim(bool train){
     string file = "../data2/knnu-iif.txt";
     if(train) file += ".0";
     ifstream in(file.c_str());
     int a, b;
     float s;
     while(in >> a >> b >> s){
          w[a].push_back(make_pair<int,float>(b,s));
     }
     for(int i = 0; i < w.size(); ++i){
          sort(w[i].begin(), w[i].end(), GreaterSecond<int,float>);
     }
     cout << "load sim finished!" << endl;
}

void predict(int u, vector< pair<int,float> > & ret){
     set<int> rated(ui[u].begin(), ui[u].end());
     vector<float> cand(ITEM_NUM, 0);
     for(int v = 0; v < w[u].size() && v < 1000; ++v){
          int vv = w[u][v].first;
          for(int i = 0; i < ui[vv].size(); ++i){
               int ii = ui[vv][i];
               if(rated.find(ii) != rated.end()) continue;
               cand[ii] += w[u][v].second;
          }
     }
     for(int i = 0; i < cand.size(); ++i)
          if(cand[i] > 0)
               ret.push_back(make_pair<int,float>(i, cand[i]));
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
}

void predictAll(bool train){
     loadData(data,train);
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ui[user].push_back(item);
          nu[user]++;
     }
     int u;
     loadSim(train);
     map<int,int> test;
     if(train) getTestSet2(test);
     else getTestSet(test);
     string file = "../ret2/results-knnu-iif.txt";
     if(train) file += ".0";
     ofstream out(file.c_str());
     for(map<int,int>::iterator k = test.begin(); k != test.end(); ++k){
          int u = k->first;
          vector< pair<int,float> > ret;
          predict(u, ret);
          
          out << u << "\t";
          for(int i = 0; i < ret.size() && i < 500; ++i)
               out << ret[i].first << "\t" << ret[i].second / ret[0].second << "\t";
          out << endl;
     }
     out.close();
}

int main(int argc, char ** argv){
     string type = argv[1];
     bool train = false;
     if(atoi(argv[2]) == 1) train = true;
     if(type == "-m")
          model(train);
     else if(type == "-p")
          predictAll(train);
     return 0;
}

