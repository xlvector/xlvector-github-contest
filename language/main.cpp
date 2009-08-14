#include "github.h"

vector< Rating > data;
vector< vector<int> > ui(USER_NUM);
vector< float > nu(USER_NUM,0), ni(ITEM_NUM,0);
UserList users;
ItemList items;
set<int> I;

void predict(int u, vector< pair<int,float> > & ret){
     set<int> rated(ui[u].begin(), ui[u].end());
     vector<float> cand(ITEM_NUM, 0);
     for(set<int>::iterator i = I.begin(); i != I.end(); ++i){
          int ii = *i;
          if(rated.find(ii) != rated.end()) continue;
          cand[ii] = sim(users[u].language, items[ii].language);
     }
     for(int i = 0; i < cand.size(); ++i)
          if(cand[i] > 0)
               ret.push_back(make_pair<int,float>(i, cand[i]));
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
}

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

void predictAll(bool train){
     loadData(data,train);
     loadUserItemData(users,items,data);
     rebuildUserLang();
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ui[user].push_back(item);
          ni[item]++;
          I.insert(item);
     }
     int u;
     map<int,int> test;
     if(train) getTestSet2(test);
     else getTestSet(test);
     string file = "../ret2/results-language-multi.txt";
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

