#include <windows.h>
#include <windowsx.h>
#include "resource.h"

#define PAI_WIDTH 49
#define PAI_HEIGHT 66
#define X_NUM 20
#define Y_NUM 10
#define WINDOW_WIDTH (X_NUM*PAI_WIDTH)
#define WINDOW_HEIGHT (Y_NUM*PAI_HEIGHT)
#define MAX_KOMASUU (X_NUM*Y_NUM)
#define ID_NEWGAME 1000
#define ID_HINT 1001

TCHAR szClassName[]=TEXT("Window");
BYTE iti2pai[MAX_KOMASUU];
BYTE pai2iti[(X_NUM-2)*(Y_NUM-2)];
HWND hWnd;
HHOOK g_hHook;

void koukan(int x,int y)
{
	if(x!=y)
	{
		int i,j;
		for(i=iti2pai[x]*4;;i++)
		{
			if(pai2iti[i]==x)break;
		}
		for(j=iti2pai[y]*4;;j++)
		{
			if(pai2iti[j]==y)break;
		}
		iti2pai[x]=iti2pai[y]-iti2pai[x];
		iti2pai[y]-=iti2pai[x];
		iti2pai[x]+=iti2pai[y];
		pai2iti[i]=pai2iti[j]-pai2iti[i];
		pai2iti[j]-=pai2iti[i];
		pai2iti[i]+=pai2iti[j];
	}
}

BYTE if0Space(BYTE a,BYTE b)
{
	BYTE t;
	if(a==b)return b;
	if(a>b)
	{
		t=a;
		a=b;
		b=t;
	}
	for(t=a+1;t<b;t++)
		if(iti2pai[t]!=(BYTE)-1)
			break;
	if(t==b)return b;
	for(t=a+X_NUM;t<b;t+=X_NUM)
		if(iti2pai[t]!=(BYTE)-1)
			break;
	if(t==b)return b;

	return 0;
}

VOID DrawLine(LPBYTE p,BYTE n)
{
	HDC hdc=GetDC(hWnd);
	HPEN hPen,hPenOld;
	hPen=CreatePen(PS_SOLID,10,RGB(0,0,255));
	hPenOld=(HPEN)SelectObject(hdc,hPen);
	LPPOINT point;
	point=(LPPOINT)GlobalAlloc(
		GMEM_FIXED,
		sizeof(POINT)*n);
	for(BYTE i=0;i<n;i++)
	{
		point[i].x=PAI_WIDTH/2+
			(p[i]%X_NUM)*PAI_WIDTH;
		point[i].y=PAI_HEIGHT/2+
			(p[i]/X_NUM)*PAI_HEIGHT;
	}
	Polyline(hdc,point,n);
	GlobalFree(point);
	SelectObject(hdc,hPenOld);
	DeleteObject(hPen);
	ReleaseDC(hWnd,hdc);
}

BYTE if1Space(BYTE a,BYTE b)
{
	BYTE ax,ay,bx,by;
	ax=a%X_NUM;
	ay=a/X_NUM;
	bx=b%X_NUM;
	by=b/X_NUM;
	if((ax==bx||ay==by)&&if0Space(a,b))
		return b;
	if(iti2pai[ax+by*X_NUM]==(BYTE)-1&&
		if0Space(ax+by*X_NUM,a)&&
		if0Space(ax+by*X_NUM,b)
		)return ax+by*X_NUM;
	if(iti2pai[bx+ay*X_NUM]==(BYTE)-1&&
		if0Space(bx+ay*X_NUM,a)&&
		if0Space(bx+ay*X_NUM,b)
		)return bx+ay*X_NUM;
	return 0;
}

VOID ATUB(DWORD data)
{
	BYTE a=HIBYTE(HIWORD(data));
	BYTE t=LOBYTE(HIWORD(data));
	BYTE u=HIBYTE(LOWORD(data));
	BYTE b=LOBYTE(LOWORD(data));
	BYTE p[4]={a,t,u,b};
	DrawLine(p,(t==u)?2:((u==b)?3:4));
	KillTimer(hWnd,0x1234);
	SetTimer(hWnd,0x1234,500,0);
}

