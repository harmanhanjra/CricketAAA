// Cricket AAA — Full Realistic C++ Game (Win32 GDI)
// YOU vs BOT — separate scoreboards, 2 innings x 2 overs, bot bowl/bat, realistic stadium
// Build (MSVC): cl /EHsc /O2 /DUNICODE main.cpp user32.lib gdi32.lib gdiplus.lib
// Build (MinGW): g++ -O2 -mwindows -std=c++17 main.cpp -lgdi32 -luser32 -lgdiplus -o CricketCpp.exe
// Run: CricketCpp.exe
#define UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#pragma comment(lib,"gdiplus.lib")
using namespace Gdiplus;

struct Team{ int score=0, wickets=0, balls=0; std::vector<std::string> overBalls; std::string oversStr(int demoOvers) const { char b[32]; sprintf(b,"%d.%d / %d.0", balls/6, balls%6, demoOvers); return b;} std::string ballsStr() const { char b[16]; sprintf(b,"%d.%d", balls/6,balls%6); return b; } };
struct BallS{ float x,y,z,vx,vy,vz,spin,swing; bool bounced=false, wasHit=false, bouncedAfterHit=false, boundary=false, live=false; };
struct Fielder{ float x,y,bx,by,speed; };

static Team youTeam, botTeam;
static int innings=1, demoOvers=2, maxWickets=5;
static std::string battingTeam="you", phase="awaiting", gameMode="both", botDiff="medium";
static BallS* ball=nullptr;
static std::vector<PointF> trail;
static float aimLine=0, aimLength=0.48f, pace01=0.78f; int swingAmt=0, spinAmt=0; std::string lastTiming="—";
static bool runInProgress=false, botHasActed=false;
static std::vector<Fielder> fielders;
static std::vector<std::string> logLines;
static std::string msgText; int msgTicks=0;
static float fieldCX=640, fieldCY=380, PitchW=140, PitchH=420, BatX, BatY, BowlX, BowlY, BoundaryR=340;
static POINT mousePos{640,380};
static std::mt19937 rng{std::random_device{}()};
static HWND hWnd;
static UINT_PTR botTimerId=1, runTimerId=2;
static std::string camMode="broadcast";

Team* Active(){ return battingTeam=="you"? &youTeam: &botTeam; }
Team* Inactive(){ return battingTeam=="you"? &botTeam: &youTeam; }
bool IsBotBatting(){ return battingTeam=="bot"; }
bool IsBotBowling(){ return battingTeam=="you"; }
std::string LegalStr(Team* t=nullptr){ Team* s=t? t: Active(); char b[16]; sprintf(b,"%d.%d", s->balls/6, s->balls%6); return b; }
std::string LengthLabel(){ if(aimLength<0.22) return "YORKER"; if(aimLength<0.38) return "FULL"; if(aimLength<0.62) return "GOOD"; if(aimLength<0.82) return "SHORT"; return "BOUNCER"; }
void Log(const std::string& m){ logLines.insert(logLines.begin(), LegalStr()+" — "+m); if(logLines.size()>22) logLines.pop_back(); }
void ShowMsg(const std::string& t){ msgText=t; msgTicks=90; }
std::string GetInitialBatter(){ return gameMode=="bowl"? "bot":"you"; }

void SpawnFielders(){
    fielders={
        {fieldCX+180, fieldCY-40, fieldCX+180, fieldCY-40, 2.7f},
        {fieldCX-220, fieldCY-210, fieldCX-220, fieldCY-210, 3.0f},
        {fieldCX+260, fieldCY+170, fieldCX+260, fieldCY+170, 2.7f},
        {fieldCX, fieldCY-260, fieldCX, fieldCY-260, 2.5f},
        {fieldCX+320, 380+80, fieldCX+320, 380+80, 2.6f}
    };
    fielders[4].y = (380+PitchH/2+80); fielders[4].by = fielders[4].y;
}

void ResetBall();
void EndInnings();
void CheckAfterBall();

