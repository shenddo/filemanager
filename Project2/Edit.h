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

    // элементы для атрибутов
    HWND hAttrName = nullptr, hAttrSize = nullptr, hAttrCreated = nullptr;
    HWND hAttrModified = nullptr, hAttrType = nullptr, hAttrPath = nullptr;

    bool bActiveLeft = true;          // какая панель активна
    std::wstring currentSelectedPath; // текущий выбранный файл

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
    void ShowHelp();                  // Показать справку
    void UpdateFileAttributes();      // Обновить атрибуты выбранного файла
    void ClearFileAttributes();       // Очистить атрибуты файла
    void RenameFile();                // Переименовать файл
};

// Глобальные функции для диалогов
INT_PTR CALLBACK CreateDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK HelpDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RenameDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);