BYTE ATB(BYTE a,BYTE t,BYTE b,BYTE bUser)
{
	BYTE u=if1Space(t,b);
	if(u)
	{
		if(bUser)
			ATUB(
			MAKELONG(MAKEWORD(b,u),
			MAKEWORD(t,a))
			);
		return 1;
	}
	return 0;
}

BYTE if2Space(BYTE a,BYTE b,BYTE bUser)
{
	BYTE t=if1Space(a,b);
	if(t)
	{
		if(bUser)ATUB(
			MAKELONG(MAKEWORD(b,b),
			MAKEWORD(t,a))
			);
		return 1;
	}
	for(t=a+1;t%X_NUM!=0&&iti2pai[t]==(BYTE)-1;t++)
	{
		if(ATB(a,t,b,bUser))return 1;
	}
	for(t=a-1;t%X_NUM!=X_NUM-1&&iti2pai[t]==(BYTE)-1;t--)
	{
		if(ATB(a,t,b,bUser))return 1;
	}
	for(t=a+X_NUM;t<MAX_KOMASUU&&
		iti2pai[t]==(BYTE)-1;t+=X_NUM)
	{
		if(ATB(a,t,b,bUser))return 1;
	}
	for(t=a-X_NUM;t<MAX_KOMASUU&&
		iti2pai[t]==(BYTE)-1;t-=X_NUM)
	{
		if(ATB(a,t,b,bUser))return 1;
	}
	return 0;
}

BOOL GetHint(LPBYTE a,LPBYTE b)
{
	int i;
	for(i=0;i<36;i++)
	{
		if(pai2iti[i*4+0]!=0&&
			pai2iti[i*4+1]!=0&&
			if2Space(pai2iti[i*4+0],pai2iti[i*4+1],0)
			){*a=pai2iti[i*4+0];*b=pai2iti[i*4+1];return 1;}
		if(pai2iti[i*4+0]!=0&&
			pai2iti[i*4+2]!=0&&
			if2Space(pai2iti[i*4+0],pai2iti[i*4+2],0)
			){*a=pai2iti[i*4+0];*b=pai2iti[i*4+2];return 1;}
		if(pai2iti[i*4+0]!=0&&
			pai2iti[i*4+3]!=0&&
			if2Space(pai2iti[i*4+0],pai2iti[i*4+3],0)
			){*a=pai2iti[i*4+0];*b=pai2iti[i*4+3];return 1;}
		if(pai2iti[i*4+1]!=0&&
			pai2iti[i*4+2]!=0&&
			if2Space(pai2iti[i*4+1],pai2iti[i*4+2],0)
			){*a=pai2iti[i*4+1];*b=pai2iti[i*4+2];return 1;}
		if(pai2iti[i*4+1]!=0&&
			pai2iti[i*4+3]!=0&&
			if2Space(pai2iti[i*4+1],pai2iti[i*4+3],0))
		{*a=pai2iti[i*4+1];*b=pai2iti[i*4+3];return 1;}
		if(pai2iti[i*4+2]!=0&&pai2iti[i*4+3]!=0&&
			if2Space(pai2iti[i*4+2],pai2iti[i*4+3],0))
		{*a=pai2iti[i*4+2];*b=pai2iti[i*4+3];return 1;}
	}
	return 0;
}

