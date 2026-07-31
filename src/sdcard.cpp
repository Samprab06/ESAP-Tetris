#include "sdcard.h"

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        printf("Failed to open directory\n");
        return;
    }
    if (!root.isDirectory())
    {
        printf("Not a directory\n");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            printf("  DIR : ");
            printf("%s\n", file.name());
            if (levels)
            {
                listDir(fs, file.path(), levels - 1);
            }
        }
        else
        {
            printf("  FILE: ");
            printf("%s", file.name());
            printf("  SIZE: ");
            printf("%d\n", file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char *path)
{
    printf("Creating Dir: %s\n", path);
    if (fs.mkdir(path))
    {
        printf("Dir created\n");
    }
    else
    {
        printf("mkdir failed\n");
    }
}

void removeDir(fs::FS &fs, const char *path)
{
    printf("Removing Dir: %s\n", path);
    if (fs.rmdir(path))
    {
        printf("Dir removed\n");
    }
    else
    {
        printf("rmdir failed\n");
    }
}

void readFile(fs::FS &fs, const char *path)
{
    printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file)
    {
        printf("Failed to open file for reading\n");
        return;
    }

    printf("Read from file: ");
    while (file.available())
    {
        putchar(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
    printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        printf("Failed to open file for writing\n");
        return;
    }
    if (file.print(message))
    {
        printf("File written\n");
    }
    else
    {
        printf("Write failed\n");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
    printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        printf("Failed to open file for appending\n");
        return;
    }
    if (file.print(message))
    {
        printf("Message appended\n");
    }
    else
    {
        printf("Append failed\n");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
    printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2))
    {
        printf("File renamed\n");
    }
    else
    {
        printf("Rename failed\n");
    }
}

void deleteFile(fs::FS &fs, const char *path)
{
    printf("Deleting file: %s\n", path);
    if (fs.remove(path))
    {
        printf("File deleted\n");
    }
    else
    {
        printf("Delete failed\n");
    }
}
