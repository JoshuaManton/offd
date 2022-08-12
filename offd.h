#pragma comment(lib, "Ole32.lib")

#include <stdio.h>
#include <windows.h>
#include <shobjidl.h>
#include <stdint.h>
#include <assert.h>

extern "C" {

enum OFFD_FLAGS {
    OFFD_MULTI_SELECT = 1 << 0,
    OFFD_ALLOW_NONEXISTENT = 1 << 1,
};

struct OFFD_Result {
    int64_t         current_path_index;
    int64_t         path_count;
    IFileDialog     *file_dialog;
    IShellItemArray *items;
    IShellItem      *item;
};

OFFD_Result offd_open_dialog_base(bool folder, bool must_exist, OFFD_FLAGS flags, uint16_t *title = nullptr);
OFFD_Result offd_open_file_dialog(OFFD_FLAGS flags = (OFFD_FLAGS)0, uint16_t *title = nullptr);
OFFD_Result offd_open_folder_dialog(OFFD_FLAGS flags = (OFFD_FLAGS)0, uint16_t *title = nullptr);
OFFD_Result offd_save_file_dialog(uint16_t **out_path, uint16_t *title = nullptr);
bool offd_next_path(OFFD_Result *result, uint16_t **out_path);
void offd_destroy_result(OFFD_Result result);

#ifdef OFFD_IMPLEMENTATION

OFFD_Result offd_open_dialog_base(bool folder, bool must_exist, OFFD_FLAGS flags, uint16_t *title /*= nullptr*/) {
    HRESULT hr;

    //
    IFileOpenDialog *file_dialog;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&file_dialog));
    if (!SUCCEEDED(hr)) {
        printf("[OFFD] CoCreateInstance() failed. Have you called CoInitialize?\n");
        return {};
    }
    assert(SUCCEEDED(hr));

    //
    DWORD additional_flags = 0;
    if (must_exist) {
        additional_flags |= FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST;
    }
    if (folder) {
        additional_flags |= FOS_PICKFOLDERS;
    }
    if (flags & OFFD_MULTI_SELECT) {
        additional_flags |= FOS_ALLOWMULTISELECT;
    }

    //
    DWORD options;
    hr = file_dialog->GetOptions(&options);
    assert(SUCCEEDED(hr));
    file_dialog->SetOptions(options | additional_flags);
    if (title != nullptr) {
        file_dialog->SetTitle((LPCWSTR)title);
    }
    hr = file_dialog->Show(NULL);
    if (!SUCCEEDED(hr)) {
        return {};
    }

    //
    IShellItemArray *items;
    hr = file_dialog->GetResults(&items);
    assert(SUCCEEDED(hr));

    //
    DWORD count;
    hr = items->GetCount(&count);
    assert(SUCCEEDED(hr));

    //
    OFFD_Result result;
    ZeroMemory(&result, sizeof(result));
    result.current_path_index = -1;
    result.path_count         = count;
    result.file_dialog        = file_dialog;
    result.items              = items;
    result.item               = nullptr;
    return result;
}

OFFD_Result offd_open_file_dialog(OFFD_FLAGS flags /*= (OFFD_FLAGS)0*/, uint16_t *title /*= nullptr*/) {
    return offd_open_dialog_base(false, (flags & OFFD_ALLOW_NONEXISTENT) == 0, flags, title);
}

OFFD_Result offd_open_folder_dialog(OFFD_FLAGS flags /*= (OFFD_FLAGS)0*/, uint16_t *title /*= nullptr*/) {
    return offd_open_dialog_base(true, (flags & OFFD_ALLOW_NONEXISTENT) == 0, flags, title);
}

OFFD_Result offd_save_file_dialog(uint16_t **out_path, uint16_t *title /*= nullptr*/) {
    HRESULT hr;

    //
    IFileSaveDialog *file_save;
    hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&file_save));
    if (!SUCCEEDED(hr)) {
        printf("[OFFD] CoCreateInstance() failed. Have you called CoInitialize?\n");
        return {};
    }
    assert(SUCCEEDED(hr));

    //
    DWORD additional_flags = 0;
    DWORD options;
    hr = file_save->GetOptions(&options);
    assert(SUCCEEDED(hr));
    file_save->SetOptions(options | additional_flags);
    if (title != nullptr) {
        file_save->SetTitle((LPCWSTR)title);
    }
    hr = file_save->Show(NULL);
    if (!SUCCEEDED(hr)) {
        return {};
    }

    //
    IShellItem *item;
    hr = file_save->GetResult(&item);
    assert(SUCCEEDED(hr));
    hr = item->GetDisplayName(SIGDN_FILESYSPATH, (LPWSTR *)out_path);
    assert(SUCCEEDED(hr));

    //
    OFFD_Result result;
    ZeroMemory(&result, sizeof(result));
    result.current_path_index = -1;
    result.path_count         = 1;
    result.file_dialog        = file_save;
    result.items              = nullptr;
    result.item               = item;
    return result;
}

bool offd_next_path(OFFD_Result *result, uint16_t **out_path) {
    if ((result->current_path_index+1) >= result->path_count) {
        return false;
    }
    if (result->item) {
        result->item->Release();
    }
    result->current_path_index += 1;
    HRESULT hr;
    hr = result->items->GetItemAt((DWORD)result->current_path_index, &result->item);
    assert(SUCCEEDED(hr));
    hr = result->item->GetDisplayName(SIGDN_FILESYSPATH, (LPWSTR *)out_path);
    assert(SUCCEEDED(hr));
    return true;
}

void offd_destroy_result(OFFD_Result result) {
    if (result.file_dialog) { result.file_dialog->Release(); result.file_dialog = nullptr; }
    if (result.items)       { result.items->Release();       result.items       = nullptr; }
    if (result.item)        { result.item->Release();        result.item        = nullptr; }
}

#endif

}
