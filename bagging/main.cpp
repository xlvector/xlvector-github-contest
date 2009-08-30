#include "github.h"

vector<Rating> data;
UserList users;
ItemList items;
vector< set<int> > ui(USER_NUM);
vector<float> nu(USER_NUM, 0), ni(ITEM_NUM, 0), nup(USER_NUM, 0), nip(ITEM_NUM, 0),nfi(ITEM_NUM, 0), nuf(USER_NUM, 0);
vector<float> item_most_popular(ITEM_NUM, 0), user_most_popular(USER_NUM, 0);
vector< map<int,float> > item_page(ITEM_NUM);
vector< map<int,float> > aa(USER_NUM);
vector< int > forkmap(ITEM_NUM);
vector< int > watchs(ITEM_NUM, 0);
vector< map<string,float> > desc(ITEM_NUM);
vector< map<string,float> > user_desc(USER_NUM);
map<string, int> tf;
int T = 4;
int T2 = 10;

void loadSimPage(){
     ifstream in("../data2/knni-cluster.txt");
     int i1,i2;
     float w;
     while(in >> i1 >> i2 >> w){
          if(w < 0.3) continue;
          item_page[i1][i2] = w;
          item_page[i2][i1] = w;
     }
     in.close();
}

void loadWatch(){
     ifstream in("../download/repo_watch.txt");
     int i,n;
     string line;
     while(getline(in,line)){
          istringstream iss(line);
          iss >> i >> n;
          watchs[i] = n;
     }
     in.close();
     ifstream in1("../download/repo_forks.txt");
     while(getline(in,line)){
          istringstream iss(line);
          iss >> i >> n;
          nfi[i] = n;
     }
     in1.close();
}

void loadFork(){
     ifstream in("../download/fork.txt");
     int i1, i2;
     while(in >> i1 >> i2)
          forkmap[i1] = i2;
     in.close();
}

void loadDesc(){
     ifstream in("../download/repo_desc.txt");
     string line;
     while(getline(in,line)){
          int id;
          string buf;
          istringstream iss(line);
          iss >> id;
          while(iss >> buf){
               if(buf.length() < 4) continue;
               desc[id][buf] = 1;
               tf[buf] += 1;
          }
     }
     in.close();
     for(int i = 0; i < data.size(); ++i){
          int uu = data[i].user;
          int ii = data[i].item;
          for(map<string,float>::iterator k = desc[ii].begin(); k != desc[ii].end(); ++k)
               user_desc[uu][k->first] = 1;
     }
}

void loadResults(const char * file, vector< map<int,float> > & test, float pv){
     test = vector< map<int,float> >(USER_NUM);
     string line;
     ifstream in(file);
     if(in.fail()) cout << file << endl;
     while(getline(in, line)){
          istringstream iss(line);
          int user;
          iss >> user;
          int item;
          float p;
          float zz = 0;
          int tn = 0;
          int N = 100;
          while(iss >> item >> p){
               if(p < pv) break;
               if(++tn < N || p > 0.9){
                    test[user][item] = p;
                    zz += p;
               }
          }
          for(map<int,float>::iterator i = test[user].begin(); i != test[user].end(); ++i){
               i->second /= sqrt(zz + 0.1);
          }
     }
}

void postProcessByLanguage(int u, vector< pair<int,float> > & reco){
     int K = 9;
     if(reco.size() <= K) return;
     for(int k = K; k < reco.size(); ++k){
          int i = reco[k].first;
          double s1 = 0.9 + 0.1 * sim(users[u].language, items[i].language);
          double s2 = 1;// + sim(users[u].repos, items[i].repos);
          reco[k].second *= s1 * s2;
     }
     sort(reco.begin() + K, reco.end(), GreaterSecond<int,float>);
}

void postProcessByDate(int u, vector< pair<int,float> > & reco){
     if(nu[u] <= T + 4) return;
     int K = 5;
     if(reco.size() <= K) return;
     for(int k = K; k < reco.size(); ++k){
          int i = reco[k].first;
          double s = 1;
          if(items[i].tm < users[u].begin_tm - 564000 || items[i].tm > users[u].end_tm + 564000) s = 0.5;
          reco[k].second *= s;
     }
     sort(reco.begin() + K, reco.end(), GreaterSecond<int,float>);
}

void userMostPopular(int u, vector< pair<int,float> > & reco){
     if(nu[u] <= T + 3) return;
     for(int k = 0; k < reco.size(); ++k){
          int i = reco[k].first;
          if(ni[i] > user_most_popular[u])
               reco[k].second *= 0.6;
     }
     sort(reco.begin(), reco.end(), GreaterSecond<int,float>);
}

void postByWatchNum(int u, vector< pair<int,float> > & reco){
     for(int k = 0; k < reco.size(); ++k){
          int i = reco[k].first;
          if(watchs[i] == 0) continue;
          if(ni[i] >= watchs[i]) reco[k].second *= 0.2;
     }
     sort(reco.begin(), reco.end(), GreaterSecond<int,float>);
}

void itemMostPopular(int u, vector< pair<int,float> > & reco){
     for(int k = 0; k < reco.size(); ++k){
          int i = reco[k].first;
          if(ni[i] < 4) continue;
          if(nu[u] > item_most_popular[i])
               reco[k].second *= 0.6;
     }
     sort(reco.begin(), reco.end(), GreaterSecond<int,float>);
}

void unPopular(int u, vector< pair<int,float> > & reco){
     if(nu[u] <= T) return;
     int t = 0;
     for(int k = 0; k < reco.size(); ++k){
          int i = reco[k].first;
          if(ni[i] > 3) ++t;
          if(t > 1) reco[k].second *= 0.9;
     }
     sort(reco.begin(), reco.end(), GreaterSecond<int,float>);
}

