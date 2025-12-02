// main.cpp
#include <windows.h>
#include <commctrl.h>
#include "Edit.h"
#include "resource.h"

#pragma comment(lib, "comctl32.lib")

Edit* g_pEdit = nullptr;

// Объявляем переменные для хранения старых процедур
WNDPROC g_OldLeftProc = nullptr;
WNDPROC g_OldRightProc = nullptr;

// Объявление подпрограммы
LRESULT CALLBACK ListViewSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        g_pEdit = (Edit*)lParam;
        g_pEdit->hDlg = hDlg;
        g_pEdit->InitControls();

        // Сохраняем старые процедуры
        g_OldLeftProc = (WNDPROC)SetWindowLongPtr(g_pEdit->hListLeft, GWLP_WNDPROC, (LONG_PTR)ListViewSubclassProc);
        g_OldRightProc = (WNDPROC)SetWindowLongPtr(g_pEdit->hListRight, GWLP_WNDPROC, (LONG_PTR)ListViewSubclassProc);

        // Сохраняем указатель на объект Edit
        SetWindowLongPtr(g_pEdit->hListLeft, GWLP_USERDATA, (LONG_PTR)g_pEdit);
        SetWindowLongPtr(g_pEdit->hListRight, GWLP_USERDATA, (LONG_PTR)g_pEdit);

        return TRUE;

    case WM_COMMAND:
        // Обработка выбора диска в комбобоксах
        if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            if (LOWORD(wParam) == IDC_COMBO_LEFT)
            {
                int index = (int)SendMessage(g_pEdit->hComboLeft, CB_GETCURSEL, 0, 0);
                if (index != CB_ERR)
                {
                    wchar_t drive[MAX_PATH] = { 0 };
                    SendMessage(g_pEdit->hComboLeft, CB_GETLBTEXT, index, (LPARAM)drive);
                    g_pEdit->RefreshFolder(g_pEdit->hListLeft, g_pEdit->hStaticLeft, drive);
                    g_pEdit->bActiveLeft = true;
                    g_pEdit->ClearFileAttributes();
                }
            }
            else if (LOWORD(wParam) == IDC_COMBO_RIGHT)
            {
                int index = (int)SendMessage(g_pEdit->hComboRight, CB_GETCURSEL, 0, 0);
                if (index != CB_ERR)
                {
                    wchar_t drive[MAX_PATH] = { 0 };
                    SendMessage(g_pEdit->hComboRight, CB_GETLBTEXT, index, (LPARAM)drive);
                    g_pEdit->RefreshFolder(g_pEdit->hListRight, g_pEdit->hStaticRight, drive);
                    g_pEdit->bActiveLeft = false;
                    g_pEdit->ClearFileAttributes();
                }
            }
        }
        else
        {
            switch (LOWORD(wParam))
            {
            case IDC_BUTTON_COPY:   g_pEdit->CopyBetweenPanels(); break;
            case IDC_BUTTON_MOVE:   g_pEdit->MoveSelected(); break;
            case IDC_BUTTON_CREATE: g_pEdit->CreateNew(); break;
            case IDC_BUTTON_DELETE: g_pEdit->DeleteSelected(); break;
            case IDC_BUTTON_RENAME: g_pEdit->RenameFile(); break;
            case IDC_BUTTON_HELP:   g_pEdit->ShowHelp(); break;
            case IDC_BUTTON_EXIT:   EndDialog(hDlg, 0); break;
            }
        }
        break;

    case WM_NOTIFY:
    {
        LPNMHDR pnm = (LPNMHDR)lParam;
        if (pnm->code == NM_DBLCLK)
        {
            g_pEdit->bActiveLeft = (pnm->idFrom == IDC_LIST_LEFT);
            if (pnm->idFrom == IDC_LIST_LEFT)  g_pEdit->OpenLeft();
            if (pnm->idFrom == IDC_LIST_RIGHT) g_pEdit->OpenRight();
        }
        else if (pnm->code == LVN_ITEMCHANGED)
        {
            // Обновляем активную панель при выборе элемента
            LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
            if (pnmv->iItem != -1 && (pnmv->uChanged & LVIF_STATE))
            {
                if (pnmv->uNewState & LVIS_SELECTED)
                {
                    g_pEdit->bActiveLeft = (pnm->idFrom == IDC_LIST_LEFT);
                    // Обновляем атрибуты выбранного файла
                    g_pEdit->UpdateFileAttributes();
                }
            }
        }
    }
    break;

    case WM_CLOSE:
        // Восстанавливаем старые процедуры перед закрытием
        if (g_OldLeftProc)
            SetWindowLongPtr(g_pEdit->hListLeft, GWLP_WNDPROC, (LONG_PTR)g_OldLeftProc);
        if (g_OldRightProc)
            SetWindowLongPtr(g_pEdit->hListRight, GWLP_WNDPROC, (LONG_PTR)g_OldRightProc);

        EndDialog(hDlg, 0);
        delete g_pEdit;
        return TRUE;
    }
    return FALSE;
}

