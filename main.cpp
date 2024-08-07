#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>

using namespace std;

const int fld_width = 20;
const int fld_height = 30;
const int scr_width = fld_width * 2;
const int scr_height = fld_height;

const char c_fig = 219;
const char c_field = 176;
const char c_figDwn = 178;

typedef char TScreenMap[scr_height][scr_width];
typedef char TFieldMap[fld_height][fld_width];

const int shp_width = 4;
const int shp_height = 4;
typedef char TShape[shp_height][shp_width];

const int WinScore = 10;

char* shpApp[] =
{
    (char*)"....****........",
    (char*)".....***.*......",
    (char*)".....**..**.....",
    (char*)"....***..*......",
    (char*)".....**.**......"
};

const int shpAppCnt = sizeof(shpApp) / sizeof(shpApp[0]);

void SetCurPos(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

class TScreen
{
    void SetEnd() {scr[scr_height-1][scr_width-1] = '\0';}
public:
    TScreenMap scr;
    TScreen() {Clear();}
    void Clear() {memset(scr, '.', sizeof(scr));}
    void Clear(int a) {memset(scr, ' ', sizeof(scr));}
    void Show() {SetCurPos(0,0); SetEnd(); cout << scr[0];}
};

class TField
{
public:
    int score;
    TFieldMap fld;
    TField() {Clear(); score = 0;}
    void Clear() {memset(fld, c_field, sizeof(fld));}
    void Put(TScreenMap &src)
    {
        for (int i = 0; i < fld_height; i++)
            for (int j = 0; j < fld_width; j++)
                src[i][j*2] = src[i][j*2 + 1] = fld[i][j];
    }
    void Burning()
    {
        for (int i = fld_height-1 ; i >= 0; i--)
        {
            bool fullLine = true;

            for (int j = 0; j < fld_width; j++)
                if (fld[i][j] != c_figDwn)
                    fullLine = false;
            if (fullLine)
            {
                for (int y = i; y >= 1; y--)
                    memcpy(fld[y], fld[y-1], sizeof(fld[y]));

                score++;
            }
        }
    }
    void PutScore(TScreenMap &src)
    {
        char scoreLine[10] = "csore= ";
        char num[3];
        itoa(score, num, 10);
        strcat(scoreLine, num);
        int len = strlen(scoreLine);
        for (int i = 0; i < len; i++)
            src[0][i] = scoreLine[i];
    }
};

class TFigure
{
    int x, y;
    TShape vid;
    char turn;
    COORD coord[shp_height * shp_width];
    int coordCnt;
    TField *field = NULL;
public:
    TFigure() {memset(this, 0, sizeof(*this));}
    void SetField(TField *_field) {field = _field;}
    void Shape(char* _vid) {memcpy(vid, _vid, sizeof(vid));}
    void Pos(int _x, int _y) {x = _x; y = _y; CalcCoord();}
    char TurnGet() {return turn;}
    void TurnSet(char _turn)
    {
        char oldTurn = _turn;
        turn = (_turn > 3 ? 0 : (_turn < 0 ? 3 : _turn));
        int chk = Check();
        if (chk == 0) return;
        if (chk == 1)
        {
            int xx = x;
            int k = (x > (fld_width / 2) ? -1 : +1);
            for (int i = 0; i < 3; i++)
            {
                x += k;
                if (Check() == 0) return;
            }
            x = xx;
        }
        turn = oldTurn;
        CalcCoord();
    }
    void Put(TScreenMap &scr)
    {
        for (int i = 0; i < coordCnt; i++)
           scr[coord[i].Y][coord[i].X * 2] = scr[coord[i].Y][coord[i].X * 2 + 1] = c_fig;
    }
    void Put(TFieldMap &fld)
    {
        for (int i = 0; i < coordCnt; i++)
           fld[coord[i].Y][coord[i].X] = c_figDwn;
    }
    bool Move(int dx, int dy)
    {
        int oldX = x, oldY = y;
        Pos(x + dx, y + dy);
        int chk = Check();
        if (chk >= 1)
        {
            Pos(oldX, oldY);
            if (chk == 2)
                return false;
        }
        return true;
    }
    int Check()
    {
        CalcCoord();
        for (int i = 0; i < coordCnt; i++)
            if (coord[i].X < 0 || coord[i].X >= fld_width)
                return 1;
        for (int i = 0; i < coordCnt; i++)
            if ( coord[i].Y >= fld_height || field->fld[coord[i].Y][coord[i].X] == c_figDwn)
                return 2;
        return 0;
    }
private:
    void CalcCoord()
    {
        int xx, yy;
        coordCnt = 0;
        for (int i = 0; i < shp_height; i++)
            for (int j = 0; j < shp_width; j++)
                if (vid[i][j] == '*')
                {
                    if (turn == 0) xx = x + j, yy = y + i;
                    if (turn == 1) xx = x + (shp_height-i-1), yy = y + j;
                    if (turn == 2) xx = x + (shp_width-j-1), yy = y + (shp_height-i-1);
                    if (turn == 3) xx = x + i, yy = y + (shp_height-j-1) + (shp_width - shp_height);
                    coord[coordCnt] = (COORD){(short)xx, (short)yy};
                    coordCnt++;
                }
    }
};

class TGame
{
    TScreen screen;
    TField field;
    TFigure figure;
    bool GameOver;
public:
    TGame()
    {
        figure.SetField(&field);
        figure.Shape(shpApp[rand() % shpAppCnt]);
        figure.Pos(fld_width/2 - shp_width/2, 0);
        GameOver = false;
    }
    void PlayerControl()
    {
        static int trn = 0;
        if (GetKeyState('W') < 0) trn += 1;
        if (trn == 1) figure.TurnSet(figure.TurnGet() + 1), trn++;
        if (GetKeyState('W') >= 0) trn = 0;

        if (GetKeyState('S') < 0) figure.Move(0,1);
        if (GetKeyState('A') < 0) figure.Move(-1,0);
        if (GetKeyState('D') < 0) figure.Move(1,0);
    }
    void Move()
    {
        static int tick = 0;
        tick++;
        if (tick >= 5)
        {
            if (!figure.Move(0,1))
            {
                figure.Put(field.fld);
                figure.Shape(shpApp[rand() % shpAppCnt]);
                figure.Pos(fld_width/2 - shp_width/2, 0);
                if (figure.Check() > 0)
                    GameOver = true;
            }
            tick = 0;
            field.Burning();
        }
    }
    void Show()
    {
        screen.Clear();
        field.Put(screen.scr);
        figure.Put(screen.scr);
        field.PutScore(screen.scr);
        screen.Show();
    }
    void ShowYouWin()
    {
        cout << "\t      * * *** * *" << endl;
        cout << "\t      * * * * * *" << endl;
        cout << "\t      *** * * * *" << endl;
        cout << "\t        * * * * *" << endl;
        cout << "\t      *** *** ***" << endl;
        cout << endl;
        cout << "\t    * * *  *  *  *" << endl;
        cout << "\t    * * *  *  ** *" << endl;
        cout << "\t    * * *  *  * **" << endl;
        cout << "\t     * *   *  *  *" << endl;
        Sleep(3000);
        exit(0);
    }
    void ShowYouLose()
    {
        cout << "\t      * * *** * *" << endl;
        cout << "\t      * * * * * *" << endl;
        cout << "\t      *** * * * *" << endl;
        cout << "\t        * * * * *" << endl;
        cout << "\t      *** *** ***" << endl;
        cout << endl;
        cout << "\t    *   *** *** ***" << endl;
        cout << "\t    *   * * *   *  " << endl;
        cout << "\t    *   * *  *  ***" << endl;
        cout << "\t    *   * *   * *  " << endl;
        cout << "\t    *** *** *** ***" << endl;
        Sleep(3000);
        exit(0);
    }
    void WinLose()
    {
        if (field.score >= WinScore)
        {
            screen.Clear(0);
            screen.Show();
            SetCurPos(0,5);
            ShowYouWin();
        }
        if (GameOver)
        {
            screen.Clear(0);
            screen.Show();
            SetCurPos(0,5);
            ShowYouLose();
        }
    }
};

int main()
{
    char command[1000];
    sprintf(command, "mode con cols=%d lines=%d", scr_width, scr_height);
    system(command);

    srand(time(NULL));
    TGame game;
    while(1)
    {
        game.PlayerControl();
        game.Move();
        game.WinLose();
        game.Show();
        if (GetKeyState(VK_ESCAPE) < 0)
            break;
        Sleep(50);
    }

    return 0;
}
