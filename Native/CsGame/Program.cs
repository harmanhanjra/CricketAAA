// Cricket AAA — Full Realistic C# Game (WinForms GDI+)
// YOU vs BOT — separate scoreboards, 2 innings x 2 overs, bot bowl/bat, realistic stadium
// Build: csc.exe /target:winexe /reference:System.Windows.Forms.dll,System.Drawing.dll Program.cs
// Run: Program.exe
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;

class Team
{
    public int Score, Wickets, Balls;
    public List<string> OverBalls = new List<string>();
    public string OversStr(int demoOvers) { return string.Format("{0}.{1} / {0}.{2}", Balls/6, Balls%6, demoOvers); }
    public string BallsStr() { return string.Format("{0}.{1}", Balls/6, Balls%6); }
}

class BallState
{
    public float X,Y,Z,Vx,Vy,Vz,Spin,Swing;
    public bool Bounced, WasHit, BouncedAfterHit, Boundary, Live;
}

class Fielder { public float X,Y,BaseX,BaseY,Speed; }

class CricketForm : Form
{
    const int DemoOvers=2, MaxWickets=5;
    Team you=new Team(), bot=new Team();
    int innings=1; string battingTeam="you"; string phase="awaiting"; // awaiting|live|dead|matchOver
    BallState ball; List<PointF> trail=new List<PointF>();
    float aimLine=0, aimLength=0.48f, pace01=0.78f; int swingAmt=0, spinAmt=0; string lastTiming="—";
    bool runInProgress=false; string gameMode="both", botDiff="medium"; bool botHasActed=false;
    Timer botTimer; Timer runTimer;
    List<Fielder> fielders=new List<Fielder>();
    Timer gameTimer; Random rng=new Random();
    Point mousePos; bool mouseInside=false;
    Dictionary<string,PointF> pitch=new Dictionary<string,PointF>();
    int W=1280,H=720;

    // camera
    string camMode="broadcast";
    Font fontSmall=new Font("Segoe UI",8), fontMid=new Font("Segoe UI",9,FontStyle.Bold), fontBig=new Font("Segoe UI",14,FontStyle.Bold), fontMono=new Font("Consolas",9);
    List<string> logLines=new List<string>();
    float BatX, BatY, BowlX, BowlY, PitchCX, PitchCY, PitchW=140, PitchH=420, BoundaryR=340;
    float fieldCX, fieldCY;

    public CricketForm()
    {
        Text="Cricket AAA — Full Realistic C# (YOU vs BOT separate boards)";
        ClientSize=new Size(1280,760);
        DoubleBuffered=true; SetStyle(ControlStyles.AllPaintingInWmPaint|ControlStyles.UserPaint|ControlStyles.OptimizedDoubleBuffer,true);
        KeyPreview=true;
        fieldCX=ClientSize.Width/2f; fieldCY=ClientSize.Height*0.52f;
        PitchCX=fieldCX; PitchCY=fieldCY;
        BatX=PitchCX; BatY=PitchCY+PitchH/2-18; BowlX=PitchCX; BowlY=PitchCY-PitchH/2+18;

        gameTimer=new Timer{Interval=16}; gameTimer.Tick+= (s,e)=>{ TickPhysics(); Invalidate(); }; gameTimer.Start();
        botTimer=new Timer{Interval=850}; botTimer.Tick+= (s,e)=>{ botTimer.Stop(); if(phase=="awaiting" && IsBotBowling()) BotBowl(); };
        runTimer=new Timer{Interval=820};

        MouseMove+= (s,e)=>{ mousePos=e.Location; mouseInside=true; if(phase=="awaiting" && !IsBotBowling()){ aimLine=Math.Max(-1,Math.Min(1,(e.X-fieldCX)/150f)); float rel=(e.Y-(PitchCY-PitchH/2))/PitchH; aimLength=Math.Max(0,Math.Min(1,rel)); } Invalidate(); };
        MouseDown+= (s,e)=>{
            if(phase!="live"||ball==null) return;
            if(IsBotBatting()) return;
            HandleShot(e.Button==MouseButtons.Right);
        };
        KeyDown+= OnKey;
        Shown+= (s,e)=>{ RestartMatch(); Focus(); };
        SpawnFielders();
    }

    void OnKey(object s, KeyEventArgs e)
    {
        if(e.KeyCode==Keys.Space){ if(phase=="awaiting" && !IsBotBowling()) CreateBall(); else if(phase=="dead") ResetBall(); e.Handled=true; }
        else if(e.KeyCode==Keys.R){ TryRun(); }
        else if(e.KeyCode==Keys.Tab){ var order=new[]{"broadcast","bowler","batter"}; int idx=Array.IndexOf(order,camMode); camMode=order[(idx+1)%order.Length]; e.Handled=true; }
        else if(e.KeyCode==Keys.Back) ResetBall();
        else if(e.KeyCode==Keys.D1) SetMode("both");
        else if(e.KeyCode==Keys.D2) SetMode("bat");
        else if(e.KeyCode==Keys.D3) SetMode("bowl");
        else if(e.KeyCode==Keys.D4) SetMode("bots");
    }

    Team Active { get { return battingTeam=="you"? you: bot; } }
    Team Inactive { get { return battingTeam=="you"? bot: you; } }
    bool IsBotBatting(){ return battingTeam=="bot"; }
    bool IsBotBowling(){ return battingTeam=="you"; }
    string LegalStr(Team t) { var s=t==null? Active: t; return string.Format("{0}.{1}", s.Balls/6, s.Balls%6); }