void CreateBall(){
    float lineOff=aimLine*110.f;
    float sx=BowlX+lineOff*0.28f, sy=BowlY-20, sz=22;
    float speedPx=3.6f+pace01*7.4f;
    float vx=(lineOff*0.02f)+swingAmt*0.06f, vy=speedPx*0.95f, vz=-(aimLength*2.2f+1.5f)+(float)( (rand()%100)/100.0 -0.5)*0.3f;
    if(ball) delete ball;
    ball=new BallS(); ball->x=sx; ball->y=sy; ball->z=sz; ball->vx=vx+(float)(rand()%100)/500.f-0.1f; ball->vy=vy; ball->vz=vz; ball->spin=spinAmt*0.04f; ball->swing=swingAmt*0.02f; ball->live=true;
    trail.clear(); phase="live";
    Log("Delivery — "+LengthLabel()+" "+(aimLine>0.4?"outside off":aimLine<-0.4?"down leg":"middle")+" • "+std::to_string((int)(pace01*100))+"%");
}
void HandleShot(bool isAgg){
    if(!ball||!ball->live) return;
    float dz=fabs(ball->y-BatY), dx=fabs(ball->x-BatX);
    bool inWin= dz<95 && dx<85 && ball->y> (fieldCY-PitchH/2-30);
    std::string timing="MISS"; float dir=aimLine*0.9f;
    if(inWin){ if(dz<28) timing="PERFECT"; else if(dz<55) timing="GOOD"; else timing="LATE"; }
    else if(ball->y> BatY+35) timing="TOO LATE"; else timing="TOO EARLY";
    lastTiming=timing;
    if(timing=="MISS"||timing=="TOO EARLY"||timing=="TOO LATE"){
        if(fabs(aimLine)>0.6 && (rand()%100)<16){ // bowled
            Team* s=Active(); s->wickets++; s->overBalls.push_back("W"); s->balls++; if(s->overBalls.size()>=6) s->overBalls.clear();
            delete ball; ball=nullptr; phase="dead"; ShowMsg("WICKET!"); Log(std::string("WICKET — ")+(battingTeam=="you"?"YOU":"BOT")+" — Bowled! Missed line"); CheckAfterBall(); return;
        }
        if(timing=="TOO EARLY"||timing=="TOO LATE"){ ball->wasHit=true; ball->vx+=dir*2.2f; ball->vy*=-0.35f; ball->vz=1.2f; ShowMsg(timing); Log(timing+" — defensive block"); }
        else { ShowMsg("BEATEN!"); Log("Beaten — dot ball"); }
        return;
    }
    float base=isAgg?8.5f:5.2f, mul= timing=="PERFECT"?1.15f: timing=="GOOD"?0.95f:0.72f;
    float speed=base*mul;
    float edge= timing=="GOOD"? (dir>0?0.6f:-0.6f): timing=="LATE"? (dir>0?1.3f:-1.3f):0;
    float ang= dir*0.95f + edge*0.12f + spinAmt*0.002f;
    ball->wasHit=true;
    ball->vx= sin(ang)*speed*1.8f + aimLine*1.1f;
    ball->vy= cos(ang)*speed*(-1) - (isAgg?4.5f:2.8f);
    ball->vz= isAgg? 4.5f+(rand()%12)/10.f:1.2f;
    if(isAgg&&timing=="PERFECT") ball->vz+=1.8f;
    ball->bouncedAfterHit=false; ball->boundary=false; ball->vx+= spinAmt*0.02f;
    ShowMsg(isAgg? (timing=="PERFECT"?"LOFTED!":"BIG SHOT"):(timing=="PERFECT"?"TIMED!":"WORKED"));
    Log(std::string(isAgg?"Aggressive":"Controlled")+" shot — "+timing+" (dir "+std::to_string((int)(dir*100))+")");
}
void WicketFallen(const std::string& reason){
    Team* s=Active(); s->wickets++; s->overBalls.push_back("W"); s->balls++; if(s->overBalls.size()>=6) s->overBalls.clear();
    if(ball){ delete ball; ball=nullptr; } phase="dead"; ShowMsg("WICKET!"); Log(std::string("WICKET — ")+(battingTeam=="you"?"YOU":"BOT")+" — "+reason); CheckAfterBall();
}
void FinishDelivery(int runs,const std::string& label){
    Team* s=Active(); s->score+=runs; s->overBalls.push_back(runs==0?"•":std::to_string(runs)); s->balls++; if(s->overBalls.size()>=6) s->overBalls.clear();
    if(ball) ball->live=false; phase="dead";
    Log((battingTeam=="you"?"YOU ":"BOT ")+label + (runs? " — "+std::to_string(runs):""));
    if(label.find("FOUR")!=std::string::npos) ShowMsg("FOUR!"); else if(label.find("SIX")!=std::string::npos) ShowMsg("SIX!"); else if(runs) ShowMsg(std::to_string(runs)+" RUN"); else ShowMsg("DOT");
    CheckAfterBall();
}
void CheckAfterBall(){
    Team* active=Active();
    if(innings==2){ int target=Inactive()->score+1; if(active->score>=target){ Log((battingTeam=="you"?"YOU":"BOT")+std::string(" chased ")+std::to_string(target-1)+"!"); EndInnings(); return; } }
    if(active->balls>=demoOvers*6 || active->wickets>=maxWickets){ SetTimer(hWnd, 99, 700, nullptr); }
    else { SetTimer(hWnd, 98, 700, nullptr); }
}
void EndInnings(){
    Team* fin=battingTeam=="you"? &youTeam: &botTeam;
    Log("END INN "+std::to_string(innings)+" — "+(battingTeam=="you"?"YOU ":"BOT ")+std::to_string(fin->score)+"/"+std::to_string(fin->wickets)+" in "+LegalStr(fin));
    if(innings==1){ innings=2; battingTeam=battingTeam=="you"?"bot":"you"; if(ball){ delete ball; ball=nullptr; } trail.clear(); phase="awaiting"; botHasActed=false; ShowMsg(std::string("INN 2 — ")+(battingTeam=="you"?"YOU":"BOT")+" NEED "+std::to_string(fin->score+1)); Log(std::string("2ND INN — ")+(battingTeam=="you"?"YOU":"BOT")+" need "+std::to_string(fin->score+1)+" to win"); if(IsBotBowling()) SetTimer(hWnd, botTimerId, 900, nullptr); InvalidateRect(hWnd,nullptr,FALSE); }
    else { phase="matchOver"; bool tie=youTeam.score==botTeam.score, youWon=youTeam.score>botTeam.score; std::string winner=tie?"TIE": youWon?"YOU WIN":"BOT WINS"; Log("MATCH OVER — "+winner+" — YOU "+std::to_string(youTeam.score)+"/"+std::to_string(youTeam.wickets)+" vs BOT "+std::to_string(botTeam.score)+"/"+std::to_string(botTeam.wickets)); ShowMsg(tie?"TIE!": youWon?"YOU WIN!":"BOT WINS!"); InvalidateRect(hWnd,nullptr,FALSE); }
}
void ResetBall(){
    if(ball){ delete ball; ball=nullptr; } trail.clear(); runInProgress=false; botHasActed=false; if(phase!="matchOver") phase="awaiting";
    KillTimer(hWnd, botTimerId);
    Team* a=Active();
    if(phase=="awaiting" && IsBotBowling() && a->balls<demoOvers*6 && a->wickets<maxWickets) SetTimer(hWnd, botTimerId, 800+rand()%450, nullptr);
    InvalidateRect(hWnd,nullptr,FALSE);
}
void RestartMatch(){
    youTeam=Team(); botTeam=Team(); innings=1; battingTeam=GetInitialBatter(); phase="awaiting"; if(ball){ delete ball; ball=nullptr; } trail.clear(); runInProgress=false; botHasActed=false; logLines.clear();
    Log("Match started — 2 overs/innings • "+std::string(battingTeam=="you"?"YOU":"BOT")+" to bat first");
    ResetBall(); SpawnFielders(); InvalidateRect(hWnd,nullptr,FALSE);
}
void BotBowl(){
    float line,length,pace,swing,spin;
    if(botDiff=="easy"){ line=(float)(rand()%100)/100.f-0.5f; length=0.35f+(rand()%40)/100.f; pace=0.55f+(rand()%25)/100.f; swing=(rand()%30)-15; spin=(rand()%30)-15; }
    else if(botDiff=="medium"){ float r=(rand()%100)/100.f; if(r<0.35) length=0.52f+(rand()%15)/100.f; else if(r<0.60) length=0.22f+(rand()%14)/100.f; else length=0.65f+(rand()%22)/100.f; line=(float)(rand()%130)/100.f-0.65f; pace=0.68f+(rand()%22)/100.f; swing=(rand()%70)-35; spin=(rand()%70)-35; }
    else { float r=(rand()%100)/100.f; if(r<0.30) length=0.50f+(rand()%10)/100.f; else if(r<0.55) length=0.18f+(rand()%8)/100.f; else if(r<0.80) length=0.70f+(rand()%12)/100.f; else length=0.88f+(rand()%10)/100.f; line=(float)(rand()%160)/100.f-0.8f; pace=0.75f+(rand()%22)/100.f; swing=(rand()%110)-55; spin=(rand()%90)-45; }
    aimLine=std::max(-1.f,std::min(1.f,line)); aimLength=std::max(0.f,std::min(1.f,length)); pace01=std::max(0.35f,std::min(1.f,pace)); swingAmt=(int)std::max(-100.f,std::min(100.f,swing)); spinAmt=(int)std::max(-100.f,std::min(100.f,spin));
    CreateBall(); Log("BOT bowls — "+LengthLabel()+" "+(aimLine>0.4?"outside off":aimLine<-0.4?"down leg":"middle")+" • "+std::to_string((int)(pace01*100))+"%");
}
void BotThinkShot(){
    if(!ball||!ball->live||botHasActed) return;
    float dz=fabs(ball->y-BatY), dx=fabs(ball->x-BatX);
    if(!(dz<95 && dx<92 && ball->y> fieldCY-PitchH/2-30)) return;
    bool willPlay, isAgg; float dir;
    float lat=fabs(ball->x-BatX); bool isWide=lat>70, isYorker=aimLength<0.28, isBouncer=aimLength>0.78;
    double r=(rand()%100)/100.0;
    if(botDiff=="easy"){ willPlay=r<0.72; isAgg=r<0.18; dir=(float)(rand()%60)/100.f-0.3f; }
    else if(botDiff=="medium"){ willPlay= isWide? r<0.55: isBouncer? r<0.68: r<0.84; isAgg= isYorker? r<0.12: r<0.32; dir=(float)(rand()%105)/100.f-0.525f + aimLine*0.30f; }
    else { willPlay= isWide? r<0.42: r<0.91; isAgg=r<0.42; float gaps[5]={-0.95f,-0.55f,0,0.55f,0.95f}; dir= (rand()%100<60)? gaps[rand()%5]*0.9f : (float)(rand()%130)/100.f-0.65f; }
    if(!willPlay){ botHasActed=true; Log("BOT leaves — shouldered arms"); return; }
    botHasActed=true; aimLine=dir;
    HandleShot(isAgg);
}
void TryRun(){
    if(IsBotBatting()&&IsBotBowling()){ ShowMsg("BOT vs BOT — AI will run"); return; }
    if(phase!="live"||!ball||!ball->wasHit){ ShowMsg("No ball to run!"); return; }
    if(runInProgress) return; runInProgress=true; Log("Run called — racing field!"); ShowMsg("RUN!");
    // immediate check simplified
    float d=1e9; for(auto &f: fielders) d=std::min(d, (float)sqrt((f.x-ball->x)*(f.x-ball->x)+(f.y-ball->y)*(f.y-ball->y)));
    if(d<72){ if((rand()%100)<17){ WicketFallen("Run out!"); runInProgress=false; } else { Log("Beaten by field — no run"); ShowMsg("NO RUN"); runInProgress=false; } }
    else { FinishDelivery(1,"1 run — single"); runInProgress=false; }
}
void TickPhysics(){
    if(ball && ball->live){
        if(IsBotBatting() && !botHasActed) BotThinkShot();
        ball->vz-=0.115f;
        ball->vx*=0.996f; ball->vy*=0.996f; ball->vz*=0.998f;
        ball->vx+= ball->spin*0.04f + ball->swing*0.02f;
        ball->x+= ball->vx; ball->y+= ball->vy; ball->z+= ball->vz;
        trail.push_back(PointF(ball->x, ball->y)); if(trail.size()>24) trail.erase(trail.begin());
        if(!ball->bounced && ball->z<=0 && ball->y> fieldCY-PitchH/2-12 && ball->y< fieldCY+PitchH/2+12){ ball->z=0; ball->vz=fabs(ball->vz)*0.58f; ball->vx*=0.92f; ball->vy*=0.92f; ball->vx+= ball->spin*0.048f; ball->bounced=true; if(ball->wasHit) ball->bouncedAfterHit=true; }
        if(ball->z<0){ ball->z=0; ball->vz*=-0.32f; if(fabs(ball->vz)<0.6) ball->vz=0; }
        if(!ball->wasHit && !ball->bouncedAfterHit && fabs(ball->x-BatX)<14 && fabs(ball->y-BatY)<10 && ball->z<9){ WicketFallen("Bowled — hit stumps"); return; }
        float dist=sqrt((ball->x-fieldCX)*(ball->x-fieldCX)+(ball->y-fieldCY)*(ball->y-fieldCY));
        if(dist>BoundaryR-12){ ball->boundary=true; if(ball->wasHit){ if(ball->bouncedAfterHit||ball->z<2.5f) FinishDelivery(4,"FOUR — beats field"); else FinishDelivery(6,"SIX — over rope!"); } else { ball->live=false; phase="dead"; Log("Ball to boundary — dot"); SetTimer(hWnd, 97, 700, nullptr); } ball->live=false; return; }
        if(!ball->wasHit && fabs(ball->vy)<0.22f && ball->z<=0.5f){ FinishDelivery(0,"Dot ball"); return; }
        if(ball->wasHit && ball->live){ float sp=sqrt(ball->vx*ball->vx+ball->vy*ball->vy); if(sp<0.9f && ball->z<=0.8f){ float nearest=1e9; for(auto &f: fielders) nearest=std::min(nearest,(float)sqrt((f.x-ball->x)*(f.x-ball->x)+(f.y-ball->y)*(f.y-ball->y))); if(nearest>110) FinishDelivery(runInProgress?1:0, runInProgress?"1 run":"Stopped — dot"); else FinishDelivery(0,"Fielded in ring — dot"); runInProgress=false; ball->live=false; } }
    }
    if(ball && ball->live && ball->wasHit){ for(auto &f: fielders){ float dx=ball->x-f.x, dy=ball->y-f.y, d=sqrt(dx*dx+dy*dy); if(d<400){ float sp=f.speed*(ball->boundary?1.05f:1.f); if(d>1){ f.x+=dx/d*sp; f.y+=dy/d*sp; } if(d<18){ ball->live=false; phase="dead"; if(!ball->boundary){ if(runInProgress){ int cr= ball->z>3.5f?0:1; if(cr) FinishDelivery(1,"Quick single"); else FinishDelivery(0,"Fielded — dot"); runInProgress=false; } else FinishDelivery(0,"Fielded — dot"); } return; } } } }
    else { for(auto &f: fielders){ f.x+=(f.bx-f.x)*0.04f; f.y+=(f.by-f.y)*0.04f; } }
    if(msgTicks>0) msgTicks--;
}

