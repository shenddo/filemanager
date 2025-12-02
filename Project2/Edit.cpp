// Edit.cpp
#include "Edit.h"
#include "resource.h"
#include <shlwapi.h>
#include <shellapi.h>
#include <ShObjIdl_core.h>
#include <locale>
#include <codecvt>
#include <string>

#pragma comment(lib, "shlwapi.lib")

// Вспомогательная структура для передачи данных в диалог
struct CreateDialogData
{
    bool isFile;
    wchar_t filename[MAX_PATH];
};

// ============ ДИАЛОГ СОЗДАНИЯ ============
// ============ ДИАЛОГ СОЗДАНИЯ ============
// ============ ДИАЛОГ СОЗДАНИЯ ============
INT_PTR CALLBACK CreateDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
    {
        // Сохраняем указатель на данные в GWLP_USERDATA
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);

        // Установить фокус на поле ввода
        SetFocus(GetDlgItem(hDlg, IDC_EDIT_FILENAME));

        // По умолчанию выбрано "Файл"
        CheckRadioButton(hDlg, IDC_RADIO_FILE, IDC_RADIO_FOLDER, IDC_RADIO_FILE);
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            // Получаем указатель на данные из GWLP_USERDATA
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

            // Сохраняем результат в структуре данных
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

void Edit::InitControls()
{
    hComboLeft = GetDlgItem(hDlg, IDC_COMBO_LEFT);
    hComboRight = GetDlgItem(hDlg, IDC_COMBO_RIGHT);
    hListLeft = GetDlgItem(hDlg, IDC_LIST_LEFT);
    hListRight = GetDlgItem(hDlg, IDC_LIST_RIGHT);
    hStaticLeft = GetDlgItem(hDlg, IDC_STATIC_LEFT);
    hStaticRight = GetDlgItem(hDlg, IDC_STATIC_RIGHT);

    // колонка "Имя"
    LVCOLUMN lvc = { 0 };
    lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.cx = 310;
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
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    ListView_SetExtendedListViewStyle(hListRight,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    RefreshDrives(hComboLeft);
    RefreshDrives(hComboRight);

    // Устанавливаем текущий диск C: по умолчанию
    int count = (int)SendMessage(hComboLeft, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; i++)
    {
        wchar_t drive[MAX_PATH] = { 0 };
        SendMessage(hComboLeft, CB_GETLBTEXT, i, (LPARAM)drive);
        if (wcsstr(drive, L"C:\\") != NULL)
        {
            SendMessage(hComboLeft, CB_SETCURSEL, i, 0);
            break;
        }
    }

    for (int i = 0; i < count; i++)
    {
        wchar_t drive[MAX_PATH] = { 0 };
        SendMessage(hComboRight, CB_GETLBTEXT, i, (LPARAM)drive);
        if (wcsstr(drive, L"C:\\") != NULL)
        {
            SendMessage(hComboRight, CB_SETCURSEL, i, 0);
            break;
        }
    }

    RefreshFolder(hListLeft, hStaticLeft, L"C:\\");
    RefreshFolder(hListRight, hStaticRight, L"C:\\");
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
        // Удаляем последнюю папку из пути
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

    // Получаем hInst из текущего модуля (самый надежный способ)
    HINSTANCE hInst = GetModuleHandle(NULL);

    // Открываем диалог создания
    INT_PTR result = DialogBoxParam(hInst,
        MAKEINTRESOURCE(IDD_CREATE_DIALOG),
        hDlg,
        CreateDlgProc,
        (LPARAM)&data);

    if (result == IDOK)
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
            // Создаем файл
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
            // Создаем папку
            success = CreateDirectoryW(fullPath.c_str(), NULL);
        }

        if (success)
        {
            // Обновляем список
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

// ============ ПЕРЕМЕЩЕНИЕ ФАЙЛА/ПАПКИ ============
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

    // Формируем полные пути
    std::wstring srcPath = src;
    std::wstring dstPath = dst;

    if (!srcPath.empty() && srcPath.back() != L'\\') srcPath += L"\\";
    if (!dstPath.empty() && dstPath.back() != L'\\') dstPath += L"\\";

    srcPath += name;
    dstPath += name;

    // Проверяем, является ли исходный элемент папкой
    DWORD attr = GetFileAttributesW(srcPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        MessageBoxW(hDlg, L"Не удалось получить атрибуты файла!", L"Ошибка", MB_ICONERROR);
        return;
    }

    // Проверяем, существует ли уже файл в целевой папке
    if (GetFileAttributesW(dstPath.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        if (MessageBoxW(hDlg, L"Целевой файл уже существует. Перезаписать?",
            L"Перемещение", MB_YESNO | MB_ICONQUESTION) != IDYES)
        {
            return;
        }
    }

    // Перемещаем файл/папку
    BOOL success = FALSE;

    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
        // Для папок используем MoveFile
        success = MoveFileW(srcPath.c_str(), dstPath.c_str());
    }
    else
    {
        // Для файлов также используем MoveFile
        success = MoveFileW(srcPath.c_str(), dstPath.c_str());
    }

    if (success)
    {
        // Обновляем обе панели
        std::wstring srcFolder = src;
        if (srcFolder.back() != L'\\') srcFolder += L"\\";
        std::wstring dstFolder = dst;
        if (dstFolder.back() != L'\\') dstFolder += L"\\";

        RefreshFolder(hSrcList, hSrcStatic, srcFolder);
        RefreshFolder(hDstList, hDstStatic, dstFolder);
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