#include "github.h"

vector< Rating > data;
vector< vector<int> > ui(USER_NUM);
vector< float > nu(USER_NUM), ni(ITEM_NUM);
vector< vector< pair<int, float> > > w(ITEM_NUM);

void model(bool train){
     loadData(data,train);
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ui[user].push_back(item);
          nu[user]++;
          ni[item]++;
     }
     vector< map<int, float> > co(ITEM_NUM);
     for(int u = 0; u < ui.size(); ++u){
          if(ui[u].empty()) continue;
          if(u % 1000 == 0) cout << u << endl;
          for(int i = 0; i < ui[u].size(); ++i){
               int ii = ui[u][i];
               for(int j = i + 1; j < ui[u].size(); ++j){
                    int jj = ui[u][j];
                    if(co[ii].find(jj) == co[ii].end()) co[ii][jj] = 0;
                    if(co[jj].find(ii) == co[jj].end()) co[jj][ii] = 0;
                    co[ii][jj] += 1 / log(3 + nu[u]);
                    co[jj][ii] += 1 / log(3 + nu[u]);
               }
          }
     }
     string file = "../data2/knni-iuf.txt";
     if(train) file += ".0";
     ofstream out(file.c_str());
     for(int i = 0; i < co.size(); ++i){
          for(map<int,float>::iterator j = co[i].begin(); j != co[i].end(); ++j){
               float s = j->second / sqrt(ni[i] * ni[j->first]);
               out << i << "\t" << j->first << "\t" << s << endl;
          }
     }
     out.close();
}

void loadSim(bool train){
     string file = "../data2/knni-iuf.txt";
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
     vector<float> nn(ITEM_NUM, 0);
     for(int i = 0; i < ui[u].size(); ++i){
          int ii = ui[u][i];
          for(int j = 0; j < w[ii].size(); ++j){
               int jj = w[ii][j].first;
               if(rated.find(jj) != rated.end()) continue;
               cand[jj] += w[ii][j].second + 0.1 * w[ii][j].second * w[ii][j].second;
               nn[jj] += 1;
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
          ni[item]++;
     }
     
     int u;
     loadSim(train);
     map<int,int> test;
     if(train) getTestSet2(test);
     else getTestSet(test);
     string file = "../ret2/results-knni-iuf2.txt";
     if(train) file += ".0";
     ofstream out(file.c_str());
     for(map<int,int>::iterator k = test.begin(); k != test.end(); ++k){
          int u = k->first;
          vector< pair<int,float> > ret, retlang;
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