LRESULT CALLBACK CBTProc(int nCode,WPARAM wParam,LPARAM lParam)
{
    switch(nCode)
	{
    case HCBT_ACTIVATE:
        {
            UnhookWindowsHookEx(g_hHook);
            RECT m,w;
            GetWindowRect((HWND)wParam,&m);
            GetWindowRect(hWnd,&w);
            SetWindowPos(
				(HWND)wParam,
				hWnd,
				(w.right+w.left-m.right+m.left)/2,
				(w.bottom+w.top-m.bottom+m.top)/2,
				0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        }
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd,
						 UINT msg,
						 WPARAM wParam,
						 LPARAM lParam)
{
	static HBITMAP hBitmap;
	static BYTE oldpai=(BYTE)-1;
	static BYTE oldx;
	static BYTE oldy;
	static BYTE autoiti1,autoiti2;
	static BYTE zansuu;
	switch(msg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_HINT:
			if(zansuu)
			{
				iti2pai[autoiti1]+=72;
				iti2pai[autoiti2]+=72;
				InvalidateRect(hWnd,0,0);
				UpdateWindow(hWnd);
				if2Space(autoiti1,autoiti2,1);
				int i;
				for(i=0;i<4;i++)
				{
					if(pai2iti[4*(iti2pai[autoiti1]-72)+i]==autoiti1)
					{
						pai2iti[4*(iti2pai[autoiti1]-72)+i]=0;
					}
					if(pai2iti[4*(iti2pai[autoiti1]-72)+i]==autoiti2)
					{
						pai2iti[4*(iti2pai[autoiti1]-72)+i]=0;
					}
				}
				iti2pai[autoiti1]=(BYTE)-1;
				iti2pai[autoiti2]=(BYTE)-1;
				zansuu--;
				if(!zansuu){
					g_hHook=SetWindowsHookEx(
						WH_CBT,
						CBTProc,
						0,
						GetCurrentThreadId()
						);
					MessageBox(
						hWnd,
						TEXT("äÆóπÅB"),
						TEXT("ìÒäpéÊÇË"),
						0);
				}
				else if(!GetHint(&autoiti1,&autoiti2))
				{
					zansuu=0;
					g_hHook=SetWindowsHookEx(
						WH_CBT,
						CBTProc,
						0,
						GetCurrentThreadId()
						);
					MessageBox(
						hWnd,
						TEXT("éËãlÇ‹ÇËÇ≈Ç∑ÅB"),
						TEXT("ìÒäpéÊÇË"),
						0);
				}
			}
			break;
		case ID_NEWGAME:
			{
				int i,j=0;
				for(i=0;i<MAX_KOMASUU;i++)
				{
					if(i<X_NUM||
					   i>X_NUM*(Y_NUM-1)||
					   i%X_NUM==0||
					   i%X_NUM==X_NUM-1)
					{
						iti2pai[i]=(BYTE)-1;
						continue;
					}
					iti2pai[i]=j/4;
					pai2iti[j]=i;
					j++;
				}
				for(i=0;i<MAX_KOMASUU;i++)
				{
					if(i<X_NUM||
					   i>X_NUM*(Y_NUM-1)||
					   i%X_NUM==0||
					   i%X_NUM==X_NUM-1)
					   continue;
					do{
						j=rand()%MAX_KOMASUU;
					}while(j<X_NUM||
					   j>X_NUM*(Y_NUM-1)||
					   j%X_NUM==0||
					   j%X_NUM==X_NUM-1);
					
					koukan(i,j);
				}
				zansuu=72;
				if(!GetHint(&autoiti1,&autoiti2))
				{
					zansuu=0;
					g_hHook=SetWindowsHookEx(
						WH_CBT,
						CBTProc,
						0,
						GetCurrentThreadId()
						);
					MessageBox(
						hWnd,
						TEXT("éËãlÇ‹ÇËÇ≈Ç∑ÅB"),
						TEXT("ìÒäpéÊÇË"),
						0);
				}
				InvalidateRect(hWnd,0,0);
			}
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		if(zansuu)
		{
			InvalidateRect(hWnd,NULL,0);
			UpdateWindow(hWnd);
			RECT rect;
			int xPos = GET_X_LPARAM(lParam)/PAI_WIDTH; 
			int yPos = GET_Y_LPARAM(lParam)/PAI_HEIGHT;
			if(iti2pai[xPos%X_NUM+yPos*X_NUM]==(BYTE)-1)
				break;//ãÛîíÇÉNÉäÉbÉNÇµÇƒÇ‡âΩÇ‡ãNÇ±ÇÁÇ»Ç¢
			if(oldpai==(BYTE)-1)
			{
				iti2pai[xPos%X_NUM+yPos*X_NUM]+=72;
				SetRect(
					&rect,
					PAI_WIDTH*xPos,
					PAI_HEIGHT*yPos,
					PAI_WIDTH+PAI_WIDTH*xPos,
					PAI_HEIGHT+PAI_HEIGHT*yPos
					);
				InvalidateRect(hWnd,&rect,0);
				oldpai=iti2pai[xPos%X_NUM+yPos*X_NUM];
				oldx=xPos;
				oldy=yPos;
			}
			else
			{
				if(xPos==oldx&&yPos==oldy)//ÇPî‘ñ⁄Ç∆ìØÇ∂ç¿ïW
				{
					iti2pai[xPos%X_NUM+yPos*X_NUM]-=72;
					SetRect(
						&rect,
						PAI_WIDTH*xPos,
						PAI_HEIGHT*yPos,
						PAI_WIDTH+PAI_WIDTH*xPos,
						PAI_HEIGHT+PAI_HEIGHT*yPos);
					InvalidateRect(hWnd,&rect,0);
				}
				else
				{
					iti2pai[xPos%X_NUM+yPos*X_NUM]+=72;
					SetRect(
						&rect,
						PAI_WIDTH*xPos,
						PAI_HEIGHT*yPos,
						PAI_WIDTH+PAI_WIDTH*xPos,
						PAI_HEIGHT+PAI_HEIGHT*yPos);
					InvalidateRect(hWnd,&rect,0);
					UpdateWindow(hWnd);
					if(oldpai==iti2pai[xPos%X_NUM+yPos*X_NUM]&&
						if2Space(
						xPos%X_NUM+yPos*X_NUM,
						oldx%X_NUM+oldy*X_NUM,1))
					{
						int i;
						BOOL b=FALSE;
						for(i=0;i<4;i++)
						{
							if(pai2iti[4*(oldpai-72)+i]==
								oldx%X_NUM+oldy*X_NUM)
							{
								if(!b&&
									(pai2iti[4*(oldpai-72)+i]==autoiti1||
									pai2iti[4*(oldpai-72)+i]==autoiti2))
									b=TRUE;
								pai2iti[4*(oldpai-72)+i]=0;
							}
							if(pai2iti[4*(oldpai-72)+i]==
								xPos%X_NUM+yPos*X_NUM)
							{
								if(!b&&
									(pai2iti[4*(oldpai-72)+i]==autoiti1||
									pai2iti[4*(oldpai-72)+i]==autoiti2))
									b=TRUE;
								pai2iti[4*(oldpai-72)+i]=0;
							}
						}
						iti2pai[xPos%X_NUM+yPos*X_NUM]=(BYTE)-1;
						iti2pai[oldx%X_NUM+oldy*X_NUM]=(BYTE)-1;

						zansuu--;
						if(!zansuu){
							g_hHook=SetWindowsHookEx(
								WH_CBT,
								CBTProc,
								0,
								GetCurrentThreadId());
							MessageBox(
								hWnd,
								TEXT("äÆóπÅB"),
								TEXT("ìÒäpéÊÇË"),0);
						}
						else if(b&&
							!GetHint(&autoiti1,&autoiti2))
						{
							zansuu=0;
							g_hHook=SetWindowsHookEx(
								WH_CBT,
								CBTProc,
								0,
								GetCurrentThreadId()
								);
							MessageBox(
								hWnd,
								TEXT("éËãlÇ‹ÇËÇ≈Ç∑ÅB"),
								TEXT("ìÒäpéÊÇË"),
								0);
						}
					}
					else
					{
						iti2pai[xPos%X_NUM+yPos*X_NUM]-=72;
						iti2pai[oldx%X_NUM+oldy*X_NUM]-=72;
						SetRect(
							&rect,
							PAI_WIDTH*xPos,
							PAI_HEIGHT*yPos,
							PAI_WIDTH+PAI_WIDTH*xPos,
							PAI_HEIGHT+PAI_HEIGHT*yPos);
						InvalidateRect(hWnd,&rect,0);
						SetRect(
							&rect,
							PAI_WIDTH*oldx,
							PAI_HEIGHT*oldy,
							PAI_WIDTH+PAI_WIDTH*oldx,
							PAI_HEIGHT+PAI_HEIGHT*oldy);
						InvalidateRect(hWnd,&rect,0);
					}
				}
				oldpai=(BYTE)-1;
			}
		}
		break;
	case WM_TIMER:
		KillTimer(hWnd,0x1234);
		InvalidateRect(hWnd,0,0);
		break;
	case WM_CREATE:
		srand(GetTickCount());
		hBitmap=LoadBitmap(
			GetModuleHandle(0),
			MAKEINTRESOURCE(IDB_BITMAP1));
		SNDMSG(hWnd,WM_COMMAND,ID_NEWGAME,0);
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc=BeginPaint(hWnd,&ps);
			HDC hdc_mem=CreateCompatibleDC(hdc);
			SelectObject(hdc_mem,hBitmap);
			int i;
			for(i=0;i<MAX_KOMASUU;i++)
			{
				if(iti2pai[i]==(BYTE)-1)
				{
					RECT rect;
					SetRect(
						&rect,
						PAI_WIDTH*(i%X_NUM),
						PAI_HEIGHT*(i/X_NUM),
						PAI_WIDTH+PAI_WIDTH*(i%X_NUM),
						PAI_HEIGHT+PAI_HEIGHT*(i/X_NUM));
					COLORREF clrPrev=SetBkColor(hdc,RGB(0,128,0));
					ExtTextOut(hdc,0,0,ETO_OPAQUE,&rect,0,0,0);
					SetBkColor(hdc,clrPrev);
				}
				else
				{
					BitBlt(
						hdc,
						(i%X_NUM)*PAI_WIDTH,
						(i/X_NUM)*PAI_HEIGHT,
						PAI_WIDTH,PAI_HEIGHT,
						hdc_mem,
						iti2pai[i]%72*PAI_WIDTH,
						(iti2pai[i]<36)?0:PAI_HEIGHT,
						SRCCOPY);
				}
			}
			DeleteDC(hdc_mem);
			EndPaint(hWnd,&ps);
		}
		break;
	case WM_DESTROY:
		DeleteObject(hBitmap);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass={
		0,
		WndProc,
		0,
		0,
		hInstance,
		LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1)),
		LoadCursor(0,IDC_ARROW),
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	hWnd=CreateWindow(
		szClassName,
		TEXT("ìÒäpéÊÇË"),
		WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,CW_USEDEFAULT,
		0,
		WINDOW_WIDTH+GetSystemMetrics(SM_CXEDGE)+
		GetSystemMetrics(SM_CXBORDER)+
		GetSystemMetrics(SM_CXDLGFRAME),
		WINDOW_HEIGHT+GetSystemMetrics(SM_CYEDGE)+
		GetSystemMetrics(SM_CYBORDER)+
		GetSystemMetrics(SM_CYDLGFRAME)+
		GetSystemMetrics(SM_CYCAPTION),
		0,
		0,
		hInstance,
		0);
	ShowWindow(hWnd,SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	ACCEL Accel[]={
		{FVIRTKEY,VK_F2,ID_NEWGAME},
		{FVIRTKEY,VK_F1,ID_HINT}
	};
	HACCEL hAccel=CreateAcceleratorTable(Accel,2);
	while(GetMessage(&msg,0,0,0))
	{
		if(!TranslateAccelerator(hWnd,hAccel,&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	DestroyAcceleratorTable(hAccel);
	ExitProcess(msg.wParam);
}
