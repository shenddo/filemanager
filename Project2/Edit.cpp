// Edit.cpp
#include "Edit.h"
#include "resource.h"
#include <shlwapi.h>
#include <shellapi.h>
#include <ShObjIdl_core.h>
#include <locale>
#include <codecvt>
#include <string>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "shlwapi.lib")

using namespace std;

// Вспомогательная структура для передачи данных в диалог
struct CreateDialogData
{
    bool isFile;
    wchar_t filename[MAX_PATH];
};

struct RenameDialogData
{
    wchar_t oldName[MAX_PATH];
    wchar_t newName[MAX_PATH];
};

// ============ ФОРМАТИРОВАНИЕ РАЗМЕРА ФАЙЛА ============
std::wstring FormatFileSize(ULONGLONG size)
{
    wstringstream ss;

    if (size < 1024)
        ss << size << L" Б";
    else if (size < 1024 * 1024)
        ss << fixed << setprecision(1) << (size / 1024.0) << L" КБ";
    else if (size < 1024 * 1024 * 1024)
        ss << fixed << setprecision(1) << (size / (1024.0 * 1024.0)) << L" МБ";
    else
        ss << fixed << setprecision(1) << (size / (1024.0 * 1024.0 * 1024.0)) << L" ГБ";

    return ss.str();
}

// ============ ФОРМАТИРОВАНИЕ ДАТЫ ============
std::wstring FormatFileTime(FILETIME ft)
{
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);

    wchar_t buffer[256];
    swprintf_s(buffer, L"%02d.%02d.%04d %02d:%02d",
        st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);

    return wstring(buffer);
}

