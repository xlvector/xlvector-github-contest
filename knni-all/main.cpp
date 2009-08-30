#include "github.h"

vector< Rating > data;
vector< vector<int> > ui(USER_NUM);
vector< float > nu(USER_NUM), ni(ITEM_NUM);
vector< vector< pair<int, float> > > w(ITEM_NUM);
UserList users;
ItemList items;

void loadSim(bool train){
     string file = "../data2/knni-iuf.txt";
     if(train) file += ".0";
     ifstream in(file.c_str());
     int a, b;
     float s;
     while(in >> a >> b >> s){
          s *= 1 + sim(items[a].repos, items[b].repos);
          s *= 1 + sim(items[a].language, items[b].language);
          double dt = abs(items[a].tm - items[b].tm);
          dt /= 86400;
          s *= exp(-0.04 * dt);
          s *= 0.8 + sim(items[a].name, items[b].name);
          w[a].push_back(make_pair<int,float>(b,s));
     }
     for(int i = 0; i < w.size(); ++i){
          sort(w[i].begin(), w[i].end(), GreaterSecond<int,float>);
     }
     cout << "load sim finished!" << endl;
}
/*
void predict(int u, vector< pair<int,float> > & ret){
     set<int> rated(ui[u].begin(), ui[u].end());
     vector<float> cand(ITEM_NUM, 0);
     for(int i = 0; i < ui[u].size(); ++i){
          int ii = ui[u][i];
          for(int j = 0; j < w[ii].size() && j < 1000; ++j){
               int jj = w[ii][j].first;
               if(rated.find(jj) != rated.end()) continue;
               cand[jj] += w[ii][j].second
                    * (1 + sim(user_repos[u], item_repos[jj]))
                    * (1 + sim(user_lang[u], item_lang[jj]));
          }
     }
     for(int i = 0; i < cand.size(); ++i)
          if(cand[i] > 0)
               ret.push_back(make_pair<int,float>(i, cand[i]));
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
}
*/
void predict2(int u, vector< pair<int,float> > & ret){
     set<int> rated(ui[u].begin(), ui[u].end());
     vector<float> cand(ITEM_NUM, 0);
     for(int i = 0; i < ui[u].size(); ++i){
          int ii = ui[u][i];
          for(int j = 0; j < w[ii].size() && j < 1000; ++j){
               int jj = w[ii][j].first;
               if(rated.find(jj) != rated.end()) continue;
               cand[jj] += w[ii][j].second;
          }
     }
     for(int i = 0; i < cand.size(); ++i)
          if(cand[i] > 0)
               ret.push_back(make_pair<int,float>(i, cand[i]));
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
}

void predictAll(bool train){
     loadData(data,train);
     loadUserItemData(users, items, data);
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
     string file = "../ret2/results-knni-all.txt";
     if(train) file += ".0";
     ofstream out(file.c_str());
     for(map<int,int>::iterator k = test.begin(); k != test.end(); ++k){
          int u = k->first;
          vector< pair<int,float> > ret;
          predict2(u, ret);
          
          out << u << "\t";
          for(int i = 0; i < ret.size() && i < 500; ++i)
               out << ret[i].first << "\t" << ret[i].second / ret[0].second << "\t";
          out << endl;
     }
     out.close();
}

int main(int argc, char ** argv){
     if(atoi(argv[1]) == 0) predictAll(false);
     else predictAll(true);
     return 0;
}

