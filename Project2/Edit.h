// Edit.h
#pragma once
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>

class Edit
{
public:
    HWND     hDlg = nullptr;          // главное окно-диалог
    HINSTANCE hInst = nullptr;

    // элементы управления
    HWND hComboLeft, hComboRight;
    HWND hListLeft, hListRight;
    HWND hStaticLeft, hStaticRight;

    bool bActiveLeft = true;          // какая панель активна

    Edit() = default;

    void InitControls();              // вызывается из WM_INITDIALOG
    void RefreshDrives(HWND hCombo);
    void RefreshFolder(HWND hList, HWND hStatic, std::wstring path);

    void OpenLeft() { bActiveLeft = true;  OpenFolder(true); }
    void OpenRight() { bActiveLeft = false; OpenFolder(false); }

    void OpenFolder(bool left);
    void CopyBetweenPanels();
    void DeleteSelected();
    void CreateNew();                 // Создание файла/папки
    void MoveSelected();              // Перемещение файла/папки
};

// Сделаем функцию глобальной, а не членом класса
INT_PTR CALLBACK CreateDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);