    void Log(string msg, Color? c=null)
    {
        logLines.Insert(0, string.Format("{0} — {1}", LegalStr(null), msg));
        if(logLines.Count>22) logLines.RemoveAt(logLines.Count-1);
    }
    void ShowMsg(string t)
    {
        // drawn in OnPaint via overlay timer
        msgText=t; msgTicks=90; // ~1.5s at 60fps
    }
    string msgText=""; int msgTicks=0;

    void SetMode(string m)
    {
        gameMode=m;
        if(phase=="matchOver" || (you.Balls==0 && bot.Balls==0)) RestartMatch();
        else { botHasActed=false; Log("Mode -> "+m); if(phase=="awaiting" && IsBotBowling()) { botTimer.Interval=700; botTimer.Start(); } Invalidate(); }
    }

    string GetInitialBatter(){ return gameMode=="bowl"? "bot":"you"; }

    void RestartMatch()
    {
        you=new Team(); bot=new Team(); innings=1; battingTeam=GetInitialBatter(); phase="awaiting"; ball=null; trail.Clear(); runInProgress=false; botHasActed=false; logLines.Clear();
        Log(string.Format("Match started — 2 overs/innings • {0} to bat first", battingTeam=="you"?"YOU":"BOT"));
        ResetBall(); SpawnFielders(); Invalidate();
    }
    void SpawnFielders()
    {
        fielders=new List<Fielder>{
            new Fielder{X=fieldCX+180,Y=PitchCY-40,BaseX=fieldCX+180,BaseY=PitchCY-40,Speed=2.7f},
            new Fielder{X=fieldCX-220,Y=fieldCY-210,BaseX=fieldCX-220,BaseY=fieldCY-210,Speed=3.0f},
            new Fielder{X=fieldCX+260,Y=fieldCY+170,BaseX=fieldCX+260,BaseY=fieldCY+170,Speed=2.7f},
            new Fielder{X=fieldCX,Y=fieldCY-260,BaseX=fieldCX,BaseY=fieldCY-260,Speed=2.5f},
            new Fielder{X=fieldCX+320,Y=PitchCY+80,BaseX=fieldCX+320,BaseY=PitchCY+80,Speed=2.6f},
        };
    }
    void ResetBall()
    {
        ball=null; trail.Clear(); runInProgress=false; botHasActed=false; if(phase!="matchOver") phase="awaiting";
        botTimer.Stop();
        var a=Active;
        if(phase=="awaiting" && IsBotBowling() && a.Balls<DemoOvers*6 && a.Wickets<MaxWickets){ botTimer.Interval=800+rng.Next(450); botTimer.Start(); }
        Invalidate();
    }
    void CreateBall()
    {
        float lineOff=aimLine*110f;
        float startX=BowlX+lineOff*0.28f, startY=BowlY-20f, startZ=22f;
        float speedPx=3.6f+pace01*7.4f;
        float vx=(lineOff*0.02f)+swingAmt*0.06f, vy=speedPx*0.95f, vz= -(aimLength*2.2f+1.5f) + (float)(rng.NextDouble()-0.5)*0.3f;
        ball=new BallState{X=startX,Y=startY,Z=startZ,Vx=vx+(float)(rng.NextDouble()-0.5)*0.2f,Vy=vy,Vz=vz,Spin=spinAmt*0.04f,Swing=swingAmt*0.02f,Bounced=false,WasHit=false,BouncedAfterHit=false,Boundary=false,Live=true};
        phase="live"; Log(string.Format("Delivery — {0} {1} • {2}%", LengthLabel(), aimLine>0.4?"outside off":aimLine<-0.4?"down leg":"middle", (int)(pace01*100)));
    }
    string LengthLabel(){ if(aimLength<0.22) return "YORKER"; if(aimLength<0.38) return "FULL"; if(aimLength<0.62) return "GOOD"; if(aimLength<0.82) return "SHORT"; return "BOUNCER"; }