void reBuildUser(){
     for(int i = 0; i < data.size(); ++i){
          int uu = data[i].user;
          int ii = data[i].item;
          for(map<string,float>::iterator k = items[ii].repos.begin(); k != items[ii].repos.end(); ++k)
               users[uu].repos[k->first] = 1;
          for(map<string,float>::iterator k = items[ii].language.begin(); k != items[ii].language.end(); ++k)
               users[uu].language[k->first] = 1;
     }
}

void candidate(int u, vector< pair<int,float> > & reco){
     if(nu[u] <= T + 5) return;
     for(int k = 0; k < reco.size(); ++k){
          int i = reco[k].first;
          if(users[u].min_line > 3 * items[i].line) reco[k].second *= 0.7;
     }
     sort(reco.begin(), reco.end(), GreaterSecond<int,float>);
}

void diversity(int u, vector< pair<int,float> > & reco){
     vector< pair<int,float> > tmp;
     int t = 0;
     for(int k = 0; k < reco.size(); ++k){
          int i = reco[k].first;
          string rek = (items[i].repos.begin())->first;
          for(int j = 0; j < k; ++j){
               string rej = (items[reco[j].first].repos.begin())->first;
               double sn = sim(items[i].name, items[reco[j].first].name);
               //double sc = sim(desc[i], desc[reco[j].first]);
               if(rek == rej){
                    reco[k].second *= 0.7;// * 100 / (100 + nu[u]);
               }else{
                    if(sn > 0.8)
                         reco[k].second *= 0.8;
                    if(nu[u] > T + 3 && item_page[i].find(reco[j].first) != item_page[i].end()){
                         float ss = item_page[i][reco[j].first];
                         reco[k].second *= 1.8 / (1.8 + ss);
                    }
               }
          }
     }
     sort(reco.begin(), reco.end(), GreaterSecond<int,float>);
}

float sigmoid(float x, int c){
     return x + 0.05 * x * x;
}

float revise_weight(float x){
     float ret = x;
     //if(x > 0.7) ret += 0.15;
     return ret;
}

void bag(vector< vector< map<int,float> > > & tests, vector< vector<float> > & weight){
     for(int k = 0; k < tests.size(); ++k){
          for(int u = 0; u < tests[k].size(); ++u){
               if(tests[k][u].empty()) continue;
               int cc = 0;
               if(nu[u] > T) cc = 1;
               if(nu[u] > T2) cc = 2;
               for(map<int,float>::iterator j = tests[k][u].begin(); j != tests[k][u].end(); ++j){
                    int item = j->first;
                    if(ui[u].find(item) != ui[u].end()) continue;
                    if(aa[u].find(item) == aa[u].end()){
                         aa[u][item] = 0;
                    }
                    aa[u][item] += sigmoid(j->second,cc) * revise_weight(weight[cc][k]);
               }
          }
     }
     ofstream out("results0.txt");
     ofstream out1("results100.txt");
     for(int i = 0; i < aa.size(); ++i){
          if(aa[i].empty()) continue;
          if(nu[i] <= T) continue;
          vector< pair<int,float> > reco(aa[i].begin(), aa[i].end());
          sort(reco.begin(), reco.end(), GreaterSecond<int,float>);
          postByWatchNum(i, reco);
          postProcessByDate(i, reco);
          int N = 30;
          if(nu[i] > T) N = 50;
          if(reco.size() > N) reco.erase(reco.begin() + N, reco.end());
          //candidate(i, reco);
          diversity(i, reco);
          userMostPopular(i, reco);
          vector<int> reco2;
          for(int k = 0; k < reco.size() && k < 10; ++k) reco2.push_back(reco[k].first);
          sort(reco2.begin(), reco2.end());
          out << i << ":" << reco2[0];
          for(int k = 1; k < reco2.size() && k < 10; ++k)
               out << "," << reco2[k];
          out << endl;
          
          out1 << i << "\t";
          for(int k = 0; k < reco.size() && k < 100; ++k)
               out1 << reco[k].first << "\t" << reco[k].second << "\t";
          out1 << endl;
     }
     out.close();
     //out1.close();
}

int main(int argc, char ** argv){
     loadData(data);
     loadUserItemData(users, items, data);
     loadSimPage();
     loadWatch();
     loadFork();
     //loadDesc();
     reBuildUser();
     for(int i = 0; i < data.size(); ++i){
          nu[data[i].user]++;
          ni[data[i].item]++;
          ui[data[i].user].insert(data[i].item);
     }
     for(int i = 0; i < data.size(); ++i){
          nup[data[i].user] += ni[data[i].item];
          nip[data[i].item] += nu[data[i].user];
          nuf[data[i].user] = max<float>(nuf[data[i].user], nfi[data[i].item]);
          user_most_popular[data[i].user] = max<float>(user_most_popular[data[i].user], ni[data[i].item]);
          item_most_popular[data[i].item] = max<float>(item_most_popular[data[i].item], nu[data[i].user]);
     }
     
     vector< vector< map<int,float> > > tests;
     vector< vector< float > > weight(3);
     ifstream in("weight.txt");
     string file;
     float wt0, wt1, wt2, pv;
     while(in >> wt0 >> wt1 >> wt2 >> pv >> file){
          if(file.find("#") != string::npos) continue;
          vector< map<int,float> > tmp;
          pv = 0;
          loadResults(file.c_str(), tmp, pv);
          tests.push_back(tmp);
          weight[0].push_back(wt0);
          weight[1].push_back(wt1);
          weight[2].push_back(wt2);
     }
     in.close();
     bag(tests, weight);
     return 0;
}

