# offd - Open the Fucking File Dialog (or, Open File/Folder Dialog)

One would expect that to prompt the user to select a file, one would call a function provided by the operating system and that function would then return the path of the file the user clicked. Microsoft, in their infinite wisdom, has turned this straightforward, pleasant API idea into a horrible trainwreck of C++ interfaces and COM objects. I have managed to untangle that disaster and am offering you this single-header library so that you may spend your days working on things that are actually worthwhile, or spending time with your loved ones, learning a skill, traveling, or literally anything else.

I cannot stress enough that this library simply should not exist. This shouldn't have been work that anybody had to do; but computing in 2022 is a disaster for reasons innumerable, so here we are.

## Step 0

Call `CoInitialize` or `CoInitializeEx` at some point in your application.

## Step 1

Put the following in ONE file in your application:
```
#define OFFD_IMPLEMENTATION
#include "offd.h"
```

If you want to use OFFD in any other files, simply add
```
#include "offd.h"
```
to that file.

## Step 2

=> To open the Open File dialog for a single file:

```
OFFD_Result offd = offd_open_file_dialog();
wchar_t *path;
if (offd_next_path(&offd, &path)) { // will return `false` if the user hit 'Cancel'
    // do something with `path`
}
offd_destroy_result(offd);
```

=> To open the Open File dialog with multi-file-select:

```
OFFD_Result offd = offd_open_file_dialog(OFFD_MULTI_SELECT);
wchar_t *path;
while (offd_next_path(&offd, &path)) { // will return `false` if the user hit 'Cancel', or we have gone through all the paths
    // do something with `path`
}
offd_destroy_result(offd);
```

=> To open the Open Folder dialog it is the same as both of the above examples work except you call offd_open_folder_dialog instead of offd_open_file_dialog.

=> To open the Save File dialog:

```
wchar_t *path = nullptr;
OFFD_Result offd = offd_save_file_dialog(&path);
if (path != nullptr) {
    // do something with `path`
}
offd_destroy_result(offd);
```
