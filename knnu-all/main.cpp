#include "github.h"

vector< Rating > data;
vector< vector<int> > iu(ITEM_NUM), ui(USER_NUM);
vector< float > nu(USER_NUM), ni(ITEM_NUM);
vector< vector< pair<int,float> > > w(USER_NUM);
UserList users;
ItemList items;

void rebuildUserLang(){
     vector< map<string,int> > ulang(USER_NUM);
     for(int i = 0; i < data.size(); ++i){
          int ii = data[i].item;
          int uu = data[i].user;
          for(map<string,float>::iterator k = items[ii].language.begin(); k != items[ii].language.end(); ++k){
               ulang[uu][k->first]++;
          }
     }
     for(int i = 0; i < ulang.size(); ++i){
          if(ulang[i].empty()) continue;
          float zz = 0;
          for(map<string,int>::iterator k = ulang[i].begin(); k != ulang[i].end(); ++k)
               zz += (float)(k->second);
          users[i].language.clear();
          for(map<string,int>::iterator k = ulang[i].begin(); k != ulang[i].end(); ++k)
               users[i].language[k->first] = (float)(k->second) / zz;
     }
}

void rebuildUserRepos(){
     vector< map<string,int> > urepos(USER_NUM);
     for(int i = 0; i < data.size(); ++i){
          int ii = data[i].item;
          int uu = data[i].user;
          for(map<string,float>::iterator k = items[ii].repos.begin(); k != items[ii].repos.end(); ++k){
               urepos[uu][k->first]++;
          }
     }
     for(int i = 0; i < urepos.size(); ++i){
          if(urepos[i].empty()) continue;
          float zz = 0;
          for(map<string,int>::iterator k = urepos[i].begin(); k != urepos[i].end(); ++k)
               zz += (float)(k->second);
          users[i].repos.clear();
          for(map<string,int>::iterator k = urepos[i].begin(); k != urepos[i].end(); ++k)
               users[i].repos[k->first] = (float)(k->second) / zz;
     }
}

void loadSim(bool train){
     string file = "../data2/knnu-iif.txt";
     if(train) file += ".0";
     ifstream in(file.c_str());
     int a, b;
     float s;
     while(in >> a >> b >> s){
          s *= 0.3 + sim(users[a].repos, users[b].repos);
          s *= 0.3 + sim(users[a].language, users[b].language);
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
     loadUserItemData(users,items,data);
     rebuildUserLang();
     rebuildUserRepos();
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ui[user].push_back(item);
     }
     int u;
     loadSim(train);
     map<int,int> test;
     if(train) getTestSet2(test);
     else getTestSet(test);
     string file = "../ret2/results-knnu-all.txt";
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
     if(atoi(argv[1]) == 0)
          predictAll(false);
     else predictAll(true);
     return 0;
}

