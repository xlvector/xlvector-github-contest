#include "github.h"

vector< Rating > data;
vector< vector<int> > ui(USER_NUM);
vector< float > nu(USER_NUM,0), ni(ITEM_NUM,0);
UserList users;
ItemList items;
set<int> I;
vector<string> user_most_repo(USER_NUM);
map< string,set<int> > repo_ids;

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
               float ss = (float)(k->second) / (float)(repo_ids[k->first].size() + 3);
               ur.push_back(make_pair<string,float>(k->first, ss));
          }
          users[i].repos = map<string,float>(ur.begin(), ur.end());
          sort(ur.begin(), ur.end(), GreaterSecond<string,float>);
          user_most_repo[i] = ur[0].first;
     }
}

void predictMost(int u, vector< pair<int,float> > & ret){
     string rp = user_most_repo[u];
     if(rp.empty()) return;
     if(repo_ids.find(rp) == repo_ids.end()) return;
     for(set<int>::iterator k = repo_ids[rp].begin(); k != repo_ids[rp].end(); ++k){
          ret.push_back(make_pair<int,float>(*k, 1));
     }
}

void predictAll(bool train){
     loadData(data,train);
     loadUserItemData(users,items,data);
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
     string file = "../ret2/results-repos-most.txt";
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

