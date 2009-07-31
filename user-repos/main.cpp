#include "github.h"

vector< Rating > data;
vector< vector<int> > iu(ITEM_NUM), ui(USER_NUM);
vector< float > nu(USER_NUM), ni(ITEM_NUM);
vector< vector< pair<int,float> > > w(USER_NUM);
vector< map<string,float> > user_repos(USER_NUM), user_lang(USER_NUM);
vector< map<string,float> > item_repos(ITEM_NUM), item_lang(ITEM_NUM);

void loadRepos(){
     ifstream in("../download/repos.txt");
     string line;
     while(getline(in,line)){
          string tmp;
          for(int i = 0; i < line.length(); ++i){
               if(line[i] == ':' || line[i] == '/' || line[i] == ',')
                    tmp += ' ';
               else tmp += line[i];
          }
          istringstream iss(tmp);
          int id;
          string buf;
          iss >> id >> buf;
          item_repos[id][buf] = 1;
     }
     in.close();
     ifstream in1("../data/user-repos.txt");
     int uu;
     float pp;
     string ll;
     while(in1 >> uu >> ll >> pp){
          user_repos[uu][ll] = pp;
     }
     in1.close();
}

void loadLang(){
     ifstream in("../download/lang.txt");
     string line;
     while(getline(in,line)){
          string tmp;
          for(int i = 0; i < line.length(); ++i){
               if(line[i] == ':' || line[i] == ';' || line[i] == ',')
                    tmp += ' ';
               else tmp += line[i];
          }
          istringstream iss(tmp);
          int id, ln;
          string buf;
          iss >> id;
          while(iss >> buf >> ln)
               item_lang[id][buf] = 1;
     }
     in.close();
     ifstream in1("../data/user-lang.txt");
     int uu;
     float pp;
     string ll;
     while(in1 >> uu >> ll >> pp){
          user_lang[uu][ll] = pp;
     }
     in1.close();
}

void loadSim(){
     loadRepos();
     loadLang();
     ifstream in("../data/knnu-iif.txt");
     int a, b;
     float s;
     while(in >> a >> b >> s){
          s *= 0.3 + sim(user_repos[a], user_repos[b]);
          s *= 0.3 + sim(user_lang[a], user_lang[b]);
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
               cand[ii] += w[u][v].second
                    * (0.1 + 0.9 * sim(user_repos[u], item_repos[ii]))
                    * (0.1 + 0.9 * sim(user_lang[u], item_lang[ii]));
          }
     }
     for(int i = 0; i < cand.size(); ++i)
          if(cand[i] > 0)
               ret.push_back(make_pair<int,float>(i, cand[i]));
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
}

void predictAll(){
     loadData(data);
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ui[user].push_back(item);
     }
     int u;
     loadSim();
     ifstream in("../download/test.txt");
     ofstream out("results-knnu-repos-lang3.txt");
     int k = 0;
     while(in >> u){
          if(++k % 200 == 0) cout << k << endl;
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
     predictAll();
     return 0;
}