    void HandleShot(bool isAgg)
    {
        if(ball==null||!ball.Live) return;
        float dz=Math.Abs(ball.Y-BatY), dx=Math.Abs(ball.X-BatX);
        bool inWindow= dz<95 && dx<85 && ball.Y> PitchCY-30;
        string timing="MISS"; float dir=aimLine*0.9f;
        if(inWindow){ if(dz<28) timing="PERFECT"; else if(dz<55) timing="GOOD"; else timing="LATE"; }
        else if(ball.Y> BatY+35) timing="TOO LATE"; else timing="TOO EARLY";
        lastTiming=timing;
        if(timing=="MISS"||timing=="TOO EARLY"||timing=="TOO LATE"){
            if(Math.Abs(aimLine)>0.6 && rng.NextDouble()<0.16){ WicketFallen("Bowled! Missed the line"); return; }
            if(timing=="TOO EARLY"||timing=="TOO LATE"){ ball.WasHit=true; ball.Vx+=dir*2.2f; ball.Vy*=-0.35f; ball.Vz=1.2f; ShowMsg(timing); Log(timing+" — defensive block"); }
            else { ShowMsg("BEATEN!"); Log("Beaten — dot ball"); }
            return;
        }
        float baseSp=isAgg?8.5f:5.2f, mul= timing=="PERFECT"?1.15f: timing=="GOOD"?0.95f:0.72f;
        float speed=baseSp*mul;
        float edge= timing=="GOOD"? Math.Sign(dir)*0.6f: timing=="LATE"? Math.Sign(dir)*1.3f:0;
        float ang= dir*0.95f + edge*0.12f + spinAmt*0.002f;
        ball.WasHit=true;
        ball.Vx= (float)Math.Sin(ang)*speed*1.8f + aimLine*1.1f;
        ball.Vy= (float)Math.Cos(ang)*speed*(-1) - (isAgg?4.5f:2.8f);
        ball.Vz= isAgg? 4.5f+(float)rng.NextDouble()*1.2f:1.2f;
        if(isAgg&&timing=="PERFECT") ball.Vz+=1.8f;
        ball.BouncedAfterHit=false; ball.Boundary=false; ball.Vx+= spinAmt*0.02f;
        ShowMsg(isAgg? (timing=="PERFECT"?"LOFTED!":"BIG SHOT"):(timing=="PERFECT"?"TIMED!":"WORKED"));
        Log(string.Format("{0} shot — {1} (dir {2})", isAgg?"Aggressive":"Controlled", timing, (int)(dir*100)));
    }
    void WicketFallen(string reason)
    {
        var s=Active; s.Wickets++; s.OverBalls.Add("W"); s.Balls++; if(s.OverBalls.Count>=6) s.OverBalls.Clear();
        ball=null; phase="dead"; ShowMsg("WICKET!"); Log(string.Format("WICKET — {0} — {1}", battingTeam=="you"?"YOU":"BOT", reason));
        CheckAfterBall(true);
    }
    void FinishDelivery(int runs,string label,string cls)
    {
        var s=Active; s.Score+=runs; s.OverBalls.Add(runs==0?"•":runs.ToString()); s.Balls++; if(s.OverBalls.Count>=6) s.OverBalls.Clear();
        if(ball!=null) ball.Live=false; phase="dead";
        Log(string.Format("{0} {1}", battingTeam=="you"?"YOU":"BOT", label)+(runs>0? string.Format(" — {0}",runs):""));
        if(label.Contains("FOUR")) ShowMsg("FOUR!"); else if(label.Contains("SIX")) ShowMsg("SIX!"); else if(runs>0) ShowMsg(runs+" RUN"); else ShowMsg("DOT");
        CheckAfterBall(false);
    }
    void CheckAfterBall(bool wasWicket)
    {
        var active=Active;
        if(innings==2){ int target=Inactive.Score+1; if(active.Score>=target){ Log(string.Format("{0} chased {1}!", battingTeam=="you"?"YOU":"BOT", target-1)); EndInnings(); return; } }
        if(active.Balls>= DemoOvers*6 || active.Wickets>=MaxWickets){ var t=new Timer{Interval=700}; t.Tick+=(s,e)=>{ t.Stop(); t.Dispose(); EndInnings(); }; t.Start(); }
        else { var t=new Timer{Interval=700}; t.Tick+=(s,e)=>{ t.Stop(); t.Dispose(); if(phase=="dead") ResetBall(); }; t.Start(); }
    }
    void EndInnings()
    {
        var fin=battingTeam=="you"? you: bot;
        Log(string.Format("END INN {0} — {1} {2}/{3} in {4}", innings, battingTeam=="you"?"YOU":"BOT", fin.Score, fin.Wickets, LegalStr(fin)));
        if(innings==1){ innings=2; battingTeam=battingTeam=="you"?"bot":"you"; ball=null; trail.Clear(); phase="awaiting"; botHasActed=false; ShowMsg(string.Format("INN 2 — {0} NEED {1}", battingTeam=="you"?"YOU":"BOT", fin.Score+1)); Log(string.Format("2ND INN — {0} need {1} to win", battingTeam=="you"?"YOU":"BOT", fin.Score+1)); if(IsBotBowling()){ botTimer.Interval=900; botTimer.Start(); } }
        else { phase="matchOver"; bool tie=you.Score==bot.Score, youWon=you.Score>bot.Score; string winner=tie?"TIE": youWon?"YOU WIN":"BOT WINS"; Log(string.Format("MATCH OVER — {0} — YOU {1}/{2} vs BOT {3}/{4}", winner, you.Score, you.Wickets, bot.Score, bot.Wickets)); ShowMsg(tie?"TIE!": youWon?"YOU WIN!":"BOT WINS!"); }
    }
    void BotBowl()
    {
        float line,length,pace,swing,spin;
        if(botDiff=="easy"){ line=(float)(rng.NextDouble()-0.5)*1.0f; length=0.35f+(float)rng.NextDouble()*0.40f; pace=0.55f+(float)rng.NextDouble()*0.25f; swing=(float)(rng.NextDouble()-0.5)*30; spin=(float)(rng.NextDouble()-0.5)*30; }
        else if(botDiff=="medium"){ float r=(float)rng.NextDouble(); if(r<0.35) length=0.52f+(float)rng.NextDouble()*0.15f; else if(r<0.60) length=0.22f+(float)rng.NextDouble()*0.14f; else length=0.65f+(float)rng.NextDouble()*0.22f; line=(float)(rng.NextDouble()-0.5)*1.3f; pace=0.68f+(float)rng.NextDouble()*0.22f; swing=(float)(rng.NextDouble()-0.5)*70; spin=(float)(rng.NextDouble()-0.5)*70; }
        else { float r=(float)rng.NextDouble(); if(r<0.30) length=0.50f+(float)rng.NextDouble()*0.10f; else if(r<0.55) length=0.18f+(float)rng.NextDouble()*0.08f; else if(r<0.80) length=0.70f+(float)rng.NextDouble()*0.12f; else length=0.88f+(float)rng.NextDouble()*0.10f; line=(float)(rng.NextDouble()-0.5)*1.6f; pace=0.75f+(float)rng.NextDouble()*0.22f; swing=(float)(rng.NextDouble()-0.5)*110; spin=(float)(rng.NextDouble()-0.5)*90; }
        aimLine=Math.Max(-1,Math.Min(1,line)); aimLength=Math.Max(0,Math.Min(1,length)); pace01=Math.Max(0.35f,Math.Min(1,pace)); swingAmt=(int)Math.Max(-100,Math.Min(100,swing)); spinAmt=(int)Math.Max(-100,Math.Min(100,spin));
        CreateBall(); Log(string.Format("BOT bowls — {0} {1} • {2}%", LengthLabel(), aimLine>0.4?"outside off":aimLine<-0.4?"down leg":"middle", (int)(pace01*100)));
    }
    void BotThinkShot()
    {
        if(ball==null||!ball.Live||botHasActed) return;
        float dz=Math.Abs(ball.Y-BatY), dx=Math.Abs(ball.X-BatX);
        if(!(dz<95 && dx<92 && ball.Y> PitchCY-30)) return;
        bool willPlay, isAgg; float dir;
        float lat=Math.Abs(ball.X-BatX); bool isWide=lat>70, isYorker=aimLength<0.28, isBouncer=aimLength>0.78;
        double r=rng.NextDouble();
        if(botDiff=="easy"){ willPlay=r<0.72; isAgg=r<0.18; dir=(float)(rng.NextDouble()-0.5)*0.6f; }
        else if(botDiff=="medium"){ willPlay= isWide? r<0.55: isBouncer? r<0.68: r<0.84; isAgg= isYorker? r<0.12: r<0.32; dir=(float)(rng.NextDouble()-0.5)*1.05f + aimLine*0.30f; }
        else { willPlay= isWide? r<0.42: r<0.91; isAgg=r<0.42; float[] gaps={-0.95f,-0.55f,0,0.55f,0.95f}; dir= rng.NextDouble()<0.6? gaps[rng.Next(5)]*0.9f : (float)(rng.NextDouble()-0.5)*1.3f; }
        if(!willPlay){ botHasActed=true; Log("BOT leaves — shouldered arms"); return; }
        botHasActed=true; aimLine=dir;
        var t=new Timer{Interval= botDiff=="hard"?28: botDiff=="medium"?52:92 }; t.Tick+=(s,e)=>{ t.Stop(); t.Dispose(); HandleShot(isAgg); var t2=new Timer{Interval=420}; t2.Tick+=(s2,e2)=>{ t2.Stop(); t2.Dispose(); if(ball!=null&&ball.WasHit&&ball.Live&&!runInProgress){ float sp=(float)Math.Sqrt(ball.Vx*ball.Vx+ball.Vy*ball.Vy); if(sp>1.8f && !ball.Boundary && rng.NextDouble()<0.52) TriggerBotRun(); } }; t2.Start(); }; t.Start();
    }
    void TriggerBotRun(){ if(runInProgress||ball==null||!ball.WasHit) return; runInProgress=true; Log("BOT calls for a run!"); ShowMsg("BOT RUN!"); var t=new Timer{Interval=820}; t.Tick+=(s,e)=>{ t.Stop(); t.Dispose(); if(ball==null||!ball.Live){ runInProgress=false; return; } float d=float.MaxValue; foreach(var f in fielders) d=Math.Min(d, (float)Math.Sqrt((f.X-ball.X)*(f.X-ball.X)+(f.Y-ball.Y)*(f.Y-ball.Y))); if(d<72){ if(rng.NextDouble()<0.16) WicketFallen("Run out! Bot run out"); else { Log("BOT — beaten by field"); ShowMsg("NO RUN"); runInProgress=false; } } else { FinishDelivery(1,"BOT takes 1",""); runInProgress=false; } }; t.Start(); }
    void TryRun(){ if(IsBotBatting()&&IsBotBowling()){ ShowMsg("BOT vs BOT — AI will run"); return; } if(phase!="live"||ball==null||!ball.WasHit){ ShowMsg("No ball to run!"); return; } if(runInProgress) return; runInProgress=true; Log("Run called — racing field!"); ShowMsg("RUN!"); var t=new Timer{Interval=820}; t.Tick+=(s,e)=>{ t.Stop(); t.Dispose(); if(ball==null||!ball.Live){ runInProgress=false; return; } float d=float.MaxValue; foreach(var f in fielders) d=Math.Min(d,(float)Math.Sqrt((f.X-ball.X)*(f.X-ball.X)+(f.Y-ball.Y)*(f.Y-ball.Y))); if(d<72){ if(rng.NextDouble()<0.17) WicketFallen("Run out!"); else { Log("Beaten by field — no run"); ShowMsg("NO RUN"); runInProgress=false; } } else { FinishDelivery(1,"1 run — single",""); runInProgress=false; } }; t.Start(); }

