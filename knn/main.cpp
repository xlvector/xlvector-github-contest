#include "github.h"

vector< Rating > data;
vector< vector<int> > ui(USER_NUM);
vector< float > nu(USER_NUM), ni(ITEM_NUM);
vector< vector< pair<int, float> > > w(ITEM_NUM);

double average(vector<float> & a){
     double ret = 0;
     double n = 0;
     for(int i = 0; i < a.size(); ++i){
          if(a[i] == 0) continue;
          ret += a[i];
          n += 1;
     }
     return ret/n;
}

void model(){
     loadData(data);
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
     ofstream out("../data/knni-iuf.txt");
     for(int i = 0; i < co.size(); ++i){
          for(map<int,float>::iterator j = co[i].begin(); j != co[i].end(); ++j){
               float s = j->second / sqrt(ni[i] * ni[j->first]);
               out << i << "\t" << j->first << "\t" << s << endl;
          }
     }
     out.close();
}

void loadSim(){
     ifstream in("../data/knni-iuf.txt");
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
     map<int, float> cand;
     map<int, int> nn;
     for(int i = 0; i < ui[u].size(); ++i){
          int ii = ui[u][i];
          for(int j = 0; j < w[ii].size() && j < 5000; ++j){
               int jj = w[ii][j].first;
               if(rated.find(jj) != rated.end()) continue;
               if(cand.find(jj) == cand.end()) cand[jj] = 0;
               cand[jj] += w[ii][j].second;
               nn[jj] += 1;
          }
     }
     /*
     for(map<int,float>::iterator i = cand.begin(); i != cand.end(); ++i){
          i->second /= sqrt((double)(nn[i->first] + 1));
     }
     */
     ret = vector< pair<int,float> >(cand.begin(), cand.end());
     sort(ret.begin(), ret.end(), GreaterSecond<int,float>);
}

void predictAll(){
     loadData(data);
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ui[user].push_back(item);
          nu[user]++;
     }
     for(int i = 0; i < data.size(); ++i){
          int user = data[i].user;
          int item = data[i].item;
          ni[item] += 1 / log(3 + nu[user]);
     }
     vector< pair<int,float> > popular;
     for(int i = 0; i < ni.size(); ++i)
          popular.push_back(make_pair<int,float>(i, ni[i]));
     sort(popular.begin(), popular.end(), GreaterSecond<int,float>);
     
     int u;
     loadSim();
     ifstream in("../download/test.txt");
     ofstream out("results.txt");
     while(in >> u){
          vector< pair<int,float> > ret;
          predict(u, ret);
          out << u << ":";
          if(!ret.empty()){
               out << ret[0].first;
               for(int i = 1; i < ret.size() && i < 10; ++i)
                    out << "," << ret[i].first;
               if(ret.size() < 10){
                    for(int i = 0; i < 10 - ret.size(); ++i)
                         out << "," << popular[i].first;
               }
          }else{
               out << popular[0].first;
               for(int i = 1; i < popular.size() && i < 10; ++i)
                    out << "," << popular[i].first;
          }
          out << endl;
     }
     out.close();
}

int main(int argc, char ** argv){
     string type = argv[1];
     if(type == "-m")
          model();
     else if(type == "-p")
          predictAll();
     return 0;
}