// ---- Rendering helpers (GDI+) ----
void FillRoundedRect(Graphics& g, Brush* br, Rect r, int rad){
    GraphicsPath p; int d=rad*2; p.AddArc(r.X,r.Y,d,d,180,90); p.AddArc(r.GetRight()-d,r.Y,d,d,270,90); p.AddArc(r.GetRight()-d,r.GetBottom()-d,d,d,0,90); p.AddArc(r.X,r.GetBottom()-d,d,d,90,90); p.CloseFigure(); g.FillPath(br,&p);
}
void DrawRoundedRect(Graphics& g, Pen* pen, Rect r, int rad){
    GraphicsPath p; int d=rad*2; p.AddArc(r.X,r.Y,d,d,180,90); p.AddArc(r.GetRight()-d,r.Y,d,d,270,90); p.AddArc(r.GetRight()-d,r.GetBottom()-d,d,d,0,90); p.AddArc(r.X,r.GetBottom()-d,d,d,90,90); p.CloseFigure(); g.DrawPath(pen,&p);
}

void DrawHUD(Graphics& g, int W, int H){
    int x=12,y=10,w=370;
    int h= 148 + (innings==2?28:0);
    Rect rect(x,y,w,h);
    SolidBrush bg(Color(230,5,18,36)); FillRoundedRect(g,&bg,rect,12);
    Pen pn(Color(38,65,99),1); DrawRoundedRect(g,&pn,rect,12);
    FontFamily ff(L"Segoe UI"); Font fTitle(&ff,8,FontStyleBold,UnitPoint);
    SolidBrush gold(Color(255,201,40)); g.DrawString(L"SCOREBOARD",-1,&fTitle, PointF(x+10,y+6), &gold);
    std::string tag= phase=="matchOver"? "MATCH OVER": innings==1?"1ST INNINGS":"2ND INNINGS";
    Font fSmall(&ff,8,FontStyleRegular,UnitPoint);
    RectF sz; g.MeasureString(std::wstring(tag.begin(),tag.end()).c_str(),-1,&fSmall, PointF(0,0), &sz);
    Rect tr(x+w-(int)sz.Width-18, y+4, (int)sz.Width+12,16); SolidBrush trBg(Color(40,255,201,40)); FillRoundedRect(g,&trBg,tr,8);
    g.DrawString(std::wstring(tag.begin(),tag.end()).c_str(),-1,&fSmall, PointF(x+w-sz.Width-12,y+6), &gold);
    bool ya=battingTeam=="you"&&phase!="matchOver", ba=battingTeam=="bot"&&phase!="matchOver";
    Rect r1(x+8,y+24,w-16,52);
    SolidBrush b1(ya? Color(40,255,201,40): Color(15,255,255,255)); FillRoundedRect(g,&b1,r1,10);
    if(ya){ Pen gp(Color(255,201,40),1.2f); DrawRoundedRect(g,&gp,r1,10); }
    SolidBrush yb(Color(255,201,40)); g.FillEllipse(&yb, r1.X+8, r1.Y+11, 28,28);
    Font fSm(&ff,8,FontStyleRegular,UnitPoint); SolidBrush blk(Color(26,17,0)); g.DrawString(L"YOU",-1,&fSm, PointF(r1.X+6,r1.Y+19), &blk);
    SolidBrush wht(Color(255,255,255)); Font fBig(&ff,11,FontStyleBold,UnitPoint);
    char s1[64]; sprintf(s1,"%d / %d", youTeam.score, youTeam.wickets); g.DrawString(std::wstring(s1,s1+strlen(s1)).c_str(),-1,&fBig, PointF(r1.X+42,r1.Y+18), &wht);
    SolidBrush dim(Color(155,182,209)); char o1[32]; sprintf(o1,"%s ov", youTeam.oversStr(demoOvers).c_str()); g.DrawString(std::wstring(o1,o1+strlen(o1)).c_str(),-1,&fSm, PointF(r1.X+120,r1.Y+24), &dim);
    std::string ov1="This over: "+(youTeam.overBalls.empty()?"—": [&]{std::string s; for(size_t i=0;i<youTeam.overBalls.size();++i){ if(i) s+=" • "; s+=youTeam.overBalls[i]; } return s; }()); g.DrawString(std::wstring(ov1.begin(),ov1.end()).c_str(),-1,&fSm, PointF(r1.X+42,r1.Y+38), &dim);
    if(ya){ Rect sr(r1.GetRight()-(int)sz.Width-14, r1.Y+12, (int)sz.Width+8,14); SolidBrush sb(Color(255,201,40)); FillRoundedRect(g,&sb,sr,7); g.DrawString(L"STRIKE",-1,&fSm, PointF(sr.X+4,sr.Y+1), &blk); }
    Rect r2(x+8,y+82,w-16,52);
    SolidBrush b2(ba? Color(40,255,201,40): Color(15,255,255,255)); FillRoundedRect(g,&b2,r2,10);
    if(ba){ Pen gp(Color(255,201,40),1.2f); DrawRoundedRect(g,&gp,r2,10); }
    SolidBrush bb(Color(30,58,138)); g.FillEllipse(&bb, r2.X+8, r2.Y+11, 28,28); SolidBrush wb(Color(255,255,255)); g.DrawString(L"BOT",-1,&fSm, PointF(r2.X+6,r2.Y+19), &wb);
    char s2[64]; sprintf(s2,"%d / %d", botTeam.score, botTeam.wickets); g.DrawString(std::wstring(s2,s2+strlen(s2)).c_str(),-1,&fBig, PointF(r2.X+42,r2.Y+18), &wht);
    char o2[32]; sprintf(o2,"%s ov", botTeam.oversStr(demoOvers).c_str()); g.DrawString(std::wstring(o2,o2+strlen(o2)).c_str(),-1,&fSm, PointF(r2.X+120,r2.Y+24), &dim);
    std::string ov2="This over: "+(botTeam.overBalls.empty()?"—": [&]{std::string s; for(size_t i=0;i<botTeam.overBalls.size();++i){ if(i) s+=" • "; s+=botTeam.overBalls[i]; } return s; }()); g.DrawString(std::wstring(ov2.begin(),ov2.end()).c_str(),-1,&fSm, PointF(r2.X+42,r2.Y+38), &dim);
    if(innings==2 && phase!="matchOver"){
        int target=Inactive()->score+1, need=target-Active()->score, left=demoOvers*6-Active()->balls;
        char ttxt[96]; if(need<=0) sprintf(ttxt,"%s WON — chased %d!", battingTeam=="you"?"YOU":"BOT", target-1); else sprintf(ttxt,"TARGET %d • NEED %d in %d balls • %d wkts left", target, need, left, maxWickets-Active()->wickets);
        Rect tr2(x+8,y+140,w-16,22); SolidBrush tb(need<=0? Color(50,76,175,80): Color(40,255,201,40)); FillRoundedRect(g,&tb,tr2,7);
        g.DrawString(std::wstring(ttxt,ttxt+strlen(ttxt)).c_str(),-1,&fSm, PointF(tr2.X+6,tr2.Y+5), &wht);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
    case WM_CREATE: hWnd=hwnd; BatX=fieldCX; BatY=fieldCY+PitchH/2-18; BowlX=fieldCX; BowlY=fieldCY-PitchH/2+18; SpawnFielders(); RestartMatch(); SetTimer(hwnd, 10, 16, nullptr); break;
    case WM_SIZE: fieldCX=LOWORD(lParam)/2.f; fieldCY=HIWORD(lParam)*0.52f; BatX=fieldCX; BatY=fieldCY+PitchH/2-18; BowlX=fieldCX; BowlY=fieldCY-PitchH/2+18; InvalidateRect(hwnd,nullptr,FALSE); break;
    case WM_MOUSEMOVE: mousePos.x=GET_X_LPARAM(lParam); mousePos.y=GET_Y_LPARAM(lParam); if(phase=="awaiting" && !IsBotBowling()){ aimLine=std::max(-1.f,std::min(1.f,(mousePos.x-fieldCX)/150.f)); float rel=(mousePos.y-(fieldCY-PitchH/2))/PitchH; aimLength=std::max(0.f,std::min(1.f,rel)); InvalidateRect(hwnd,nullptr,FALSE); } break;
    case WM_LBUTTONDOWN: if(phase=="live"&&ball&&!IsBotBatting()) HandleShot(false); break;
    case WM_RBUTTONDOWN: if(phase=="live"&&ball&&!IsBotBatting()) HandleShot(true); break;
    case WM_KEYDOWN:
        if(wParam==VK_SPACE){ if(phase=="awaiting"&&!IsBotBowling()) CreateBall(); else if(phase=="dead") ResetBall(); }
        else if(wParam=='R'){ TryRun(); }
        else if(wParam==VK_TAB){ camMode= camMode=="broadcast"?"bowler": camMode=="bowler"?"batter":"broadcast"; InvalidateRect(hwnd,nullptr,FALSE); }
        else if(wParam==VK_BACK) ResetBall();
        else if(wParam=='1') { gameMode="both"; RestartMatch(); }
        else if(wParam=='2') { gameMode="bat"; RestartMatch(); }
        else if(wParam=='3') { gameMode="bowl"; RestartMatch(); }
        else if(wParam=='4') { gameMode="bots"; RestartMatch(); }
        break;
    case WM_TIMER:
        if(wParam==10){ TickPhysics(); InvalidateRect(hwnd,nullptr,FALSE); }
        else if(wParam==botTimerId){ KillTimer(hwnd,botTimerId); if(phase=="awaiting") BotBowl(); }
        else if(wParam==98){ KillTimer(hwnd,98); if(phase=="dead") ResetBall(); }
        else if(wParam==99){ KillTimer(hwnd,99); EndInnings(); }
        else if(wParam==97){ KillTimer(hwnd,97); ResetBall(); }
        break;
    case WM_PAINT:{
        PAINTSTRUCT ps; HDC hdc=BeginPaint(hwnd,&ps);
        RECT rc; GetClientRect(hwnd,&rc); int W=rc.right, H=rc.bottom;
        HDC mem=CreateCompatibleDC(hdc); HBITMAP bmp=CreateCompatibleBitmap(hdc,W,H); SelectObject(mem,bmp);
        Graphics g(mem); g.SetSmoothingMode(SmoothingModeAntiAlias); g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        // bg
        LinearGradientBrush bgGrad(Point(0,0), Point(0,H), Color(4,10,20), Color(12,30,60)); g.FillRectangle(&bgGrad, 0,0,W,H);
        // field ellipse
        SolidBrush fieldBr(Color(14,77,14)); g.FillEllipse(&fieldBr, (int)(fieldCX-BoundaryR),(int)(fieldCY-BoundaryR),(int)BoundaryR*2,(int)BoundaryR*2);
        Pen fieldPen(Color(180,255,255,255),2); g.DrawEllipse(&fieldPen, (int)(fieldCX-BoundaryR),(int)(fieldCY-BoundaryR),(int)BoundaryR*2,(int)BoundaryR*2);
        // pitch
        SolidBrush pitchBr(Color(200,170,122)); g.FillRectangle(&pitchBr, (int)(fieldCX-PitchW/2),(int)(fieldCY-PitchH/2),(int)PitchW,(int)PitchH);
        SolidBrush pitch2(Color(217,193,154)); g.FillRectangle(&pitch2, (int)(fieldCX-PitchW/2+14),(int)(fieldCY-PitchH/2),(int)PitchW-28,(int)PitchH);
        Pen pitchPen(Color(220,255,255,255),1.2f); g.DrawRectangle(&pitchPen, (int)(fieldCX-PitchW/2),(int)(fieldCY-PitchH/2),(int)PitchW,(int)PitchH);
        Pen creasePen(Color(255,255,255),1.5f); g.DrawLine(&creasePen, (int)(fieldCX-PitchW/2-12),(int)(fieldCY-PitchH/2+28),(int)(fieldCX+PitchW/2+12),(int)(fieldCY-PitchH/2+28));
        g.DrawLine(&creasePen, (int)(fieldCX-PitchW/2-12),(int)(fieldCY+PitchH/2-28),(int)(fieldCX+PitchW/2+12),(int)(fieldCY+PitchH/2-28));
        // stumps
        SolidBrush stumpBr(Color(232,220,192)); for(int k=0;k<2;k++){ float sx=BatX, sy=k==0?BatY:BowlY; for(int i=-1;i<=1;i++) g.FillRectangle(&stumpBr, (int)(sx+i*8-2),(int)(sy-7),4,14); }
        // fielders
        SolidBrush fBr(Color(15,42,75)); Pen fp(Color(74,122,184),1.5f); FontFamily ff(L"Segoe UI"); Font fs(&ff,8,FontStyleRegular,UnitPoint);
        for(auto &f: fielders){ g.FillEllipse(&fBr, (int)(f.x-11),(int)(f.y-11),22,22); g.DrawEllipse(&fp,(int)(f.x-11),(int)(f.y-11),22,22); SolidBrush wb(Color(255,255,255)); g.DrawString(L"F",-1,&fs, PointF(f.x-5,f.y-7), &wb); }
        // bowler/batter
        SolidBrush bowlBr(Color(30,58,138)); g.FillEllipse(&bowlBr,(int)(BowlX-14),(int)(BowlY-14),28,28);
        if(IsBotBowling()){ Pen gp(Color(255,201,40),2.5f); gp.SetDashStyle(DashStyleDash); g.DrawEllipse(&gp,(int)(BowlX-14),(int)(BowlY-14),28,28); }
        SolidBrush yb(Color(255,201,40)); g.FillEllipse(&yb,(int)(BatX-14),(int)(BatY-14),28,28);
        Pen batPen(IsBotBatting()? Color(30,64,175): Color(122,86,0), 2.2f); g.DrawEllipse(&batPen,(int)(BatX-14),(int)(BatY-14),28,28);
        SolidBrush batBr(Color(107,58,10)); g.FillRectangle(&batBr,(int)(BatX-3),(int)(BatY-22),6,22);
        // aiming
        if(phase=="awaiting"){ float tx=fieldCX+aimLine*110, ty=fieldCY-PitchH/2+aimLength*PitchH; Pen ap(Color(255,201,40),2); ap.SetDashStyle(DashStyleDash); g.DrawLine(&ap, BowlX,BowlY,tx,ty); SolidBrush ab(Color(255,201,40)); g.FillEllipse(&ab, (int)(tx-6),(int)(ty-6),12,12); }
        // trail
        if(trail.size()>1){ Pen tp(Color(90,255,255,255),2); for(size_t i=1;i<trail.size();++i) g.DrawLine(&tp, trail[i-1], trail[i]); }
        // ball shadow + ball
        if(ball){
            float shR=std::max(4.f, 11-ball->z*0.35f); SolidBrush sh(Color(80,0,0,0)); g.FillEllipse(&sh, (int)(ball->x-shR/2),(int)(ball->y+6-ball->z*0.12f-shR/3),(int)shR,(int)(shR*0.6f));
            bool isWin= phase=="live" && ball->y> BatY-110 && ball->y< BatY+40 && !ball->wasHit;
            float r=std::max(7.f, 9-ball->z*0.06f); SolidBrush bb(isWin? Color(255,201,40): Color(197,40,40)); g.FillEllipse(&bb, (int)(ball->x-r),(int)(ball->y-ball->z*0.9f-r),(int)r*2,(int)r*2);
            Pen bp(Color(255,255,255),1.6f); g.DrawEllipse(&bp,(int)(ball->x-r),(int)(ball->y-ball->z*0.9f-r),(int)r*2,(int)r*2);
        }
        // HUD
        DrawHUD(g,W,H);
        // msg
        if(msgTicks>0 && !msgText.empty()){
            Font fm(&ff,9,FontStyleBold,UnitPoint); RectF sz; g.MeasureString(std::wstring(msgText.begin(),msgText.end()).c_str(),-1,&fm, PointF(0,0), &sz);
            int mx=(W-(int)sz.Width)/2-10, my=18; SolidBrush mb(Color(255,201,40)); Rect mr(mx,my,(int)sz.Width+20,22); FillRoundedRect(g,&mb,mr,11);
            SolidBrush tb(Color(26,17,0)); g.DrawString(std::wstring(msgText.begin(),msgText.end()).c_str(),-1,&fm, PointF(mx+10,my+3), &tb);
        }
        // top bar
        SolidBrush barBg(Color(8,26,51)); g.FillRectangle(&barBg, 0,0,W,34);
        Font fBar(&ff,8,FontStyleRegular,UnitPoint); SolidBrush barTx(Color(155,182,209));
        std::string bar="CRICKET AAA — C++ REALISTIC  |  1:BOTH 2:BAT 3:BOWL 4:BOTS  |  SPACE bowl  L/R shot  R run  TAB cam  BKSP reset";
        g.DrawString(std::wstring(bar.begin(),bar.end()).c_str(),-1,&fBar, PointF(8,10), &barTx);
        // controls mini
        // blit
        BitBlt(hdc,0,0,W,H,mem,0,0,SRCCOPY);
        DeleteObject(bmp); DeleteDC(mem);
        EndPaint(hwnd,&ps); break;
    }
    case WM_DESTROY: PostQuitMessage(0); break;
    default: return DefWindowProc(hwnd,msg,wParam,lParam);
    }
    return 0;
}
int WINAPI wWinMain(HINSTANCE hInst,HINSTANCE,LPWSTR,int nCmdShow){
    GdiplusStartupInput si; ULONG_PTR tok; GdiplusStartup(&tok,&si,nullptr);
    WNDCLASS wc={0}; wc.lpfnWndProc=WndProc; wc.hInstance=hInst; wc.lpszClassName=L"CricketCpp"; wc.hCursor=LoadCursor(nullptr,IDC_ARROW); wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    RegisterClass(&wc);
    HWND hwnd=CreateWindow(L"CricketCpp", L"Cricket AAA — C++ Realistic (YOU vs BOT separate boards) — 1:BOTH 2:BAT 3:BOWL 4:BOTS", WS_OVERLAPPEDWINDOW, 100,60,1280,760, nullptr,nullptr,hInst,nullptr);
    ShowWindow(hwnd,nCmdShow); UpdateWindow(hwnd);
    MSG m; while(GetMessage(&m,nullptr,0,0)){ TranslateMessage(&m); DispatchMessage(&m); }
    GdiplusShutdown(tok); return 0;
}
