#pragma once

#ifndef SDCARD_H
#define SDCARD_H

#include <SD.h>
#include <FS.h>
#include <SPI.h>
#include "audio.h"

/// @brief Lists directory contents recursively
/// @param fs Filesystem reference
/// @param dirname Directory path
/// @param levels Recursion depth
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);

/// @brief Creates directory on SD card
/// @param fs Filesystem reference
/// @param path Directory path
void createDir(fs::FS &fs, const char *path);

/// @brief Removes directory from SD card
/// @param fs Filesystem reference
/// @param path Directory path
void removeDir(fs::FS &fs, const char *path);

/// @brief Reads and prints file contents
/// @param fs Filesystem reference
/// @param path File path
void readFile(fs::FS &fs, const char *path);

/// @brief Writes message to file overwriting
/// @param fs Filesystem reference
/// @param path File path
/// @param message Data to write
void writeFile(fs::FS &fs, const char *path, const char *message);

/// @brief Appends message to file
/// @param fs Filesystem reference
/// @param path File path
/// @param message Data to append
void appendFile(fs::FS &fs, const char *path, const char *message);

/// @brief Renames file or moves on SD
/// @param fs Filesystem reference
/// @param path1 Original path
/// @param path2 New path
void renameFile(fs::FS &fs, const char *path1, const char *path2);

/// @brief Deletes file from SD card
/// @param fs Filesystem reference
/// @param path File path
void deleteFile(fs::FS &fs, const char *path);

#endif