    void TickPhysics()
    {
        if(ball!=null && ball.Live)
        {
            if(IsBotBatting() && !botHasActed) BotThinkShot();
            ball.Vz-=0.115f; // gravity on Z (height)
            ball.Vx*=0.996f; ball.Vy*=0.996f; ball.Vz*=0.998f;
            ball.Vx+= ball.Spin*0.04f + ball.Swing*0.02f;
            ball.X+= ball.Vx; ball.Y+= ball.Vy; ball.Z+= ball.Vz;
            trail.Add(new PointF(ball.X, ball.Y)); if(trail.Count>24) trail.RemoveAt(0);
            if(!ball.Bounced && ball.Z<=0 && ball.Y> PitchCY-PitchH/2-12 && ball.Y< PitchCY+PitchH/2+12){ ball.Z=0; ball.Vz=Math.Abs(ball.Vz)*0.58f; ball.Vx*=0.92f; ball.Vy*=0.92f; ball.Vx+= ball.Spin*0.048f; ball.Bounced=true; if(ball.WasHit) ball.BouncedAfterHit=true; }
            if(ball.Z<0){ ball.Z=0; ball.Vz*=-0.32f; if(Math.Abs(ball.Vz)<0.6) ball.Vz=0; }
            if(!ball.WasHit && !ball.BouncedAfterHit && Math.Abs(ball.X-BatX)<14 && Math.Abs(ball.Y-BatY)<10 && ball.Z<9){ WicketFallen("Bowled — hit the stumps"); return; }
            float dist=(float)Math.Sqrt((ball.X-fieldCX)*(ball.X-fieldCX)+(ball.Y-fieldCY)*(ball.Y-fieldCY));
            if(dist>BoundaryR-12){ ball.Boundary=true; if(ball.WasHit){ if(ball.BouncedAfterHit||ball.Z<2.5f) FinishDelivery(4,"FOUR — beats the field","b4"); else FinishDelivery(6,"SIX — over the rope!","b6"); } else { ball.Live=false; phase="dead"; Log("Ball to boundary — dot"); var t=new Timer{Interval=700}; t.Tick+=(s,e)=>{ t.Stop(); t.Dispose(); ResetBall(); }; t.Start(); } ball.Live=false; return; }
            if(!ball.WasHit && Math.Abs(ball.Vy)<0.22f && ball.Z<=0.5f){ FinishDelivery(0,"Dot ball",""); return; }
            if(ball.WasHit && ball.Live){ float sp=(float)Math.Sqrt(ball.Vx*ball.Vx+ball.Vy*ball.Vy); if(sp<0.9f && ball.Z<=0.8f){ float nearest=float.MaxValue; foreach(var f in fielders) nearest=Math.Min(nearest,(float)Math.Sqrt((f.X-ball.X)*(f.X-ball.X)+(f.Y-ball.Y)*(f.Y-ball.Y))); if(nearest>110) FinishDelivery(runInProgress?1:0, runInProgress?"1 run":"Stopped — dot",""); else FinishDelivery(0,"Fielded in ring — dot",""); runInProgress=false; ball.Live=false; } }
        }
        // fielders chase
        if(ball!=null && ball.Live && ball.WasHit)
        {
            foreach(var f in fielders){
                float dx=ball.X-f.X, dy=ball.Y-f.Y, d=(float)Math.Sqrt(dx*dx+dy*dy);
                if(d<400){ float sp=f.Speed*(ball.Boundary?1.05f:1f); if(d>1){ f.X+=dx/d*sp; f.Y+=dy/d*sp; } if(d<18){ ball.Live=false; phase="dead"; if(!ball.Boundary){ if(runInProgress){ int cr= ball.Z>3.5f?0:1; if(cr>0) FinishDelivery(1,"Quick single",""); else FinishDelivery(0,"Fielded — dot",""); runInProgress=false; } else FinishDelivery(0,"Fielded — dot",""); } return; } }
            }
        } else { foreach(var f in fielders){ f.X+=(f.BaseX-f.X)*0.04f; f.Y+=(f.BaseY-f.Y)*0.04f; } }
        if(msgTicks>0) msgTicks--;
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        var g=e.Graphics; g.SmoothingMode=SmoothingMode.AntiAlias; g.TextRenderingHint=System.Drawing.Text.TextRenderingHint.ClearTypeGridFit;
        // bg
        using(var bg=new LinearGradientBrush(ClientRectangle, Color.FromArgb(4,10,20), Color.FromArgb(12,30,60), 90f)) g.FillRectangle(bg, ClientRectangle);
        // field — radial
        var fieldRect=new RectangleF(fieldCX-BoundaryR, fieldCY-BoundaryR, BoundaryR*2, BoundaryR*2);
        using(var fieldBrush=new PathGradientBrush(new[]{fieldRect.Location, new PointF(fieldRect.Right, fieldRect.Top), new PointF(fieldRect.Right, fieldRect.Bottom), new PointF(fieldRect.Left, fieldRect.Bottom)})){
            // fallback to ellipse
        }
        // simpler: ellipse field
        g.FillEllipse(new SolidBrush(Color.FromArgb(14,77,14)), fieldRect);
        using(var p=new Pen(Color.FromArgb(180,255,255,255),2)) g.DrawEllipse(p, fieldRect);
        // pitch
        float px=PitchCX-PitchW/2, py=PitchCY-PitchH/2;
        g.FillRectangle(new SolidBrush(Color.FromArgb(200,170,122)), px, py, PitchW, PitchH);
        g.FillRectangle(new SolidBrush(Color.FromArgb(217,193,154)), px+14, py, PitchW-28, PitchH);
        g.DrawRectangle(new Pen(Color.FromArgb(220,255,255,255),1.2f), px, py, PitchW, PitchH);
        g.DrawLine(new Pen(Color.White,1.5f), px-12, PitchCY-PitchH/2+28, px+PitchW+12, PitchCY-PitchH/2+28);
        g.DrawLine(new Pen(Color.White,1.5f), px-12, PitchCY+PitchH/2-28, px+PitchW+12, PitchCY+PitchH/2-28);
        // stumps
        for(int k=0;k<2;k++){ float sx=BatX, sy=k==0?BatY:BowlY; for(int i=-1;i<=1;i++) g.FillRectangle(new SolidBrush(Color.FromArgb(232,220,192)), sx+i*8-2, sy-7, 4, 14); }
        // fielders
        foreach(var f in fielders){ g.FillEllipse(new SolidBrush(Color.FromArgb(15,42,75)), f.X-11, f.Y-11, 22,22); g.DrawEllipse(new Pen(Color.FromArgb(74,122,184),1.5f), f.X-11, f.Y-11, 22,22); g.DrawString("F", fontSmall, Brushes.White, f.X-5, f.Y-7); }
        // bowler/batter
        bool bowlBot=IsBotBowling(), batBot=IsBotBatting();
        // bowler
        g.FillEllipse(new SolidBrush(Color.FromArgb(30,58,138)), BowlX-14, BowlY-14, 28,28);
        if(bowlBot){ g.DrawEllipse(new Pen(Color.Gold,2.5f){DashStyle=DashStyle.Dash}, BowlX-14, BowlY-14, 28,28); }
        g.DrawString(bowlBot?"BOT":"BOWL", fontSmall, Brushes.White, BowlX-16, BowlY-26);
        // batter
        g.FillEllipse(new SolidBrush(Color.Gold), BatX-14, BatY-14, 28,28);
        g.DrawEllipse(new Pen(batBot? Color.FromArgb(30,64,175): Color.FromArgb(122,86,0), 2.2f), BatX-14, BatY-14, 28,28);
        g.FillRectangle(new SolidBrush(Color.FromArgb(107,58,10)), BatX-3, BatY-22, 6,22);
        g.DrawString(batBot?"BOT":"BAT", fontSmall, new SolidBrush(Color.FromArgb(26,17,0)), BatX-12, BatY-6);
        // aiming
        if(phase=="awaiting"){ float tx=fieldCX+aimLine*110, ty=PitchCY-PitchH/2+aimLength*PitchH; using(var pen=new Pen(Color.Gold,2){DashStyle=DashStyle.Dash}) g.DrawLine(pen, BowlX,BowlY,tx,ty); g.FillEllipse(Brushes.Gold, tx-6, ty-6, 12,12); g.DrawString("TARGET", fontSmall, Brushes.Black, tx-18, ty-16); }
        // trail
        if(trail.Count>1){ using(var pen=new Pen(Color.FromArgb(90,255,255,255),2)) for(int i=1;i<trail.Count;i++) g.DrawLine(pen, trail[i-1], trail[i]); }
        // ball shadow + ball
        if(ball!=null){
            float shR=Math.Max(4, 11-ball.Z*0.35f);
            g.FillEllipse(new SolidBrush(Color.FromArgb(80,0,0,0)), ball.X-shR/2, ball.Y+6-ball.Z*0.12f-shR/3, shR, shR*0.6f);
            bool isWindow= phase=="live" && ball.Y> BatY-110 && ball.Y< BatY+40 && !ball.WasHit;
            float r=Math.Max(7, 9-ball.Z*0.06f);
            g.FillEllipse(new SolidBrush(isWindow? Color.Gold: Color.FromArgb(197,40,40)), ball.X-r, ball.Y-ball.Z*0.9f-r, r*2, r*2);
            g.DrawEllipse(new Pen(Color.White,1.6f), ball.X-r, ball.Y-ball.Z*0.9f-r, r*2, r*2);
            if(ball.Z>2) g.DrawString(ball.Z.ToString("0.0")+"m", fontSmall, Brushes.White, ball.X+10, ball.Y-ball.Z*0.9f-10);
        }
        // HUD — top left split boards
        DrawHUD(g);
        // controls top right
        DrawControls(g);
        // bowling bottom left
        DrawBowling(g);
        // log right
        DrawLog(g);
        // msg
        if(msgTicks>0 && !string.IsNullOrEmpty(msgText)){
            var sz=g.MeasureString(msgText, fontMid);
            int mx=(int)((ClientSize.Width-sz.Width)/2-10), my=16;
            var mr=new Rectangle(mx,my,(int)sz.Width+20,22);
            g.FillRoundedRect(Brushes.Gold, mr, 11);
            g.DrawString(msgText, fontMid, new SolidBrush(Color.FromArgb(26,17,0)), mx+10, my+3);
        }
    }
    void DrawHUD(Graphics g)
    {
        int x=12,y=10,w=370;
        var rect=new Rectangle(x,y,w, 148 + (innings==2?28:0));
        g.FillRoundedRect(new SolidBrush(Color.FromArgb(230,5,18,36)), rect, 12);
        g.DrawRoundedRect(new Pen(Color.FromArgb(38,65,99),1), rect, 12);
        g.DrawString("SCOREBOARD", new Font("Segoe UI",8,FontStyle.Bold), new SolidBrush(Color.Gold), x+10,y+6);
        string tag= phase=="matchOver"? "MATCH OVER": innings==1?"1ST INNINGS":"2ND INNINGS";
        var tsz=g.MeasureString(tag, fontSmall); var trt=new Rectangle((int)(x+w-tsz.Width-18), y+4, (int)tsz.Width+12,16); g.FillRoundedRect(new SolidBrush(Color.FromArgb(40,255,201,40)), trt,8); g.DrawString(tag, fontSmall, new SolidBrush(Color.Gold), x+w-tsz.Width-12, y+6);
        // YOU
        bool ya=battingTeam=="you"&&phase!="matchOver", ba=battingTeam=="bot"&&phase!="matchOver";
        var r1=new Rectangle(x+8,y+24,w-16,52);
        g.FillRoundedRect(new SolidBrush(ya? Color.FromArgb(40,255,201,40): Color.FromArgb(15,255,255,255)), r1,10);
        if(ya) g.DrawRoundedRect(new Pen(Color.Gold,1.2f), r1,10);
        g.FillEllipse(new SolidBrush(Color.Gold), r1.X+8, r1.Y+11, 28,28); g.DrawString("YOU", fontSmall, new SolidBrush(Color.FromArgb(26,17,0)), r1.X+6, r1.Y+19);
        g.DrawString("YOU"+(ya?" * BATTING":""), fontSmall, new SolidBrush(ya? Color.Gold: Color.FromArgb(155,182,209)), r1.X+42, r1.Y+6);
        g.DrawString(string.Format("{0} / {1}", you.Score, you.Wickets), fontBig, Brushes.White, r1.X+42, r1.Y+18);
        g.DrawString(string.Format("{0} ov", you.OversStr(DemoOvers)), fontSmall, new SolidBrush(Color.FromArgb(155,182,209)), r1.X+120, r1.Y+24);
        g.DrawString("This over: "+(you.OverBalls.Count>0?string.Join(" • ",you.OverBalls):"—"), fontSmall, new SolidBrush(Color.FromArgb(155,182,209)), r1.X+42, r1.Y+38);
        if(ya){ var s="STRIKE"; var ssz=g.MeasureString(s,fontSmall); var sr=new Rectangle((int)(r1.Right- ssz.Width-14), r1.Y+12, (int)ssz.Width+8,14); g.FillRoundedRect(Brushes.Gold, sr,7); g.DrawString(s,fontSmall,new SolidBrush(Color.FromArgb(26,17,0)), r1.Right- ssz.Width-10, r1.Y+13); }
        // BOT
        var r2=new Rectangle(x+8,y+82,w-16,52);
        g.FillRoundedRect(new SolidBrush(ba? Color.FromArgb(40,255,201,40): Color.FromArgb(15,255,255,255)), r2,10);
        if(ba) g.DrawRoundedRect(new Pen(Color.Gold,1.2f), r2,10);
        g.FillEllipse(new SolidBrush(Color.FromArgb(30,58,138)), r2.X+8, r2.Y+11, 28,28); g.DrawString("BOT", fontSmall, Brushes.White, r2.X+6, r2.Y+19);
        g.DrawString("BOT"+(ba?" * BATTING":""), fontSmall, new SolidBrush(ba? Color.Gold: Color.FromArgb(155,182,209)), r2.X+42, r2.Y+6);
        g.DrawString(string.Format("{0} / {1}", bot.Score, bot.Wickets), fontBig, Brushes.White, r2.X+42, r2.Y+18);
        g.DrawString(string.Format("{0} ov", bot.OversStr(DemoOvers)), fontSmall, new SolidBrush(Color.FromArgb(155,182,209)), r2.X+120, r2.Y+24);
        g.DrawString("This over: "+(bot.OverBalls.Count>0?string.Join(" • ",bot.OverBalls):"—"), fontSmall, new SolidBrush(Color.FromArgb(155,182,209)), r2.X+42, r2.Y+38);
        if(ba){ var s="STRIKE"; var ssz=g.MeasureString(s,fontSmall); var sr2=new Rectangle((int)(r2.Right- ssz.Width-14), r2.Y+12, (int)ssz.Width+8,14); g.FillRoundedRect(Brushes.Gold, sr2,7); g.DrawString(s,fontSmall,new SolidBrush(Color.FromArgb(26,17,0)), r2.Right- ssz.Width-10, r2.Y+13); }
        // target
        if(innings==2 && phase!="matchOver"){
            int target=Inactive.Score+1, need=target-Active.Score, left=DemoOvers*6-Active.Balls;
            string ttxt= need<=0? string.Format("{0} WON — chased {1}!", battingTeam=="you"?"YOU":"BOT", target-1): string.Format("TARGET {0} • NEED {1} in {2} balls • {3} wkts left", target, need, left, MaxWickets-Active.Wickets);
            var tr=new Rectangle(x+8,y+140,w-16,22); g.FillRoundedRect(new SolidBrush(need<=0? Color.FromArgb(50,76,175,80): Color.FromArgb(40,255,201,40)), tr,7); g.DrawString(ttxt, fontSmall, Brushes.White, tr.X+6, tr.Y+5);
        } else if(innings==1 && (you.Balls>0||bot.Balls>0)){
            var tr=new Rectangle(x+8,y+140,w-16,22); g.FillRoundedRect(new SolidBrush(Color.FromArgb(30,120,180,255)), tr,7); g.DrawString(string.Format("1st innings — {0}/{1} ({2})", Active.Score, Active.Wickets, LegalStr(null)), fontSmall, Brushes.White, tr.X+6, tr.Y+5);
        }
    }
    void DrawControls(Graphics g)
    {
        int x=ClientSize.Width-312,y=10,w=300,h=168;
        var r=new Rectangle(x,y,w,h);
        g.FillRoundedRect(new SolidBrush(Color.FromArgb(230,5,18,36)), r,10); g.DrawRoundedRect(new Pen(Color.FromArgb(38,65,99),1), r,10);
        g.DrawString("CONTROLS", new Font("Segoe UI",7,FontStyle.Bold), new SolidBrush(Color.Gold), x+8,y+6);
        string[] rows={"Aim line / shot dir|Mouse X","Aim length|Mouse Y","Bowl|Space","Controlled shot|L Click","Aggressive loft|R Click","Take run|R","Cycle camera|Tab","Reset|Backspace"};
        for(int i=0;i<rows.Length;i++){ var parts=rows[i].Split('|'); g.DrawString(parts[0], fontSmall, new SolidBrush(Color.FromArgb(155,182,209)), x+8, y+22+i*15); g.DrawString(parts[1], fontSmall, Brushes.White, x+w- g.MeasureString(parts[1],fontSmall).Width-8, y+22+i*15); }
        g.DrawString("Bowling: hover to set line/length, Space. Batting: gold = window", fontSmall, new SolidBrush(Color.FromArgb(143,176,208)), x+8, y+144);
    }
    void DrawBowling(Graphics g)
    {
        int x=12,y=ClientSize.Height-98,w=320,h=88;
        var r=new Rectangle(x,y,w,h);
        g.FillRoundedRect(new SolidBrush(Color.FromArgb(230,5,18,36)), r,10); g.DrawRoundedRect(new Pen(Color.FromArgb(38,65,99),1), r,10);
        g.DrawString("BOWLING INPUTS", new Font("Segoe UI",7,FontStyle.Bold), new SolidBrush(Color.Gold), x+8,y+6);
        g.DrawString(string.Format("Pace {0}%  Swing {1}  Spin {2}", (int)(pace01*100), swingAmt, spinAmt), fontSmall, Brushes.White, x+8, y+22);
        g.DrawString(string.Format("Line {0} • {1}   Timing {2}", (int)(aimLine*100), LengthLabel(), lastTiming), fontSmall, new SolidBrush(Color.FromArgb(143,176,208)), x+8, y+38);
        // bars
        g.FillRectangle(new SolidBrush(Color.FromArgb(40,255,255,255)), x+8,y+56, w-16,6);
        g.FillRectangle(Brushes.Gold, x+8,y+56, (w-16)*pace01,6);
        bool dis=IsBotBowling()||phase=="matchOver";
        if(dis) g.DrawString("BOT BOWLING — inputs locked", fontSmall, new SolidBrush(Color.Gold), x+8, y+68);
    }
    void DrawLog(Graphics g)
    {
        int x=ClientSize.Width-312,y=186,w=300,h=ClientSize.Height-186-48;
        var r=new Rectangle(x,y,w,h);
        g.FillRoundedRect(new SolidBrush(Color.FromArgb(230,8,26,51)), r,10); g.DrawRoundedRect(new Pen(Color.FromArgb(26,58,92),1), r,10);
        for(int i=0;i<Math.Min(logLines.Count, 18);i++){
            var t=logLines[i];
            Color c= Color.FromArgb(155,182,209);
            if(t.Contains("WICKET")) c=Color.FromArgb(255,150,150);
            else if(t.Contains("FOUR")) c=Color.FromArgb(120,220,120);
            else if(t.Contains("SIX")) c=Color.FromArgb(255,220,120);
            g.DrawString(t, new Font("Consolas",7.5f), new SolidBrush(c), x+6, y+8+i*13);
        }
    }
    protected override void OnResize(EventArgs e){ base.OnResize(e); fieldCX=ClientSize.Width/2f; fieldCY=ClientSize.Height*0.52f; PitchCX=fieldCX; PitchCY=fieldCY; BatY=PitchCY+PitchH/2-18; BowlY=PitchCY-PitchH/2+18; Invalidate(); }

    [STAThread] static void Main(){ Application.EnableVisualStyles(); Application.Run(new CricketForm()); }
}
static class Ext{ public static void FillRoundedRect(this Graphics g, Brush b, Rectangle r, int rad){ using(var p=Rounded(r,rad)) g.FillPath(b,p); } public static void DrawRoundedRect(this Graphics g, Pen pen, Rectangle r, int rad){ using(var p=Rounded(r,rad)) g.DrawPath(pen,p); } static GraphicsPath Rounded(Rectangle r,int rad){ var p=new GraphicsPath(); int d=rad*2; p.AddArc(r.X,r.Y,d,d,180,90); p.AddArc(r.Right-d,r.Y,d,d,270,90); p.AddArc(r.Right-d,r.Bottom-d,d,d,0,90); p.AddArc(r.X,r.Bottom-d,d,d,90,90); p.CloseFigure(); return p; } }
