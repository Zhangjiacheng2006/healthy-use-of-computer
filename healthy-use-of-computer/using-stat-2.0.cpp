#include<ctime>
#include<bitset>
#include<string>
#include<vector>
#include<cstring>
#include<fstream>
#include<iostream>
#include<algorithm>
#include<windows.h>
using namespace std;
const std::string path=std::string(getenv("AppData"))+"\\time-monitor"; // Path for the data
const std::string datPath=std::string(getenv("AppData"))+"\\time-monitor\\data.log"; // Path for logs from all times
const std::string todPath=std::string(getenv("AppData"))+"\\time-monitor\\temp.log"; // Path for logs today(detailed)
const std::string setPath=std::string(getenv("AppData"))+"\\time-monitor\\setts"; // Path for settings
struct ForbidUnit{ // Forbid you from using your computer from bHour:bMin:bSec to eHour:eMin:eSec on these days
	__int8 bHour,bMin,bSec;
	__int8 eHour,eMin,eSec;
	__int16 bYear; __int8 bMonth,bDay;
	__int16 eYear; __int8 eMonth,eDay;
	__int8 feq; __int32 appId;
};
struct TotTimeUnit{ // Total time for you to use your programs on these days
	__int8 Hour,Min,Sec;
	__int16 bYear; __int8 bMonth,bDay;
	__int16 eYear; __int8 eMonth,eDay;
	__int8 feq; __int32 appId;
};
vector<ForbidUnit> fobs;
vector<TotTimeUnit> ttts;
vector<__int32> wlss;
vector<wstring> appMap;
__int8 mode=3,cah=-1,pltmod=0;
string password; bool nedpas=1;
int countSec(const tm &beg,const tm &end){ // count seconds from beg to end (if in the same day)
	if(beg.tm_year!=end.tm_year||beg.tm_yday!=end.tm_yday) return 0; // not in same day
	return (end.tm_hour*3600 + end.tm_min*60 + end.tm_sec) - (beg.tm_hour*3600 + beg.tm_min*60 + beg.tm_sec);
}
void loadSet(){ // get & change settings
	// read settings
	int szfobs,szttts,szapmp,szwlss; __int8 _cah;
	ifstream file(setPath,ios::binary); if(file){
		file.read(reinterpret_cast<char*>(&_cah),sizeof(_cah));
		if(cah==_cah) return; else cah=_cah;
		fobs.clear(); ttts.clear(); appMap.clear();
		file.read(reinterpret_cast<char*>(&mode),sizeof(mode));
		file.read(reinterpret_cast<char*>(&szfobs),sizeof(szfobs));
		for(int i=0;i<szfobs;i++){
			ForbidUnit cur; file.read(reinterpret_cast<char*>(&cur),sizeof(cur)); fobs.push_back(cur);
		}
		file.read(reinterpret_cast<char*>(&szttts),sizeof(szttts));
		for(int i=0;i<szttts;i++){
			TotTimeUnit cur; file.read(reinterpret_cast<char*>(&cur),sizeof(cur)); ttts.push_back(cur);
		}
		file.read(reinterpret_cast<char*>(&szwlss),sizeof(szwlss));
		for(int i=0;i<szwlss;i++){
			__int32 cur; file.read(reinterpret_cast<char*>(&cur),sizeof(cur)); wlss.push_back(cur);
		}
		file.read(reinterpret_cast<char*>(&szapmp),sizeof(szapmp));
		for(int i=0;i<szapmp;i++){
			int len; file.read(reinterpret_cast<char*>(&len),sizeof(len));
			char *data=new char[len]; file.read(data,len);
			int n=(size_t)MultiByteToWideChar(CP_ACP,0,data,len,NULL,0);
			wchar_t *res=new wchar_t[n];
			MultiByteToWideChar(CP_ACP,0,data,len,res,n);
			wstring str(res,n); appMap.push_back(str);
			delete[] data; delete[] res;
		}
		int passLen; file.read(reinterpret_cast<char*>(&passLen),sizeof(passLen));
		char *data=new char[passLen]; file.read(data,passLen);
		password=string(data,passLen);
		delete[] data;
		file.close();
	}
}
void writeSet(){
	cah=(cah+1)&0x7f; ofstream file(setPath,ios::binary); int cur;
	file.write(reinterpret_cast<char*>(&cah),sizeof(cah));
	file.write(reinterpret_cast<char*>(&mode),sizeof(mode));
	cur=fobs.size(); file.write(reinterpret_cast<char*>(&cur),sizeof(cur));
	for(auto u:fobs) file.write(reinterpret_cast<char*>( &u ),sizeof( u ));
	cur=ttts.size(); file.write(reinterpret_cast<char*>(&cur),sizeof(cur));
	for(auto u:ttts) file.write(reinterpret_cast<char*>( &u ),sizeof( u ));
	cur=wlss.size(); file.write(reinterpret_cast<char*>(&cur),sizeof(cur));
	for(auto u:wlss) file.write(reinterpret_cast<char*>( &u ),sizeof( u ));
	cur=appMap.size(); file.write(reinterpret_cast<char*>(&cur),sizeof(cur));
	for(wstring u:appMap){
		int len=WideCharToMultiByte(CP_ACP,0,u.data(),u.length(),NULL,0,NULL,NULL);
		char *data=new char[len]; WideCharToMultiByte(CP_ACP,0,u.data(),u.length(),data,len,NULL,NULL);
		file.write(reinterpret_cast<char*>(&len),sizeof(len));
		file.write(data,len); delete[] data;
	}
	cur=password.length(); file.write(reinterpret_cast<char*>(&cur),sizeof(cur));
	file.write(password.data(),cur); file.close();
}
bool isunum(string str){
	for(char c:str)if(!isdigit(c))return false;
	return true;
}
bool cmp(const int &aY,const int &aM,const int &aD,const int &bY,const int &bM,const int &bD){
	return (aY<bY)||(aY==bY&&(aM<bM||(aM==bM&&aD<bD)));
}
wstring ctw(string str){
	int n=MultiByteToWideChar(CP_ACP,0,str.data(),str.length(),NULL,0); wchar_t *res=new wchar_t[n];
	MultiByteToWideChar(CP_ACP,0,str.data(),str.length(),res,n); wstring ress(res,n);
	delete[] res; return ress;
}
string wtc(wstring str){
	int n=WideCharToMultiByte(CP_ACP,0,str.data(),str.length(),NULL,0,NULL,NULL); char *res=new char[n];
	WideCharToMultiByte(CP_ACP,0,str.data(),str.length(),res,n,NULL,NULL); string ress(res,n);
	delete[] res; return ress;
}
bool wantPass(){
	if(password.empty()) return true;
	if(nedpas==false)    return true;
	cout<<"input password to continue: ";
	string str; getline(cin,str); for(unsigned int i=0;i<str.length();i++)
		if(str[i]>='A'&&str[i]<='Z') str[i]=str[i]-'A'+'a';
	if(str==password) return true;
	puts("wrong password"); return false;
}
vector<vector<int>> query(int bY,int bM,int bD,int eY,int eM,int eD){ vector<vector<int>> res;
	if(bY<100) bY+=2000; if(eY<100) eY+=2000;
	if(bY>9999||bY<=1970||eY>9999||eY<=1970){puts("invalid date (invalid year)!");return res;}
	if(bM>12||bM<1||eM>12||eM<1){puts("invalid date (invalid month)!");return res;}
	if(bD<1||eD<1){puts("invalid date (invalid day in month)!");return res;}
	if((bM==1||bM==3||bM==5||bM==7||bM==8||bM==10||bM==12)&&bD>31){puts("invalid date (invalid day in month)!");return res;}
	if((bM==4||bM==6||bM==9||bM==11)&&bD>30){puts("invalid date (invalid day in month)!");return res;}
	if((bM==2&&((bY%4==0&&bY%100!=0)||bY%400==0))&&bD>29){puts("invalid date (invalid day in month)!");return res;}
	if((bM==2&&((bY%4!=0||bY%100==0)&&bY%400!=0))&&bD>28){puts("invalid date (invalid day in month)!");return res;}
	if((eM==1||eM==3||eM==5||eM==7||eM==8||eM==10||eM==12)&&eD>31){puts("invalid date (invalid day in month)!");return res;}
	if((eM==4||eM==6||eM==9||eM==11)&&eD>30){puts("invalid date (invalid day in month)!");return res;}
	if((eM==2&&((eY%4==0&&eY%100!=0)||eY%400==0))&&eD>29){puts("invalid date (invalid day in month)!");return res;}
	if((eM==2&&((eY%4!=0||eY%100==0)&&eY%400!=0))&&eD>28){puts("invalid date (invalid day in month)!");return res;}
	if(cmp(eY,eM,eD,bY,bM,bD)){puts("invalid date (bef after end)!");return res;}
	ifstream todfile(todPath); // today save
	ifstream hstfile(datPath); // hist save
	if((!todfile)&&(!hstfile)){
		puts("time-monitor.exe never run, please try to run it first");
		puts("if time-monitor.exe is running, please wait about another 10 minutes."); return res;
	} todfile.close();
	while(hstfile){
		__int16 y;__int8 m,d;
		hstfile.read(reinterpret_cast<char*>(&y),sizeof(y));
		hstfile.read(reinterpret_cast<char*>(&m),sizeof(m));
		hstfile.read(reinterpret_cast<char*>(&d),sizeof(d));
		if((hstfile.eof())||(!hstfile)) break;
		if(!(cmp(y,m,d,bY,bM,bD)||cmp(eY,eM,eD,y,m,d))){
			vector<int> unit; unit.push_back(y);
			unit.push_back(m); unit.push_back(d);
			int totT; hstfile.read(reinterpret_cast<char*>(&totT),sizeof(totT));
			unit.push_back(totT);
			for(int i=0;i<24;i++){
				__int16 tihi; hstfile.read(reinterpret_cast<char*>(&tihi),sizeof(tihi));
				unit.push_back(tihi);
			} res.push_back(unit);
		}
	} hstfile.close();
	todfile.open(todPath); tm begT,endT;
	todfile.read(reinterpret_cast<char*>(&begT),sizeof(begT));
	int y=begT.tm_year+1900,m=begT.tm_mon+1,d=begT.tm_mday;
	if(cmp(y,m,d,bY,bM,bD)||cmp(eY,eM,eD,y,m,d)){ todfile.close();if(res.empty()){puts("no data ...");}return res; }
	vector<int> unit; unit.push_back(y); unit.push_back(m); unit.push_back(d);
	todfile.read(reinterpret_cast<char*>(&endT),sizeof(endT));
	int totT; todfile.read(reinterpret_cast<char*>(&totT),sizeof(totT));
	totT+=countSec(begT,endT); unit.push_back(totT);
	for(int i=0;i<24;i++){
		__int16 tihi; todfile.read(reinterpret_cast<char*>(&tihi),sizeof(tihi));
		unit.push_back(tihi);
	} res.push_back(unit);
	return res;
}
int main(){
	loadSet(); puts("using-stat interactive (input \'help\' for help)");
	#define eql(a,b) (cmds.size()>a&&cmds[a]==b)
	while(!cin.eof()){
		cout<<">> "; string str; getline(cin,str); vector<string> cmds;
		if(str[0]=='!'){ system(str.substr(1).c_str());continue; }
		for(unsigned int i=0;i<str.length();i++)
			if(str[i]>='A'&&str[i]<='Z') str[i]=str[i]-'A'+'a';
		while(!str.empty()){
			int len=str.find(' ');
			if(len==-1){cmds.push_back(str);break;}
			cmds.push_back(str.substr(0,len));
			if(cmds.back().length()==0) cmds.pop_back();
			str=str.substr(len+1);
		}
		if(cmds.empty()) continue;
		#define needpass if(!wantPass())continue;
		if(eql(0,"help")){ // help
			ifstream fin("help.txt");
			while(!fin.eof()){ string hlp; getline(fin,hlp); cout<<hlp<<endl;}
		}else if(eql(0,"exit")){ // exit
			break;
		}else if((eql(0,"plot")||eql(0,"plt"))||(eql(0,"detailed")||eql(0,"det"))||eql(0,"d")||isunum(cmds[0])){
			int nxt=1; if((eql(0,"plot")||eql(0,"plt"))){
				if     (eql(nxt,"n")){pltmod=0;nxt++;}
				else if(eql(nxt,"h")){pltmod=1;nxt++;}
				else if(eql(nxt,"s")){pltmod=2;nxt++;}
				if(eql(nxt,"d")||eql(nxt,"detailed")) nxt++;
			} if(isunum(cmds[0])) nxt=0;
			vector<vector<int>> dat;
			if(cmds.size()-nxt>=6&&isunum(cmds[nxt])&&isunum(cmds[nxt+1])&&isunum(cmds[nxt+2])
				&&isunum(cmds[nxt+3])&&isunum(cmds[nxt+4])&&isunum(cmds[nxt+5])){
				dat=query(stoi(cmds[nxt]),stoi(cmds[nxt+1]),stoi(cmds[nxt+2]),stoi(cmds[nxt+3]),stoi(cmds[nxt+4]),stoi(cmds[nxt+5]));
			}else if(cmds.size()-nxt>=3&&isunum(cmds[nxt])&&isunum(cmds[nxt+1])&&isunum(cmds[nxt+2])){
				dat=query(stoi(cmds[nxt]),stoi(cmds[nxt+1]),stoi(cmds[nxt+2]),stoi(cmds[nxt]),stoi(cmds[nxt+1]),stoi(cmds[nxt+2]));
			}else if(eql(nxt,"today")||eql(nxt,"day")){
				time_t cur=time(0); tm *curT=localtime(&cur);
				int y=curT->tm_year+1900,m=curT->tm_mon+1,d=curT->tm_mday;
				dat=query(y,m,d,y,m,d);
			}else if(eql(nxt,"week")){
				time_t cur=time(0); tm *curT=localtime(&cur);
				int ey=curT->tm_year+1900,em=curT->tm_mon+1,ed=curT->tm_mday;
				cur=time(0)-86400u*7; curT=localtime(&cur);
				int by=curT->tm_year+1900,bm=curT->tm_mon+1,bd=curT->tm_mday;
				dat=query(by,bm,bd,ey,em,ed);
			}
			for(vector<int> vec:dat){
				cout<<"On "<<vec[0]<<'/'<<vec[1]<<'/'<<vec[2]<<": spent ";
				if( vec[3]/3600    ) cout<<( vec[3]/3600)<<"h ";
				if((vec[3]%3600)/60) cout<<((vec[3]%3600)/60)<<"m ";
				if( vec[3]%60      ) cout<<(vec[3]%60)<<"s "; cout<<endl;
				if((eql(0,"plot")||eql(0,"plt"))){
					for(int i=60/(1<<(2-pltmod));i>=1;i--){for(int j=4;j<28;j++){
						const char nmap[]{'#','.','-','='};	cout<<' ';
						if(vec[j]/60==0) cout<<' ';
						else if((vec[j]/60-1)/(1<<(2-pltmod))==i-1) cout<<nmap[(vec[j]/60)%4*(pltmod+1)];
						else if((vec[j]/60-1)/(1<<(2-pltmod))>i-1) cout<<"#"; else cout<<' ';
						cout<<' ';
					}puts("");}
					for(int j=1;j<=24;j++){cout<<"---";} cout<<endl;
				}
				if((eql(0,"plot")||eql(0,"plt"))||(eql(0,"detailed")||eql(0,"det"))||eql(0,"d")){
					for(int j=1;j<=24;j++){if(j<11)cout<<' ';cout<<j-1<<' ';} cout<<endl;
					for(int j=4;j< 28;j++){if(vec[j]/60<10) cout<<' '; cout<<vec[j]/60<<' ';} cout<<endl;
				}
			}
		}else if(eql(0,"recMode")||eql(0,"rmd")){ needpass //change recording mode
			if     (eql(1,"0")){ if(mode!=0){mode=0; writeSet();} }
			else if(eql(1,"1")){ if(mode!=1){mode=1; writeSet();} }
			else if(eql(1,"2")){ if(mode!=2){mode=2; writeSet();} }
			else if(eql(1,"3")){ if(mode!=3){mode=3; writeSet();} }
			else{ cout<<"recording using mode "<<mode<<endl; }
		}else if((eql(0,"forbid")||eql(0,"sfb"))&&cmds.size()>6){ needpass
			int nxt=1,flag=1; ForbidUnit cur;
			cur.bHour=stoi(cmds[nxt++]); cur.bMin=stoi(cmds[nxt++]); cur.bSec=stoi(cmds[nxt++]);
			cur.eHour=stoi(cmds[nxt++]); cur.eMin=stoi(cmds[nxt++]); cur.eSec=stoi(cmds[nxt++]);
			if(eql(nxt,"from")){ nxt++;
				cur.bYear=stoi(cmds[nxt++]); cur.bMonth=stoi(cmds[nxt++]); cur.bDay=stoi(cmds[nxt++]);
			}else{cur.bYear=1900; cur.bMonth=1; cur.bDay=1;}
			if(eql(nxt,"to")){ nxt++;
				cur.eYear=stoi(cmds[nxt++]); cur.eMonth=stoi(cmds[nxt++]); cur.eDay=stoi(cmds[nxt++]);
			}else{cur.eYear=9999; cur.eMonth=12; cur.eDay=31;}
			if(cmp(cur.eYear,cur.eMonth,cur.eDay,cur.bYear,cur.bMonth,cur.bDay)){
				puts("Date error, \'from\' after \'to\'"); flag=0;
			}
			if(flag && cmds.size()>nxt+1&&cmds[nxt]=="every"){ nxt++;
				if     (cmds[nxt]=="mon") cur.feq=1;
				else if(cmds[nxt]=="tue") cur.feq=2;
				else if(cmds[nxt]=="wed") cur.feq=4;
				else if(cmds[nxt]=="thu") cur.feq=8;
				else if(cmds[nxt]=="fri") cur.feq=16;
				else if(cmds[nxt]=="sat") cur.feq=32;
				else if(cmds[nxt]=="sun") cur.feq=64;
				else if(cmds[nxt]=="day") cur.feq=127;
				else if(cmds[nxt]=="weekday") cur.feq=31;
				else if(cmds[nxt]=="weekend") cur.feq=96;
				else if(isunum(cmds[nxt])) cur.feq=stoi(cmds[nxt]);
				else {puts("invalid frequency."); flag=0;} nxt++;
			} else cur.feq=127;
			wstring appName=L""; cur.appId=-1;
			if(flag && cmds.size()>nxt+1&&cmds[nxt]=="for"){ nxt++; appName=ctw(cmds[nxt++]); }
			for(int id=0;id<appMap.size();id++) if(appMap[id]==appName){ cur.appId=id;break; }
			if(cur.appId==-1){ cur.appId=appMap.size(); appMap.push_back(appName); }
			if(flag){
				fobs.push_back(cur); writeSet();
				cout<<"Success "<<int(cur.bYear)<<'/'<<int(cur.bMonth)<<'/'<<int(cur.bDay)<<"-"<<int(cur.eYear)<<'/'<<int(cur.eMonth)<<'/'<<int(cur.eDay)
					<<' '<<int(cur.bHour)<<':'<<int(cur.bMin)<<':'<<int(cur.bSec)<<"-"<<int(cur.eHour)<<':'<<int(cur.eMin)<<':'<<int(cur.eSec)<<' '
					<<"feq="<<bitset<7>(int(cur.feq))<<" for "<<(appMap[cur.appId].empty()?string("all apps"):wtc(appMap[cur.appId]))<<endl;
			}
		}else if(eql(0,"getforbid")||eql(0,"gfb")||((eql(0,"forbid")&&cmds.size()==1))){
			for(int i=0;i<fobs.size();i++){
				ForbidUnit cur=fobs[i];
				cout<<"Id="<<i<<": "<<int(cur.bYear)<<'/'<<int(cur.bMonth)<<'/'<<int(cur.bDay)<<"-"<<int(cur.eYear)<<'/'<<int(cur.eMonth)<<'/'<<int(cur.eDay)
					<<' '<<int(cur.bHour)<<':'<<int(cur.bMin)<<':'<<int(cur.bSec)<<"-"<<int(cur.eHour)<<':'<<int(cur.eMin)<<':'<<int(cur.eSec)<<' '
					<<"feq="<<bitset<7>(int(cur.feq))<<" for "<<(appMap[cur.appId].empty()?string("all apps"):wtc(appMap[cur.appId]))<<endl;
			}
			if(fobs.size()==0) puts("list is empty.");
		}else if(eql(0,"remforbid")||eql(0,"rfb")){ needpass
			vector<int> lst;
			for(int nxt=1;cmds.size()>nxt;nxt++) if(isunum(cmds[nxt])){
				lst.push_back(stoi(cmds[nxt]));
				if(lst.back()>=fobs.size()||lst.back()<0){
					lst.pop_back();
					if(cmds.size()==2) puts("invalid index.");
				}
			}
			if(lst.empty()){puts("nothing to remove.");}
			else{
				sort(lst.begin(),lst.end(),[](const int &a,const int &b){return a>b;});
				lst.erase(unique(lst.begin(),lst.end()),lst.end());
				for(int i:lst){ fobs.erase(fobs.begin()+i);}
				writeSet(); cout<<"Successfully removed "<<lst.size()<<" rules."<<endl;
			}
		}else if((eql(0,"tottime")||eql(0,"stt"))&&cmds.size()>3){ needpass
			int nxt=1,flag=1; TotTimeUnit cur;
			cur.Hour=stoi(cmds[nxt++]); cur.Min=stoi(cmds[nxt++]); cur.Sec=stoi(cmds[nxt++]);
			if(eql(nxt,"from")){ nxt++;
				cur.bYear=stoi(cmds[nxt++]); cur.bMonth=stoi(cmds[nxt++]); cur.bDay=stoi(cmds[nxt++]);
			}else{cur.bYear=1900; cur.bMonth=1; cur.bDay=1;}
			if(eql(nxt,"to")){ nxt++;
				cur.eYear=stoi(cmds[nxt++]); cur.eMonth=stoi(cmds[nxt++]); cur.eDay=stoi(cmds[nxt++]);
			}else{cur.eYear=9999; cur.eMonth=12; cur.eDay=31;}
			if(cmp(cur.eYear,cur.eMonth,cur.eDay,cur.bYear,cur.bMonth,cur.bDay)){
				puts("Date error, \'from\' after \'to\'"); flag=0;
			}
			if(flag && cmds.size()>nxt+1&&cmds[nxt]=="every"){ nxt++;
				if     (cmds[nxt]=="mon") cur.feq=1;
				else if(cmds[nxt]=="tue") cur.feq=2;
				else if(cmds[nxt]=="wed") cur.feq=4;
				else if(cmds[nxt]=="thu") cur.feq=8;
				else if(cmds[nxt]=="fri") cur.feq=16;
				else if(cmds[nxt]=="sat") cur.feq=32;
				else if(cmds[nxt]=="sun") cur.feq=64;
				else if(cmds[nxt]=="day") cur.feq=127;
				else if(cmds[nxt]=="weekday") cur.feq=31;
				else if(cmds[nxt]=="weekend") cur.feq=96;
				else if(isunum(cmds[nxt])) cur.feq=stoi(cmds[nxt]);
				else {puts("invalid frequency."); flag=0;} nxt++;
			} else cur.feq=127;
			wstring appName=L""; cur.appId=-1;
			if(flag && cmds.size()>nxt+1&&cmds[nxt]=="for"){ nxt++; appName=ctw(cmds[nxt++]); }
			for(int id=0;id<appMap.size();id++) if(appMap[id]==appName){ cur.appId=id;break; }
			if(cur.appId==-1){ cur.appId=appMap.size(); appMap.push_back(appName); }
			if(flag){
				ttts.push_back(cur); writeSet();
				cout<<"Success "<<int(cur.bYear)<<'/'<<int(cur.bMonth)<<'/'<<int(cur.bDay)<<"-"<<int(cur.eYear)<<'/'<<int(cur.eMonth)<<'/'<<int(cur.eDay)
					<<' '<<int(cur.Hour)<<':'<<int(cur.Min)<<':'<<int(cur.Sec)<<" feq="<<bitset<7>(int(cur.feq))<<" for "<<(appMap[cur.appId].empty()?string("all apps"):wtc(appMap[cur.appId]))<<endl;
			}
		}else if(eql(0,"gettottime")||eql(0,"gtt")||(eql(0,"tottime")&&cmds.size()==1)){
			for(int i=0;i<ttts.size();i++){
				TotTimeUnit cur=ttts[i];
				cout<<"Id="<<i<<": "<<int(cur.bYear)<<'/'<<int(cur.bMonth)<<'/'<<int(cur.bDay)<<"-"<<int(cur.eYear)<<'/'<<int(cur.eMonth)<<'/'<<int(cur.eDay)
					<<' '<<int(cur.Hour)<<':'<<int(cur.Min)<<':'<<int(cur.Sec)<<" feq="<<bitset<7>(int(cur.feq))<<" for "<<(appMap[cur.appId].empty()?string("all apps"):wtc(appMap[cur.appId]))<<endl;
			}
			if(ttts.size()==0) puts("list is empty.");
		}else if(eql(0,"remtottime")||eql(0,"rtt")){ needpass
			vector<int> lst;
			for(int nxt=1;cmds.size()>nxt;nxt++) if(isunum(cmds[nxt])){
				lst.push_back(stoi(cmds[nxt]));
				if(lst.back()>=ttts.size()||lst.back()<0){
					lst.pop_back();
					if(cmds.size()==2) puts("invalid index.");
				}
			}
			if(lst.empty()){puts("nothing to remove.");}
			else{
				sort(lst.begin(),lst.end(),[](const int &a,const int &b){return a>b;});
				lst.erase(unique(lst.begin(),lst.end()),lst.end());
				for(int i:lst){ ttts.erase(ttts.begin()+i);}
				writeSet(); cout<<"Successfully removed "<<lst.size()<<" rules."<<endl;
			}
		}else if((eql(0,"whitelist")||eql(0,"swl"))&&cmds.size()>1){ needpass
			wstring appName=ctw(cmds[1]); int appId=-1;
			for(int id=0;id<appMap.size();id++) if(appMap[id]==appName){ appId=id;break; }
			if(appId==-1){ appId=appMap.size(); appMap.push_back(appName); }
			wlss.push_back(appId); writeSet(); cout<<"Success "<<wtc(appMap[appId])<<endl;
		}else if(eql(0,"getwhitelist")||eql(0,"gwl")||(eql(0,"whitelist")&&cmds.size()==1)){
			for(int i=0;i<wlss.size();i++) cout<<"Id="<<i<<": "<<wtc(appMap[wlss[i]])<<endl;
			if(wlss.size()==0) puts("list is empty.");
		}else if(eql(0,"remwhitelist")||eql(0,"rwl")){ needpass
			vector<int> lst;
			for(int nxt=1;cmds.size()>nxt;nxt++) if(isunum(cmds[nxt])){
				lst.push_back(stoi(cmds[nxt]));
				if(lst.back()>=wlss.size()||lst.back()<0){
					lst.pop_back();
					if(cmds.size()==2) puts("invalid index.");
				}
			}
			if(lst.empty()){puts("nothing to remove.");}
			else{
				sort(lst.begin(),lst.end(),[](const int &a,const int &b){return a>b;});
				lst.erase(unique(lst.begin(),lst.end()),lst.end());
				for(int i:lst){ wlss.erase(wlss.begin()+i);}
				writeSet(); cout<<"Successfully removed "<<lst.size()<<" rules."<<endl;
			}
		}else if(eql(0,"password")||eql(0,"pas")){ needpass
			password=cmds.size()>1?cmds[1]:string(""); writeSet();
		}else if(eql(0,"free")||eql(0,"fre")){ needpass
			nedpas=false;
		}else if(eql(0,"lock")||eql(0,"lck")){
			nedpas=true;
		}else if((eql(0,"show")&&eql(1,"window"))||eql(0,"swd")){ needpass
			ShowWindow(FindWindow(0,"time-monitor-author=ZJC-ver1.0"),SW_SHOWNORMAL);
		}else if((eql(0,"hide")&&eql(1,"window"))||eql(0,"hwd")){ needpass
			ShowWindow(FindWindow(0,"time-monitor-author=ZJC-ver1.0"),SW_HIDE);
		}else puts("invalid input.");
	}
	return 0;
}
