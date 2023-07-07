/// \file
/// \brief Support to find files
///
///
///	 filesearch:
///
///	 ATTENTION : make sure there is enouth space in filename to put a full
/// path (255 or 512) 	 if needmd5check == 0 there is no md5 check 	 if
/// completepath then filename will be change with the full path and name
/// maxsearchdepth == 0 only search given directory, no subdirs 	 return
/// FS_NOTFOUND 	        FS_MD5SUMBAD; 	        FS_FOUND

#include <stdio.h>
#ifdef __GNUC__
#include <dirent.h>
#endif
#include <string.h>
#include <sys/stat.h>

#include "d_netfil.h"
#include "filesrch.h"
#include "m_menu.h" // Addons_option_Onchange
#include "m_misc.h"
#include "z_zone.h"

static CV_PossibleValue_t addons_cons_t[] = {{0, "Default"},
#if 1
                                             {1, "HOME"},
                                             {2, "SRB2"},
#endif
                                             {3, "CUSTOM"},
                                             {0, NULL}};

consvar_t cv_addons_option = {"addons_option",
                              "Default",
                              CV_SAVE | CV_CALL,
                              addons_cons_t,
                              Addons_option_Onchange,
                              0,
                              NULL,
                              NULL,
                              0,
                              0,
                              NULL};
