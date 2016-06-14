// CppTicTacToe.cpp : Defines the entry point for the application. Win32 API is AWFUL
//

#include "stdafx.h"
#include "CppTicTacToe.h"
#include <windowsx.h>
#include <string>
#include <algorithm>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CPPTICTACTOE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CPPTICTACTOE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CPPTICTACTOE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH); //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CPPTICTACTOE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//

const int CELL_SIZE = 150;
const int SIGN_SIZE = 128;
HBRUSH hBrushP1, hBrushP2, hBrushPWin;
HICON hIconP1, hIconP2;

COLORREF colorPlayer1Text = RGB(0, 0, 200);    // blue 'X' sign => blue text 
COLORREF colorPlayer2Text = RGB(0, 200, 0);    // green 'O' sign => green text
COLORREF colorPlayer1Bg   = RGB(25, 150, 25);  // blue 'X' sign => green background
COLORREF colorPlayer2Bg   = RGB(25, 25, 150);  // green 'O' sign => blue background

int playerTurn = 1;
int gameBoard[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int winner = 0;
int wins[3];

BOOL GetGameBoardRect(HWND &hwnd, RECT *pRect)
{
    RECT rc;
    if (GetClientRect(hwnd, &rc)) {
        pRect->left = ((rc.right - rc.left) - CELL_SIZE * 3) / 2;
        pRect->top = ((rc.bottom - rc.top) - CELL_SIZE * 3) / 2;

        pRect->right = pRect->left + CELL_SIZE * 3;
        pRect->bottom = pRect->top + CELL_SIZE * 3;
return TRUE;
    }

    SetRectEmpty(pRect);
    return FALSE;
}

void DrawLine(HDC &hdc, int x1, int y1, int x2, int y2)
{
    MoveToEx(hdc, x1, y1, nullptr);
    LineTo(hdc, x2, y2);
}

int GetCellNumberFromPoint(HWND& hWnd, int x, int y)
{
    POINT pt{ x, y };
    RECT rc;
    if (GetGameBoardRect(hWnd, &rc) && PtInRect(&rc, pt))  // clicked inside game board
    {
        // Normalize point ( 0 to 3 * CELL_SIZE )
        x = pt.x - rc.left;
        y = pt.y - rc.top;

        int column = x / CELL_SIZE;
        int row = y / CELL_SIZE;

        // convert to index ( 0 to 8 )
        return column + row * 3;
    }
    return -1;  // outside game board
}

BOOL GetCellRect(HWND &hWnd, int index, RECT *pRect)
{
    RECT rcBoard;
    if (GetGameBoardRect(hWnd, &rcBoard))
    {
        //Convert index from 0 to 8 into (x, y) pair
        int x = index % 3; // Column number
        int y = index / 3; // Row number

        pRect->left = rcBoard.left + x * CELL_SIZE + 1;
        pRect->top = rcBoard.top + y * CELL_SIZE + 1;
        pRect->right = pRect->left + CELL_SIZE - 1;
        pRect->bottom = pRect->top + CELL_SIZE - 1;

        return TRUE;
    }

    return FALSE;
}

/*
 Returns:
 0 - no winner
 1 - Player 1 wins
 2 - Player 2 wins
 -1 - Tie - no winner

 board coordinates:
     0, 1, 2,
     3, 4, 5,
     6, 7, 8
*/
int GetWinner(int(&wins)[3]) {
    int cells[] = { // possible win combinations
        0,1,2,
        3,4,5,
        6,7,8,
        0,3,6,
        1,4,7,
        2,5,8,
        0,4,8,
        2,4,6
    };

    for (unsigned int i = 0; i < std::size(cells); i += 3) {
        if (gameBoard[cells[i]] && gameBoard[cells[i]] == gameBoard[cells[i + 1]] 
                                && gameBoard[cells[i]] == gameBoard[cells[i + 2]])
        {
            // Winner found !!!
            wins[0] = cells[i];
            wins[1] = cells[i + 1];
            wins[2] = cells[i + 2];
            return gameBoard[cells[i]];
        }
    }

    // No Winner found, check for empty places on the board
    for (unsigned int i = 0; i < std::size(gameBoard); ++i)
        if (!gameBoard[i])
            return 0;

    // No more empty places on the board, it's a Tie
    return -1;

}

void ShowTurn(HWND hWnd, HDC hdc)
{
    const WCHAR *ptrText = nullptr;

    switch (winner) {
        case 0:  // still playing 
            {
                SetTextColor(hdc, playerTurn == 1 ? colorPlayer1Text : colorPlayer2Text);
                ptrText = playerTurn == 1 ? L"Turn: Player 1" : L"Turn: Player 2";
            }
            break;
        case 1:  // Player 1 wins
            {
                SetTextColor(hdc, colorPlayer1Text);
                ptrText = L"Player 1 is the Winner!!!";
            }
            break;
        case 2:  // Player 2 wins
            {
                SetTextColor(hdc, colorPlayer2Text);
                ptrText = L"Player 2 is the Winner!!!";
            }
            break;
        case -1:  // It's a tie
            {
                SetTextColor(hdc, RGB(200, 200, 0));  // neutral Yellow
                ptrText = L"It's a Tie!!!";
            }
    }

    RECT rc;
    if (GetClientRect(hWnd, &rc))
    {
        rc.top = rc.bottom - 48;

        FillRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
        SetBkMode(hdc, TRANSPARENT);  // transparent text background
        DrawText(hdc, ptrText, lstrlen(ptrText), &rc, DT_CENTER);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
            hBrushP1 = CreateSolidBrush(colorPlayer1Bg);
            hBrushP2 = CreateSolidBrush(colorPlayer2Bg);
            hBrushPWin = CreateSolidBrush(RGB(200, 200, 0));  // neutral Yellow
            hIconP1 = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_P1), IMAGE_ICON, SIGN_SIZE, SIGN_SIZE, LR_DEFAULTCOLOR);
            hIconP2 = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_P2), IMAGE_ICON, SIGN_SIZE, SIGN_SIZE, LR_DEFAULTCOLOR);
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_FILE_NEWGAME:
                {
                    if (IDYES == MessageBox(hWnd, L"Are you sure you want to start a new game?", L"New Game", MB_YESNO | MB_ICONQUESTION))
                    {
                        // Reset and start new game;
                        playerTurn = 1;
                        winner = 0;
                        std::fill(gameBoard, std::end(gameBoard), 0);
                        // Force rePaint message
                        InvalidateRect(hWnd, NULL, TRUE);
                        UpdateWindow(hWnd);
                    }
                }
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_GETMINMAXINFO:
        {
            MINMAXINFO *pMinMax = (MINMAXINFO*)lParam;

            pMinMax->ptMinTrackSize.x = CELL_SIZE * 4 - 25;
            pMinMax->ptMinTrackSize.y = CELL_SIZE * 4;
        }
        break;
    case WM_LBUTTONDOWN:
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            if (playerTurn <= 0) break;

            int index = GetCellNumberFromPoint(hWnd, xPos, yPos);
            if (HDC hdc = GetDC(hWnd)) 
            {
                
                //WCHAR temp[20];
                //wsprintf(temp, L"Index = %d", index);
                //TextOut(hdc, xPos, yPos, temp, lstrlen(temp));
                
                //Get cell dimension from Index
                if (index != -1)
                {
                    RECT rcCell;
                    if (!gameBoard[index] && GetCellRect(hWnd, index, &rcCell))
                    {
                        gameBoard[index] = playerTurn;  // set gameBoard status

                        // draw gameBoard cells
                        FillRect(hdc, &rcCell, playerTurn == 1 ? hBrushP1 : hBrushP2);
                        DrawIconEx(hdc, rcCell.left + (CELL_SIZE - SIGN_SIZE)/2, rcCell.top + (CELL_SIZE - SIGN_SIZE) / 2, 
                                   playerTurn == 1 ? hIconP1 : hIconP2, SIGN_SIZE, SIGN_SIZE, 0, nullptr, DI_NORMAL);
                        
                        winner = GetWinner(wins);  // check for game winner
                        
                        if (winner > 0)
                        {
                            //We have a winner!!!
                            ShowTurn(hWnd, hdc);  // Display turn
                            
                            for (int winIndex : wins) {  // draw winning gameBoard cells
                                GetCellRect(hWnd, winIndex, &rcCell);
                                FillRect(hdc, &rcCell, hBrushPWin);
                                DrawIconEx(hdc, rcCell.left + (CELL_SIZE - SIGN_SIZE) / 2, rcCell.top + (CELL_SIZE - SIGN_SIZE) / 2,
                                    playerTurn == 1 ? hIconP1 : hIconP2, SIGN_SIZE, SIGN_SIZE, 0, nullptr, DI_NORMAL);
                            }

                            MessageBox(
                                hWnd, winner == 1 ? L"Player 1 is the Winner!!!" : L"Player 2 is the Winner!!!", L"!!!Win!!!",
                                MB_OK | MB_ICONINFORMATION);
                            playerTurn = 0;
                            
                        }
                        else if (winner == -1)
                        {
                            // It's a Tie!!!
                            ShowTurn(hWnd, hdc);  // Display turn
                            MessageBox(
                                hWnd, L"It's a Tie!!!", L"!!!Tie!!!",
                                MB_OK | MB_ICONEXCLAMATION);
                            playerTurn = 0;
                        }
                        else
                        {
                            playerTurn = playerTurn == 1 ? 2 : 1;
                            ShowTurn(hWnd, hdc);  // Display turn
                        }
                    }
                }
                ReleaseDC(hWnd, hdc);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            RECT rcClient;
            if (GetClientRect(hWnd, &rcClient))
            {
                // Draw Player 1 and Player 2 text
                static const WCHAR player1Text[] = L"Player 1";
                static const WCHAR player2Text[] = L"Player 2";

                SetBkMode(hdc, TRANSPARENT);  // transparent text background
                
                SetTextColor(hdc, colorPlayer1Text);  // set text color to red for Player 1
                TextOut(hdc, 16, 16, player1Text, std::size(player1Text));

                SetTextColor(hdc, colorPlayer2Text);  // set text color to blue for Player 2
                TextOut(hdc, rcClient.right - 80, 16, player2Text, std::size(player2Text));

                ShowTurn(hWnd, hdc);
            }

            RECT rc;
            if (GetGameBoardRect(hWnd, &rc))
            {
                // Draw game board
                FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

                for (unsigned int i = 0; i < 4; ++i) {
                    // Draw vertical lines
                    DrawLine(hdc, rc.left + CELL_SIZE * i, rc.top, rc.left + CELL_SIZE * i, rc.bottom);
                    // Draw horizontal lines
                    DrawLine(hdc, rc.left, rc.top + CELL_SIZE * i, rc.right, rc.top + CELL_SIZE * i);
                }

                RECT rcCell;
                for (unsigned int i = 0; i < std::size(gameBoard); ++i)  // redraw gameBoard cells
                {
                    if (gameBoard[i] && GetCellRect(hWnd, i, &rcCell))
                    {
                        FillRect(hdc, &rcCell, gameBoard[i] == 1 ? hBrushP1 : hBrushP2);
                        DrawIconEx(hdc, rcCell.left + (CELL_SIZE - SIGN_SIZE) / 2, rcCell.top + (CELL_SIZE - SIGN_SIZE) / 2,
                            gameBoard[i] == 1 ? hIconP1 : hIconP2, SIGN_SIZE, SIGN_SIZE, 0, nullptr, DI_NORMAL);
                    }
                }

                if (winner > 0)  // redraw winning gameBoard cells
                {
                    for (int winIndex : wins) {
                        GetCellRect(hWnd, winIndex, &rcCell);
                        FillRect(hdc, &rcCell, hBrushPWin);
                        DrawIconEx(hdc, rcCell.left + (CELL_SIZE - SIGN_SIZE) / 2, rcCell.top + (CELL_SIZE - SIGN_SIZE) / 2,
                            winner == 1 ? hIconP1 : hIconP2, SIGN_SIZE, SIGN_SIZE, 0, nullptr, DI_NORMAL);
                    }
                }
            }

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        DeleteObject(hBrushP1);
        DeleteObject(hBrushP2);
        //DeleteObject(hBrushPWin);
        
        DeleteObject(hIconP1);
        DeleteObject(hIconP1);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
