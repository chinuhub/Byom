/** @file hio_helper.h Helpers for the hiosys plugin */

#include <iostream>
#include <vector>

#ifndef FAUDES_HIO_HELPER_H
#define FAUDES_HIO_HELPER_H

/**
 * Convenience function: list folder content
 * This is a wrapper to the respective cfl_helper.h function --- and should be eliminated.
 *
 * @param pPath
 *   path to the folder to list
 * @return 
 *   string list with names of folder and/or files found 
 */
std::vector<std::string> ListFolderContent(const char* pPath);


#endif