consvar_t cv_addons_folder = {
    "addons_folder", "", CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t addons_md5_cons_t[] = {
    {0, "Name"}, {1, "Contents"}, {0, NULL}};
consvar_t cv_addons_md5 = {
    "addons_md5", "Name", CV_SAVE, addons_md5_cons_t, NULL, 0, NULL, NULL, 0, 0,
    NULL};

consvar_t cv_addons_showall = {
    "addons_showall", "No", CV_SAVE, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_addons_search_case = {"addons_search_case",
                                   "No",
                                   CV_SAVE,
                                   CV_YesNo,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL,
                                   0,
                                   0,
                                   NULL};

static CV_PossibleValue_t addons_search_type_cons_t[] = {
    {0, "Start"}, {1, "Anywhere"}, {0, NULL}};
consvar_t cv_addons_search_type = {"addons_search_type",
                                   "Anywhere",
                                   CV_SAVE,
                                   addons_search_type_cons_t,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL,
                                   0,
                                   0,
                                   NULL};

char menupath[1024];
size_t menupathindex[menudepth];
size_t menudepthleft = menudepth;

char menusearch[MAXSTRINGLENGTH + 1];

char **dirmenu, **coredirmenu;       // core only local for this file
size_t sizedirmenu, sizecoredirmenu; // ditto
size_t dir_on[menudepth];
UINT8 refreshdirmenu = 0;
char *refreshdirname = NULL;

filestatus_t filesearch(char *filename, const char *startpath,
                        const UINT8 *wantedmd5sum, boolean completepath,
                        int maxsearchdepth) {
  filestatus_t retval = FS_NOTFOUND;
  DIR **dirhandle;
  struct dirent *dent;
  struct stat fsstat;
  int found = 0;
  char *searchname = strdup(filename);
  int depthleft = maxsearchdepth;
  char searchpath[1024];
  size_t *searchpathindex;

  dirhandle = (DIR **)malloc(maxsearchdepth * sizeof(DIR *));
  searchpathindex = (size_t *)malloc(maxsearchdepth * sizeof(size_t));

  strcpy(searchpath, startpath);
  searchpathindex[--depthleft] = strlen(searchpath) + 1;

  dirhandle[depthleft] = opendir(searchpath);

  if (dirhandle[depthleft] == NULL) {
    free(searchname);
    free(dirhandle);
    free(searchpathindex);
    return FS_NOTFOUND;
  }

  if (searchpath[searchpathindex[depthleft] - 2] != '/') {
    searchpath[searchpathindex[depthleft] - 1] = '/';
    searchpath[searchpathindex[depthleft]] = 0;
  } else
    searchpathindex[depthleft]--;

  while ((!found) && (depthleft < maxsearchdepth)) {
    searchpath[searchpathindex[depthleft]] = 0;
    dent = readdir(dirhandle[depthleft]);

    if (!dent) {
      closedir(dirhandle[depthleft++]);
      continue;
    }

    if (dent->d_name[0] == '.' &&
        (dent->d_name[1] == '\0' ||
         (dent->d_name[1] == '.' && dent->d_name[2] == '\0'))) {
      // we don't want to scan uptree
      continue;
    }

    // okay, now we actually want searchpath to incorporate d_name
    strcpy(&searchpath[searchpathindex[depthleft]], dent->d_name);

    if (stat(searchpath, &fsstat) <
        0) // do we want to follow symlinks? if not: change it to lstat
      ;    // was the file (re)moved? can't stat it
    else if (S_ISDIR(fsstat.st_mode) && depthleft) {
      searchpathindex[--depthleft] = strlen(searchpath) + 1;
      dirhandle[depthleft] = opendir(searchpath);
      if (!dirhandle[depthleft]) {
        // can't open it... maybe no read-permissions
        // go back to previous dir
        depthleft++;
      }

      searchpath[searchpathindex[depthleft] - 1] = '/';
      searchpath[searchpathindex[depthleft]] = 0;
    } else if (!strcasecmp(searchname, dent->d_name)) {
      switch (checkfilemd5(searchpath, wantedmd5sum)) {
      case FS_FOUND:
        if (completepath)
          strcpy(filename, searchpath);
        else
          strcpy(filename, dent->d_name);
        retval = FS_FOUND;
        found = 1;
        break;
      case FS_MD5SUMBAD:
        retval = FS_MD5SUMBAD;
        break;
      default: // prevent some compiler warnings
        break;
      }
    }
  }

  for (; depthleft < maxsearchdepth; closedir(dirhandle[depthleft++]))
    ;

  free(searchname);
  free(searchpathindex);
  free(dirhandle);

  return retval;
}

char exttable[NUM_EXT_TABLE][7] =
    { // maximum extension length (currently 4) plus 3 (null
      // terminator, stop, and length including previous two)
        "\5.txt",  "\5.cfg", // exec
        "\5.wad",
#ifdef USE_KART
        "\6.kart",
#endif
        "\5.pk3",  "\5.soc", "\5.lua"}; // addfile

char filenamebuf[MAX_WADFILES][MAX_WADPATH];

static boolean filemenucmp(char *haystack, char *needle) {
  static char localhaystack[128];
  strlcpy(localhaystack, haystack, 128);
  if (!cv_addons_search_case.value)
    strupr(localhaystack);
  if (cv_addons_search_type.value)
    return (strstr(localhaystack, needle) != 0);
  return (!strncmp(localhaystack, needle, menusearch[0]));
}

void closefilemenu(boolean validsize) {
  // search
  if (dirmenu) {
    if (dirmenu != coredirmenu) {
      if (dirmenu[0] && ((UINT8)(dirmenu[0][DIR_TYPE]) == EXT_NORESULTS)) {
        Z_Free(dirmenu[0]);
        dirmenu[0] = NULL;
      }
      Z_Free(dirmenu);
    }
    dirmenu = NULL;
    sizedirmenu = 0;
  }

  if (coredirmenu) {
    // core
    if (validsize) {
      for (; sizecoredirmenu > 0; sizecoredirmenu--) {
        Z_Free(coredirmenu[sizecoredirmenu - 1]);
        coredirmenu[sizecoredirmenu - 1] = NULL;
      }
    } else
      sizecoredirmenu = 0;

    Z_Free(coredirmenu);
    coredirmenu = NULL;
  }

  if (refreshdirname)
    Z_Free(refreshdirname);
  refreshdirname = NULL;
}

void searchfilemenu(char *tempname) {
  size_t i, first;
  char localmenusearch[MAXSTRINGLENGTH] = "";

  if (dirmenu) {
    if (dirmenu != coredirmenu) {
      if (dirmenu[0] && ((UINT8)(dirmenu[0][DIR_TYPE]) == EXT_NORESULTS)) {
        Z_Free(dirmenu[0]);
        dirmenu[0] = NULL;
      }
      // Z_Free(dirmenu); -- Z_Realloc later tho...
    } else
      dirmenu = NULL;
  }

  first = (((UINT8)(coredirmenu[0][DIR_TYPE]) == EXT_UP) ? 1 : 0); // skip UP...

  if (!menusearch[0]) {
    if (dirmenu)
      Z_Free(dirmenu);
    dirmenu = coredirmenu;
    sizedirmenu = sizecoredirmenu;

    if (tempname) {
      for (i = first; i < sizedirmenu; i++) {
        if (!strcmp(dirmenu[i] + DIR_STRING, tempname)) {
          dir_on[menudepthleft] = i;
          break;
        }
      }

      if (i == sizedirmenu)
        dir_on[menudepthleft] = first;

      Z_Free(tempname);
    }

    return;
  }

  strcpy(localmenusearch, menusearch + 1);
  if (!cv_addons_search_case.value)
    strupr(localmenusearch);

  sizedirmenu = 0;
  for (i = first; i < sizecoredirmenu; i++) {
    if (filemenucmp(coredirmenu[i] + DIR_STRING, localmenusearch))
      sizedirmenu++;
  }

  if (!sizedirmenu) // no results...
  {
    if ((!(dirmenu = Z_Realloc(dirmenu, sizeof(char *), PU_STATIC, NULL))) ||
        !(dirmenu[0] = Z_StrDup(va("%c\13No results...", EXT_NORESULTS))))
      I_Error("searchfilemenu(): could not create \"No results...\".");
    sizedirmenu = 1;
    dir_on[menudepthleft] = 0;
    if (tempname)
      Z_Free(tempname);
    return;
  }

  if (!(dirmenu =
            Z_Realloc(dirmenu, sizedirmenu * sizeof(char *), PU_STATIC, NULL)))
    I_Error("searchfilemenu(): could not reallocate dirmenu.");

  sizedirmenu = 0;
  for (i = first; i < sizecoredirmenu; i++) {
    if (filemenucmp(coredirmenu[i] + DIR_STRING, localmenusearch)) {
      if (tempname && !strcmp(coredirmenu[i] + DIR_STRING, tempname)) {
        dir_on[menudepthleft] = sizedirmenu;
        Z_Free(tempname);
        tempname = NULL;
      }
      dirmenu[sizedirmenu++] = coredirmenu[i]; // pointer reuse
    }
  }

  if (tempname) {
    dir_on[menudepthleft] = 0; // first; -- can't be first, causes problems
    Z_Free(tempname);
  }
}

boolean preparefilemenu(boolean samedepth, boolean replayhut) {
  DIR *dirhandle;
  struct dirent *dent;
  struct stat fsstat;
  size_t pos = 0, folderpos = 0, numfolders = 0;
  char *tempname = NULL;

  if (samedepth) {
    if (dirmenu && dirmenu[dir_on[menudepthleft]])
      tempname = Z_StrDup(dirmenu[dir_on[menudepthleft]] +
                          DIR_STRING); // don't need to I_Error if can't make -
                                       // not important, just QoL
  } else
    menusearch[0] = menusearch[1] = 0; // clear search

  if (!(dirhandle = opendir(menupath))) // get directory
  {
    closefilemenu(true);
    return false;
  }

  for (; sizecoredirmenu > 0; sizecoredirmenu--) // clear out existing items
  {
    Z_Free(coredirmenu[sizecoredirmenu - 1]);
    coredirmenu[sizecoredirmenu - 1] = NULL;
  }

  while (true) {
    menupath[menupathindex[menudepthleft]] = 0;
    dent = readdir(dirhandle);

    if (!dent)
      break;
    else if (dent->d_name[0] == '.' &&
             (dent->d_name[1] == '\0' ||
              (dent->d_name[1] == '.' && dent->d_name[2] == '\0')))
      continue; // we don't want to scan uptree

    strcpy(&menupath[menupathindex[menudepthleft]], dent->d_name);

    if (stat(menupath, &fsstat) <
        0) // do we want to follow symlinks? if not: change it to lstat
      ;    // was the file (re)moved? can't stat it
    else   // is a file or directory
    {
      if (!S_ISDIR(fsstat.st_mode)) // file
      {
        size_t len = strlen(dent->d_name) + 1;
        if (replayhut) {
          if (strcasecmp(".lmp", dent->d_name + len - 5))
            continue; // Not a replay
        } else if (!cv_addons_showall.value) {
          UINT8 ext;
          for (ext = 0; ext < NUM_EXT_TABLE; ext++)
            if (!strcasecmp(exttable[ext] + 1,
                            dent->d_name + len - (exttable[ext][0])))
              break; // extension comparison
          if (ext == NUM_EXT_TABLE)
            continue; // not an addfile-able (or exec-able) file
        }
      } else // directory
        numfolders++;

      sizecoredirmenu++;
    }
  }

  if (!sizecoredirmenu) {
    closedir(dirhandle);
    closefilemenu(false);
    if (tempname)
      Z_Free(tempname);
    return false;
  }

  if (menudepthleft != menudepth - 1) // Make room for UP...
  {
    sizecoredirmenu++;
    numfolders++;
    folderpos++;
  }

  if (dirmenu && dirmenu == coredirmenu)
    dirmenu = NULL;

  if (!(coredirmenu = Z_Realloc(coredirmenu, sizecoredirmenu * sizeof(char *),
                                PU_STATIC, NULL))) {
    closedir(dirhandle); // just in case
    I_Error("preparefilemenu(): could not reallocate coredirmenu.");
  }

  rewinddir(dirhandle);

  while ((pos + folderpos) < sizecoredirmenu) {
    menupath[menupathindex[menudepthleft]] = 0;
    dent = readdir(dirhandle);

    if (!dent)
      break;
    else if (dent->d_name[0] == '.' &&
             (dent->d_name[1] == '\0' ||
              (dent->d_name[1] == '.' && dent->d_name[2] == '\0')))
      continue; // we don't want to scan uptree

    strcpy(&menupath[menupathindex[menudepthleft]], dent->d_name);

    if (stat(menupath, &fsstat) <
        0) // do we want to follow symlinks? if not: change it to lstat
      ;    // was the file (re)moved? can't stat it
    else   // is a file or directory
    {
      char *temp;
      size_t len = strlen(dent->d_name) + 1;
      UINT8 ext = EXT_FOLDER;
      UINT8 folder;

      if (!S_ISDIR(fsstat.st_mode)) // file
      {
        if (!((numfolders + pos) < sizecoredirmenu))
          continue; // crash prevention

        if (replayhut) {
          if (strcasecmp(".lmp", dent->d_name + len - 5))
            continue;    // Not a replay
          ext = EXT_TXT; // This isn't used anywhere but better safe than sorry
                         // for messing with this...
        } else {
          for (; ext < NUM_EXT_TABLE; ext++)
            if (!strcasecmp(exttable[ext] + 1,
                            dent->d_name + len - (exttable[ext][0])))
              break; // extension comparison
          if (ext == NUM_EXT_TABLE && !cv_addons_showall.value)
            continue;       // not an addfile-able (or exec-able) file
          ext += EXT_START; // moving to be appropriate position

          if (ext >= EXT_LOADSTART) {
            size_t i;
            for (i = 0; i < numwadfiles; i++) {
              if (!filenamebuf[i][0]) {
                strncpy(filenamebuf[i], wadfiles[i]->filename, MAX_WADPATH);
                filenamebuf[i][MAX_WADPATH - 1] = '\0';
                nameonly(filenamebuf[i]);
              }

              if (strcmp(dent->d_name, filenamebuf[i]))
                continue;
              if (cv_addons_md5.value &&
                  !checkfilemd5(menupath, wadfiles[i]->md5sum))
                continue;

              ext |= EXT_LOADED;
            }
          } else if (ext == EXT_TXT) {
            if (!strcmp(dent->d_name, "log.txt") ||
                !strcmp(dent->d_name, "errorlog.txt"))
              ext |= EXT_LOADED;
          }

          if (!strcmp(dent->d_name, configfile))
            ext |= EXT_LOADED;
        }

        folder = 0;
      } else // directory
        len += (folder = 1);

      if (len > 255)
        len = 255;

      if (!(temp = Z_Malloc((len + DIR_STRING + folder) * sizeof(char),
                            PU_STATIC, NULL)))
        I_Error("preparefilemenu(): could not create file entry.");
      temp[DIR_TYPE] = ext;
      temp[DIR_LEN] = (UINT8)(len);
      strlcpy(temp + DIR_STRING, dent->d_name, len);
      if (folder) {
        strcpy(temp + len, PATHSEP);
        coredirmenu[folderpos++] = temp;
      } else if (replayhut) // Reverse-alphabetical on just the files; acts as a
                            // fake "most recent first" with the current
                            // filename format
        coredirmenu[sizecoredirmenu - 1 - pos++] = temp;
      else
        coredirmenu[numfolders + pos++] = temp;
    }
  }

  closedir(dirhandle);

  if ((menudepthleft != menudepth - 1) // now for UP... entry
      && !(coredirmenu[0] = Z_StrDup(va("%c\5UP...", EXT_UP))))
    I_Error("preparefilemenu(): could not create \"UP...\".");

  menupath[menupathindex[menudepthleft]] = 0;
  sizecoredirmenu =
      (numfolders +
       pos); // just in case things shrink between opening and rewind

  if (!sizecoredirmenu) {
    dir_on[menudepthleft] = 0;
    closefilemenu(false);
    return false;
  }

  searchfilemenu(tempname);

  return true;
}