// ============ ДИАЛОГ СОЗДАНИЯ ============
INT_PTR CALLBACK CreateDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
        SetFocus(GetDlgItem(hDlg, IDC_EDIT_FILENAME));
        CheckRadioButton(hDlg, IDC_RADIO_FILE, IDC_RADIO_FOLDER, IDC_RADIO_FILE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            CreateDialogData* pData = (CreateDialogData*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
            if (!pData)
            {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }

            wchar_t filename[MAX_PATH];
            GetDlgItemTextW(hDlg, IDC_EDIT_FILENAME, filename, MAX_PATH);

            if (wcslen(filename) == 0)
            {
                MessageBoxW(hDlg, L"Введите имя файла или папки", L"Ошибка", MB_ICONERROR);
                return TRUE;
            }

            bool isFile = (IsDlgButtonChecked(hDlg, IDC_RADIO_FILE) == BST_CHECKED);

            pData->isFile = isFile;
            wcscpy_s(pData->filename, MAX_PATH, filename);

            EndDialog(hDlg, IDOK);
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

// ============ ДИАЛОГ СПРАВКИ ============
// ============ ДИАЛОГ СПРАВКИ С RICHEDIT ============
// ============ ДИАЛОГ СПРАВКИ ============
INT_PTR CALLBACK HelpDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
    {
        // Используем обычный статический текст вместо RichEdit для простоты
        HWND hStatic = GetDlgItem(hDlg, -1); // Первый статик контрол в диалоге

        wstring helpText =
            L"ФАЙЛОВЫЙ МЕНЕДЖЕР - СПРАВКА\n\n"
            L"ОСНОВНЫЕ ВОЗМОЖНОСТИ:\n"
            L"• Просмотр файлов и папок на дисках\n"
            L"• Копирование, перемещение, удаление\n"
            L"• Создание новых файлов и папок\n"
            L"• Переименование файлов и папок\n"
            L"• Просмотр атрибутов файлов\n\n"
            L"ГОРЯЧИЕ КЛАВИШИ:\n"
            L"F1 - Открыть справку\n"
            L"F2 - Переименовать выделенный элемент\n"
            L"F5 - Копировать в другую панель\n"
            L"F6 - Переместить в другую панель\n"
            L"F7 - Создать файл/папку\n"
            L"F8 - Удалить выделенный элемент\n"
            L"Enter - Открыть файл или папку\n"
            L"Backspace - Перейти на уровень выше\n"
            L"Tab - Переключить активную панель\n"
            L"← → - Переключение между панелями\n"
            L"Пробел - Показать атрибуты файла\n\n"
            L"УПРАВЛЕНИЕ МЫШЬЮ:\n"
            L"• Двойной клик - открыть файл/папку\n"
            L"• Выбор диска - через выпадающий список\n"
            L"• Выбор файла - щелчок мыши\n"
            L"• Атрибуты файла отображаются справа\n\n"
            L"КНОПКИ УПРАВЛЕНИЯ:\n"
            L"Копировать - F5\n"
            L"Переместить - F6\n"
            L"Создать - F7\n"
            L"Удалить - F8\n"
            L"Переименовать - F2\n"
            L"Справка - F1\n\n"
            L"ПАНЕЛЬ АТРИБУТОВ:\n"
            L"Показывает информацию о выбранном файле:\n"
            L"• Имя файла\n"
            L"• Тип (файл/папка)\n"
            L"• Размер (для файлов)\n"
            L"• Дата создания\n"
            L"• Дата последнего изменения\n"
            L"• Полный путь\n\n"
            L"ВЕРСИЯ:\n"
            L"Файловый менеджер 1.0\n"
            L"Курсовой проект по программированию";

        SetWindowTextW(hStatic, helpText.c_str());
        return TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
    }
    return FALSE;
}

// ============ ДИАЛОГ ПЕРЕИМЕНОВАНИЯ ============
INT_PTR CALLBACK RenameDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
    {
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
        RenameDialogData* pData = (RenameDialogData*)lParam;

        wchar_t title[256];
        swprintf_s(title, L"Переименовать: %s", pData->oldName);
        SetWindowTextW(hDlg, title);

        SetDlgItemTextW(hDlg, IDC_RENAME_EDIT, pData->oldName);
        SendDlgItemMessageW(hDlg, IDC_RENAME_EDIT, EM_SETSEL, 0, -1);
        SetFocus(GetDlgItem(hDlg, IDC_RENAME_EDIT));

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            RenameDialogData* pData = (RenameDialogData*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
            if (!pData)
            {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }

            wchar_t newName[MAX_PATH];
            GetDlgItemTextW(hDlg, IDC_RENAME_EDIT, newName, MAX_PATH);

            if (wcslen(newName) == 0)
            {
                MessageBoxW(hDlg, L"Введите новое имя", L"Ошибка", MB_ICONERROR);
                return TRUE;
            }

            wcscpy_s(pData->newName, MAX_PATH, newName);
            EndDialog(hDlg, IDOK);
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

void Edit::InitControls()
{
    hComboLeft = GetDlgItem(hDlg, IDC_COMBO_LEFT);
    hComboRight = GetDlgItem(hDlg, IDC_COMBO_RIGHT);
    hListLeft = GetDlgItem(hDlg, IDC_LIST_LEFT);
    hListRight = GetDlgItem(hDlg, IDC_LIST_RIGHT);
    hStaticLeft = GetDlgItem(hDlg, IDC_STATIC_LEFT);
    hStaticRight = GetDlgItem(hDlg, IDC_STATIC_RIGHT);

    // Элементы для атрибутов
    hAttrName = GetDlgItem(hDlg, IDC_ATTR_NAME);
    hAttrSize = GetDlgItem(hDlg, IDC_ATTR_SIZE);
    hAttrCreated = GetDlgItem(hDlg, IDC_ATTR_CREATED);
    hAttrModified = GetDlgItem(hDlg, IDC_ATTR_MODIFIED);
    hAttrType = GetDlgItem(hDlg, IDC_ATTR_TYPE);
    hAttrPath = GetDlgItem(hDlg, IDC_ATTR_PATH);

    // колонка "Имя" - одна колонка на всю ширину
    LVCOLUMN lvc = { 0 };
    lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.cx = 265; // Начальная ширина колонки
    lvc.pszText = (LPWSTR)L"Имя файла";
    lvc.iSubItem = 0;
    ListView_InsertColumn(hListLeft, 0, &lvc);
    ListView_InsertColumn(hListRight, 0, &lvc);

    // системные иконки
    SHFILEINFO shfi;
    HIMAGELIST hImg = (HIMAGELIST)SHGetFileInfo(L"C:\\", 0, &shfi,
        sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

    ListView_SetImageList(hListLeft, hImg, LVSIL_SMALL);
    ListView_SetImageList(hListRight, hImg, LVSIL_SMALL);

    ListView_SetExtendedListViewStyle(hListLeft,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    ListView_SetExtendedListViewStyle(hListRight,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

    RefreshDrives(hComboLeft);
    RefreshDrives(hComboRight);

    // Устанавливаем C: по умолчанию
    int count = (int)SendMessage(hComboLeft, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; i++)
    {
        wchar_t drive[MAX_PATH] = { 0 };
        SendMessage(hComboLeft, CB_GETLBTEXT, i, (LPARAM)drive);
        if (wcsncmp(drive, L"C:\\", 3) == 0)
        {
            SendMessage(hComboLeft, CB_SETCURSEL, i, 0);
            break;
        }
    }

    for (int i = 0; i < count; i++)
    {
        wchar_t drive[MAX_PATH] = { 0 };
        SendMessage(hComboRight, CB_GETLBTEXT, i, (LPARAM)drive);
        if (wcsncmp(drive, L"C:\\", 3) == 0)
        {
            SendMessage(hComboRight, CB_SETCURSEL, i, 0);
            break;
        }
    }

    RefreshFolder(hListLeft, hStaticLeft, L"C:\\");
    RefreshFolder(hListRight, hStaticRight, L"C:\\");

    // Очищаем атрибуты при запуске
    ClearFileAttributes();

    // Автоматически подгоняем ширину столбцов
    AutoSizeColumns();
}
// ============ АВТОМАТИЧЕСКОЕ ИЗМЕНЕНИЕ ШИРИНЫ СТОЛБЦОВ ============
void Edit::AutoSizeColumns()
{
    // Автоматически подгоняем ширину столбца под ширину ListView
    RECT rc;
    GetClientRect(hListLeft, &rc);
    int width = rc.right - rc.left - 5; // Минус небольшой отступ
    if (width > 0)
    {
        ListView_SetColumnWidth(hListLeft, 0, width);
    }

    GetClientRect(hListRight, &rc);
    width = rc.right - rc.left - 5;
    if (width > 0)
    {
        ListView_SetColumnWidth(hListRight, 0, width);
    }
}
void Edit::RefreshDrives(HWND hCombo)
{
    SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
    wchar_t buf[2048] = {};
    GetLogicalDriveStrings(2047, buf);
    for (wchar_t* p = buf; *p; p += wcslen(p) + 1)
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)p);
}

void Edit::RefreshFolder(HWND hList, HWND hStatic, std::wstring path)
{
    ListView_DeleteAllItems(hList);

    if (path.back() != L'\\') path += L"\\";
    std::wstring search = path + L"*";

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(search.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        SetWindowTextW(hStatic, path.c_str());
        return;
    }

    int index = 0;
    do
    {
        if (fd.cFileName[0] == L'.' && fd.cFileName[1] == 0) continue; // "."

        LVITEMW item = {};
        item.mask = LVIF_TEXT | LVIF_IMAGE;
        item.iItem = index++;
        item.pszText = fd.cFileName;

        std::wstring full = path + fd.cFileName;
        SHFILEINFO shfi;
        if (SHGetFileInfoW(full.c_str(), 0, &shfi, sizeof(shfi),
            SHGFI_SYSICONINDEX | SHGFI_SMALLICON))
            item.iImage = shfi.iIcon;

        // ".."
        if (wcscmp(fd.cFileName, L"..") == 0)
        {
            if (path.length() <= 3) continue;   // корень диска — не показываем ".."
            item.iImage = 0;
        }

        ListView_InsertItem(hList, &item);
    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
    SetWindowTextW(hStatic, path.c_str());

    // Очищаем атрибуты при смене папки
    ClearFileAttributes();
}

void Edit::OpenFolder(bool left)
{
    HWND hList = left ? hListLeft : hListRight;
    HWND hStatic = left ? hStaticLeft : hStaticRight;

    int sel = ListView_GetSelectedCount(hList) ? ListView_GetNextItem(hList, -1, LVNI_SELECTED) : -1;
    if (sel == -1) return;

    wchar_t name[MAX_PATH] = { 0 };
    ListView_GetItemText(hList, sel, 0, name, MAX_PATH);

    wchar_t curPath[MAX_PATH] = { 0 };
    GetWindowTextW(hStatic, curPath, MAX_PATH);
    std::wstring path = curPath;

    // === Логика ".." ===
    if (wcscmp(name, L"..") == 0)
    {
        if (path.back() == L'\\') path.pop_back();
        size_t pos = path.find_last_of(L'\\');
        if (pos != std::wstring::npos)
            path = path.substr(0, pos + 1);
        else
            path = L"C:\\";

        RefreshFolder(hList, hStatic, path);
        return;
    }

    // === Обычная папка или файл ===
    if (path.back() != L'\\') path += L"\\";
    path += name;

    DWORD attr = GetFileAttributesW(path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) return;

    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
        path += L"\\";
        RefreshFolder(hList, hStatic, path);
    }
    else
    {
        // Открываем файл
        ShellExecuteW(hDlg, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
}

void Edit::CopyBetweenPanels()
{
    HWND hSrcList = bActiveLeft ? hListLeft : hListRight;
    HWND hDstList = bActiveLeft ? hListRight : hListLeft;
    HWND hSrcStatic = bActiveLeft ? hStaticLeft : hStaticRight;
    HWND hDstStatic = bActiveLeft ? hStaticRight : hStaticLeft;

    int sel = ListView_GetNextItem(hSrcList, -1, LVNI_SELECTED);
    if (sel == -1) { MessageBoxW(hDlg, L"Выделите файл/папку!", L"Копирование", MB_ICONWARNING); return; }

    wchar_t name[MAX_PATH];
    ListView_GetItemText(hSrcList, sel, 0, name, MAX_PATH);

    // Пропускаем ".."
    if (wcscmp(name, L"..") == 0)
    {
        MessageBoxW(hDlg, L"Нельзя копировать '..'!", L"Копирование", MB_ICONWARNING);
        return;
    }

    wchar_t src[MAX_PATH], dst[MAX_PATH];
    GetWindowTextW(hSrcStatic, src, MAX_PATH);
    GetWindowTextW(hDstStatic, dst, MAX_PATH);

    if (src[wcslen(src) - 1] != L'\\') wcscat_s(src, L"\\");
    if (dst[wcslen(dst) - 1] != L'\\') wcscat_s(dst, L"\\");
    wcscat_s(src, name);
    wcscat_s(dst, name);

    SHFILEINFO shfi;
    SHGetFileInfoW(src, 0, &shfi, sizeof(shfi), SHGFI_ATTRIBUTES);
    if (shfi.dwAttributes & SFGAO_SYSTEM)
    {
        MessageBoxW(hDlg, L"Системный файл нельзя копировать!", L"Ошибка", MB_ICONSTOP);
        return;
    }

    if (CopyFileW(src, dst, FALSE))
    {
        std::wstring p(dst);
        size_t pos = p.find_last_of(L'\\');
        if (pos != std::wstring::npos) p.erase(pos);
        else p.clear();
        RefreshFolder(hDstList, hDstStatic, p + L"\\");
    }
    else
        MessageBoxW(hDlg, L"Не удалось скопировать!", L"Ошибка", MB_ICONERROR);
}

void Edit::DeleteSelected()
{
    HWND hList = bActiveLeft ? hListLeft : hListRight;
    HWND hStatic = bActiveLeft ? hStaticLeft : hStaticRight;

    int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
    if (sel == -1) return;

    wchar_t name[MAX_PATH] = { 0 };
    ListView_GetItemText(hList, sel, 0, name, MAX_PATH);

    // Пропускаем ".."
    if (wcscmp(name, L"..") == 0)
    {
        MessageBoxW(hDlg, L"Нельзя удалить '..'!", L"Удаление", MB_ICONWARNING);
        return;
    }

    wchar_t fullpath[MAX_PATH] = { 0 };
    GetWindowTextW(hStatic, fullpath, MAX_PATH);
    if (fullpath[wcslen(fullpath) - 1] != L'\\') wcscat_s(fullpath, L"\\");
    wcscat_s(fullpath, name);

    if (MessageBoxW(hDlg, (L"Удалить \"" + std::wstring(name) + L"\"?").c_str(),
        L"Удаление", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        DWORD attr = GetFileAttributesW(fullpath);
        BOOL success = FALSE;

        if (attr & FILE_ATTRIBUTE_DIRECTORY)
            success = RemoveDirectoryW(fullpath);
        else
            success = DeleteFileW(fullpath);

        if (success)
        {
            wchar_t currentPath[MAX_PATH] = { 0 };
            GetWindowTextW(hStatic, currentPath, MAX_PATH);
            std::wstring folder = currentPath;
            if (folder.back() != L'\\') folder += L"\\";
            RefreshFolder(hList, hStatic, folder);

            // Очищаем атрибуты
            ClearFileAttributes();
        }
        else
        {
            MessageBoxW(hDlg, L"Не удалось удалить!", L"Ошибка", MB_ICONERROR);
        }
    }
}

void Edit::CreateNew()
{
    CreateDialogData data = { true, L"" };

    HINSTANCE hInst = GetModuleHandle(NULL);

    if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CREATE_DIALOG), hDlg,
        CreateDlgProc, (LPARAM)&data) == IDOK)
    {
        HWND hList = bActiveLeft ? hListLeft : hListRight;
        HWND hStatic = bActiveLeft ? hStaticLeft : hStaticRight;

        wchar_t currentPath[MAX_PATH] = { 0 };
        GetWindowTextW(hStatic, currentPath, MAX_PATH);
        std::wstring fullPath = currentPath;

        if (fullPath.empty() || fullPath.back() != L'\\')
            fullPath += L"\\";

        fullPath += data.filename;

        BOOL success = FALSE;

        if (data.isFile)
        {
            HANDLE hFile = CreateFileW(fullPath.c_str(),
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_NEW,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFile);
                success = TRUE;
            }
        }
        else
        {
            success = CreateDirectoryW(fullPath.c_str(), NULL);
        }

        if (success)
        {
            std::wstring folder = currentPath;
            if (folder.empty() || folder.back() != L'\\')
                folder += L"\\";
            RefreshFolder(hList, hStatic, folder);
        }
        else
        {
            DWORD error = GetLastError();
            if (error == ERROR_FILE_EXISTS || error == ERROR_ALREADY_EXISTS)
            {
                MessageBoxW(hDlg,
                    L"Файл или папка с таким именем уже существует!",
                    L"Ошибка",
                    MB_ICONERROR);
            }
            else
            {
                MessageBoxW(hDlg,
                    L"Не удалось создать объект!",
                    L"Ошибка",
                    MB_ICONERROR);
            }
        }
    }
}

void Edit::MoveSelected()
{
    HWND hSrcList = bActiveLeft ? hListLeft : hListRight;
    HWND hDstList = bActiveLeft ? hListRight : hListLeft;
    HWND hSrcStatic = bActiveLeft ? hStaticLeft : hStaticRight;
    HWND hDstStatic = bActiveLeft ? hStaticRight : hStaticLeft;

    int sel = ListView_GetNextItem(hSrcList, -1, LVNI_SELECTED);
    if (sel == -1)
    {
        MessageBoxW(hDlg, L"Выделите файл/папку для перемещения!",
            L"Перемещение", MB_ICONWARNING);
        return;
    }

    wchar_t name[MAX_PATH] = { 0 };
    ListView_GetItemText(hSrcList, sel, 0, name, MAX_PATH);

    // Пропускаем ".."
    if (wcscmp(name, L"..") == 0)
    {
        MessageBoxW(hDlg, L"Нельзя переместить '..'!", L"Перемещение", MB_ICONWARNING);
        return;
    }

    wchar_t src[MAX_PATH] = { 0 };
    wchar_t dst[MAX_PATH] = { 0 };

    GetWindowTextW(hSrcStatic, src, MAX_PATH);
    GetWindowTextW(hDstStatic, dst, MAX_PATH);

    std::wstring srcPath = src;
    std::wstring dstPath = dst;

    if (!srcPath.empty() && srcPath.back() != L'\\') srcPath += L"\\";
    if (!dstPath.empty() && dstPath.back() != L'\\') dstPath += L"\\";

    srcPath += name;
    dstPath += name;

    DWORD attr = GetFileAttributesW(srcPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        MessageBoxW(hDlg, L"Не удалось получить атрибуты файла!", L"Ошибка", MB_ICONERROR);
        return;
    }

    if (GetFileAttributesW(dstPath.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        if (MessageBoxW(hDlg, L"Целевой файл уже существует. Перезаписать?",
            L"Перемещение", MB_YESNO | MB_ICONQUESTION) != IDYES)
        {
            return;
        }
    }

    BOOL success = FALSE;

    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
        success = MoveFileW(srcPath.c_str(), dstPath.c_str());
    }
    else
    {
        success = MoveFileW(srcPath.c_str(), dstPath.c_str());
    }

    if (success)
    {
        std::wstring srcFolder = src;
        if (srcFolder.back() != L'\\') srcFolder += L"\\";
        std::wstring dstFolder = dst;
        if (dstFolder.back() != L'\\') dstFolder += L"\\";

        RefreshFolder(hSrcList, hSrcStatic, srcFolder);
        RefreshFolder(hDstList, hDstStatic, dstFolder);

        // Очищаем атрибуты
        ClearFileAttributes();
    }
    else
    {
        DWORD error = GetLastError();
        if (error == ERROR_ACCESS_DENIED)
        {
            MessageBoxW(hDlg, L"Нет прав для перемещения файла!", L"Ошибка", MB_ICONERROR);
        }
        else
        {
            MessageBoxW(hDlg, L"Не удалось переместить файл!", L"Ошибка", MB_ICONERROR);
        }
    }
}

// ============ ПОКАЗАТЬ СПРАВКУ ============
void Edit::ShowHelp()
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    DialogBox(hInst, MAKEINTRESOURCE(IDD_HELP_DIALOG), hDlg, HelpDlgProc);
}

// ============ ОЧИСТКА АТРИБУТОВ ФАЙЛА ============
void Edit::ClearFileAttributes()
{
    if (hAttrName) SetWindowTextW(hAttrName, L"");
    if (hAttrSize) SetWindowTextW(hAttrSize, L"");
    if (hAttrCreated) SetWindowTextW(hAttrCreated, L"");
    if (hAttrModified) SetWindowTextW(hAttrModified, L"");
    if (hAttrType) SetWindowTextW(hAttrType, L"");
    if (hAttrPath) SetWindowTextW(hAttrPath, L"");
}

// ============ ОБНОВЛЕНИЕ АТРИБУТОВ ФАЙЛА ============
void Edit::UpdateFileAttributes()
{
    HWND hList = bActiveLeft ? hListLeft : hListRight;
    HWND hStatic = bActiveLeft ? hStaticLeft : hStaticRight;

    int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
    if (sel == -1)
    {
        ClearFileAttributes();
        return;
    }

    wchar_t name[MAX_PATH] = { 0 };
    ListView_GetItemText(hList, sel, 0, name, MAX_PATH);

    // Пропускаем ".."
    if (wcscmp(name, L"..") == 0)
    {
        ClearFileAttributes();
        return;
    }

    wchar_t fullpath[MAX_PATH] = { 0 };
    GetWindowTextW(hStatic, fullpath, MAX_PATH);
    if (fullpath[wcslen(fullpath) - 1] != L'\\')
        wcscat_s(fullpath, L"\\");
    wcscat_s(fullpath, name);

    currentSelectedPath = fullpath;

    // Получаем информацию о файле
    WIN32_FILE_ATTRIBUTE_DATA fileAttr;
    if (!GetFileAttributesExW(fullpath, GetFileExInfoStandard, &fileAttr))
    {
        ClearFileAttributes();
        return;
    }

    // Имя файла
    SetWindowTextW(hAttrName, name);

    // Тип файла
    if (fileAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        SetWindowTextW(hAttrType, L"Папка");
    else
        SetWindowTextW(hAttrType, L"Файл");

    // Размер файла
    if (!(fileAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        ULONGLONG fileSize = ((ULONGLONG)fileAttr.nFileSizeHigh << 32) | fileAttr.nFileSizeLow;
        wstring sizeStr = FormatFileSize(fileSize);
        SetWindowTextW(hAttrSize, sizeStr.c_str());
    }
    else
    {
        SetWindowTextW(hAttrSize, L"---");
    }

    // Дата создания
    wstring createdStr = FormatFileTime(fileAttr.ftCreationTime);
    SetWindowTextW(hAttrCreated, createdStr.c_str());

    // Дата изменения
    wstring modifiedStr = FormatFileTime(fileAttr.ftLastWriteTime);
    SetWindowTextW(hAttrModified, modifiedStr.c_str());

    // Путь
    SetWindowTextW(hAttrPath, fullpath);
}

// ============ ПЕРЕИМЕНОВАНИЕ ФАЙЛА ============
void Edit::RenameFile()
{
    HWND hList = bActiveLeft ? hListLeft : hListRight;
    HWND hStatic = bActiveLeft ? hStaticLeft : hStaticRight;

    int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
    if (sel == -1)
    {
        MessageBoxW(hDlg, L"Выделите файл или папку для переименования!",
            L"Переименование", MB_ICONWARNING);
        return;
    }

    wchar_t oldName[MAX_PATH] = { 0 };
    ListView_GetItemText(hList, sel, 0, oldName, MAX_PATH);

    // Пропускаем ".."
    if (wcscmp(oldName, L"..") == 0)
    {
        MessageBoxW(hDlg, L"Нельзя переименовать '..'!", L"Переименование", MB_ICONWARNING);
        return;
    }

    RenameDialogData data = { 0 };
    wcscpy_s(data.oldName, MAX_PATH, oldName);

    HINSTANCE hInst = GetModuleHandle(NULL);

    if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RENAME_DIALOG), hDlg,
        RenameDlgProc, (LPARAM)&data) == IDOK)
    {
        // Формируем старый и новый пути
        wchar_t oldPath[MAX_PATH] = { 0 };
        wchar_t newPath[MAX_PATH] = { 0 };

        GetWindowTextW(hStatic, oldPath, MAX_PATH);
        wcscpy_s(newPath, MAX_PATH, oldPath);

        if (oldPath[wcslen(oldPath) - 1] != L'\\')
        {
            wcscat_s(oldPath, L"\\");
            wcscat_s(newPath, L"\\");
        }

        wcscat_s(oldPath, oldName);
        wcscat_s(newPath, data.newName);

        // Переименовываем
        if (MoveFileW(oldPath, newPath))
        {
            // Обновляем список
            wchar_t currentPath[MAX_PATH] = { 0 };
            GetWindowTextW(hStatic, currentPath, MAX_PATH);
            std::wstring folder = currentPath;
            if (folder.back() != L'\\') folder += L"\\";
            RefreshFolder(hList, hStatic, folder);

            // Обновляем атрибуты
            UpdateFileAttributes();
        }
        else
        {
            DWORD error = GetLastError();
            if (error == ERROR_ACCESS_DENIED)
            {
                MessageBoxW(hDlg, L"Нет прав для переименования файла!",
                    L"Ошибка", MB_ICONERROR);
            }
            else if (error == ERROR_FILE_EXISTS || error == ERROR_ALREADY_EXISTS)
            {
                MessageBoxW(hDlg, L"Файл с таким именем уже существует!",
                    L"Ошибка", MB_ICONERROR);
            }
            else
            {
                MessageBoxW(hDlg, L"Не удалось переименовать файл!",
                    L"Ошибка", MB_ICONERROR);
            }
        }
    }
}