static_assert(_WIN32,"your platform isn't accomodated yet");
#include<algorithm>
#include<windows.h>
#include<fstream>
#include<vector>
#include<string>
#include<thread>
#include<queue>
#include<ctime>
#include<map>
#include<set>
using namespace std;

const int GRAB_INTERVAL=1000; //ms
const int REST_INTERVAL=100; //s

const std::string path=std::string(getenv("AppData"))+"\\time-monitor"; // Path for the data
const std::string datPath=std::string(getenv("AppData"))+"\\time-monitor\\data.log"; // Path for logs from all times
const std::string todPath=std::string(getenv("AppData"))+"\\time-monitor\\temp.log"; // Path for logs today(detailed)
const std::string setPath=std::string(getenv("AppData"))+"\\time-monitor\\setts"; // Path for settings

struct Aca{
	set<int> id; Aca *fail;
	map<wchar_t,Aca*> nxt;
	Aca(Aca *f){fail=f;id.clear();nxt.clear();}
}; vector<Aca*> pool; Aca* rt;
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
vector<__int32> appTime;
vector<bool> appAllow;
vector<bool> appRstr;

__int16 tih[24];
tm endT,begT; int sumT=0;
__int8 mode=3,cah=-1;

int countSec(const tm &beg,const tm &end){ // count seconds from beg to end (if in the same day)
	if(beg.tm_year!=end.tm_year||beg.tm_yday!=end.tm_yday) return 0; // not in same day
	return (end.tm_hour*3600 + end.tm_min*60 + end.tm_sec) - (beg.tm_hour*3600 + beg.tm_min*60 + beg.tm_sec);
}
void updateTIH(const tm &beg,const tm &end){ // update tih : from beg to end (if in the same day), user used the computer
	if(beg.tm_year!=end.tm_year||beg.tm_yday!=end.tm_yday) return; // not in same day
	if(beg.tm_hour==end.tm_hour){
		tih[beg.tm_hour]+=(end.tm_min*60+end.tm_sec)-(beg.tm_min*60+beg.tm_sec);
	}else{
		tih[beg.tm_hour]+=3600-(beg.tm_min*60+beg.tm_sec);
		for(int i=beg.tm_hour+1;i<end.tm_hour;i++) tih[i]=3600;
		tih[end.tm_hour]+=(end.tm_min*60+end.tm_sec);
	}
}
void savToData(tm cur){ // bury the past into data, prepare for a brand new day
	std::ofstream file(datPath,std::ios::binary|std::ios::app);
	DWORD tTt=sumT+countSec(begT,endT);
	__int16 u=endT.tm_year+1900; file.write(reinterpret_cast<char*>(&u),sizeof(u));
	__int8 v=endT.tm_mon+1; file.write(reinterpret_cast<char*>(&v),sizeof(v));
	v=endT.tm_mday; file.write(reinterpret_cast<char*>(&v),sizeof(v));
	
	file.write(reinterpret_cast<char*>(&tTt),sizeof(tTt));
	file.write(reinterpret_cast<char*>(tih),sizeof(tih)); file.close();
	begT=endT=cur; sumT=0; memset(tih,0,sizeof(tih));
}
void savToCache(){ // when quited, save the state
	std::ofstream file(todPath,std::ios::binary);
	__int16 tihc[24];int sumtc=sumT+countSec(begT,endT);
	for(int i=0;i<24;i++) tihc[i]=tih[i]; updateTIH(begT,endT);
	file.write(reinterpret_cast<char*>(&begT),sizeof(begT));
	file.write(reinterpret_cast<char*>(&endT),sizeof(endT));
	file.write(reinterpret_cast<char*>(&sumtc),sizeof(sumtc));
	file.write(reinterpret_cast<char*>(tih),sizeof(tih));
	for(int i=0;i<24;i++) tih[i]=tihc[i];
	int szapmp=appMap.size(),len;
	file.write(reinterpret_cast<char*>(&szapmp),sizeof(szapmp));
	for(wstring u:appMap){
		int len=WideCharToMultiByte(CP_ACP,0,u.data(),u.length(),NULL,0,NULL,NULL);
		char *data=new char[len]; WideCharToMultiByte(CP_ACP,0,u.data(),u.length(),data,len,NULL,NULL);
		file.write(reinterpret_cast<char*>(&len),sizeof(len));
		file.write(data,len); delete[] data;
	}
	for(__int32 u:appTime) file.write(reinterpret_cast<char*>(&u),sizeof(u));
	file.close();
}
void loadCache(){ // when started, resume to the state when last quited
	time_t cur=time(0);tm *curT=localtime(&cur); endT=begT=*curT;
	std::ifstream file(todPath,std::ios::binary); if(file){
		file.read(reinterpret_cast<char*>(&begT),sizeof(begT));
		file.read(reinterpret_cast<char*>(&endT),sizeof(endT));
		file.read(reinterpret_cast<char*>(&sumT),sizeof(sumT));
		file.read(reinterpret_cast<char*>(tih),sizeof(tih));
		if(curT->tm_year!=endT.tm_year||curT->tm_yday!=endT.tm_yday){
			updateTIH(begT,endT); savToData(*curT);
		}

		appMap.clear(); appTime.clear();
		int szapmp; file.read(reinterpret_cast<char*>(&szapmp),sizeof(szapmp));
		for(int i=0;i<szapmp;i++){
			int len; file.read(reinterpret_cast<char*>(&len),sizeof(len));
			char *data=new char[len]; file.read(data,len);
			wchar_t *res=new wchar_t[len];
			int n=(size_t)MultiByteToWideChar(CP_ACP,0,data,len,NULL,0);
			MultiByteToWideChar(CP_ACP,0,data,len,res,n); res[n]=0;
			wstring str(res); appMap.push_back(str);
			delete[] data; delete[] res;
		}
		for(int i=0;i<szapmp;i++){
			__int32 cur; file.read(reinterpret_cast<char*>(&cur),sizeof(cur));
			appTime.push_back(cur);
		} file.close();
	}
}
void loadSet(){ // get & change settings
	// cache appTime
	map<wstring,int> cmap;
	for(int i=0;i<appMap.size();i++) cmap.insert(make_pair(appMap[i],appTime[i]));
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
			wchar_t *res=new wchar_t[len];
			int n=(size_t)MultiByteToWideChar(CP_ACP,0,data,len,NULL,0);
			MultiByteToWideChar(CP_ACP,0,data,len,res,n); res[n]=0;
			wstring str(res); appMap.push_back(str);
			delete[] data; delete[] res;
		} file.close();
	}
	// build ACAM
	for(unsigned int i=0;i<pool.size();i++) delete pool[i];
	pool.clear(); rt=new Aca(nullptr);rt->fail=rt; pool.push_back(rt);
	for(int i=0;i<appMap.size();i++){
		wstring str=appMap[i];
		Aca *u=rt; for(wchar_t c:str){
			if(u->nxt.count(c)==0){
				u->nxt.insert(make_pair(c,new Aca(rt)));
				pool.push_back(u->nxt[c]);
			} u=u->nxt[c];
		} u->id.insert(i);
	}
	queue<Aca*> q;
	for(auto p:rt->nxt){ p.second->fail=rt; q.push(p.second); }
	while(!q.empty()){
		Aca *u=q.front(); q.pop();
		for(auto p:u->nxt)if(p.second){
			p.second->fail=u->fail->nxt.count(p.first)?u->fail->nxt[p.first]:rt;
			q.push(p.second);
		}
		for(auto p:u->fail->nxt)if(u->nxt.count(p.first)==0)
			u->nxt[p.first]=p.second;
		u->id.insert(u->fail->id.begin(),u->fail->id.end());
	}
	// reload appTime
	appTime.clear();
	for(int i=0;i<appMap.size();i++)
		appTime.push_back(cmap.count(appMap[i])?cmap[appMap[i]]:0);
	appAllow.resize(appTime.size(),false);
	appRstr.resize(appTime.size(),false);
}
struct LoadRestrictPara{int id;HWND topw;};
LoadRestrictPara restrictPara;
DWORD WINAPI loadRestrict(PVOID p){
	const wstring title=L"time is up";
	LoadRestrictPara u=*(LoadRestrictPara*)p;
	wstring str=appMap[u.id];
	if(appAllow[u.id]) return 0;
	int res=MessageBoxW(u.topw,(L"根据你设定的对 "+str+L" 的使用规则\n你现在不能使用它了，我来帮你关掉\n你可以拒绝，但只能换来一分钟时间保存状态").data(),title.data(),MB_YESNO|MB_ICONSTOP|MB_TOPMOST);
	if(res==IDYES) SendMessage(u.topw,WM_CLOSE,0,0);
	else{
		if(appRstr[u.id]){
			SendMessage(u.topw,WM_CLOSE,0,0);
			const wstring text=L"给过你一分钟了";
			MessageBoxW(0,text.data(),title.data(),MB_OK|MB_ICONSTOP|MB_TOPMOST);
		}else{
			appAllow[u.id]=1; Sleep(60000); appAllow[u.id]=0; appRstr[u.id]=1;
			SendMessage(u.topw,WM_CLOSE,0,0);
		}
	}
	return 0;
}
bool KMoperated=false;
void mySleep(){ // sleeping , but still activated.
	#define SLEEP_INTERVAL 50
	clock_t t0=clock();
	while(clock()-t0<=GRAB_INTERVAL*CLOCKS_PER_SEC/1000){
		loadSet();
		if((mode&1)&&(!KMoperated)) for(int i=1;i<0xff;i++) if(GetAsyncKeyState(i)&0x8000){KMoperated=true;break;}
		Sleep(SLEEP_INTERVAL);
	}
	savToCache();
	#undef SLEEP_INTERVAL
}
const DWORD iSize=GetSystemMetrics(SM_CXSCREEN)*GetSystemMetrics(SM_CYSCREEN)*4;
BYTE* scrGrab(){ // grab current screen (raw data), need to delete the returning pointer manually
	RECT rect; SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
	int wid=rect.right-rect.left, hgt=rect.bottom-rect.top;
	HDC src=GetDC(0),hdc=CreateCompatibleDC(src);
	HBITMAP hbmp=CreateCompatibleBitmap(src,wid,hgt);
	SelectObject(hdc,hbmp);
	BitBlt(hdc,rect.left,rect.top,wid,hgt,src,0,0,SRCCOPY);
	BITMAP bmp; GetObject(hbmp,sizeof(BITMAP),&bmp);
	BITMAPINFOHEADER bih={
		sizeof(BITMAPINFOHEADER), bmp.bmWidth, bmp.bmHeight,
		1, bmp.bmBitsPixel, BI_RGB, iSize, 0, 0, 0, 0
	};
	BYTE* p=new BYTE[iSize];
	GetDIBits(hdc,hbmp,0,hgt,p,(LPBITMAPINFO)&bih,DIB_RGB_COLORS);
	DeleteObject(hbmp); DeleteDC(hdc); ReleaseDC(0,src);
	return p;
}
BYTE *befF,*curF; POINT befM,curM;
void UpdateStat(){ // update status
	if(mode&1){befM=curM;GetCursorPos(&curM);}
	if(mode&2){delete[] befF;befF=curF;curF=scrGrab();}
}
bool isOperated(){ // judge if the computer is operated
	if(mode&1){
		if(KMoperated){KMoperated=false;return true;} // if any key is pressed, you might be using your computer
		if(befM.x!=curM.x||befM.y!=curM.y)return true; // if the cursor moved, you might be using your computer
	}if(mode&2){
		if(!befF) return false; // true or false anyway
		return memcmp(befF,curF,iSize); // if the screen changed, you might be using your computer
	} return false;
}
void mainloop(){ // the main loop
	time_t cur=time(0); tm *curT=localtime(&cur);
	if(curT->tm_year!=endT.tm_year||curT->tm_yday!=endT.tm_yday){
		updateTIH(begT,endT); savToData(*curT); savToCache();
	}
	UpdateStat(); if(isOperated()){
		if(countSec(endT,*curT)>=REST_INTERVAL){
			updateTIH(begT,endT); sumT+=countSec(begT,endT);
			begT=endT=*curT;
		}
		// check restricted page
		HWND hwnd=GetForegroundWindow();
		char name[256];wchar_t wname[256]; GetWindowTextA(hwnd,name,256);
		int n=(size_t)MultiByteToWideChar(CP_ACP,0,name,strlen(name),NULL,0);
		MultiByteToWideChar(CP_ACP,0,name,strlen(name),wname,n);
		set<int> ieds;
		Aca *u=rt; for(int i=0;i<n;i++){
			wchar_t c=wname[i]; u=u->nxt.count(c)?u->nxt[c]:rt;
			ieds.insert(u->id.begin(),u->id.end());
		}
		#define compDate(aY,aM,aD,bY,bM,bD) ((aY)<(bY)||((aY)==(bY)&&((aM)<(bM)||((aM)==(bM)&&(aD)<=(bD)))))
		int secs=countSec(endT,*curT); for(int i:ieds) appTime[i]+=secs; // update appTime
		int flag=-1; // judge block
		for(ForbidUnit u:fobs){ // check fob
			if(ieds.count(u.appId)
				&& ((1<<curT->tm_wday)&u.feq)
				&& compDate(u.bYear,u.bMonth,u.bDay,curT->tm_year+1900,curT->tm_mon+1,curT->tm_mday)
				&& compDate(curT->tm_year+1900,curT->tm_mon+1,curT->tm_mday,u.eYear,u.eMonth,u.eDay)
				&&((compDate(u.bHour,u.bMin,u.bSec,u.eHour,u.eMin,u.eSec)
				&& compDate(u.bHour,u.bMin,u.bSec,curT->tm_hour,curT->tm_min,curT->tm_sec)
				&& compDate(curT->tm_hour,curT->tm_min,curT->tm_sec,u.eHour,u.eMin,u.eSec))
				||((!compDate(u.bHour,u.bMin,u.bSec,u.eHour,u.eMin,u.eSec))
				&&(compDate(u.bHour,u.bMin,u.bSec,curT->tm_hour,curT->tm_min,curT->tm_sec)
				|| compDate(curT->tm_hour,curT->tm_min,curT->tm_sec,u.eHour,u.eMin,u.eSec))))
			) flag=u.appId;
		}
		for(TotTimeUnit u:ttts){ // check ttt
			if(ieds.count(u.appId)
				&& ((1<<curT->tm_wday)&u.feq)
				&& compDate(u.bYear,u.bMonth,u.bDay,curT->tm_year+1900,curT->tm_mon+1,curT->tm_mday)
				&& compDate(curT->tm_year+1900,curT->tm_mon+1,curT->tm_mday,u.eYear,u.eMonth,u.eDay)
				&& appTime[u.appId]>(int(u.Hour)*3600+int(u.Min)*60+int(u.Sec))
			) flag=u.appId;
		}
		for(__int32 u:wlss) if(ieds.count(u)) flag=-1; // check whitelist
		if(flag!=-1&&strcmp(name,"time is up")!=0&&strcmp(name,"time-monitor-author=ZJC-ver1.0")!=0){
			restrictPara.id=flag; restrictPara.topw=hwnd;
			CreateThread(NULL,0,loadRestrict,PVOID(&restrictPara),0,NULL);
		}
		endT=*curT;
	}
	mySleep();
}
int main(){
	// hide window
	if(SetConsoleTitle("time-monitor-author=ZJC-ver1.0")){
		Sleep(100); ShowWindow(FindWindow(0,"time-monitor-author=ZJC-ver1.0"),SW_SHOWNORMAL);
		Sleep(100); ShowWindow(FindWindow(0,"time-monitor-author=ZJC-ver1.0"),SW_HIDE);
	} else ShowWindow(GetForegroundWindow(),SW_HIDE);
	// ensure path
	system(("mkdir \""+std::string(path)+"\"").c_str());
	// main operation
	loadCache(); loadSet();
	atexit(savToCache);
	while(true) mainloop();
	return 0;
}