// === ИСПРАВЛЕННАЯ подпрограмма ListView ===
LRESULT CALLBACK ListViewSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Edit* pEdit = (Edit*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (msg == WM_KEYDOWN && pEdit)
    {
        switch (wParam)
        {
        case VK_F1:
            pEdit->ShowHelp();
            return 0;

        case VK_F2:
            pEdit->RenameFile();
            return 0;

        case VK_F5:
            pEdit->CopyBetweenPanels();
            return 0;

        case VK_F6:
            pEdit->MoveSelected();
            return 0;

        case VK_F7:
            pEdit->CreateNew();
            return 0;

        case VK_F8:
            pEdit->DeleteSelected();
            return 0;

        case VK_RETURN:
            // Enter - открываем выделенный элемент
            if (hWnd == pEdit->hListLeft)
                pEdit->OpenLeft();
            else if (hWnd == pEdit->hListRight)
                pEdit->OpenRight();
            return 0;

        case VK_BACK:
        {
            // Backspace - идем на уровень вверх
            bool isLeft = (hWnd == pEdit->hListLeft);
            HWND hStatic = isLeft ? pEdit->hStaticLeft : pEdit->hStaticRight;
            HWND hList = isLeft ? pEdit->hListLeft : pEdit->hListRight;

            wchar_t curPath[MAX_PATH] = { 0 };
            GetWindowTextW(hStatic, curPath, MAX_PATH);
            std::wstring path = curPath;

            // Убираем последнюю папку из пути (если не корень)
            if (path.length() > 3 && path.back() == L'\\')
                path.pop_back();

            size_t pos = path.find_last_of(L'\\');
            if (pos != std::wstring::npos && path.length() > 3)
            {
                path = path.substr(0, pos + 1);
                pEdit->RefreshFolder(hList, hStatic, path);
                pEdit->ClearFileAttributes();
            }
            return 0;
        }

        case VK_TAB:
            // Переключаем активную панель
            pEdit->bActiveLeft = (hWnd == pEdit->hListRight);
            SetFocus(pEdit->bActiveLeft ? pEdit->hListLeft : pEdit->hListRight);
            return 0;

        case VK_LEFT:
            // Переключаем фокус на другую панель (влево)
            if (hWnd == pEdit->hListRight)
            {
                pEdit->bActiveLeft = true;
                SetFocus(pEdit->hListLeft);
                pEdit->UpdateFileAttributes();
            }
            return 0;

        case VK_RIGHT:
            // Переключаем фокус на другую панель (вправо)
            if (hWnd == pEdit->hListLeft)
            {
                pEdit->bActiveLeft = false;
                SetFocus(pEdit->hListRight);
                pEdit->UpdateFileAttributes();
            }
            return 0;

        case VK_SPACE:
            // Пробел - показать атрибуты файла
            pEdit->UpdateFileAttributes();
            return 0;
        }
    }

    // Вызываем оригинальную процедуру
    WNDPROC oldProc = (hWnd == pEdit->hListLeft) ? g_OldLeftProc : g_OldRightProc;
    if (oldProc)
        return CallWindowProc(oldProc, hWnd, msg, wParam, lParam);

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int)
{
    InitCommonControls();
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MAIN), nullptr, DialogProc, (LPARAM)new Edit());
    return 0;
}