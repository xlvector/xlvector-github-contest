#include "github.h"

vector< Rating > data;
vector< vector<int> > ui(USER_NUM);
vector< float > nu(USER_NUM,0), ni(ITEM_NUM,0);
UserList users;
ItemList items;
set<int> I;
vector<string> user_most_repo(USER_NUM);
map< string,set<int> > repo_ids;
map< string,set<int> > owner_ids;

void predict(int u, vector< pair<int,float> > & ret){
     set<int> rated(ui[u].begin(), ui[u].end());
     vector<float> cand(ITEM_NUM, 0);
     for(set<int>::iterator i = I.begin(); i != I.end(); ++i){
          int ii = *i;
          if(rated.find(ii) != rated.end()) continue;
          cand[ii] = sim2(users[u].repos, items[ii].repos);
     }
     for(int i = 0; i < cand.size(); ++i)
          if(cand[i] > 0)
               ret.push_back(make_pair<int,float>(i, cand[i]));
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
}

void rebuildUserRepos(){
     vector< map<string,int> > urepos(USER_NUM);
     for(int i = 0; i < data.size(); ++i){
          int ii = data[i].item;
          int uu = data[i].user;
          for(map<string,float>::iterator k = items[ii].repos.begin(); k != items[ii].repos.end(); ++k){
               urepos[uu][k->first]++;
               repo_ids[k->first].insert(ii);
          }
     }
     for(int i = 0; i < urepos.size(); ++i){
          if(urepos[i].empty()) continue;
          float zz = 0;
          for(map<string,int>::iterator k = urepos[i].begin(); k != urepos[i].end(); ++k){
               zz += (float)(k->second);
          }
          users[i].repos.clear();
          
          vector< pair<string,float> > ur;
          for(map<string,int>::iterator k = urepos[i].begin(); k != urepos[i].end(); ++k){
               float ss = (float)(k->second) / (float)(repo_ids[k->first].size() + 3.5);
               ur.push_back(make_pair<string,float>(k->first, ss));
          }
          users[i].repos = map<string,float>(ur.begin(), ur.end());
          sort(ur.begin(), ur.end(), GreaterSecond<string,float>);
          user_most_repo[i] = ur[0].first;
     }
}

void predict2(int u, vector< pair<int,float> > & ret){
     set<int> rated(ui[u].begin(), ui[u].end());
     vector<float> cand(ITEM_NUM, 0);
     for(set<int>::iterator i = I.begin(); i != I.end(); ++i){
          int ii = *i;
          if(rated.find(ii) != rated.end()) continue;
          cand[ii] = sim(users[u].repos, items[ii].repos);
     }
     for(int i = 0; i < cand.size(); ++i)
          if(cand[i] > 0)
               ret.push_back(make_pair<int,float>(i, cand[i]));
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
}

void predictMost(int u, vector< pair<int,float> > & ret){
     set<int> rated(ui[u].begin(), ui[u].end());
     string rp = user_most_repo[u];
     if(rp.empty()) return;
     map<int,float> tmp;
     if(repo_ids.find(rp) == repo_ids.end()) return;
     for(set<int>::iterator k = repo_ids[rp].begin(); k != repo_ids[rp].end(); ++k){
          if(rated.find(*k) != rated.end()) continue;
          tmp[*k] = 1;
     }
     for(set<int>::iterator k = owner_ids[rp].begin(); k != owner_ids[rp].end(); ++k){
          if(rated.find(*k) != rated.end()) continue;
          tmp[*k] = 1;
     }
     ret = vector< pair<int,float> >(tmp.begin(), tmp.end());
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
}

void loadAll(){
     ifstream in2("../download/repo_col.txt");
     string line;
     while(getline(in2,line)){
          int id;
          string rp,name;
          istringstream iss(line);
          iss >> id >> rp;
          while(iss >> name)
               if(rp.find(name) == 0) continue;
          owner_ids[name].insert(id);
     }
     in2.close();
}

void predictAll(bool train){
     loadData(data,train);
     loadUserItemData(users,items,data);
     loadAll();
     rebuildUserRepos();
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ui[user].push_back(item);
          I.insert(item);
          ni[item]++;
          nu[user]++;
     }
     int u;
     map<int,int> test;
     if(train) getTestSet2(test);
     else getTestSet(test);
     string file = "../ret2/results-repos-most-col.txt";
     if(train) file += ".0";
     ofstream out(file.c_str());
     for(map<int,int>::iterator k = test.begin(); k != test.end(); ++k){
          int u = k->first;
          vector< pair<int,float> > ret;
          predictMost(u, ret);
          
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

